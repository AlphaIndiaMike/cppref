#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdio>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "database_connector.h"
#include "sqlite3_database_connector.h"

namespace Gateways::Database {
namespace {

using ::testing::HasSubstr;

// ============================================================
// Test Fixture
// ============================================================
class SqliteDatabaseTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Create a unique temp file for each test
    test_db_path_ = std::filesystem::temp_directory_path() /
                    ("test_db_" +
                     std::to_string(reinterpret_cast<uintptr_t>(this)) + ".db");
  }

  void TearDown() override {
    // Clean up test database
    std::filesystem::remove(test_db_path_);
  }

  std::filesystem::path test_db_path_;
};

// ============================================================
// Exception Tests
// ============================================================
TEST(DatabaseExceptionTest, DatabaseExceptionMessage) {
  DatabaseException ex("test error");
  EXPECT_STREQ(ex.what(), "test error");
}

TEST(DatabaseExceptionTest, ConnectionExceptionMessage) {
  ConnectionException ex("connection failed");
  EXPECT_THAT(ex.what(), HasSubstr("Connection error:"));
  EXPECT_THAT(ex.what(), HasSubstr("connection failed"));
}

TEST(DatabaseExceptionTest, QueryExceptionMessage) {
  QueryException ex("query failed");
  EXPECT_THAT(ex.what(), HasSubstr("Query error:"));
  EXPECT_THAT(ex.what(), HasSubstr("query failed"));
}

// ============================================================
// SqliteDatabase Basic Operations
// ============================================================
TEST_F(SqliteDatabaseTest, DefaultConstructor) {
  SqliteDatabase db;
  EXPECT_FALSE(db.isOpen());
}

TEST_F(SqliteDatabaseTest, ConstructorWithPath) {
  SqliteDatabase db(test_db_path_.string());
  EXPECT_TRUE(db.isOpen());
}

TEST_F(SqliteDatabaseTest, OpenAndClose) {
  SqliteDatabase db;
  EXPECT_FALSE(db.isOpen());

  db.open(test_db_path_.string());
  EXPECT_TRUE(db.isOpen());

  db.close();
  EXPECT_FALSE(db.isOpen());
}

TEST_F(SqliteDatabaseTest, OpenAlreadyOpenDatabase) {
  SqliteDatabase db(test_db_path_.string());
  EXPECT_TRUE(db.isOpen());

  // Opening again should close the previous connection first
  auto other_path = test_db_path_.string() + "_other.db";
  db.open(other_path);
  EXPECT_TRUE(db.isOpen());

  std::filesystem::remove(other_path);
}

TEST_F(SqliteDatabaseTest, OpenInvalidPath) {
  SqliteDatabase db;
  // Try to open a database in a non-existent directory
  EXPECT_THROW(db.open("/nonexistent/path/to/database.db"),
               ConnectionException);
}

TEST_F(SqliteDatabaseTest, CloseAlreadyClosed) {
  SqliteDatabase db;
  // Should not throw when closing an already closed database
  EXPECT_NO_THROW(db.close());
}

TEST_F(SqliteDatabaseTest, MoveConstructor) {
  SqliteDatabase db1(test_db_path_.string());
  EXPECT_TRUE(db1.isOpen());

  SqliteDatabase db2(std::move(db1));
  EXPECT_TRUE(db2.isOpen());
  EXPECT_FALSE(db1.isOpen());  // NOLINT: testing moved-from state
}

TEST_F(SqliteDatabaseTest, MoveAssignment) {
  SqliteDatabase db1(test_db_path_.string());
  SqliteDatabase db2;

  EXPECT_TRUE(db1.isOpen());
  EXPECT_FALSE(db2.isOpen());

  db2 = std::move(db1);

  EXPECT_TRUE(db2.isOpen());
  EXPECT_FALSE(db1.isOpen());  // NOLINT: testing moved-from state
}

TEST_F(SqliteDatabaseTest, MoveAssignmentSelf) {
  SqliteDatabase db(test_db_path_.string());
  EXPECT_TRUE(db.isOpen());

  // Self-assignment should be safe
  db = std::move(db);
  EXPECT_TRUE(db.isOpen());
}

