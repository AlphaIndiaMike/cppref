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
    test_db_path_ = std::filesystem::temp_directory_path() /
                    ("test_db_" +
                     std::to_string(reinterpret_cast<uintptr_t>(this)) + ".db");
  }

  void TearDown() override { std::filesystem::remove(test_db_path_); }

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

  auto other_path = test_db_path_.string() + "_other.db";
  db.open(other_path);
  EXPECT_TRUE(db.isOpen());

  std::filesystem::remove(other_path);
}

TEST_F(SqliteDatabaseTest, OpenInvalidPath) {
  SqliteDatabase db;
  EXPECT_THROW(db.open("/nonexistent/path/to/database.db"),
               ConnectionException);
}

TEST_F(SqliteDatabaseTest, CloseAlreadyClosed) {
  SqliteDatabase db;
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

  std::unique_ptr<IStatement> stmt1 =
      db.prepare("INSERT INTO test (value) VALUES (?)");
  stmt1->bind(1, std::string("test"));

  EXPECT_NO_THROW(stmt1->executeInsert());
}

TEST_F(SqliteDatabaseTest, StatementMoveAssignment) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");

  std::unique_ptr<IStatement> stmt1 =
      db.prepare("INSERT INTO test (value) VALUES (?)");
  std::unique_ptr<IStatement> stmt2 =
      db.prepare("INSERT INTO test (value) VALUES (?)");

  stmt2 = std::move(stmt1);

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

TEST_F(SqliteDatabaseTest, TransactionScopeCommit) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");

  {
    auto txn = db.transaction();
    db.execute("INSERT INTO test (value) VALUES ('test')");
    txn.commit();
  }

  auto result = db.query("SELECT COUNT(*) FROM test");
  EXPECT_EQ(std::get<int64_t>(result[0][0]), 1);
}

TEST_F(SqliteDatabaseTest, TransactionScopeAutoRollback) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");

  {
    auto txn = db.transaction();
    db.execute("INSERT INTO test (value) VALUES ('test')");
    // No commit - should auto-rollback
  }

  auto result = db.query("SELECT COUNT(*) FROM test");
  EXPECT_EQ(std::get<int64_t>(result[0][0]), 0);
}

TEST_F(SqliteDatabaseTest, TransactionScopeRollbackOnException) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");

  try {
    auto txn = db.transaction();
    db.execute("INSERT INTO test (value) VALUES ('test')");
    throw std::runtime_error("Simulated error");
    txn.commit();  // Never reached
  } catch (const std::runtime_error&) {
    // Expected
  }

  auto result = db.query("SELECT COUNT(*) FROM test");
  EXPECT_EQ(std::get<int64_t>(result[0][0]), 0);
}

TEST_F(SqliteDatabaseTest, TransactionScopeExplicitRollback) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");

  {
    auto txn = db.transaction();
    db.execute("INSERT INTO test (value) VALUES ('test')");
    txn.rollback();
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
  stmt->bind(1, std::string("unique_value"));

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

  auto stmt = db.prepare("DELETE FROM parent WHERE id = ?");
  stmt->bind(1, int64_t{1});

  EXPECT_THROW(stmt->executeUpdate(), QueryException);
}

TEST_F(SqliteDatabaseTest, BindInvalidIndex) {
  SqliteDatabase db(test_db_path_.string());

  auto stmt = db.prepare("SELECT ?");

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

  if (std::holds_alternative<std::vector<uint8_t>>(result[0][0])) {
    EXPECT_EQ(std::get<std::vector<uint8_t>>(result[0][0]),
              std::vector<uint8_t>{});
  } else {
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

  SqliteDatabase db2(test_db_path_.string());
  EXPECT_TRUE(db2.isOpen());
}

TEST_F(SqliteDatabaseTest, StatementDestructorFinalizesStatement) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (id INTEGER)");

  { auto stmt = db.prepare("SELECT * FROM test"); }

  EXPECT_NO_THROW(db.execute("INSERT INTO test VALUES (1)"));
}

TEST_F(SqliteDatabaseTest, SqliteStatementMoveConstructorDirect) {
  SqliteDatabase db(test_db_path_.string());

  auto stmt_ptr = db.prepare("SELECT 1");
  SqliteStatement* raw = dynamic_cast<SqliteStatement*>(stmt_ptr.release());

  SqliteStatement moved_stmt(std::move(*raw));

  auto result = moved_stmt.execute();
  EXPECT_EQ(result.size(), 1u);

  delete raw;
}

