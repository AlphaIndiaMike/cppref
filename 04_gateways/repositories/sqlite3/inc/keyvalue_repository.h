#ifndef REPOSITORIES_KEYVALUE_REPOSITORY_H_
#define REPOSITORIES_KEYVALUE_REPOSITORY_H_

#include <optional>
#include <string>
#include <vector>

#include "database_connector.h"
#include "entities.h"

namespace Gateways::Repositories::Sqlite3 {

class KeyValueRepository {
 public:
  explicit KeyValueRepository(Gateways::Database::IDatabase& db);

  // Schema management
  void initSchema();

  // CRUD operations
  void set(const std::string& key, const std::string& value,
           const std::optional<std::string>& description = std::nullopt);
  void set(const Entities::Setting& setting);

  std::optional<Entities::Setting> get(const std::string& key);
  std::optional<std::string> getValue(const std::string& key);

  bool exists(const std::string& key);
  void remove(const std::string& key);

  // Bulk operations
  std::vector<Entities::Setting> getAll();
  std::vector<Entities::Setting> getByPrefix(const std::string& prefix);
  std::vector<std::string> getKeys();
  std::vector<std::string> getKeysByPrefix(const std::string& prefix);

  void removeByPrefix(const std::string& prefix);
  void clear();

  // Count
  int64_t count();
  int64_t countByPrefix(const std::string& prefix);

 private:
  Gateways::Database::IDatabase& m_db;
};

}  // namespace Gateways::Repositories::Sqlite3

#endif  // REPOSITORIES_KEYVALUE_REPOSITORY_H_