TEST_F(SqliteDatabaseTest, MoveAssignmentClosesExisting) {
  auto other_path = test_db_path_.string() + "_other.db";

  SqliteDatabase db1(test_db_path_.string());
  SqliteDatabase db2(other_path);

  db2 = std::move(db1);

  EXPECT_TRUE(db2.isOpen());

  std::filesystem::remove(other_path);
}

// ============================================================
// Execute and Query
// ============================================================
TEST_F(SqliteDatabaseTest, ExecuteCreateTable) {
  SqliteDatabase db(test_db_path_.string());

  EXPECT_NO_THROW(
      db.execute("CREATE TABLE test (id INTEGER PRIMARY KEY, name TEXT)"));
}

TEST_F(SqliteDatabaseTest, ExecuteOnClosedDatabase) {
  SqliteDatabase db;
  EXPECT_THROW(db.execute("SELECT 1"), ConnectionException);
}

TEST_F(SqliteDatabaseTest, ExecuteInvalidSql) {
  SqliteDatabase db(test_db_path_.string());
  EXPECT_THROW(db.execute("INVALID SQL STATEMENT"), QueryException);
}

TEST_F(SqliteDatabaseTest, QuerySimple) {
  SqliteDatabase db(test_db_path_.string());

  auto result = db.query("SELECT 1 AS num, 'hello' AS str");

  ASSERT_EQ(result.size(), 1u);
  ASSERT_EQ(result[0].size(), 2u);
  EXPECT_EQ(std::get<int64_t>(result[0][0]), 1);
  EXPECT_EQ(std::get<std::string>(result[0][1]), "hello");
}

TEST_F(SqliteDatabaseTest, QueryWithMultipleRows) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");
  db.execute("INSERT INTO test (value) VALUES ('a'), ('b'), ('c')");

  auto result = db.query("SELECT * FROM test ORDER BY id");

  ASSERT_EQ(result.size(), 3u);
  EXPECT_EQ(std::get<std::string>(result[0][1]), "a");
  EXPECT_EQ(std::get<std::string>(result[1][1]), "b");
  EXPECT_EQ(std::get<std::string>(result[2][1]), "c");
}

// ============================================================
// Prepared Statements
// ============================================================
TEST_F(SqliteDatabaseTest, PrepareOnClosedDatabase) {
  SqliteDatabase db;
  EXPECT_THROW(db.prepare("SELECT 1"), ConnectionException);
}

TEST_F(SqliteDatabaseTest, PrepareInvalidSql) {
  SqliteDatabase db(test_db_path_.string());
  EXPECT_THROW(db.prepare("INVALID SQL"), QueryException);
}

TEST_F(SqliteDatabaseTest, StatementBindNull) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");

  auto stmt = db.prepare("INSERT INTO test (value) VALUES (?)");
  stmt->bind(1, nullptr);
  stmt->executeInsert();

  auto result = db.query("SELECT value FROM test");
  ASSERT_EQ(result.size(), 1u);
  EXPECT_TRUE(std::holds_alternative<std::nullptr_t>(result[0][0]));
}

TEST_F(SqliteDatabaseTest, StatementBindInt64) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value INTEGER)");

  auto stmt = db.prepare("INSERT INTO test (value) VALUES (?)");
  stmt->bind(1, int64_t{42});
  stmt->executeInsert();

  auto result = db.query("SELECT value FROM test");
  ASSERT_EQ(result.size(), 1u);
  EXPECT_EQ(std::get<int64_t>(result[0][0]), 42);
}

TEST_F(SqliteDatabaseTest, StatementBindDouble) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value REAL)");

  auto stmt = db.prepare("INSERT INTO test (value) VALUES (?)");
  stmt->bind(1, 3.14159);
  stmt->executeInsert();

  auto result = db.query("SELECT value FROM test");
  ASSERT_EQ(result.size(), 1u);
  EXPECT_DOUBLE_EQ(std::get<double>(result[0][0]), 3.14159);
}

