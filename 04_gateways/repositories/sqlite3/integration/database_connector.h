#ifndef GATEWAYS_DATABASE_DATABASE_CONNECTOR_H_
#define GATEWAYS_DATABASE_DATABASE_CONNECTOR_H_

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace Gateways::Database {

// ============================================================
// Type Definitions
// ============================================================

using DbValue = std::variant<std::nullptr_t, int64_t, double, std::string,
                             std::vector<uint8_t>>;
using DbRow = std::vector<DbValue>;
using DbResult = std::vector<DbRow>;

// ============================================================
// Exceptions
// ============================================================

class DatabaseException : public std::runtime_error {
 public:
  explicit DatabaseException(const std::string& msg)
      : std::runtime_error(msg) {}
};

class ConnectionException : public DatabaseException {
 public:
  explicit ConnectionException(const std::string& msg)
      : DatabaseException("Connection error: " + msg) {}
};

class QueryException : public DatabaseException {
 public:
  explicit QueryException(const std::string& msg)
      : DatabaseException("Query error: " + msg) {}
};

// ============================================================
// IStatement Interface
// ============================================================

class IStatement {
 public:
  virtual ~IStatement() = default;

  // Bind parameters (1-based index)
  virtual IStatement& bind(int index, std::nullptr_t) = 0;
  virtual IStatement& bind(int index, int64_t value) = 0;
  virtual IStatement& bind(int index, double value) = 0;
  virtual IStatement& bind(int index, const std::string& value) = 0;
  virtual IStatement& bind(int index, const std::vector<uint8_t>& blob) = 0;

  // Execution
  virtual DbResult execute() = 0;
  virtual int64_t executeInsert() = 0;
  virtual int executeUpdate() = 0;
  virtual void reset() = 0;
};

// ============================================================
// IDatabase Interface
// ============================================================

class IDatabase {
 public:
  virtual ~IDatabase() = default;

  // Connection management
  virtual void open(const std::string& path) = 0;
  virtual void close() = 0;
  virtual bool isOpen() const = 0;

  // Statement preparation
  virtual std::unique_ptr<IStatement> prepare(const std::string& sql) = 0;

  // Direct execution
  virtual void execute(const std::string& sql) = 0;
  virtual DbResult query(const std::string& sql) = 0;

  // Transaction control
  virtual void beginTransaction() = 0;
  virtual void commit() = 0;
  virtual void rollback() = 0;

  // Metadata
  virtual int64_t lastInsertRowId() const = 0;
  virtual int changesCount() const = 0;
};

}  // namespace Gateways::Database

#endif  // GATEWAYS_DATABASE_DATABASE_CONNECTOR_H_