TEST_F(SqliteDatabaseTest, SqliteStatementMoveAssignmentDirect) {
  SqliteDatabase db(test_db_path_.string());

  auto stmt1_ptr = db.prepare("SELECT 1");
  auto stmt2_ptr = db.prepare("SELECT 2");

  SqliteStatement* stmt1 = dynamic_cast<SqliteStatement*>(stmt1_ptr.release());
  SqliteStatement* stmt2 = dynamic_cast<SqliteStatement*>(stmt2_ptr.release());

  *stmt2 = std::move(*stmt1);

  auto result = stmt2->execute();
  EXPECT_EQ(result.size(), 1u);

  delete stmt1;
  delete stmt2;
}

TEST_F(SqliteDatabaseTest, SqliteStatementMoveAssignmentSelf) {
  SqliteDatabase db(test_db_path_.string());

  auto stmt_ptr = db.prepare("SELECT 1");
  SqliteStatement* stmt = dynamic_cast<SqliteStatement*>(stmt_ptr.release());

  *stmt = std::move(*stmt);

  auto result = stmt->execute();
  EXPECT_EQ(result.size(), 1u);

  delete stmt;
}

TEST_F(SqliteDatabaseTest, ExecuteInsertErrorPath) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT UNIQUE)");
  db.execute("INSERT INTO test (value) VALUES ('duplicate')");

  auto stmt = db.prepare("INSERT INTO test (value) VALUES (?)");
  stmt->bind(1, std::string("duplicate"));

  EXPECT_THROW(stmt->executeInsert(), QueryException);
}

TEST_F(SqliteDatabaseTest, SqliteDatabaseMoveAssignmentSelfDirect) {
  SqliteDatabase db(test_db_path_.string());
  EXPECT_TRUE(db.isOpen());

  SqliteDatabase* db_ptr = &db;
  *db_ptr = std::move(db);

  EXPECT_TRUE(db_ptr->isOpen());
}

// ============================================================
// Bulk Insert Tests
// ============================================================
TEST_F(SqliteDatabaseTest, BulkInsertEmpty) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");

  int inserted = db.bulkInsert("test", {"value"}, {});
  EXPECT_EQ(inserted, 0);

  auto result = db.query("SELECT COUNT(*) FROM test");
  EXPECT_EQ(std::get<int64_t>(result[0][0]), 0);
}

TEST_F(SqliteDatabaseTest, BulkInsertSingleRow) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");

  int inserted = db.bulkInsert("test", {"value"}, {{std::string("one")}});
  EXPECT_EQ(inserted, 1);

  auto result = db.query("SELECT value FROM test");
  ASSERT_EQ(result.size(), 1u);
  EXPECT_EQ(std::get<std::string>(result[0][0]), "one");
}

TEST_F(SqliteDatabaseTest, BulkInsertMultipleRows) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (id INTEGER PRIMARY KEY, a INTEGER, b TEXT)");

  std::vector<std::vector<DbValue>> rows = {
      {int64_t{1}, std::string("one")},
      {int64_t{2}, std::string("two")},
      {int64_t{3}, std::string("three")},
  };

  int inserted = db.bulkInsert("test", {"a", "b"}, rows);
  EXPECT_EQ(inserted, 3);

  auto result = db.query("SELECT a, b FROM test ORDER BY a");
  ASSERT_EQ(result.size(), 3u);
  EXPECT_EQ(std::get<int64_t>(result[0][0]), 1);
  EXPECT_EQ(std::get<std::string>(result[0][1]), "one");
  EXPECT_EQ(std::get<int64_t>(result[2][0]), 3);
  EXPECT_EQ(std::get<std::string>(result[2][1]), "three");
}

TEST_F(SqliteDatabaseTest, BulkInsertTimeSeries) {
  SqliteDatabase db(test_db_path_.string());
  db.execute(
      "CREATE TABLE timeseries ("
      "  timestamp INTEGER NOT NULL,"
      "  value REAL NOT NULL"
      ")");

  // Insert 1000 time-series points
  std::vector<std::vector<DbValue>> rows;
  rows.reserve(1000);
  for (int64_t t = 0; t < 1000; ++t) {
    rows.push_back({t, static_cast<double>(t) * 0.1});
  }

  int inserted = db.bulkInsert("timeseries", {"timestamp", "value"}, rows);
  EXPECT_EQ(inserted, 1000);

  auto result = db.query("SELECT COUNT(*) FROM timeseries");
  EXPECT_EQ(std::get<int64_t>(result[0][0]), 1000);

  result = db.query("SELECT value FROM timeseries WHERE timestamp = 500");
  EXPECT_DOUBLE_EQ(std::get<double>(result[0][0]), 50.0);
}

