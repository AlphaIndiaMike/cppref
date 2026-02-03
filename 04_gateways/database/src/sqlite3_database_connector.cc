#include "sqlite3_database_connector.h"

#include <sstream>
#include <utility>

namespace Gateways::Database {

// ============================================================
// SqliteStatement Implementation
// ============================================================

SqliteStatement::SqliteStatement(sqlite3* db, const std::string& sql)
    : m_db(db), m_stmt(nullptr) {
  int result = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &m_stmt, nullptr);
  checkError(result, "prepare statement");
}

SqliteStatement::~SqliteStatement() {
  if (m_stmt) {
    sqlite3_finalize(m_stmt);
  }
}

SqliteStatement::SqliteStatement(SqliteStatement&& other) noexcept
    : m_db(other.m_db), m_stmt(other.m_stmt) {
  other.m_stmt = nullptr;
}

SqliteStatement& SqliteStatement::operator=(SqliteStatement&& other) noexcept {
  if (this != &other) {
    if (m_stmt) {
      sqlite3_finalize(m_stmt);
    }
    m_db = other.m_db;
    m_stmt = other.m_stmt;
    other.m_stmt = nullptr;
  }
  return *this;
}

IStatement& SqliteStatement::bind(int index, std::nullptr_t) {
  checkError(sqlite3_bind_null(m_stmt, index), "bind null");
  return *this;
}

IStatement& SqliteStatement::bind(int index, int64_t value) {
  checkError(sqlite3_bind_int64(m_stmt, index, value), "bind int64");
  return *this;
}

IStatement& SqliteStatement::bind(int index, double value) {
  checkError(sqlite3_bind_double(m_stmt, index, value), "bind double");
  return *this;
}

IStatement& SqliteStatement::bind(int index, const std::string& value) {
  checkError(
      sqlite3_bind_text(m_stmt, index, value.c_str(), -1, SQLITE_TRANSIENT),
      "bind text");
  return *this;
}

IStatement& SqliteStatement::bind(int index, const std::vector<uint8_t>& blob) {
  checkError(sqlite3_bind_blob(m_stmt, index, blob.data(),
                               static_cast<int>(blob.size()), SQLITE_TRANSIENT),
             "bind blob");
  return *this;
}

void SqliteStatement::bindValue(int index, const DbValue& value) {
  std::visit(
      [this, index](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::nullptr_t>) {
          bind(index, nullptr);
        } else if constexpr (std::is_same_v<T, int64_t>) {
          bind(index, arg);
        } else if constexpr (std::is_same_v<T, double>) {
          bind(index, arg);
        } else if constexpr (std::is_same_v<T, std::string>) {
          bind(index, arg);
        } else if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
          bind(index, arg);
        }
      },
      value);
}

DbResult SqliteStatement::execute() {
  DbResult results;
  int columnCount = sqlite3_column_count(m_stmt);

  int result;
  while ((result = sqlite3_step(m_stmt)) == SQLITE_ROW) {
    DbRow row;
    row.reserve(columnCount);

    for (int i = 0; i < columnCount; ++i) {
      row.push_back(extractColumn(i));
    }

    results.push_back(std::move(row));
  }

  if (result != SQLITE_DONE) {
    checkError(result, "execute");
  }

  return results;
}

int64_t SqliteStatement::executeInsert() {
  int result = sqlite3_step(m_stmt);
  if (result != SQLITE_DONE) {
    checkError(result, "executeInsert");
  }
  return sqlite3_last_insert_rowid(m_db);
}

int SqliteStatement::executeUpdate() {
  int result = sqlite3_step(m_stmt);
  if (result != SQLITE_DONE) {
    checkError(result, "executeUpdate");
  }
  return sqlite3_changes(m_db);
}

void SqliteStatement::reset() {
  sqlite3_reset(m_stmt);
  sqlite3_clear_bindings(m_stmt);
}

int SqliteStatement::executeBatch(
    const std::vector<std::vector<DbValue>>& paramSets) {
  int totalChanges = 0;

  for (const auto& params : paramSets) {
    reset();

    for (size_t i = 0; i < params.size(); ++i) {
      bindValue(static_cast<int>(i + 1), params[i]);
    }

    int result = sqlite3_step(m_stmt);
    if (result != SQLITE_DONE) {
      checkError(result, "executeBatch");
    }

    totalChanges += sqlite3_changes(m_db);
  }

  return totalChanges;
}

void SqliteStatement::checkError(int result, const std::string& context) {
  if (result != SQLITE_OK && result != SQLITE_ROW && result != SQLITE_DONE) {
    throw QueryException(context + ": " + sqlite3_errmsg(m_db));
  }
}

DbValue SqliteStatement::extractColumn(int col) {
  int type = sqlite3_column_type(m_stmt, col);

  switch (type) {
    case SQLITE_NULL:
      return nullptr;

    case SQLITE_INTEGER:
      return static_cast<int64_t>(sqlite3_column_int64(m_stmt, col));

    case SQLITE_FLOAT:
      return sqlite3_column_double(m_stmt, col);

    case SQLITE_TEXT: {
      const char* text =
          reinterpret_cast<const char*>(sqlite3_column_text(m_stmt, col));
      return std::string(text ? text : "");
    }

    case SQLITE_BLOB: {
      const uint8_t* data =
          static_cast<const uint8_t*>(sqlite3_column_blob(m_stmt, col));
      int size = sqlite3_column_bytes(m_stmt, col);
      return std::vector<uint8_t>(data, data + size);
    }

    default:
      return nullptr;
  }
}

// ============================================================
// TransactionScope Implementation
// ============================================================

