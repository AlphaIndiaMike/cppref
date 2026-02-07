#ifndef USE_CASES_I_ACCOUNT_REPOSITORY_H_
#define USE_CASES_I_ACCOUNT_REPOSITORY_H_

#include <optional>
#include <string>
#include <vector>

#include "entities.h"

namespace UseCases {

// Repository interface defined by use cases.
// Gateways implement this interface.
class IAccountRepository {
 public:
  virtual ~IAccountRepository() = default;

  // Account operations
  virtual void createAccount(const Entities::Account& account) = 0;
  virtual std::optional<Entities::Account> getAccount(
      const std::string& id) = 0;
  virtual std::optional<Entities::Account> getAccountByName(
      const std::string& name) = 0;
  virtual std::vector<Entities::Account> getAllAccounts() = 0;
  virtual void updateAccount(const Entities::Account& account) = 0;
  virtual void deleteAccount(const std::string& id) = 0;

  virtual bool accountExists(const std::string& id) = 0;
  virtual bool accountExistsByName(const std::string& name) = 0;

  // Account property operations
  virtual void setProperty(
      const std::string& account_id, const std::string& key,
      const std::string& value,
      const std::optional<std::string>& description = std::nullopt) = 0;
  virtual void setProperty(const Entities::AccountProperty& property) = 0;

  virtual std::optional<Entities::AccountProperty> getProperty(
      const std::string& account_id, const std::string& key) = 0;
  virtual std::optional<std::string> getPropertyValue(
      const std::string& account_id, const std::string& key) = 0;

  virtual std::vector<Entities::AccountProperty> getProperties(
      const std::string& account_id) = 0;
  virtual std::vector<Entities::AccountProperty> getPropertiesByPrefix(
      const std::string& account_id, const std::string& prefix) = 0;

  virtual bool propertyExists(const std::string& account_id,
                              const std::string& key) = 0;
  virtual void removeProperty(const std::string& account_id,
                              const std::string& key) = 0;
  virtual void removePropertiesByPrefix(const std::string& account_id,
                                        const std::string& prefix) = 0;
  virtual void clearProperties(const std::string& account_id) = 0;

  // Count
  virtual int64_t countAccounts() = 0;
  virtual int64_t countProperties(const std::string& account_id) = 0;
};

}  // namespace UseCases

#endif  // USE_CASES_I_ACCOUNT_REPOSITORY_H_
