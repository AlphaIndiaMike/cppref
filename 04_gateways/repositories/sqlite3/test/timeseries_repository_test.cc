#include "timeseries_repository.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <string>
#include <vector>

#include "sqlite3_database_connector.h"

namespace Gateways::Repositories::Sqlite3 {
namespace {

class TimeSeriesRepositoryTest : public ::testing::Test {
 protected:
  void SetUp() override {
    db_ = std::make_unique<Gateways::Database::SqliteDatabase>(":memory:");
    repo_ = std::make_unique<TimeSeriesRepository>(*db_);
    repo_->initSchema();
  }

  std::unique_ptr<Gateways::Database::SqliteDatabase> db_;
  std::unique_ptr<TimeSeriesRepository> repo_;
};

// ============================================================
// Schema
// ============================================================
TEST_F(TimeSeriesRepositoryTest, InitSchemaCreatesTablesIfNotExist) {
  // initSchema called in SetUp, calling again should not throw
  EXPECT_NO_THROW(repo_->initSchema());
}

// ============================================================
// Asset CRUD
// ============================================================
TEST_F(TimeSeriesRepositoryTest, CreateAndGetAsset) {
  Entities::Asset asset{"asset-1", "Temperature", "Room temperature",
                        "sensor-a"};
  repo_->createAsset(asset);

  auto retrieved = repo_->getAsset("asset-1");
  ASSERT_TRUE(retrieved.has_value());
  EXPECT_EQ(retrieved->id, "asset-1");
  EXPECT_EQ(retrieved->name, "Temperature");
  EXPECT_EQ(retrieved->description, "Room temperature");
  EXPECT_EQ(retrieved->source, "sensor-a");
}

TEST_F(TimeSeriesRepositoryTest, GetAssetNotFound) {
  auto retrieved = repo_->getAsset("nonexistent");
  EXPECT_FALSE(retrieved.has_value());
}

TEST_F(TimeSeriesRepositoryTest, GetAllAssets) {
  repo_->createAsset({"a1", "Asset A", "", ""});
  repo_->createAsset({"a2", "Asset B", "", ""});
  repo_->createAsset({"a3", "Asset C", "", ""});

  auto assets = repo_->getAllAssets();
  ASSERT_EQ(assets.size(), 3u);
  // Ordered by name
  EXPECT_EQ(assets[0].name, "Asset A");
  EXPECT_EQ(assets[1].name, "Asset B");
  EXPECT_EQ(assets[2].name, "Asset C");
}

TEST_F(TimeSeriesRepositoryTest, GetAllAssetsEmpty) {
  auto assets = repo_->getAllAssets();
  EXPECT_TRUE(assets.empty());
}

TEST_F(TimeSeriesRepositoryTest, UpdateAsset) {
  repo_->createAsset({"a1", "Original", "Desc", "Src"});

  Entities::Asset updated{"a1", "Updated", "New Desc", "New Src"};
  repo_->updateAsset(updated);

  auto retrieved = repo_->getAsset("a1");
  ASSERT_TRUE(retrieved.has_value());
  EXPECT_EQ(retrieved->name, "Updated");
  EXPECT_EQ(retrieved->description, "New Desc");
  EXPECT_EQ(retrieved->source, "New Src");
}

TEST_F(TimeSeriesRepositoryTest, DeleteAsset) {
  repo_->createAsset({"a1", "Asset", "", ""});
  ASSERT_TRUE(repo_->getAsset("a1").has_value());

  repo_->deleteAsset("a1");
  EXPECT_FALSE(repo_->getAsset("a1").has_value());
}

TEST_F(TimeSeriesRepositoryTest, DeleteAssetCascadesToTimeSeries) {
  repo_->createAsset({"a1", "Asset", "", ""});
  repo_->createUnit({"u1", "X", "Unit X"});
  repo_->addPoint({"a1", 1000, "u1", 42.0});

  auto points = repo_->getPoints("a1", 0, 2000);
  ASSERT_EQ(points.size(), 1u);

  repo_->deleteAsset("a1");

  points = repo_->getPoints("a1", 0, 2000);
  EXPECT_TRUE(points.empty());
}

// ============================================================
// Unit CRUD
// ============================================================
TEST_F(TimeSeriesRepositoryTest, CreateAndGetUnit) {
  Entities::Unit unit{"degC", "°C", "Degrees Celsius"};
  repo_->createUnit(unit);

  auto retrieved = repo_->getUnit("degC");
  ASSERT_TRUE(retrieved.has_value());
  EXPECT_EQ(retrieved->id, "degC");
  EXPECT_EQ(retrieved->symbol, "°C");
  EXPECT_EQ(retrieved->name, "Degrees Celsius");
}

TEST_F(TimeSeriesRepositoryTest, GetUnitNotFound) {
  auto retrieved = repo_->getUnit("nonexistent");
  EXPECT_FALSE(retrieved.has_value());
}

TEST_F(TimeSeriesRepositoryTest, GetAllUnits) {
  repo_->createUnit({"u1", "A", "Unit A"});
  repo_->createUnit({"u2", "B", "Unit B"});

  auto units = repo_->getAllUnits();
  ASSERT_EQ(units.size(), 2u);
}

TEST_F(TimeSeriesRepositoryTest, UpdateUnit) {
  repo_->createUnit({"u1", "X", "Original"});

  Entities::Unit updated{"u1", "Y", "Updated"};
  repo_->updateUnit(updated);

  auto retrieved = repo_->getUnit("u1");
  ASSERT_TRUE(retrieved.has_value());
  EXPECT_EQ(retrieved->symbol, "Y");
  EXPECT_EQ(retrieved->name, "Updated");
}

TEST_F(TimeSeriesRepositoryTest, DeleteUnit) {
  repo_->createUnit({"u1", "X", "Unit"});
  ASSERT_TRUE(repo_->getUnit("u1").has_value());

  repo_->deleteUnit("u1");
  EXPECT_FALSE(repo_->getUnit("u1").has_value());
}

// ============================================================
// Unit Conversion CRUD
// ============================================================
TEST_F(TimeSeriesRepositoryTest, CreateAndGetConversion) {
  repo_->createUnit({"EUR", "€", "Euro"});
  repo_->createUnit({"USD", "$", "US Dollar"});

  Entities::UnitConversion conv{"EUR", "USD", 1.08};
  repo_->createConversion(conv);

  auto retrieved = repo_->getConversion("EUR", "USD");
  ASSERT_TRUE(retrieved.has_value());
  EXPECT_EQ(retrieved->from_unit_id, "EUR");
  EXPECT_EQ(retrieved->to_unit_id, "USD");
  EXPECT_DOUBLE_EQ(retrieved->factor, 1.08);
}

TEST_F(TimeSeriesRepositoryTest, GetConversionNotFound) {
  auto retrieved = repo_->getConversion("X", "Y");
  EXPECT_FALSE(retrieved.has_value());
}

TEST_F(TimeSeriesRepositoryTest, GetConversionsFrom) {
  repo_->createUnit({"EUR", "€", "Euro"});
  repo_->createUnit({"USD", "$", "US Dollar"});
  repo_->createUnit({"GBP", "£", "British Pound"});

  repo_->createConversion({"EUR", "USD", 1.08});
  repo_->createConversion({"EUR", "GBP", 0.85});
  repo_->createConversion({"USD", "GBP", 0.79});

  auto conversions = repo_->getConversionsFrom("EUR");
  ASSERT_EQ(conversions.size(), 2u);
}

TEST_F(TimeSeriesRepositoryTest, GetAllConversions) {
  repo_->createUnit({"A", "a", "A"});
  repo_->createUnit({"B", "b", "B"});

  repo_->createConversion({"A", "B", 2.0});
  repo_->createConversion({"B", "A", 0.5});

  auto conversions = repo_->getAllConversions();
  ASSERT_EQ(conversions.size(), 2u);
}

TEST_F(TimeSeriesRepositoryTest, UpdateConversion) {
  repo_->createUnit({"EUR", "€", "Euro"});
  repo_->createUnit({"USD", "$", "US Dollar"});
  repo_->createConversion({"EUR", "USD", 1.08});

  Entities::UnitConversion updated{"EUR", "USD", 1.10};
  repo_->updateConversion(updated);

  auto retrieved = repo_->getConversion("EUR", "USD");
  ASSERT_TRUE(retrieved.has_value());
  EXPECT_DOUBLE_EQ(retrieved->factor, 1.10);
}

TEST_F(TimeSeriesRepositoryTest, DeleteConversion) {
  repo_->createUnit({"A", "a", "A"});
  repo_->createUnit({"B", "b", "B"});
  repo_->createConversion({"A", "B", 2.0});

  ASSERT_TRUE(repo_->getConversion("A", "B").has_value());

  repo_->deleteConversion("A", "B");
  EXPECT_FALSE(repo_->getConversion("A", "B").has_value());
}

// ============================================================
// Time Series Point CRUD
// ============================================================
TEST_F(TimeSeriesRepositoryTest, AddAndGetPoint) {
  repo_->createAsset({"a1", "Asset", "", ""});
  repo_->createUnit({"u1", "X", "Unit"});

  Entities::TimeSeriesPoint point{"a1", 1000, "u1", 42.5};
  repo_->addPoint(point);

  auto points = repo_->getPoints("a1", 0, 2000);
  ASSERT_EQ(points.size(), 1u);
  EXPECT_EQ(points[0].asset_id, "a1");
  EXPECT_EQ(points[0].timestamp_ms, 1000);
  EXPECT_EQ(points[0].unit_id, "u1");
  EXPECT_DOUBLE_EQ(points[0].value, 42.5);
}

TEST_F(TimeSeriesRepositoryTest, AddPointReplacesExisting) {
  repo_->createAsset({"a1", "Asset", "", ""});
  repo_->createUnit({"u1", "X", "Unit"});

  repo_->addPoint({"a1", 1000, "u1", 10.0});
  repo_->addPoint({"a1", 1000, "u1", 20.0});  // Same key, different value

  auto points = repo_->getPoints("a1", 0, 2000);
  ASSERT_EQ(points.size(), 1u);
  EXPECT_DOUBLE_EQ(points[0].value, 20.0);
}

TEST_F(TimeSeriesRepositoryTest, AddPointsMultiple) {
  repo_->createAsset({"a1", "Asset", "", ""});
  repo_->createUnit({"u1", "X", "Unit"});

  std::vector<Entities::TimeSeriesPoint> points = {
      {"a1", 1000, "u1", 1.0},
      {"a1", 2000, "u1", 2.0},
      {"a1", 3000, "u1", 3.0},
  };
  repo_->addPoints(points);

  auto retrieved = repo_->getPoints("a1", 0, 4000);
  ASSERT_EQ(retrieved.size(), 3u);
  EXPECT_DOUBLE_EQ(retrieved[0].value, 1.0);
  EXPECT_DOUBLE_EQ(retrieved[1].value, 2.0);
  EXPECT_DOUBLE_EQ(retrieved[2].value, 3.0);
}

TEST_F(TimeSeriesRepositoryTest, AddPointsEmpty) {
  // Should not throw
  EXPECT_NO_THROW(repo_->addPoints({}));
}

TEST_F(TimeSeriesRepositoryTest, GetPointsWithUnitFilter) {
  repo_->createAsset({"a1", "Asset", "", ""});
  repo_->createUnit({"u1", "X", "Unit 1"});
  repo_->createUnit({"u2", "Y", "Unit 2"});

  repo_->addPoint({"a1", 1000, "u1", 1.0});
  repo_->addPoint({"a1", 2000, "u2", 2.0});
  repo_->addPoint({"a1", 3000, "u1", 3.0});

  auto points = repo_->getPoints("a1", "u1", 0, 4000);
  ASSERT_EQ(points.size(), 2u);
  EXPECT_EQ(points[0].unit_id, "u1");
  EXPECT_EQ(points[1].unit_id, "u1");
}

TEST_F(TimeSeriesRepositoryTest, GetPointsTimeRange) {
  repo_->createAsset({"a1", "Asset", "", ""});
  repo_->createUnit({"u1", "X", "Unit"});

  repo_->addPoint({"a1", 1000, "u1", 1.0});
  repo_->addPoint({"a1", 2000, "u1", 2.0});
  repo_->addPoint({"a1", 3000, "u1", 3.0});
  repo_->addPoint({"a1", 4000, "u1", 4.0});

  auto points = repo_->getPoints("a1", 2000, 3000);
  ASSERT_EQ(points.size(), 2u);
  EXPECT_EQ(points[0].timestamp_ms, 2000);
  EXPECT_EQ(points[1].timestamp_ms, 3000);
}

TEST_F(TimeSeriesRepositoryTest, GetLatestPoint) {
  repo_->createAsset({"a1", "Asset", "", ""});
  repo_->createUnit({"u1", "X", "Unit"});

  repo_->addPoint({"a1", 1000, "u1", 1.0});
  repo_->addPoint({"a1", 3000, "u1", 3.0});
  repo_->addPoint({"a1", 2000, "u1", 2.0});

  auto latest = repo_->getLatestPoint("a1");
  ASSERT_TRUE(latest.has_value());
  EXPECT_EQ(latest->timestamp_ms, 3000);
  EXPECT_DOUBLE_EQ(latest->value, 3.0);
}

TEST_F(TimeSeriesRepositoryTest, GetLatestPointNotFound) {
  auto latest = repo_->getLatestPoint("nonexistent");
  EXPECT_FALSE(latest.has_value());
}

TEST_F(TimeSeriesRepositoryTest, GetLatestPointWithUnit) {
  repo_->createAsset({"a1", "Asset", "", ""});
  repo_->createUnit({"u1", "X", "Unit 1"});
  repo_->createUnit({"u2", "Y", "Unit 2"});

  repo_->addPoint({"a1", 1000, "u1", 1.0});
  repo_->addPoint({"a1", 2000, "u2", 2.0});
  repo_->addPoint({"a1", 3000, "u1", 3.0});

  auto latest = repo_->getLatestPoint("a1", "u2");
  ASSERT_TRUE(latest.has_value());
  EXPECT_EQ(latest->timestamp_ms, 2000);
  EXPECT_EQ(latest->unit_id, "u2");
}

TEST_F(TimeSeriesRepositoryTest, GetLatestPointWithUnitNotFound) {
  repo_->createAsset({"a1", "Asset", "", ""});
  repo_->createUnit({"u1", "X", "Unit"});

  auto latest = repo_->getLatestPoint("a1", "u1");
  EXPECT_FALSE(latest.has_value());
}

TEST_F(TimeSeriesRepositoryTest, DeletePointsRange) {
  repo_->createAsset({"a1", "Asset", "", ""});
  repo_->createUnit({"u1", "X", "Unit"});

  repo_->addPoint({"a1", 1000, "u1", 1.0});
  repo_->addPoint({"a1", 2000, "u1", 2.0});
  repo_->addPoint({"a1", 3000, "u1", 3.0});

  repo_->deletePoints("a1", 1500, 2500);

  auto points = repo_->getPoints("a1", 0, 4000);
  ASSERT_EQ(points.size(), 2u);
  EXPECT_EQ(points[0].timestamp_ms, 1000);
  EXPECT_EQ(points[1].timestamp_ms, 3000);
}

TEST_F(TimeSeriesRepositoryTest, DeleteAllPoints) {
  repo_->createAsset({"a1", "Asset", "", ""});
  repo_->createUnit({"u1", "X", "Unit"});

  repo_->addPoint({"a1", 1000, "u1", 1.0});
  repo_->addPoint({"a1", 2000, "u1", 2.0});

  repo_->deleteAllPoints("a1");

  auto points = repo_->getPoints("a1", 0, 3000);
  EXPECT_TRUE(points.empty());
}

// ============================================================
// Unit Conversion Utility
// ============================================================
TEST_F(TimeSeriesRepositoryTest, ConvertSameUnit) {
  auto result = repo_->convert(100.0, "EUR", "EUR");
  ASSERT_TRUE(result.has_value());
  EXPECT_DOUBLE_EQ(*result, 100.0);
}

TEST_F(TimeSeriesRepositoryTest, ConvertDirect) {
  repo_->createUnit({"EUR", "€", "Euro"});
  repo_->createUnit({"USD", "$", "US Dollar"});
  repo_->createConversion({"EUR", "USD", 1.10});

  auto result = repo_->convert(100.0, "EUR", "USD");
  ASSERT_TRUE(result.has_value());
  EXPECT_DOUBLE_EQ(*result, 110.0);
}

TEST_F(TimeSeriesRepositoryTest, ConvertReverse) {
  repo_->createUnit({"EUR", "€", "Euro"});
  repo_->createUnit({"USD", "$", "US Dollar"});
  repo_->createConversion({"EUR", "USD", 2.0});

  // No direct USD->EUR, but EUR->USD exists
  auto result = repo_->convert(100.0, "USD", "EUR");
  ASSERT_TRUE(result.has_value());
  EXPECT_DOUBLE_EQ(*result, 50.0);
}

TEST_F(TimeSeriesRepositoryTest, ConvertNotFound) {
  repo_->createUnit({"A", "a", "A"});
  repo_->createUnit({"B", "b", "B"});

  auto result = repo_->convert(100.0, "A", "B");
  EXPECT_FALSE(result.has_value());
}

TEST_F(TimeSeriesRepositoryTest, ConvertReverseZeroFactor) {
  repo_->createUnit({"A", "a", "A"});
  repo_->createUnit({"B", "b", "B"});
  repo_->createConversion({"B", "A", 0.0});  // Zero factor

  auto result = repo_->convert(100.0, "A", "B");
  EXPECT_FALSE(result.has_value());  // Can't divide by zero
}

// ============================================================
// Time Series - Synthetic Asset for Dynamic Conversions
// ============================================================
TEST_F(TimeSeriesRepositoryTest, SyntheticAssetForDynamicConversion) {
  // EUR/USD exchange rate as a time series
  repo_->createAsset({"EUR/USD", "EUR to USD Rate", "Exchange rate", "forex"});
  repo_->createUnit({"ratio", "×", "Ratio"});

  repo_->addPoint({"EUR/USD", 1704067200000, "ratio", 1.08});
  repo_->addPoint({"EUR/USD", 1704153600000, "ratio", 1.09});
  repo_->addPoint({"EUR/USD", 1704240000000, "ratio", 1.10});

  auto latest = repo_->getLatestPoint("EUR/USD");
  ASSERT_TRUE(latest.has_value());
  EXPECT_DOUBLE_EQ(latest->value, 1.10);
}

// ============================================================
// Bulk Time Series Performance
// ============================================================
TEST_F(TimeSeriesRepositoryTest, BulkInsertPerformance) {
  repo_->createAsset({"a1", "Asset", "", ""});
  repo_->createUnit({"u1", "X", "Unit"});

  std::vector<Entities::TimeSeriesPoint> points;
  points.reserve(10000);
  for (int64_t i = 0; i < 10000; ++i) {
    points.push_back({"a1", i * 1000, "u1", static_cast<double>(i)});
  }

  EXPECT_NO_THROW(repo_->addPoints(points));

  auto retrieved = repo_->getPoints("a1", 0, 10000000);
  EXPECT_EQ(retrieved.size(), 10000u);
}

}  // namespace
}  // namespace Gateways::Repositories::Sqlite3