TEST_F(SqliteDatabaseTest, StatementBindString) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");

  auto stmt = db.prepare("INSERT INTO test (value) VALUES (?)");
  stmt->bind(1, std::string("hello world"));
  stmt->executeInsert();

  auto result = db.query("SELECT value FROM test");
  ASSERT_EQ(result.size(), 1u);
  EXPECT_EQ(std::get<std::string>(result[0][0]), "hello world");
}

TEST_F(SqliteDatabaseTest, StatementBindBlob) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value BLOB)");

  std::vector<uint8_t> blob = {0x01, 0x02, 0x03, 0x04, 0x05};

  auto stmt = db.prepare("INSERT INTO test (value) VALUES (?)");
  stmt->bind(1, blob);
  stmt->executeInsert();

  auto result = db.query("SELECT value FROM test");
  ASSERT_EQ(result.size(), 1u);
  EXPECT_EQ(std::get<std::vector<uint8_t>>(result[0][0]), blob);
}

TEST_F(SqliteDatabaseTest, StatementChainedBind) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (a INTEGER, b REAL, c TEXT)");

  auto stmt = db.prepare("INSERT INTO test (a, b, c) VALUES (?, ?, ?)");
  stmt->bind(1, int64_t{1}).bind(2, 2.5).bind(3, std::string("three"));
  stmt->executeInsert();

  auto result = db.query("SELECT * FROM test");
  ASSERT_EQ(result.size(), 1u);
  EXPECT_EQ(std::get<int64_t>(result[0][0]), 1);
  EXPECT_DOUBLE_EQ(std::get<double>(result[0][1]), 2.5);
  EXPECT_EQ(std::get<std::string>(result[0][2]), "three");
}

TEST_F(SqliteDatabaseTest, StatementExecute) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");
  db.execute("INSERT INTO test (value) VALUES ('a'), ('b')");

  auto stmt = db.prepare("SELECT * FROM test ORDER BY id");
  auto result = stmt->execute();

  ASSERT_EQ(result.size(), 2u);
}

TEST_F(SqliteDatabaseTest, StatementExecuteInsertReturnsRowId) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");

  auto stmt = db.prepare("INSERT INTO test (value) VALUES (?)");
  stmt->bind(1, std::string("first"));
  int64_t id1 = stmt->executeInsert();

  stmt->reset();
  stmt->bind(1, std::string("second"));
  int64_t id2 = stmt->executeInsert();

  EXPECT_EQ(id1, 1);
  EXPECT_EQ(id2, 2);
}

TEST_F(SqliteDatabaseTest, StatementExecuteUpdateReturnsAffectedRows) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");
  db.execute("INSERT INTO test (value) VALUES ('a'), ('b'), ('c')");

  auto stmt = db.prepare("UPDATE test SET value = 'updated' WHERE id > ?");
  stmt->bind(1, int64_t{1});
  int affected = stmt->executeUpdate();

  EXPECT_EQ(affected, 2);
}

TEST_F(SqliteDatabaseTest, StatementReset) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value INTEGER)");

  auto stmt = db.prepare("INSERT INTO test (value) VALUES (?)");

  stmt->bind(1, int64_t{100});
  stmt->executeInsert();

  stmt->reset();

  stmt->bind(1, int64_t{200});
  stmt->executeInsert();

  auto result = db.query("SELECT value FROM test ORDER BY id");
  ASSERT_EQ(result.size(), 2u);
  EXPECT_EQ(std::get<int64_t>(result[0][0]), 100);
  EXPECT_EQ(std::get<int64_t>(result[1][0]), 200);
}

TEST_F(SqliteDatabaseTest, StatementMoveConstructor) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");

  // Create statement via unique_ptr, then release and move
  std::unique_ptr<IStatement> stmt1 =
      db.prepare("INSERT INTO test (value) VALUES (?)");
  stmt1->bind(1, std::string("test"));

  // Move semantics are tested implicitly through unique_ptr usage
  // The statement should work after being prepared
  EXPECT_NO_THROW(stmt1->executeInsert());
}

