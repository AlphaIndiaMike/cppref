#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace Gateways::Database {

// ============================================================
// Database value types
// ============================================================
using DbValue = std::variant<std::nullptr_t, int64_t, double, std::string,
                             std::vector<uint8_t>  // BLOB
                             >;

using DbRow = std::vector<DbValue>;
using DbResult = std::vector<DbRow>;

// ============================================================
// Database exceptions
// ============================================================
class DatabaseException : public std::runtime_error {
 public:
  explicit DatabaseException(const std::string& message)
      : std::runtime_error(message) {}
};

class ConnectionException : public DatabaseException {
 public:
  explicit ConnectionException(const std::string& message)
      : DatabaseException("Connection error: " + message) {}
};

class QueryException : public DatabaseException {
 public:
  explicit QueryException(const std::string& message)
      : DatabaseException("Query error: " + message) {}
};

// ============================================================
// Prepared statement interface
// ============================================================
class IStatement {
 public:
  virtual ~IStatement() = default;

  virtual IStatement& bind(int index, std::nullptr_t) = 0;
  virtual IStatement& bind(int index, int64_t value) = 0;
  virtual IStatement& bind(int index, double value) = 0;
  virtual IStatement& bind(int index, const std::string& value) = 0;
  virtual IStatement& bind(int index, const std::vector<uint8_t>& blob) = 0;

  virtual DbResult execute() = 0;
  virtual int64_t executeInsert() = 0;  // Returns last insert rowid
  virtual int executeUpdate() = 0;      // Returns rows affected

  virtual void reset() = 0;
};

// ============================================================
// Database interface
// ============================================================
class IDatabase {
 public:
  virtual ~IDatabase() = default;

  virtual void open(const std::string& path) = 0;
  virtual void close() = 0;
  virtual bool isOpen() const = 0;

  virtual std::unique_ptr<IStatement> prepare(const std::string& sql) = 0;

  virtual void execute(const std::string& sql) = 0;
  virtual DbResult query(const std::string& sql) = 0;

  virtual void beginTransaction() = 0;
  virtual void commit() = 0;
  virtual void rollback() = 0;

  virtual int64_t lastInsertRowId() const = 0;
  virtual int changesCount() const = 0;
};

// ============================================================
// RAII Transaction guard
// ============================================================
class Transaction {
 public:
  explicit Transaction(IDatabase& db) : m_db(db), m_committed(false) {
    m_db.beginTransaction();
  }

  ~Transaction() {
    if (!m_committed) {
      try {
        m_db.rollback();
      } catch (...) {
        // Suppress exceptions in destructor
      }
    }
  }

  void commit() {
    m_db.commit();
    m_committed = true;
  }

  // Non-copyable, non-movable
  Transaction(const Transaction&) = delete;
  Transaction& operator=(const Transaction&) = delete;

 private:
  IDatabase& m_db;
  bool m_committed;
};

}  // namespace Gateways::Database
