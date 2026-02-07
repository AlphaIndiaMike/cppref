#ifndef REPOSITORIES_ACCOUNT_REPOSITORY_H_
#define REPOSITORIES_ACCOUNT_REPOSITORY_H_

#include <optional>
#include <string>
#include <vector>

#include "database_connector.h"
#include "entities.h"
#ifndef UNIT_TEST
#include "i_account_repository"
#define OVERRIDE override
#define USE_CASE : public UseCases::IAccountRepository
#else
#define OVERRIDE
#define USE_CASE
#endif

namespace Gateways::Repositories::Sqlite3 {

class AccountRepository USE_CASE {
 public:
  explicit AccountRepository(Gateways::Database::IDatabase& db);

  // Schema management
  void initSchema();

  // Account CRUD
  void createAccount(const Entities::Account& account) OVERRIDE;
  std::optional<Entities::Account> getAccount(const std::string& id) OVERRIDE;
  std::optional<Entities::Account> getAccountByName(
      const std::string& name) OVERRIDE;
  std::vector<Entities::Account> getAllAccounts() OVERRIDE;
  void updateAccount(const Entities::Account& account) OVERRIDE;
  void deleteAccount(const std::string& id) OVERRIDE;

  bool accountExists(const std::string& id) OVERRIDE;
  bool accountExistsByName(const std::string& name) OVERRIDE;

  // Account Property CRUD
  void setProperty(
      const std::string& account_id, const std::string& key,
      const std::string& value,
      const std::optional<std::string>& description = std::nullopt) OVERRIDE;
  void setProperty(const Entities::AccountProperty& property) OVERRIDE;

  std::optional<Entities::AccountProperty> getProperty(
      const std::string& account_id, const std::string& key) OVERRIDE;
  std::optional<std::string> getPropertyValue(const std::string& account_id,
                                              const std::string& key) OVERRIDE;

  std::vector<Entities::AccountProperty> getProperties(
      const std::string& account_id) OVERRIDE;
  std::vector<Entities::AccountProperty> getPropertiesByPrefix(
      const std::string& account_id, const std::string& prefix) OVERRIDE;

  bool propertyExists(const std::string& account_id,
                      const std::string& key) OVERRIDE;
  void removeProperty(const std::string& account_id,
                      const std::string& key) OVERRIDE;
  void removePropertiesByPrefix(const std::string& account_id,
                                const std::string& prefix) OVERRIDE;
  void clearProperties(const std::string& account_id) OVERRIDE;

  // Count
  int64_t countAccounts() OVERRIDE;
  int64_t countProperties(const std::string& account_id) OVERRIDE;

 private:
  Gateways::Database::IDatabase& m_db;
};

}  // namespace Gateways::Repositories::Sqlite3

#endif  // REPOSITORIES_ACCOUNT_REPOSITORY_H_