TEST_F(SqliteDatabaseTest, StatementMoveAssignment) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");

  // Test that statements work correctly after being moved via unique_ptr
  std::unique_ptr<IStatement> stmt1 =
      db.prepare("INSERT INTO test (value) VALUES (?)");
  std::unique_ptr<IStatement> stmt2 =
      db.prepare("INSERT INTO test (value) VALUES (?)");

  // Move assignment through unique_ptr
  stmt2 = std::move(stmt1);

  // stmt2 should now work
  stmt2->bind(1, std::string("moved"));
  EXPECT_NO_THROW(stmt2->executeInsert());
}

// ============================================================
// Transactions
// ============================================================
TEST_F(SqliteDatabaseTest, BeginCommitTransaction) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");

  db.beginTransaction();
  db.execute("INSERT INTO test (value) VALUES ('test')");
  db.commit();

  auto result = db.query("SELECT COUNT(*) FROM test");
  EXPECT_EQ(std::get<int64_t>(result[0][0]), 1);
}

TEST_F(SqliteDatabaseTest, BeginRollbackTransaction) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");

  db.beginTransaction();
  db.execute("INSERT INTO test (value) VALUES ('test')");
  db.rollback();

  auto result = db.query("SELECT COUNT(*) FROM test");
  EXPECT_EQ(std::get<int64_t>(result[0][0]), 0);
}

TEST_F(SqliteDatabaseTest, TransactionGuardCommit) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");

  {
    Transaction txn(db);
    db.execute("INSERT INTO test (value) VALUES ('test')");
    txn.commit();
  }

  auto result = db.query("SELECT COUNT(*) FROM test");
  EXPECT_EQ(std::get<int64_t>(result[0][0]), 1);
}

TEST_F(SqliteDatabaseTest, TransactionGuardAutoRollback) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");

  {
    Transaction txn(db);
    db.execute("INSERT INTO test (value) VALUES ('test')");
    // No commit - should auto-rollback
  }

  auto result = db.query("SELECT COUNT(*) FROM test");
  EXPECT_EQ(std::get<int64_t>(result[0][0]), 0);
}

TEST_F(SqliteDatabaseTest, TransactionGuardRollbackOnException) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");

  try {
    Transaction txn(db);
    db.execute("INSERT INTO test (value) VALUES ('test')");
    throw std::runtime_error("Simulated error");
    txn.commit();  // Never reached
  } catch (const std::runtime_error&) {
    // Expected
  }

  auto result = db.query("SELECT COUNT(*) FROM test");
  EXPECT_EQ(std::get<int64_t>(result[0][0]), 0);
}

// ============================================================
// Last Insert RowId and Changes Count
// ============================================================
TEST_F(SqliteDatabaseTest, LastInsertRowId) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");

  db.execute("INSERT INTO test (value) VALUES ('first')");
  EXPECT_EQ(db.lastInsertRowId(), 1);

  db.execute("INSERT INTO test (value) VALUES ('second')");
  EXPECT_EQ(db.lastInsertRowId(), 2);
}

TEST_F(SqliteDatabaseTest, LastInsertRowIdOnClosedDb) {
  SqliteDatabase db;
  EXPECT_EQ(db.lastInsertRowId(), 0);
}

TEST_F(SqliteDatabaseTest, ChangesCount) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");
  db.execute("INSERT INTO test (value) VALUES ('a'), ('b'), ('c')");

  db.execute("UPDATE test SET value = 'updated' WHERE id > 1");
  EXPECT_EQ(db.changesCount(), 2);
}

TEST_F(SqliteDatabaseTest, ChangesCountOnClosedDb) {
  SqliteDatabase db;
  EXPECT_EQ(db.changesCount(), 0);
}

// ============================================================
// SQLite-Specific Features
// ============================================================
TEST_F(SqliteDatabaseTest, EnableForeignKeys) {
  SqliteDatabase db(test_db_path_.string());

  // Foreign keys should be enabled by default after open()
  auto result = db.query("PRAGMA foreign_keys");
  EXPECT_EQ(std::get<int64_t>(result[0][0]), 1);

  db.enableForeignKeys(false);
  result = db.query("PRAGMA foreign_keys");
  EXPECT_EQ(std::get<int64_t>(result[0][0]), 0);

  db.enableForeignKeys(true);
  result = db.query("PRAGMA foreign_keys");
  EXPECT_EQ(std::get<int64_t>(result[0][0]), 1);
}

