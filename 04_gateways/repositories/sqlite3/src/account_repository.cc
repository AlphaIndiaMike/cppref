#include "account_repository.h"

namespace Gateways::Repositories::Sqlite3 {

using namespace Gateways::Database;

AccountRepository::AccountRepository(IDatabase& db) : m_db(db) {}

void AccountRepository::initSchema() {
  m_db.execute(R"(
    CREATE TABLE IF NOT EXISTS accounts (
      id TEXT PRIMARY KEY,
      name TEXT NOT NULL UNIQUE,
      password_hash BLOB,
      created_at INTEGER NOT NULL
    )
  )");

  m_db.execute(R"(
    CREATE TABLE IF NOT EXISTS account_properties (
      account_id TEXT NOT NULL,
      key TEXT NOT NULL,
      value TEXT NOT NULL,
      description TEXT,
      PRIMARY KEY (account_id, key),
      FOREIGN KEY (account_id) REFERENCES accounts(id) ON DELETE CASCADE
    )
  )");

  m_db.execute(
      "CREATE INDEX IF NOT EXISTS idx_accounts_name ON accounts(name)");
}

// ============================================================
// Account CRUD
// ============================================================

void AccountRepository::createAccount(const Entities::Account& account) {
  auto stmt = m_db.prepare(
      "INSERT INTO accounts (id, name, password_hash, created_at) "
      "VALUES (?, ?, ?, ?)");
  stmt->bind(1, account.id).bind(2, account.name);

  if (account.password_hash) {
    stmt->bind(3, *account.password_hash);
  } else {
    stmt->bind(3, nullptr);
  }

  stmt->bind(4, account.created_at);
  stmt->executeInsert();
}

std::optional<Entities::Account> AccountRepository::getAccount(
    const std::string& id) {
  auto stmt = m_db.prepare(
      "SELECT id, name, password_hash, created_at FROM accounts WHERE id = ?");
  stmt->bind(1, id);
  auto result = stmt->execute();

  if (result.empty()) {
    return std::nullopt;
  }

  Entities::Account account;
  account.id = std::get<std::string>(result[0][0]);
  account.name = std::get<std::string>(result[0][1]);

  if (!std::holds_alternative<std::nullptr_t>(result[0][2])) {
    account.password_hash = std::get<std::vector<uint8_t>>(result[0][2]);
  }

  account.created_at = std::get<int64_t>(result[0][3]);

  return account;
}

std::optional<Entities::Account> AccountRepository::getAccountByName(
    const std::string& name) {
  auto stmt = m_db.prepare(
      "SELECT id, name, password_hash, created_at FROM accounts WHERE name = "
      "?");
  stmt->bind(1, name);
  auto result = stmt->execute();

  if (result.empty()) {
    return std::nullopt;
  }

  Entities::Account account;
  account.id = std::get<std::string>(result[0][0]);
  account.name = std::get<std::string>(result[0][1]);

  if (!std::holds_alternative<std::nullptr_t>(result[0][2])) {
    account.password_hash = std::get<std::vector<uint8_t>>(result[0][2]);
  }

  account.created_at = std::get<int64_t>(result[0][3]);

  return account;
}

std::vector<Entities::Account> AccountRepository::getAllAccounts() {
  auto result = m_db.query(
      "SELECT id, name, password_hash, created_at FROM accounts ORDER BY name");

  std::vector<Entities::Account> accounts;
  accounts.reserve(result.size());

  for (const auto& row : result) {
    Entities::Account account;
    account.id = std::get<std::string>(row[0]);
    account.name = std::get<std::string>(row[1]);

    if (!std::holds_alternative<std::nullptr_t>(row[2])) {
      account.password_hash = std::get<std::vector<uint8_t>>(row[2]);
    }

    account.created_at = std::get<int64_t>(row[3]);
    accounts.push_back(std::move(account));
  }

  return accounts;
}

void AccountRepository::updateAccount(const Entities::Account& account) {
  auto stmt = m_db.prepare(
      "UPDATE accounts SET name = ?, password_hash = ?, created_at = ? "
      "WHERE id = ?");
  stmt->bind(1, account.name);

  if (account.password_hash) {
    stmt->bind(2, *account.password_hash);
  } else {
    stmt->bind(2, nullptr);
  }

  stmt->bind(3, account.created_at).bind(4, account.id);
  stmt->executeUpdate();
}

void AccountRepository::deleteAccount(const std::string& id) {
  auto stmt = m_db.prepare("DELETE FROM accounts WHERE id = ?");
  stmt->bind(1, id);
  stmt->executeUpdate();
}

bool AccountRepository::accountExists(const std::string& id) {
  auto stmt = m_db.prepare("SELECT 1 FROM accounts WHERE id = ?");
  stmt->bind(1, id);
  auto result = stmt->execute();
  return !result.empty();
}

bool AccountRepository::accountExistsByName(const std::string& name) {
  auto stmt = m_db.prepare("SELECT 1 FROM accounts WHERE name = ?");
  stmt->bind(1, name);
  auto result = stmt->execute();
  return !result.empty();
}

// ============================================================
// Account Property CRUD
// ============================================================

void AccountRepository::setProperty(
    const std::string& account_id, const std::string& key,
    const std::string& value, const std::optional<std::string>& description) {
  auto stmt = m_db.prepare(
      "INSERT OR REPLACE INTO account_properties "
      "(account_id, key, value, description) VALUES (?, ?, ?, ?)");
  stmt->bind(1, account_id).bind(2, key).bind(3, value);

  if (description) {
    stmt->bind(4, *description);
  } else {
    stmt->bind(4, nullptr);
  }

  stmt->executeInsert();
}

void AccountRepository::setProperty(const Entities::AccountProperty& property) {
  setProperty(property.account_id, property.key, property.value,
              property.description);
}

std::optional<Entities::AccountProperty> AccountRepository::getProperty(
    const std::string& account_id, const std::string& key) {
  auto stmt = m_db.prepare(
      "SELECT account_id, key, value, description FROM account_properties "
      "WHERE account_id = ? AND key = ?");
  stmt->bind(1, account_id).bind(2, key);
  auto result = stmt->execute();

  if (result.empty()) {
    return std::nullopt;
  }

  Entities::AccountProperty property;
  property.account_id = std::get<std::string>(result[0][0]);
  property.key = std::get<std::string>(result[0][1]);
  property.value = std::get<std::string>(result[0][2]);

  if (!std::holds_alternative<std::nullptr_t>(result[0][3])) {
    property.description = std::get<std::string>(result[0][3]);
  }

  return property;
}

std::optional<std::string> AccountRepository::getPropertyValue(
    const std::string& account_id, const std::string& key) {
  auto stmt = m_db.prepare(
      "SELECT value FROM account_properties WHERE account_id = ? AND key = ?");
  stmt->bind(1, account_id).bind(2, key);
  auto result = stmt->execute();

  if (result.empty()) {
    return std::nullopt;
  }

  return std::get<std::string>(result[0][0]);
}

std::vector<Entities::AccountProperty> AccountRepository::getProperties(
    const std::string& account_id) {
  auto stmt = m_db.prepare(
      "SELECT account_id, key, value, description FROM account_properties "
      "WHERE account_id = ? ORDER BY key");
  stmt->bind(1, account_id);
  auto result = stmt->execute();

  std::vector<Entities::AccountProperty> properties;
  properties.reserve(result.size());

  for (const auto& row : result) {
    Entities::AccountProperty property;
    property.account_id = std::get<std::string>(row[0]);
    property.key = std::get<std::string>(row[1]);
    property.value = std::get<std::string>(row[2]);

    if (!std::holds_alternative<std::nullptr_t>(row[3])) {
      property.description = std::get<std::string>(row[3]);
    }

    properties.push_back(std::move(property));
  }

  return properties;
}

std::vector<Entities::AccountProperty> AccountRepository::getPropertiesByPrefix(
    const std::string& account_id, const std::string& prefix) {
  auto stmt = m_db.prepare(
      "SELECT account_id, key, value, description FROM account_properties "
      "WHERE account_id = ? AND key LIKE ? ORDER BY key");
  stmt->bind(1, account_id).bind(2, prefix + "%");
  auto result = stmt->execute();

  std::vector<Entities::AccountProperty> properties;
  properties.reserve(result.size());

  for (const auto& row : result) {
    Entities::AccountProperty property;
    property.account_id = std::get<std::string>(row[0]);
    property.key = std::get<std::string>(row[1]);
    property.value = std::get<std::string>(row[2]);

    if (!std::holds_alternative<std::nullptr_t>(row[3])) {
      property.description = std::get<std::string>(row[3]);
    }

    properties.push_back(std::move(property));
  }

  return properties;
}

bool AccountRepository::propertyExists(const std::string& account_id,
                                       const std::string& key) {
  auto stmt = m_db.prepare(
      "SELECT 1 FROM account_properties WHERE account_id = ? AND key = ?");
  stmt->bind(1, account_id).bind(2, key);
  auto result = stmt->execute();
  return !result.empty();
}

void AccountRepository::removeProperty(const std::string& account_id,
                                       const std::string& key) {
  auto stmt = m_db.prepare(
      "DELETE FROM account_properties WHERE account_id = ? AND key = ?");
  stmt->bind(1, account_id).bind(2, key);
  stmt->executeUpdate();
}

void AccountRepository::removePropertiesByPrefix(const std::string& account_id,
                                                 const std::string& prefix) {
  auto stmt = m_db.prepare(
      "DELETE FROM account_properties WHERE account_id = ? AND key LIKE ?");
  stmt->bind(1, account_id).bind(2, prefix + "%");
  stmt->executeUpdate();
}

void AccountRepository::clearProperties(const std::string& account_id) {
  auto stmt =
      m_db.prepare("DELETE FROM account_properties WHERE account_id = ?");
  stmt->bind(1, account_id);
  stmt->executeUpdate();
}

// ============================================================
// Count
// ============================================================

int64_t AccountRepository::countAccounts() {
  auto result = m_db.query("SELECT COUNT(*) FROM accounts");
  return std::get<int64_t>(result[0][0]);
}

int64_t AccountRepository::countProperties(const std::string& account_id) {
  auto stmt = m_db.prepare(
      "SELECT COUNT(*) FROM account_properties WHERE account_id = ?");
  stmt->bind(1, account_id);
  auto result = stmt->execute();
  return std::get<int64_t>(result[0][0]);
}

}  // namespace Gateways::Repositories::Sqlite3
