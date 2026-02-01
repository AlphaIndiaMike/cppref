#pragma once

#include "database_connector.h"
#include "sqlite3.h"

namespace Gateways::Database {

// ============================================================
// SQLite Prepared Statement
// ============================================================
class SqliteStatement : public IStatement {
 public:
  SqliteStatement(sqlite3* db, const std::string& sql);
  ~SqliteStatement() override;

  // Non-copyable
  SqliteStatement(const SqliteStatement&) = delete;
  SqliteStatement& operator=(const SqliteStatement&) = delete;

  // Movable
  SqliteStatement(SqliteStatement&& other) noexcept;
  SqliteStatement& operator=(SqliteStatement&& other) noexcept;

  IStatement& bind(int index, std::nullptr_t) override;
  IStatement& bind(int index, int64_t value) override;
  IStatement& bind(int index, double value) override;
  IStatement& bind(int index, const std::string& value) override;
  IStatement& bind(int index, const std::vector<uint8_t>& blob) override;

  DbResult execute() override;
  int64_t executeInsert() override;
  int executeUpdate() override;

  void reset() override;

 private:
  sqlite3* m_db;
  sqlite3_stmt* m_stmt;

  void checkError(int result, const std::string& context);
  DbValue extractColumn(int col);
};

// ============================================================
// SQLite Database Implementation
// ============================================================
class SqliteDatabase : public IDatabase {
 public:
  SqliteDatabase() = default;
  explicit SqliteDatabase(const std::string& path);
  ~SqliteDatabase() override;

  // Non-copyable
  SqliteDatabase(const SqliteDatabase&) = delete;
  SqliteDatabase& operator=(const SqliteDatabase&) = delete;

  // Movable
  SqliteDatabase(SqliteDatabase&& other) noexcept;
  SqliteDatabase& operator=(SqliteDatabase&& other) noexcept;

  void open(const std::string& path) override;
  void close() override;
  bool isOpen() const override;

  std::unique_ptr<IStatement> prepare(const std::string& sql) override;

  void execute(const std::string& sql) override;
  DbResult query(const std::string& sql) override;

  void beginTransaction() override;
  void commit() override;
  void rollback() override;

  int64_t lastInsertRowId() const override;
  int changesCount() const override;

  // SQLite-specific
  void enableForeignKeys(bool enable = true);
  void setJournalMode(const std::string& mode);  // WAL, DELETE, etc.

 private:
  sqlite3* m_db = nullptr;

  void checkError(int result, const std::string& context);
};

}  // namespace Gateways::Database