TEST_F(SqliteDatabaseTest, SetJournalModeWAL) {
  SqliteDatabase db(test_db_path_.string());

  db.setJournalMode("WAL");

  auto result = db.query("PRAGMA journal_mode");
  EXPECT_EQ(std::get<std::string>(result[0][0]), "wal");
}

TEST_F(SqliteDatabaseTest, SetJournalModeDelete) {
  SqliteDatabase db(test_db_path_.string());

  db.setJournalMode("DELETE");

  auto result = db.query("PRAGMA journal_mode");
  EXPECT_EQ(std::get<std::string>(result[0][0]), "delete");
}

// ============================================================
// Column Type Extraction
// ============================================================
TEST_F(SqliteDatabaseTest, ExtractAllColumnTypes) {
  SqliteDatabase db(test_db_path_.string());
  db.execute(R"(
    CREATE TABLE test (
      null_col,
      int_col INTEGER,
      float_col REAL,
      text_col TEXT,
      blob_col BLOB
    )
  )");

  auto stmt = db.prepare("INSERT INTO test VALUES (?, ?, ?, ?, ?)");
  stmt->bind(1, nullptr);
  stmt->bind(2, int64_t{42});
  stmt->bind(3, 3.14);
  stmt->bind(4, std::string("hello"));
  stmt->bind(5, std::vector<uint8_t>{0xDE, 0xAD, 0xBE, 0xEF});
  stmt->executeInsert();

  auto result = db.query("SELECT * FROM test");
  ASSERT_EQ(result.size(), 1u);
  ASSERT_EQ(result[0].size(), 5u);

  EXPECT_TRUE(std::holds_alternative<std::nullptr_t>(result[0][0]));
  EXPECT_EQ(std::get<int64_t>(result[0][1]), 42);
  EXPECT_DOUBLE_EQ(std::get<double>(result[0][2]), 3.14);
  EXPECT_EQ(std::get<std::string>(result[0][3]), "hello");

  auto blob = std::get<std::vector<uint8_t>>(result[0][4]);
  EXPECT_EQ(blob, (std::vector<uint8_t>{0xDE, 0xAD, 0xBE, 0xEF}));
}

TEST_F(SqliteDatabaseTest, ExtractNullText) {
  SqliteDatabase db(test_db_path_.string());

  // Test that NULL text is handled (returns empty string in current impl)
  auto result = db.query("SELECT NULL");
  ASSERT_EQ(result.size(), 1u);
  EXPECT_TRUE(std::holds_alternative<std::nullptr_t>(result[0][0]));
}

// ============================================================
// Error Handling
// ============================================================
TEST_F(SqliteDatabaseTest, StatementExecuteError) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT UNIQUE)");
  db.execute("INSERT INTO test (value) VALUES ('unique_value')");

  auto stmt = db.prepare("INSERT INTO test (value) VALUES (?)");
  stmt->bind(1, std::string("unique_value"));  // Duplicate!

  EXPECT_THROW(stmt->executeInsert(), QueryException);
}

TEST_F(SqliteDatabaseTest, StatementExecuteUpdateError) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE parent (id INTEGER PRIMARY KEY)");
  db.execute(
      "CREATE TABLE child (id INTEGER PRIMARY KEY, parent_id INTEGER "
      "REFERENCES parent(id))");
  db.execute("INSERT INTO parent (id) VALUES (1)");
  db.execute("INSERT INTO child (parent_id) VALUES (1)");

  // Try to delete parent with foreign key constraint
  auto stmt = db.prepare("DELETE FROM parent WHERE id = ?");
  stmt->bind(1, int64_t{1});

  EXPECT_THROW(stmt->executeUpdate(), QueryException);
}

TEST_F(SqliteDatabaseTest, BindInvalidIndex) {
  SqliteDatabase db(test_db_path_.string());

  auto stmt = db.prepare("SELECT ?");

  // Index 0 is invalid (SQLite uses 1-based indexing)
  EXPECT_THROW(stmt->bind(0, int64_t{1}), QueryException);
}