TEST_F(SqliteDatabaseTest, BulkInsertWithNulls) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (a INTEGER, b TEXT, c REAL)");

  std::vector<std::vector<DbValue>> rows = {
      {int64_t{1}, nullptr, 1.5},
      {nullptr, std::string("two"), 2.5},
      {int64_t{3}, std::string("three"), nullptr},
  };

  int inserted = db.bulkInsert("test", {"a", "b", "c"}, rows);
  EXPECT_EQ(inserted, 3);

  auto result = db.query("SELECT * FROM test ORDER BY ROWID");
  ASSERT_EQ(result.size(), 3u);

  EXPECT_EQ(std::get<int64_t>(result[0][0]), 1);
  EXPECT_TRUE(std::holds_alternative<std::nullptr_t>(result[0][1]));

  EXPECT_TRUE(std::holds_alternative<std::nullptr_t>(result[1][0]));
  EXPECT_EQ(std::get<std::string>(result[1][1]), "two");

  EXPECT_TRUE(std::holds_alternative<std::nullptr_t>(result[2][2]));
}

TEST_F(SqliteDatabaseTest, BulkInsertWithBlobs) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (id INTEGER, data BLOB)");

  std::vector<std::vector<DbValue>> rows = {
      {int64_t{1}, std::vector<uint8_t>{0x01, 0x02}},
      {int64_t{2}, std::vector<uint8_t>{0x03, 0x04, 0x05}},
  };

  int inserted = db.bulkInsert("test", {"id", "data"}, rows);
  EXPECT_EQ(inserted, 2);

  auto result = db.query("SELECT data FROM test WHERE id = 2");
  auto blob = std::get<std::vector<uint8_t>>(result[0][0]);
  EXPECT_EQ(blob, (std::vector<uint8_t>{0x03, 0x04, 0x05}));
}

// ============================================================
// Bulk Execute Tests
// ============================================================
TEST_F(SqliteDatabaseTest, BulkExecuteEmpty) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (value INTEGER)");

  int affected = db.bulkExecute("INSERT INTO test VALUES (?)", {});
  EXPECT_EQ(affected, 0);
}

TEST_F(SqliteDatabaseTest, BulkExecuteInsert) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (value INTEGER)");

  std::vector<std::vector<DbValue>> params = {
      {int64_t{10}},
      {int64_t{20}},
      {int64_t{30}},
  };

  int affected = db.bulkExecute("INSERT INTO test VALUES (?)", params);
  EXPECT_EQ(affected, 3);

  auto result = db.query("SELECT SUM(value) FROM test");
  EXPECT_EQ(std::get<int64_t>(result[0][0]), 60);
}

TEST_F(SqliteDatabaseTest, BulkExecuteUpdate) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value INTEGER)");
  db.execute("INSERT INTO test VALUES (1, 10), (2, 20), (3, 30)");

  std::vector<std::vector<DbValue>> params = {
      {int64_t{100}, int64_t{1}},
      {int64_t{200}, int64_t{2}},
  };

  int affected =
      db.bulkExecute("UPDATE test SET value = ? WHERE id = ?", params);
  EXPECT_EQ(affected, 2);

  auto result = db.query("SELECT value FROM test ORDER BY id");
  EXPECT_EQ(std::get<int64_t>(result[0][0]), 100);
  EXPECT_EQ(std::get<int64_t>(result[1][0]), 200);
  EXPECT_EQ(std::get<int64_t>(result[2][0]), 30);
}

TEST_F(SqliteDatabaseTest, BulkExecuteDelete) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (id INTEGER PRIMARY KEY)");
  db.execute("INSERT INTO test VALUES (1), (2), (3), (4), (5)");

  std::vector<std::vector<DbValue>> params = {
      {int64_t{2}},
      {int64_t{4}},
  };

  int affected = db.bulkExecute("DELETE FROM test WHERE id = ?", params);
  EXPECT_EQ(affected, 2);

  auto result = db.query("SELECT COUNT(*) FROM test");
  EXPECT_EQ(std::get<int64_t>(result[0][0]), 3);
}

// ============================================================
// Bulk Select Tests
// ============================================================
TEST_F(SqliteDatabaseTest, BulkSelectEmpty) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");

  auto result = db.bulkSelect("SELECT * FROM test WHERE id = ?", {});
  EXPECT_TRUE(result.empty());
}

TEST_F(SqliteDatabaseTest, BulkSelectSingleParam) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");
  db.execute("INSERT INTO test VALUES (1, 'one'), (2, 'two'), (3, 'three')");

  auto result =
      db.bulkSelect("SELECT value FROM test WHERE id = ?", {{int64_t{2}}});

  ASSERT_EQ(result.size(), 1u);
  EXPECT_EQ(std::get<std::string>(result[0][0]), "two");
}

