#include "keyvalue_repository.h"

namespace Gateways::Repositories::Sqlite3 {

using namespace Gateways::Database;

KeyValueRepository::KeyValueRepository(IDatabase& db) : m_db(db) {}

void KeyValueRepository::initSchema() {
  m_db.execute(R"(
    CREATE TABLE IF NOT EXISTS settings (
      key TEXT PRIMARY KEY,
      value TEXT NOT NULL,
      description TEXT
    )
  )");
}

// ============================================================
// CRUD Operations
// ============================================================

void KeyValueRepository::set(const std::string& key, const std::string& value,
                             const std::optional<std::string>& description) {
  auto stmt = m_db.prepare(
      "INSERT OR REPLACE INTO settings (key, value, description) "
      "VALUES (?, ?, ?)");
  stmt->bind(1, key).bind(2, value);

  if (description) {
    stmt->bind(3, *description);
  } else {
    stmt->bind(3, nullptr);
  }

  stmt->executeInsert();
}

void KeyValueRepository::set(const Entities::Setting& setting) {
  set(setting.key, setting.value, setting.description);
}

std::optional<Entities::Setting> KeyValueRepository::get(
    const std::string& key) {
  auto stmt = m_db.prepare(
      "SELECT key, value, description FROM settings WHERE key = ?");
  stmt->bind(1, key);
  auto result = stmt->execute();

  if (result.empty()) {
    return std::nullopt;
  }

  Entities::Setting setting;
  setting.key = std::get<std::string>(result[0][0]);
  setting.value = std::get<std::string>(result[0][1]);

  if (!std::holds_alternative<std::nullptr_t>(result[0][2])) {
    setting.description = std::get<std::string>(result[0][2]);
  }

  return setting;
}

std::optional<std::string> KeyValueRepository::getValue(
    const std::string& key) {
  auto stmt = m_db.prepare("SELECT value FROM settings WHERE key = ?");
  stmt->bind(1, key);
  auto result = stmt->execute();

  if (result.empty()) {
    return std::nullopt;
  }

  return std::get<std::string>(result[0][0]);
}

bool KeyValueRepository::exists(const std::string& key) {
  auto stmt = m_db.prepare("SELECT 1 FROM settings WHERE key = ?");
  stmt->bind(1, key);
  auto result = stmt->execute();
  return !result.empty();
}

void KeyValueRepository::remove(const std::string& key) {
  auto stmt = m_db.prepare("DELETE FROM settings WHERE key = ?");
  stmt->bind(1, key);
  stmt->executeUpdate();
}

// ============================================================
// Bulk Operations
// ============================================================

std::vector<Entities::Setting> KeyValueRepository::getAll() {
  auto result =
      m_db.query("SELECT key, value, description FROM settings ORDER BY key");

  std::vector<Entities::Setting> settings;
  settings.reserve(result.size());

  for (const auto& row : result) {
    Entities::Setting setting;
    setting.key = std::get<std::string>(row[0]);
    setting.value = std::get<std::string>(row[1]);

    if (!std::holds_alternative<std::nullptr_t>(row[2])) {
      setting.description = std::get<std::string>(row[2]);
    }

    settings.push_back(std::move(setting));
  }

  return settings;
}

std::vector<Entities::Setting> KeyValueRepository::getByPrefix(
    const std::string& prefix) {
  auto stmt = m_db.prepare(
      "SELECT key, value, description FROM settings "
      "WHERE key LIKE ? ORDER BY key");
  stmt->bind(1, prefix + "%");
  auto result = stmt->execute();

  std::vector<Entities::Setting> settings;
  settings.reserve(result.size());

  for (const auto& row : result) {
    Entities::Setting setting;
    setting.key = std::get<std::string>(row[0]);
    setting.value = std::get<std::string>(row[1]);

    if (!std::holds_alternative<std::nullptr_t>(row[2])) {
      setting.description = std::get<std::string>(row[2]);
    }

    settings.push_back(std::move(setting));
  }

  return settings;
}

std::vector<std::string> KeyValueRepository::getKeys() {
  auto result = m_db.query("SELECT key FROM settings ORDER BY key");

  std::vector<std::string> keys;
  keys.reserve(result.size());

  for (const auto& row : result) {
    keys.push_back(std::get<std::string>(row[0]));
  }

  return keys;
}

std::vector<std::string> KeyValueRepository::getKeysByPrefix(
    const std::string& prefix) {
  auto stmt =
      m_db.prepare("SELECT key FROM settings WHERE key LIKE ? ORDER BY key");
  stmt->bind(1, prefix + "%");
  auto result = stmt->execute();

  std::vector<std::string> keys;
  keys.reserve(result.size());

  for (const auto& row : result) {
    keys.push_back(std::get<std::string>(row[0]));
  }

  return keys;
}

void KeyValueRepository::removeByPrefix(const std::string& prefix) {
  auto stmt = m_db.prepare("DELETE FROM settings WHERE key LIKE ?");
  stmt->bind(1, prefix + "%");
  stmt->executeUpdate();
}

void KeyValueRepository::clear() { m_db.execute("DELETE FROM settings"); }

// ============================================================
// Count
// ============================================================

int64_t KeyValueRepository::count() {
  auto result = m_db.query("SELECT COUNT(*) FROM settings");
  return std::get<int64_t>(result[0][0]);
}

int64_t KeyValueRepository::countByPrefix(const std::string& prefix) {
  auto stmt = m_db.prepare("SELECT COUNT(*) FROM settings WHERE key LIKE ?");
  stmt->bind(1, prefix + "%");
  auto result = stmt->execute();
  return std::get<int64_t>(result[0][0]);
}

}  // namespace Gateways::Repositories::Sqlite3