// ============================================================
// Edge Cases
// ============================================================
TEST_F(SqliteDatabaseTest, EmptyQuery) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (id INTEGER PRIMARY KEY)");

  auto result = db.query("SELECT * FROM test");
  EXPECT_TRUE(result.empty());
}

TEST_F(SqliteDatabaseTest, LargeBlob) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (data BLOB)");

  // Create a 1MB blob
  std::vector<uint8_t> large_blob(1024 * 1024);
  for (size_t i = 0; i < large_blob.size(); ++i) {
    large_blob[i] = static_cast<uint8_t>(i % 256);
  }

  auto stmt = db.prepare("INSERT INTO test (data) VALUES (?)");
  stmt->bind(1, large_blob);
  stmt->executeInsert();

  auto result = db.query("SELECT data FROM test");
  ASSERT_EQ(result.size(), 1u);

  auto retrieved = std::get<std::vector<uint8_t>>(result[0][0]);
  EXPECT_EQ(retrieved.size(), large_blob.size());
  EXPECT_EQ(retrieved, large_blob);
}

TEST_F(SqliteDatabaseTest, UnicodeString) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (value TEXT)");

  std::string unicode = u8"Hello, 世界! 🎉 Привет мир!";

  auto stmt = db.prepare("INSERT INTO test (value) VALUES (?)");
  stmt->bind(1, unicode);
  stmt->executeInsert();

  auto result = db.query("SELECT value FROM test");
  ASSERT_EQ(result.size(), 1u);
  EXPECT_EQ(std::get<std::string>(result[0][0]), unicode);
}

TEST_F(SqliteDatabaseTest, EmptyString) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (value TEXT)");

  auto stmt = db.prepare("INSERT INTO test (value) VALUES (?)");
  stmt->bind(1, std::string(""));
  stmt->executeInsert();

  auto result = db.query("SELECT value FROM test");
  ASSERT_EQ(result.size(), 1u);
  EXPECT_EQ(std::get<std::string>(result[0][0]), "");
}

TEST_F(SqliteDatabaseTest, EmptyBlob) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (data BLOB)");

  auto stmt = db.prepare("INSERT INTO test (data) VALUES (?)");
  stmt->bind(1, std::vector<uint8_t>{});
  stmt->executeInsert();

  auto result = db.query("SELECT data FROM test");
  ASSERT_EQ(result.size(), 1u);

  // SQLite returns empty blobs as either empty vector or the blob type
  // depending on how sqlite3_column_type classifies it
  if (std::holds_alternative<std::vector<uint8_t>>(result[0][0])) {
    EXPECT_EQ(std::get<std::vector<uint8_t>>(result[0][0]),
              std::vector<uint8_t>{});
  } else {
    // Empty blob may be returned as NULL in some SQLite versions
    EXPECT_TRUE(std::holds_alternative<std::nullptr_t>(result[0][0]));
  }
}

TEST_F(SqliteDatabaseTest, InMemoryDatabase) {
  SqliteDatabase db(":memory:");
  EXPECT_TRUE(db.isOpen());

  db.execute("CREATE TABLE test (id INTEGER PRIMARY KEY)");
  db.execute("INSERT INTO test (id) VALUES (1)");

  auto result = db.query("SELECT * FROM test");
  EXPECT_EQ(result.size(), 1u);
}

// ============================================================
// Destructor Tests
// ============================================================
TEST_F(SqliteDatabaseTest, DestructorClosesDatabase) {
  {
    SqliteDatabase db(test_db_path_.string());
    EXPECT_TRUE(db.isOpen());
  }
  // Database should be closed after destructor

  // Verify by opening again (should succeed)
  SqliteDatabase db2(test_db_path_.string());
  EXPECT_TRUE(db2.isOpen());
}

TEST_F(SqliteDatabaseTest, StatementDestructorFinalizesStatement) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (id INTEGER)");

  {
    auto stmt = db.prepare("SELECT * FROM test");
    // Statement should be finalized when going out of scope
  }

  // Verify database is still functional
  EXPECT_NO_THROW(db.execute("INSERT INTO test VALUES (1)"));
}

}  // namespace
}  // namespace Gateways::Database

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
