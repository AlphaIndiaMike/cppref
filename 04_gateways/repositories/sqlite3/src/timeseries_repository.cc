#include "timeseries_repository.h"

namespace Gateways::Repositories::Sqlite3 {

using namespace Gateways::Database;

TimeSeriesRepository::TimeSeriesRepository(IDatabase& db) : m_db(db) {}

void TimeSeriesRepository::initSchema() {
  m_db.execute(R"(
    CREATE TABLE IF NOT EXISTS assets (
      id TEXT PRIMARY KEY,
      name TEXT NOT NULL,
      description TEXT NOT NULL DEFAULT '',
      source TEXT NOT NULL DEFAULT ''
    )
  )");

  m_db.execute(R"(
    CREATE TABLE IF NOT EXISTS units (
      id TEXT PRIMARY KEY,
      symbol TEXT NOT NULL,
      name TEXT NOT NULL
    )
  )");

  m_db.execute(R"(
    CREATE TABLE IF NOT EXISTS unit_conversions (
      from_unit_id TEXT NOT NULL,
      to_unit_id TEXT NOT NULL,
      factor REAL NOT NULL,
      PRIMARY KEY (from_unit_id, to_unit_id),
      FOREIGN KEY (from_unit_id) REFERENCES units(id) ON DELETE CASCADE,
      FOREIGN KEY (to_unit_id) REFERENCES units(id) ON DELETE CASCADE
    )
  )");

  m_db.execute(R"(
    CREATE TABLE IF NOT EXISTS timeseries (
      asset_id TEXT NOT NULL,
      timestamp_ms INTEGER NOT NULL,
      unit_id TEXT NOT NULL,
      value REAL NOT NULL,
      PRIMARY KEY (asset_id, timestamp_ms, unit_id),
      FOREIGN KEY (asset_id) REFERENCES assets(id) ON DELETE CASCADE,
      FOREIGN KEY (unit_id) REFERENCES units(id) ON DELETE CASCADE
    )
  )");

  m_db.execute(
      "CREATE INDEX IF NOT EXISTS idx_timeseries_asset_time "
      "ON timeseries(asset_id, timestamp_ms)");
}

// ============================================================
// Asset CRUD
// ============================================================

void TimeSeriesRepository::createAsset(const Entities::Asset& asset) {
  auto stmt = m_db.prepare(
      "INSERT INTO assets (id, name, description, source) VALUES (?, ?, ?, ?)");
  stmt->bind(1, asset.id)
      .bind(2, asset.name)
      .bind(3, asset.description)
      .bind(4, asset.source);
  stmt->executeInsert();
}

std::optional<Entities::Asset> TimeSeriesRepository::getAsset(
    const std::string& id) {
  auto stmt = m_db.prepare(
      "SELECT id, name, description, source FROM assets WHERE id = ?");
  stmt->bind(1, id);
  auto result = stmt->execute();

  if (result.empty()) {
    return std::nullopt;
  }

  return Entities::Asset{
      std::get<std::string>(result[0][0]),
      std::get<std::string>(result[0][1]),
      std::get<std::string>(result[0][2]),
      std::get<std::string>(result[0][3]),
  };
}

std::vector<Entities::Asset> TimeSeriesRepository::getAllAssets() {
  auto result = m_db.query(
      "SELECT id, name, description, source FROM assets ORDER BY name");

  std::vector<Entities::Asset> assets;
  assets.reserve(result.size());

  for (const auto& row : result) {
    assets.push_back({
        std::get<std::string>(row[0]),
        std::get<std::string>(row[1]),
        std::get<std::string>(row[2]),
        std::get<std::string>(row[3]),
    });
  }

  return assets;
}

void TimeSeriesRepository::updateAsset(const Entities::Asset& asset) {
  auto stmt = m_db.prepare(
      "UPDATE assets SET name = ?, description = ?, source = ? WHERE id = ?");
  stmt->bind(1, asset.name)
      .bind(2, asset.description)
      .bind(3, asset.source)
      .bind(4, asset.id);
  stmt->executeUpdate();
}

void TimeSeriesRepository::deleteAsset(const std::string& id) {
  auto stmt = m_db.prepare("DELETE FROM assets WHERE id = ?");
  stmt->bind(1, id);
  stmt->executeUpdate();
}

// ============================================================
// Unit CRUD
// ============================================================

void TimeSeriesRepository::createUnit(const Entities::Unit& unit) {
  auto stmt =
      m_db.prepare("INSERT INTO units (id, symbol, name) VALUES (?, ?, ?)");
  stmt->bind(1, unit.id).bind(2, unit.symbol).bind(3, unit.name);
  stmt->executeInsert();
}

std::optional<Entities::Unit> TimeSeriesRepository::getUnit(
    const std::string& id) {
  auto stmt = m_db.prepare("SELECT id, symbol, name FROM units WHERE id = ?");
  stmt->bind(1, id);
  auto result = stmt->execute();

  if (result.empty()) {
    return std::nullopt;
  }

  return Entities::Unit{
      std::get<std::string>(result[0][0]),
      std::get<std::string>(result[0][1]),
      std::get<std::string>(result[0][2]),
  };
}

std::vector<Entities::Unit> TimeSeriesRepository::getAllUnits() {
  auto result = m_db.query("SELECT id, symbol, name FROM units ORDER BY name");

  std::vector<Entities::Unit> units;
  units.reserve(result.size());

  for (const auto& row : result) {
    units.push_back({
        std::get<std::string>(row[0]),
        std::get<std::string>(row[1]),
        std::get<std::string>(row[2]),
    });
  }

  return units;
}

void TimeSeriesRepository::updateUnit(const Entities::Unit& unit) {
  auto stmt =
      m_db.prepare("UPDATE units SET symbol = ?, name = ? WHERE id = ?");
  stmt->bind(1, unit.symbol).bind(2, unit.name).bind(3, unit.id);
  stmt->executeUpdate();
}

void TimeSeriesRepository::deleteUnit(const std::string& id) {
  auto stmt = m_db.prepare("DELETE FROM units WHERE id = ?");
  stmt->bind(1, id);
  stmt->executeUpdate();
}

// ============================================================
// Unit Conversion CRUD
// ============================================================

void TimeSeriesRepository::createConversion(
    const Entities::UnitConversion& conversion) {
  auto stmt = m_db.prepare(
      "INSERT INTO unit_conversions (from_unit_id, to_unit_id, factor) "
      "VALUES (?, ?, ?)");
  stmt->bind(1, conversion.from_unit_id)
      .bind(2, conversion.to_unit_id)
      .bind(3, conversion.factor);
  stmt->executeInsert();
}

std::optional<Entities::UnitConversion> TimeSeriesRepository::getConversion(
    const std::string& from_unit_id, const std::string& to_unit_id) {
  auto stmt = m_db.prepare(
      "SELECT from_unit_id, to_unit_id, factor FROM unit_conversions "
      "WHERE from_unit_id = ? AND to_unit_id = ?");
  stmt->bind(1, from_unit_id).bind(2, to_unit_id);
  auto result = stmt->execute();

  if (result.empty()) {
    return std::nullopt;
  }

  return Entities::UnitConversion{
      std::get<std::string>(result[0][0]),
      std::get<std::string>(result[0][1]),
      std::get<double>(result[0][2]),
  };
}

std::vector<Entities::UnitConversion> TimeSeriesRepository::getConversionsFrom(
    const std::string& from_unit_id) {
  auto stmt = m_db.prepare(
      "SELECT from_unit_id, to_unit_id, factor FROM unit_conversions "
      "WHERE from_unit_id = ?");
  stmt->bind(1, from_unit_id);
  auto result = stmt->execute();

  std::vector<Entities::UnitConversion> conversions;
  conversions.reserve(result.size());

  for (const auto& row : result) {
    conversions.push_back({
        std::get<std::string>(row[0]),
        std::get<std::string>(row[1]),
        std::get<double>(row[2]),
    });
  }

  return conversions;
}

std::vector<Entities::UnitConversion>
TimeSeriesRepository::getAllConversions() {
  auto result = m_db.query(
      "SELECT from_unit_id, to_unit_id, factor FROM unit_conversions");

  std::vector<Entities::UnitConversion> conversions;
  conversions.reserve(result.size());

  for (const auto& row : result) {
    conversions.push_back({
        std::get<std::string>(row[0]),
        std::get<std::string>(row[1]),
        std::get<double>(row[2]),
    });
  }

  return conversions;
}

void TimeSeriesRepository::updateConversion(
    const Entities::UnitConversion& conversion) {
  auto stmt = m_db.prepare(
      "UPDATE unit_conversions SET factor = ? "
      "WHERE from_unit_id = ? AND to_unit_id = ?");
  stmt->bind(1, conversion.factor)
      .bind(2, conversion.from_unit_id)
      .bind(3, conversion.to_unit_id);
  stmt->executeUpdate();
}

void TimeSeriesRepository::deleteConversion(const std::string& from_unit_id,
                                            const std::string& to_unit_id) {
  auto stmt = m_db.prepare(
      "DELETE FROM unit_conversions WHERE from_unit_id = ? AND to_unit_id = ?");
  stmt->bind(1, from_unit_id).bind(2, to_unit_id);
  stmt->executeUpdate();
}

// ============================================================
// Time Series Point CRUD
// ============================================================

void TimeSeriesRepository::addPoint(const Entities::TimeSeriesPoint& point) {
  auto stmt = m_db.prepare(
      "INSERT OR REPLACE INTO timeseries "
      "(asset_id, timestamp_ms, unit_id, value) VALUES (?, ?, ?, ?)");
  stmt->bind(1, point.asset_id)
      .bind(2, point.timestamp_ms)
      .bind(3, point.unit_id)
      .bind(4, point.value);
  stmt->executeInsert();
}

void TimeSeriesRepository::addPoints(
    const std::vector<Entities::TimeSeriesPoint>& points) {
  if (points.empty()) {
    return;
  }

  auto stmt = m_db.prepare(
      "INSERT OR REPLACE INTO timeseries "
      "(asset_id, timestamp_ms, unit_id, value) VALUES (?, ?, ?, ?)");

  m_db.beginTransaction();
  try {
    for (const auto& point : points) {
      stmt->reset();
      stmt->bind(1, point.asset_id)
          .bind(2, point.timestamp_ms)
          .bind(3, point.unit_id)
          .bind(4, point.value);
      stmt->executeInsert();
    }
    m_db.commit();
  } catch (...) {
    m_db.rollback();
    throw;
  }
}

std::vector<Entities::TimeSeriesPoint> TimeSeriesRepository::getPoints(
    const std::string& asset_id, int64_t from_ms, int64_t to_ms) {
  auto stmt = m_db.prepare(
      "SELECT asset_id, timestamp_ms, unit_id, value FROM timeseries "
      "WHERE asset_id = ? AND timestamp_ms >= ? AND timestamp_ms <= ? "
      "ORDER BY timestamp_ms");
  stmt->bind(1, asset_id).bind(2, from_ms).bind(3, to_ms);
  auto result = stmt->execute();

  std::vector<Entities::TimeSeriesPoint> points;
  points.reserve(result.size());

  for (const auto& row : result) {
    points.push_back({
        std::get<std::string>(row[0]),
        std::get<int64_t>(row[1]),
        std::get<std::string>(row[2]),
        std::get<double>(row[3]),
    });
  }

  return points;
}

std::vector<Entities::TimeSeriesPoint> TimeSeriesRepository::getPoints(
    const std::string& asset_id, const std::string& unit_id, int64_t from_ms,
    int64_t to_ms) {
  auto stmt = m_db.prepare(
      "SELECT asset_id, timestamp_ms, unit_id, value FROM timeseries "
      "WHERE asset_id = ? AND unit_id = ? "
      "AND timestamp_ms >= ? AND timestamp_ms <= ? "
      "ORDER BY timestamp_ms");
  stmt->bind(1, asset_id).bind(2, unit_id).bind(3, from_ms).bind(4, to_ms);
  auto result = stmt->execute();

  std::vector<Entities::TimeSeriesPoint> points;
  points.reserve(result.size());

  for (const auto& row : result) {
    points.push_back({
        std::get<std::string>(row[0]),
        std::get<int64_t>(row[1]),
        std::get<std::string>(row[2]),
        std::get<double>(row[3]),
    });
  }

  return points;
}

std::optional<Entities::TimeSeriesPoint> TimeSeriesRepository::getLatestPoint(
    const std::string& asset_id) {
  auto stmt = m_db.prepare(
      "SELECT asset_id, timestamp_ms, unit_id, value FROM timeseries "
      "WHERE asset_id = ? ORDER BY timestamp_ms DESC LIMIT 1");
  stmt->bind(1, asset_id);
  auto result = stmt->execute();

  if (result.empty()) {
    return std::nullopt;
  }

  return Entities::TimeSeriesPoint{
      std::get<std::string>(result[0][0]),
      std::get<int64_t>(result[0][1]),
      std::get<std::string>(result[0][2]),
      std::get<double>(result[0][3]),
  };
}

std::optional<Entities::TimeSeriesPoint> TimeSeriesRepository::getLatestPoint(
    const std::string& asset_id, const std::string& unit_id) {
  auto stmt = m_db.prepare(
      "SELECT asset_id, timestamp_ms, unit_id, value FROM timeseries "
      "WHERE asset_id = ? AND unit_id = ? "
      "ORDER BY timestamp_ms DESC LIMIT 1");
  stmt->bind(1, asset_id).bind(2, unit_id);
  auto result = stmt->execute();

  if (result.empty()) {
    return std::nullopt;
  }

  return Entities::TimeSeriesPoint{
      std::get<std::string>(result[0][0]),
      std::get<int64_t>(result[0][1]),
      std::get<std::string>(result[0][2]),
      std::get<double>(result[0][3]),
  };
}

void TimeSeriesRepository::deletePoints(const std::string& asset_id,
                                        int64_t from_ms, int64_t to_ms) {
  auto stmt = m_db.prepare(
      "DELETE FROM timeseries "
      "WHERE asset_id = ? AND timestamp_ms >= ? AND timestamp_ms <= ?");
  stmt->bind(1, asset_id).bind(2, from_ms).bind(3, to_ms);
  stmt->executeUpdate();
}

void TimeSeriesRepository::deleteAllPoints(const std::string& asset_id) {
  auto stmt = m_db.prepare("DELETE FROM timeseries WHERE asset_id = ?");
  stmt->bind(1, asset_id);
  stmt->executeUpdate();
}

// ============================================================
// Utility
// ============================================================

std::optional<double> TimeSeriesRepository::convert(
    double value, const std::string& from_unit_id,
    const std::string& to_unit_id) {
  if (from_unit_id == to_unit_id) {
    return value;
  }

  auto conversion = getConversion(from_unit_id, to_unit_id);
  if (conversion) {
    return value * conversion->factor;
  }

  // Try reverse conversion
  auto reverse = getConversion(to_unit_id, from_unit_id);
  if (reverse && reverse->factor != 0.0) {
    return value / reverse->factor;
  }

  return std::nullopt;
}

}  // namespace Gateways::Repositories::Sqlite3
