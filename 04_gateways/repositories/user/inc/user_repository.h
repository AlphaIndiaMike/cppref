#pragma once

#include <memory>

#include "database.h"
#include "user_entity_interface.h"

namespace Infrastructure::Repositories {

class SqlUserRepository : public UseCases::IUserRepository {
 public:
  explicit SqlUserRepository(std::shared_ptr<Database::IDatabase> database);

  void initializeSchema();

  std::optional<Entities::User> findById(const std::string& id) override;
  std::optional<Entities::User> findByEmail(const std::string& email) override;
  std::vector<Entities::User> findAll() override;
  Entities::User save(const Entities::User& user) override;
  bool remove(const std::string& id) override;

 private:
  std::shared_ptr<Database::IDatabase> m_db;

  Entities::User mapRowToEntity(const Database::DbRow& row);
};

}  // namespace Infrastructure::Repositories