TransactionScope::TransactionScope(SqliteDatabase& db)
    : m_db(db), m_finished(false) {
  m_db.beginTransaction();
}

TransactionScope::~TransactionScope() {
  if (!m_finished) {
    try {
      m_db.rollback();
    } catch (...) {
      // Suppress exceptions in destructor
    }
  }
}

void TransactionScope::commit() {
  m_db.commit();
  m_finished = true;
}

void TransactionScope::rollback() {
  m_db.rollback();
  m_finished = true;
}

// ============================================================
// SqliteDatabase Implementation
// ============================================================

SqliteDatabase::SqliteDatabase(const std::string& path) { open(path); }

SqliteDatabase::~SqliteDatabase() { close(); }

SqliteDatabase::SqliteDatabase(SqliteDatabase&& other) noexcept
    : m_db(other.m_db) {
  other.m_db = nullptr;
}

SqliteDatabase& SqliteDatabase::operator=(SqliteDatabase&& other) noexcept {
  if (this != &other) {
    close();
    m_db = other.m_db;
    other.m_db = nullptr;
  }
  return *this;
}

void SqliteDatabase::open(const std::string& path) {
  if (m_db) {
    close();
  }

  int result = sqlite3_open(path.c_str(), &m_db);
  if (result != SQLITE_OK) {
    std::string error = m_db ? sqlite3_errmsg(m_db) : "Unknown error";
    if (m_db) {
      sqlite3_close(m_db);
      m_db = nullptr;
    }
    throw ConnectionException(error);
  }

  enableForeignKeys(true);
}

void SqliteDatabase::close() {
  if (m_db) {
    sqlite3_close(m_db);
    m_db = nullptr;
  }
}

bool SqliteDatabase::isOpen() const { return m_db != nullptr; }

std::unique_ptr<IStatement> SqliteDatabase::prepare(const std::string& sql) {
  if (!m_db) {
    throw ConnectionException("Database not open");
  }
  return std::make_unique<SqliteStatement>(m_db, sql);
}

void SqliteDatabase::execute(const std::string& sql) {
  if (!m_db) {
    throw ConnectionException("Database not open");
  }

  char* errorMsg = nullptr;
  int result = sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, &errorMsg);

  if (result != SQLITE_OK) {
    std::string error = errorMsg ? errorMsg : "Unknown error";
    sqlite3_free(errorMsg);
    throw QueryException(error);
  }
}

DbResult SqliteDatabase::query(const std::string& sql) {
  auto stmt = prepare(sql);
  return stmt->execute();
}

void SqliteDatabase::beginTransaction() { execute("BEGIN TRANSACTION"); }

void SqliteDatabase::commit() { execute("COMMIT"); }

void SqliteDatabase::rollback() { execute("ROLLBACK"); }

TransactionScope SqliteDatabase::transaction() {
  return TransactionScope(*this);
}

int64_t SqliteDatabase::lastInsertRowId() const {
  return m_db ? sqlite3_last_insert_rowid(m_db) : 0;
}

int SqliteDatabase::changesCount() const {
  return m_db ? sqlite3_changes(m_db) : 0;
}

void SqliteDatabase::enableForeignKeys(bool enable) {
  execute(enable ? "PRAGMA foreign_keys = ON" : "PRAGMA foreign_keys = OFF");
}

void SqliteDatabase::setJournalMode(const std::string& mode) {
  execute("PRAGMA journal_mode = " + mode);
}

int SqliteDatabase::bulkInsert(const std::string& table,
                               const std::vector<std::string>& columns,
                               const std::vector<std::vector<DbValue>>& rows) {
  if (rows.empty()) {
    return 0;
  }

  // Build: INSERT INTO table (c1, c2, ...) VALUES (?, ?, ...)
  std::ostringstream sql;
  sql << "INSERT INTO " << table << " (";

  for (size_t i = 0; i < columns.size(); ++i) {
    if (i > 0) sql << ", ";
    sql << columns[i];
  }

  sql << ") VALUES (";

  for (size_t i = 0; i < columns.size(); ++i) {
    if (i > 0) sql << ", ";
    sql << "?";
  }
  sql << ")";

  auto stmt = prepare(sql.str());
  auto* sqliteStmt = dynamic_cast<SqliteStatement*>(stmt.get());

  auto txn = transaction();
  int totalInserted = sqliteStmt->executeBatch(rows);
  txn.commit();

  return totalInserted;
}

int SqliteDatabase::bulkExecute(
    const std::string& sql,
    const std::vector<std::vector<DbValue>>& paramSets) {
  if (paramSets.empty()) {
    return 0;
  }

  auto stmt = prepare(sql);
  auto* sqliteStmt = dynamic_cast<SqliteStatement*>(stmt.get());

  auto txn = transaction();
  int totalAffected = sqliteStmt->executeBatch(paramSets);
  txn.commit();

  return totalAffected;
}

DbResult SqliteDatabase::bulkSelect(
    const std::string& sql,
    const std::vector<std::vector<DbValue>>& paramSets) {
  if (paramSets.empty()) {
    return {};
  }

  auto stmt = prepare(sql);
  auto* sqliteStmt = dynamic_cast<SqliteStatement*>(stmt.get());

  DbResult combinedResults;

  for (const auto& params : paramSets) {
    stmt->reset();

    for (size_t i = 0; i < params.size(); ++i) {
      sqliteStmt->bindValue(static_cast<int>(i + 1), params[i]);
    }

    DbResult rows = stmt->execute();
    combinedResults.insert(combinedResults.end(),
                           std::make_move_iterator(rows.begin()),
                           std::make_move_iterator(rows.end()));
  }

  return combinedResults;
}

}  // namespace Gateways::Database
