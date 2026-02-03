#include "keyvalue_repository.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "sqlite3_database_connector.h"

namespace Gateways::Repositories::Sqlite3 {
namespace {

class KeyValueRepositoryTest : public ::testing::Test {
 protected:
  void SetUp() override {
    db_ = std::make_unique<Gateways::Database::SqliteDatabase>(":memory:");
    repo_ = std::make_unique<KeyValueRepository>(*db_);
    repo_->initSchema();
  }

  std::unique_ptr<Gateways::Database::SqliteDatabase> db_;
  std::unique_ptr<KeyValueRepository> repo_;
};

// ============================================================
// Schema
// ============================================================
TEST_F(KeyValueRepositoryTest, InitSchemaCreatesTablesIfNotExist) {
  EXPECT_NO_THROW(repo_->initSchema());
}

// ============================================================
// Basic CRUD
// ============================================================
TEST_F(KeyValueRepositoryTest, SetAndGet) {
  repo_->set("key1", "value1");

  auto setting = repo_->get("key1");
  ASSERT_TRUE(setting.has_value());
  EXPECT_EQ(setting->key, "key1");
  EXPECT_EQ(setting->value, "value1");
  EXPECT_FALSE(setting->description.has_value());
}

TEST_F(KeyValueRepositoryTest, SetAndGetWithDescription) {
  repo_->set("key1", "value1", "A description");

  auto setting = repo_->get("key1");
  ASSERT_TRUE(setting.has_value());
  EXPECT_EQ(setting->value, "value1");
  ASSERT_TRUE(setting->description.has_value());
  EXPECT_EQ(*setting->description, "A description");
}

TEST_F(KeyValueRepositoryTest, SetUsingSetting) {
  Entities::Setting setting{"key1", "value1", "desc"};
  repo_->set(setting);

  auto retrieved = repo_->get("key1");
  ASSERT_TRUE(retrieved.has_value());
  EXPECT_EQ(retrieved->value, "value1");
  EXPECT_EQ(retrieved->description, "desc");
}

TEST_F(KeyValueRepositoryTest, SetUpdatesExisting) {
  repo_->set("key1", "value1");
  repo_->set("key1", "value2");

  auto setting = repo_->get("key1");
  ASSERT_TRUE(setting.has_value());
  EXPECT_EQ(setting->value, "value2");
}

TEST_F(KeyValueRepositoryTest, SetUpdatesDescription) {
  repo_->set("key1", "value1", "desc1");
  repo_->set("key1", "value1", "desc2");

  auto setting = repo_->get("key1");
  ASSERT_TRUE(setting.has_value());
  EXPECT_EQ(setting->description, "desc2");
}

TEST_F(KeyValueRepositoryTest, SetClearsDescription) {
  repo_->set("key1", "value1", "desc");
  repo_->set("key1", "value1", std::nullopt);

  auto setting = repo_->get("key1");
  ASSERT_TRUE(setting.has_value());
  EXPECT_FALSE(setting->description.has_value());
}

TEST_F(KeyValueRepositoryTest, GetNotFound) {
  auto setting = repo_->get("nonexistent");
  EXPECT_FALSE(setting.has_value());
}

TEST_F(KeyValueRepositoryTest, GetValue) {
  repo_->set("key1", "value1");

  auto value = repo_->getValue("key1");
  ASSERT_TRUE(value.has_value());
  EXPECT_EQ(*value, "value1");
}

TEST_F(KeyValueRepositoryTest, GetValueNotFound) {
  auto value = repo_->getValue("nonexistent");
  EXPECT_FALSE(value.has_value());
}

TEST_F(KeyValueRepositoryTest, Exists) {
  repo_->set("key1", "value1");

  EXPECT_TRUE(repo_->exists("key1"));
  EXPECT_FALSE(repo_->exists("nonexistent"));
}

TEST_F(KeyValueRepositoryTest, Remove) {
  repo_->set("key1", "value1");
  ASSERT_TRUE(repo_->exists("key1"));

  repo_->remove("key1");
  EXPECT_FALSE(repo_->exists("key1"));
}

TEST_F(KeyValueRepositoryTest, RemoveNonexistent) {
  // Should not throw
  EXPECT_NO_THROW(repo_->remove("nonexistent"));
}

// ============================================================
// Bulk Operations
// ============================================================
TEST_F(KeyValueRepositoryTest, GetAll) {
  repo_->set("key1", "value1");
  repo_->set("key2", "value2");
  repo_->set("key3", "value3");

  auto all = repo_->getAll();
  ASSERT_EQ(all.size(), 3u);
  // Ordered by key
  EXPECT_EQ(all[0].key, "key1");
  EXPECT_EQ(all[1].key, "key2");
  EXPECT_EQ(all[2].key, "key3");
}

TEST_F(KeyValueRepositoryTest, GetAllEmpty) {
  auto all = repo_->getAll();
  EXPECT_TRUE(all.empty());
}

TEST_F(KeyValueRepositoryTest, GetByPrefix) {
  repo_->set("app.theme", "dark");
  repo_->set("app.language", "en");
  repo_->set("user.name", "John");
  repo_->set("user.email", "john@example.com");

  auto appSettings = repo_->getByPrefix("app.");
  ASSERT_EQ(appSettings.size(), 2u);
  EXPECT_EQ(appSettings[0].key, "app.language");
  EXPECT_EQ(appSettings[1].key, "app.theme");
}

TEST_F(KeyValueRepositoryTest, GetByPrefixNoMatch) {
  repo_->set("key1", "value1");

  auto result = repo_->getByPrefix("other.");
  EXPECT_TRUE(result.empty());
}

TEST_F(KeyValueRepositoryTest, GetKeys) {
  repo_->set("key1", "value1");
  repo_->set("key2", "value2");

  auto keys = repo_->getKeys();
  ASSERT_EQ(keys.size(), 2u);
  EXPECT_EQ(keys[0], "key1");
  EXPECT_EQ(keys[1], "key2");
}

TEST_F(KeyValueRepositoryTest, GetKeysByPrefix) {
  repo_->set("app.theme", "dark");
  repo_->set("app.language", "en");
  repo_->set("user.name", "John");

  auto keys = repo_->getKeysByPrefix("app.");
  ASSERT_EQ(keys.size(), 2u);
  EXPECT_EQ(keys[0], "app.language");
  EXPECT_EQ(keys[1], "app.theme");
}

TEST_F(KeyValueRepositoryTest, RemoveByPrefix) {
  repo_->set("app.theme", "dark");
  repo_->set("app.language", "en");
  repo_->set("user.name", "John");

  repo_->removeByPrefix("app.");

  EXPECT_FALSE(repo_->exists("app.theme"));
  EXPECT_FALSE(repo_->exists("app.language"));
  EXPECT_TRUE(repo_->exists("user.name"));
}

TEST_F(KeyValueRepositoryTest, Clear) {
  repo_->set("key1", "value1");
  repo_->set("key2", "value2");

  repo_->clear();

  EXPECT_EQ(repo_->count(), 0);
}

// ============================================================
// Count
// ============================================================
TEST_F(KeyValueRepositoryTest, Count) {
  EXPECT_EQ(repo_->count(), 0);

  repo_->set("key1", "value1");
  EXPECT_EQ(repo_->count(), 1);

  repo_->set("key2", "value2");
  EXPECT_EQ(repo_->count(), 2);
}

TEST_F(KeyValueRepositoryTest, CountByPrefix) {
  repo_->set("app.theme", "dark");
  repo_->set("app.language", "en");
  repo_->set("user.name", "John");

  EXPECT_EQ(repo_->countByPrefix("app."), 2);
  EXPECT_EQ(repo_->countByPrefix("user."), 1);
  EXPECT_EQ(repo_->countByPrefix("other."), 0);
}

// ============================================================
// Edge Cases
// ============================================================
TEST_F(KeyValueRepositoryTest, EmptyKey) {
  repo_->set("", "value");

  auto setting = repo_->get("");
  ASSERT_TRUE(setting.has_value());
  EXPECT_EQ(setting->value, "value");
}

TEST_F(KeyValueRepositoryTest, EmptyValue) {
  repo_->set("key", "");

  auto setting = repo_->get("key");
  ASSERT_TRUE(setting.has_value());
  EXPECT_EQ(setting->value, "");
}

TEST_F(KeyValueRepositoryTest, UnicodeKeyAndValue) {
  repo_->set("настройка", "значение", "описание");

  auto setting = repo_->get("настройка");
  ASSERT_TRUE(setting.has_value());
  EXPECT_EQ(setting->value, "значение");
  EXPECT_EQ(setting->description, "описание");
}

TEST_F(KeyValueRepositoryTest, SpecialCharactersInValue) {
  std::string value = "line1\nline2\ttab\"quote'apostrophe";
  repo_->set("key", value);

  auto setting = repo_->get("key");
  ASSERT_TRUE(setting.has_value());
  EXPECT_EQ(setting->value, value);
}

TEST_F(KeyValueRepositoryTest, LongValue) {
  std::string longValue(100000, 'x');
  repo_->set("key", longValue);

  auto setting = repo_->get("key");
  ASSERT_TRUE(setting.has_value());
  EXPECT_EQ(setting->value.size(), 100000u);
}

// ============================================================
// Typical Use Cases
// ============================================================
TEST_F(KeyValueRepositoryTest, ApplicationSettings) {
  repo_->set("ui.theme", "dark", "Application color theme");
  repo_->set("ui.language", "en", "Interface language");
  repo_->set("ui.fontSize", "14", "Font size in pixels");
  repo_->set("algo.momentum.rating", "5", "Algorithm rating");
  repo_->set("algo.momentum.enabled", "true", "Whether algorithm is enabled");

  auto uiSettings = repo_->getByPrefix("ui.");
  EXPECT_EQ(uiSettings.size(), 3u);

  auto algoSettings = repo_->getByPrefix("algo.momentum.");
  EXPECT_EQ(algoSettings.size(), 2u);
}

TEST_F(KeyValueRepositoryTest, AssetSourceMapping) {
  repo_->set("asset.AAPL.source", "yahoo", "Data source for AAPL");
  repo_->set("asset.GOOGL.source", "alphavantage", "Data source for GOOGL");
  repo_->set("asset.BTC.source", "coinbase", "Data source for BTC");

  auto source = repo_->getValue("asset.AAPL.source");
  ASSERT_TRUE(source.has_value());
  EXPECT_EQ(*source, "yahoo");
}

}  // namespace
}  // namespace Gateways::Repositories::Sqlite3
