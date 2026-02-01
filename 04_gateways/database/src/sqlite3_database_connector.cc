#include "sqlite3_database_connector.h"

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

  // Enable foreign keys by default
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

}  // namespace Gateways::Database
