#ifndef REPOSITORIES_ACCOUNT_REPOSITORY_H_
#define REPOSITORIES_ACCOUNT_REPOSITORY_H_

#include <optional>
#include <string>
#include <vector>

#include "database_connector.h"
#include "entities.h"

namespace Gateways::Repositories::Sqlite3 {

class AccountRepository {
 public:
  explicit AccountRepository(Gateways::Database::IDatabase& db);

  // Schema management
  void initSchema();

  // Account CRUD
  void createAccount(const Entities::Account& account);
  std::optional<Entities::Account> getAccount(const std::string& id);
  std::optional<Entities::Account> getAccountByName(const std::string& name);
  std::vector<Entities::Account> getAllAccounts();
  void updateAccount(const Entities::Account& account);
  void deleteAccount(const std::string& id);

  bool accountExists(const std::string& id);
  bool accountExistsByName(const std::string& name);

  // Account Property CRUD
  void setProperty(
      const std::string& account_id, const std::string& key,
      const std::string& value,
      const std::optional<std::string>& description = std::nullopt);
  void setProperty(const Entities::AccountProperty& property);

  std::optional<Entities::AccountProperty> getProperty(
      const std::string& account_id, const std::string& key);
  std::optional<std::string> getPropertyValue(const std::string& account_id,
                                              const std::string& key);

  std::vector<Entities::AccountProperty> getProperties(
      const std::string& account_id);
  std::vector<Entities::AccountProperty> getPropertiesByPrefix(
      const std::string& account_id, const std::string& prefix);

  bool propertyExists(const std::string& account_id, const std::string& key);
  void removeProperty(const std::string& account_id, const std::string& key);
  void removePropertiesByPrefix(const std::string& account_id,
                                const std::string& prefix);
  void clearProperties(const std::string& account_id);

  // Count
  int64_t countAccounts();
  int64_t countProperties(const std::string& account_id);

 private:
  Gateways::Database::IDatabase& m_db;
};

}  // namespace Gateways::Repositories::Sqlite3

#endif  // REPOSITORIES_ACCOUNT_REPOSITORY_H_