TEST_F(SqliteDatabaseTest, BulkSelectMultipleParams) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");
  db.execute("INSERT INTO test VALUES (1, 'one'), (2, 'two'), (3, 'three')");

  std::vector<std::vector<DbValue>> params = {
      {int64_t{1}},
      {int64_t{3}},
  };

  auto result = db.bulkSelect("SELECT value FROM test WHERE id = ?", params);

  ASSERT_EQ(result.size(), 2u);
  EXPECT_EQ(std::get<std::string>(result[0][0]), "one");
  EXPECT_EQ(std::get<std::string>(result[1][0]), "three");
}

TEST_F(SqliteDatabaseTest, BulkSelectMultipleRowsPerParam) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (category TEXT, value INTEGER)");
  db.execute(
      "INSERT INTO test VALUES "
      "('a', 1), ('a', 2), ('b', 3), ('b', 4), ('c', 5)");

  std::vector<std::vector<DbValue>> params = {
      {std::string("a")},
      {std::string("b")},
  };

  auto result =
      db.bulkSelect("SELECT value FROM test WHERE category = ?", params);

  // 2 rows for 'a' + 2 rows for 'b' = 4 total
  ASSERT_EQ(result.size(), 4u);
}

TEST_F(SqliteDatabaseTest, BulkSelectNoMatches) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");
  db.execute("INSERT INTO test VALUES (1, 'one')");

  auto result = db.bulkSelect("SELECT value FROM test WHERE id = ?",
                              {{int64_t{99}}, {int64_t{100}}});

  EXPECT_TRUE(result.empty());
}

// ============================================================
// Statement executeBatch Tests
// ============================================================
TEST_F(SqliteDatabaseTest, StatementExecuteBatch) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (value INTEGER)");

  auto stmt = db.prepare("INSERT INTO test VALUES (?)");
  auto* sqliteStmt = dynamic_cast<SqliteStatement*>(stmt.get());

  std::vector<std::vector<DbValue>> params = {
      {int64_t{1}},
      {int64_t{2}},
      {int64_t{3}},
  };

  db.beginTransaction();
  int affected = sqliteStmt->executeBatch(params);
  db.commit();

  EXPECT_EQ(affected, 3);

  auto result = db.query("SELECT SUM(value) FROM test");
  EXPECT_EQ(std::get<int64_t>(result[0][0]), 6);
}

TEST_F(SqliteDatabaseTest, StatementBindValue) {
  SqliteDatabase db(test_db_path_.string());
  db.execute("CREATE TABLE test (a, b INTEGER, c REAL, d TEXT, e BLOB)");

  auto stmt = db.prepare("INSERT INTO test VALUES (?, ?, ?, ?, ?)");
  auto* sqliteStmt = dynamic_cast<SqliteStatement*>(stmt.get());

  sqliteStmt->bindValue(1, nullptr);
  sqliteStmt->bindValue(2, int64_t{42});
  sqliteStmt->bindValue(3, 3.14);
  sqliteStmt->bindValue(4, std::string("hello"));
  sqliteStmt->bindValue(5, std::vector<uint8_t>{0xAB, 0xCD});
  sqliteStmt->executeInsert();

  auto result = db.query("SELECT * FROM test");
  ASSERT_EQ(result.size(), 1u);
  EXPECT_TRUE(std::holds_alternative<std::nullptr_t>(result[0][0]));
  EXPECT_EQ(std::get<int64_t>(result[0][1]), 42);
  EXPECT_DOUBLE_EQ(std::get<double>(result[0][2]), 3.14);
  EXPECT_EQ(std::get<std::string>(result[0][3]), "hello");
  EXPECT_EQ(std::get<std::vector<uint8_t>>(result[0][4]),
            (std::vector<uint8_t>{0xAB, 0xCD}));
}

// ============================================================
// Performance Sanity Check
// ============================================================
TEST_F(SqliteDatabaseTest, BulkInsertPerformance) {
  SqliteDatabase db(test_db_path_.string());
  db.setJournalMode("WAL");
  db.execute("CREATE TABLE perf_test (ts INTEGER, val REAL)");

  // Insert 10,000 rows - should complete quickly with bulk insert
  std::vector<std::vector<DbValue>> rows;
  rows.reserve(10000);
  for (int64_t i = 0; i < 10000; ++i) {
    rows.push_back({i, static_cast<double>(i)});
  }

  int inserted = db.bulkInsert("perf_test", {"ts", "val"}, rows);
  EXPECT_EQ(inserted, 10000);

  auto result = db.query("SELECT COUNT(*) FROM perf_test");
  EXPECT_EQ(std::get<int64_t>(result[0][0]), 10000);
}

}  // namespace
}  // namespace Gateways::Database

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
