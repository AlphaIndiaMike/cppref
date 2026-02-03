#ifndef GATEWAYS_DATABASE_SQLITE3_DATABASE_CONNECTOR_H_
#define GATEWAYS_DATABASE_SQLITE3_DATABASE_CONNECTOR_H_

#include <memory>
#include <string>
#include <vector>

#include "database_connector.h"
#include "sqlite3.h"

namespace Gateways::Database {

// ============================================================
// SqliteStatement
// ============================================================

class SqliteStatement : public IStatement {
 public:
  SqliteStatement(sqlite3* db, const std::string& sql);
  ~SqliteStatement() override;

  SqliteStatement(const SqliteStatement&) = delete;
  SqliteStatement& operator=(const SqliteStatement&) = delete;
  SqliteStatement(SqliteStatement&& other) noexcept;
  SqliteStatement& operator=(SqliteStatement&& other) noexcept;

  // Bind parameters (1-based index)
  IStatement& bind(int index, std::nullptr_t) override;
  IStatement& bind(int index, int64_t value) override;
  IStatement& bind(int index, double value) override;
  IStatement& bind(int index, const std::string& value) override;
  IStatement& bind(int index, const std::vector<uint8_t>& blob) override;

  // Execution
  DbResult execute() override;
  int64_t executeInsert() override;
  int executeUpdate() override;
  void reset() override;

  // Batch execution: runs statement for each parameter set.
  // Each inner vector contains values for one execution (bound in order).
  // Returns total affected rows.
  int executeBatch(const std::vector<std::vector<DbValue>>& paramSets);

  // Bind a DbValue variant (1-based index)
  void bindValue(int index, const DbValue& value);

 private:
  void checkError(int result, const std::string& context);
  DbValue extractColumn(int col);

  sqlite3* m_db;
  sqlite3_stmt* m_stmt;
};

// ============================================================
// TransactionScope - RAII transaction guard
// ============================================================

class SqliteDatabase;

class TransactionScope {
 public:
  explicit TransactionScope(SqliteDatabase& db);
  ~TransactionScope();

  TransactionScope(const TransactionScope&) = delete;
  TransactionScope& operator=(const TransactionScope&) = delete;
  TransactionScope(TransactionScope&&) = delete;
  TransactionScope& operator=(TransactionScope&&) = delete;

  void commit();
  void rollback();

 private:
  SqliteDatabase& m_db;
  bool m_finished;
};

// ============================================================
// SqliteDatabase
// ============================================================

class SqliteDatabase : public IDatabase {
 public:
  SqliteDatabase() = default;
  explicit SqliteDatabase(const std::string& path);
  ~SqliteDatabase() override;

  SqliteDatabase(const SqliteDatabase&) = delete;
  SqliteDatabase& operator=(const SqliteDatabase&) = delete;
  SqliteDatabase(SqliteDatabase&& other) noexcept;
  SqliteDatabase& operator=(SqliteDatabase&& other) noexcept;

  // Connection
  void open(const std::string& path) override;
  void close() override;
  bool isOpen() const override;

  // Statement preparation
  std::unique_ptr<IStatement> prepare(const std::string& sql) override;

  // Direct execution
  void execute(const std::string& sql) override;
  DbResult query(const std::string& sql) override;

  // Transaction control
  void beginTransaction() override;
  void commit() override;
  void rollback() override;

  // RAII transaction scope
  TransactionScope transaction();

  // Metadata
  int64_t lastInsertRowId() const override;
  int changesCount() const override;

  // SQLite-specific settings
  void enableForeignKeys(bool enable);
  void setJournalMode(const std::string& mode);

  // Bulk insert: inserts multiple rows into a table.
  // Returns total rows inserted.
  int bulkInsert(const std::string& table,
                 const std::vector<std::string>& columns,
                 const std::vector<std::vector<DbValue>>& rows);

  // Bulk execute: runs parameterized SQL with multiple parameter sets.
  // Returns total affected rows.
  int bulkExecute(const std::string& sql,
                  const std::vector<std::vector<DbValue>>& paramSets);

  // Bulk select: executes parameterized query for each parameter set.
  // Returns combined results from all executions.
  DbResult bulkSelect(const std::string& sql,
                      const std::vector<std::vector<DbValue>>& paramSets);

 private:
  sqlite3* m_db = nullptr;
};

}  // namespace Gateways::Database

#endif  // GATEWAYS_DATABASE_SQLITE3_DATABASE_CONNECTOR_H_
