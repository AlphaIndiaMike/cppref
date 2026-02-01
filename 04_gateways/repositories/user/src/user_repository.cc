#include "user_repository.h"

#include <chrono>

namespace Infrastructure::Repositories {

SqlUserRepository::SqlUserRepository(
    std::shared_ptr<Database::IDatabase> database)
    : m_db(std::move(database)) {}

void SqlUserRepository::initializeSchema() {
  m_db->execute(R"(
        CREATE TABLE IF NOT EXISTS users (
            id TEXT PRIMARY KEY,
            email TEXT UNIQUE NOT NULL,
            name TEXT NOT NULL,
            created_at INTEGER NOT NULL
        )
    )");

  m_db->execute(R"(
        CREATE INDEX IF NOT EXISTS idx_users_email ON users(email)
    )");
}

std::optional<Entities::User> SqlUserRepository::findById(
    const std::string& id) {
  auto stmt = m_db->prepare(
      "SELECT id, email, name, created_at FROM users WHERE id = ?");
  stmt->bind(1, id);

  auto results = stmt->execute();

  if (results.empty()) {
    return std::nullopt;
  }

  return mapRowToEntity(results[0]);
}

std::optional<Entities::User> SqlUserRepository::findByEmail(
    const std::string& email) {
  auto stmt = m_db->prepare(
      "SELECT id, email, name, created_at FROM users WHERE email = ?");
  stmt->bind(1, email);

  auto results = stmt->execute();

  if (results.empty()) {
    return std::nullopt;
  }

  return mapRowToEntity(results[0]);
}

std::vector<Entities::User> SqlUserRepository::findAll() {
  auto results = m_db->query(
      "SELECT id, email, name, created_at FROM users ORDER BY created_at DESC");

  std::vector<Entities::User> users;
  users.reserve(results.size());

  for (const auto& row : results) {
    users.push_back(mapRowToEntity(row));
  }

  return users;
}

Entities::User SqlUserRepository::save(const Entities::User& user) {
  auto stmt = m_db->prepare(R"(
        INSERT INTO users (id, email, name, created_at)
        VALUES (?, ?, ?, ?)
        ON CONFLICT(id) DO UPDATE SET
            email = excluded.email,
            name = excluded.name
    )");

  auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                       user.createdAt.time_since_epoch())
                       .count();

  stmt->bind(1, user.id)
      .bind(2, user.email)
      .bind(3, user.name)
      .bind(4, static_cast<int64_t>(timestamp));

  stmt->executeUpdate();

  return user;
}

bool SqlUserRepository::remove(const std::string& id) {
  auto stmt = m_db->prepare("DELETE FROM users WHERE id = ?");
  stmt->bind(1, id);

  int affected = stmt->executeUpdate();
  return affected > 0;
}

Entities::User SqlUserRepository::mapRowToEntity(const Database::DbRow& row) {
  auto timestamp = std::get<int64_t>(row[3]);

  return Entities::User{
      .id = std::get<std::string>(row[0]),
      .email = std::get<std::string>(row[1]),
      .name = std::get<std::string>(row[2]),
      .createdAt = std::chrono::system_clock::from_time_t(timestamp)};
}

}  // namespace Infrastructure::Repositories
