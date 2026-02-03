#ifndef REPOSITORIES_TIMESERIES_REPOSITORY_H_
#define REPOSITORIES_TIMESERIES_REPOSITORY_H_

#include <optional>
#include <string>
#include <vector>

#include "database_connector.h"
#include "entities.h"
namespace Gateways::Repositories::Sqlite3 {

using namespace Gateways::Database;

class TimeSeriesRepository {
 public:
  explicit TimeSeriesRepository(IDatabase& db);

  // Schema management
  void initSchema();

  // Asset CRUD
  void createAsset(const Entities::Asset& asset);
  std::optional<Entities::Asset> getAsset(const std::string& id);
  std::vector<Entities::Asset> getAllAssets();
  void updateAsset(const Entities::Asset& asset);
  void deleteAsset(const std::string& id);

  // Unit CRUD
  void createUnit(const Entities::Unit& unit);
  std::optional<Entities::Unit> getUnit(const std::string& id);
  std::vector<Entities::Unit> getAllUnits();
  void updateUnit(const Entities::Unit& unit);
  void deleteUnit(const std::string& id);

  // Unit Conversion CRUD
  void createConversion(const Entities::UnitConversion& conversion);
  std::optional<Entities::UnitConversion> getConversion(
      const std::string& from_unit_id, const std::string& to_unit_id);
  std::vector<Entities::UnitConversion> getConversionsFrom(
      const std::string& from_unit_id);
  std::vector<Entities::UnitConversion> getAllConversions();
  void updateConversion(const Entities::UnitConversion& conversion);
  void deleteConversion(const std::string& from_unit_id,
                        const std::string& to_unit_id);

  // Time Series Point CRUD
  void addPoint(const Entities::TimeSeriesPoint& point);
  void addPoints(const std::vector<Entities::TimeSeriesPoint>& points);

  std::vector<Entities::TimeSeriesPoint> getPoints(const std::string& asset_id,
                                                   int64_t from_ms,
                                                   int64_t to_ms);
  std::vector<Entities::TimeSeriesPoint> getPoints(const std::string& asset_id,
                                                   const std::string& unit_id,
                                                   int64_t from_ms,
                                                   int64_t to_ms);

  std::optional<Entities::TimeSeriesPoint> getLatestPoint(
      const std::string& asset_id);
  std::optional<Entities::TimeSeriesPoint> getLatestPoint(
      const std::string& asset_id, const std::string& unit_id);

  void deletePoints(const std::string& asset_id, int64_t from_ms,
                    int64_t to_ms);
  void deleteAllPoints(const std::string& asset_id);

  // Utility
  std::optional<double> convert(double value, const std::string& from_unit_id,
                                const std::string& to_unit_id);

 private:
  Gateways::Database::IDatabase& m_db;
};

}  // namespace Gateways::Repositories::Sqlite3

#endif  // REPOSITORIES_TIMESERIES_REPOSITORY_H_
