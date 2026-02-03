#include "account_repository.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "sqlite3_database_connector.h"

namespace Gateways::Repositories::Sqlite3 {
namespace {

class AccountRepositoryTest : public ::testing::Test {
 protected:
  void SetUp() override {
    db_ = std::make_unique<Gateways::Database::SqliteDatabase>(":memory:");
    repo_ = std::make_unique<AccountRepository>(*db_);
    repo_->initSchema();
  }

  std::unique_ptr<Gateways::Database::SqliteDatabase> db_;
  std::unique_ptr<AccountRepository> repo_;
};

// ============================================================
// Schema
// ============================================================
TEST_F(AccountRepositoryTest, InitSchemaCreatesTablesIfNotExist) {
  EXPECT_NO_THROW(repo_->initSchema());
}

// ============================================================
// Account CRUD
// ============================================================
TEST_F(AccountRepositoryTest, CreateAndGetAccount) {
  Entities::Account account;
  account.id = "acc-123";
  account.name = "John Doe";
  account.created_at = 1704067200000;

  repo_->createAccount(account);

  auto retrieved = repo_->getAccount("acc-123");
  ASSERT_TRUE(retrieved.has_value());
  EXPECT_EQ(retrieved->id, "acc-123");
  EXPECT_EQ(retrieved->name, "John Doe");
  EXPECT_FALSE(retrieved->password_hash.has_value());
  EXPECT_EQ(retrieved->created_at, 1704067200000);
}

TEST_F(AccountRepositoryTest, CreateAccountWithPasswordHash) {
  Entities::Account account;
  account.id = "acc-123";
  account.name = "John Doe";
  account.password_hash = std::vector<uint8_t>{0x01, 0x02, 0x03, 0x04};
  account.created_at = 1704067200000;

  repo_->createAccount(account);

  auto retrieved = repo_->getAccount("acc-123");
  ASSERT_TRUE(retrieved.has_value());
  ASSERT_TRUE(retrieved->password_hash.has_value());
  EXPECT_EQ(*retrieved->password_hash,
            (std::vector<uint8_t>{0x01, 0x02, 0x03, 0x04}));
}

TEST_F(AccountRepositoryTest, GetAccountNotFound) {
  auto retrieved = repo_->getAccount("nonexistent");
  EXPECT_FALSE(retrieved.has_value());
}

TEST_F(AccountRepositoryTest, GetAccountByName) {
  Entities::Account account{"acc-123", "John Doe", std::nullopt, 1704067200000};
  repo_->createAccount(account);

  auto retrieved = repo_->getAccountByName("John Doe");
  ASSERT_TRUE(retrieved.has_value());
  EXPECT_EQ(retrieved->id, "acc-123");
}

TEST_F(AccountRepositoryTest, GetAccountByNameNotFound) {
  auto retrieved = repo_->getAccountByName("Unknown");
  EXPECT_FALSE(retrieved.has_value());
}

TEST_F(AccountRepositoryTest, GetAllAccounts) {
  repo_->createAccount({"a1", "Alice", std::nullopt, 1000});
  repo_->createAccount({"a2", "Bob", std::nullopt, 2000});
  repo_->createAccount({"a3", "Charlie", std::nullopt, 3000});

  auto accounts = repo_->getAllAccounts();
  ASSERT_EQ(accounts.size(), 3u);
  // Ordered by name
  EXPECT_EQ(accounts[0].name, "Alice");
  EXPECT_EQ(accounts[1].name, "Bob");
  EXPECT_EQ(accounts[2].name, "Charlie");
}

TEST_F(AccountRepositoryTest, GetAllAccountsEmpty) {
  auto accounts = repo_->getAllAccounts();
  EXPECT_TRUE(accounts.empty());
}

TEST_F(AccountRepositoryTest, UpdateAccount) {
  repo_->createAccount({"a1", "Original", std::nullopt, 1000});

  Entities::Account updated;
  updated.id = "a1";
  updated.name = "Updated";
  updated.password_hash = std::vector<uint8_t>{0xAB, 0xCD};
  updated.created_at = 2000;

  repo_->updateAccount(updated);

  auto retrieved = repo_->getAccount("a1");
  ASSERT_TRUE(retrieved.has_value());
  EXPECT_EQ(retrieved->name, "Updated");
  ASSERT_TRUE(retrieved->password_hash.has_value());
  EXPECT_EQ(*retrieved->password_hash, (std::vector<uint8_t>{0xAB, 0xCD}));
  EXPECT_EQ(retrieved->created_at, 2000);
}

TEST_F(AccountRepositoryTest, UpdateAccountClearPassword) {
  std::vector<uint8_t> hash = {0x01, 0x02};
  repo_->createAccount({"a1", "User", hash, 1000});

  Entities::Account updated{"a1", "User", std::nullopt, 1000};
  repo_->updateAccount(updated);

  auto retrieved = repo_->getAccount("a1");
  ASSERT_TRUE(retrieved.has_value());
  EXPECT_FALSE(retrieved->password_hash.has_value());
}

TEST_F(AccountRepositoryTest, DeleteAccount) {
  repo_->createAccount({"a1", "User", std::nullopt, 1000});
  ASSERT_TRUE(repo_->getAccount("a1").has_value());

  repo_->deleteAccount("a1");
  EXPECT_FALSE(repo_->getAccount("a1").has_value());
}

TEST_F(AccountRepositoryTest, DeleteAccountCascadesToProperties) {
  repo_->createAccount({"a1", "User", std::nullopt, 1000});
  repo_->setProperty("a1", "key1", "value1");
  repo_->setProperty("a1", "key2", "value2");

  ASSERT_EQ(repo_->countProperties("a1"), 2);

  repo_->deleteAccount("a1");

  // Properties should be deleted via cascade
  EXPECT_EQ(repo_->countProperties("a1"), 0);
}

TEST_F(AccountRepositoryTest, AccountExists) {
  repo_->createAccount({"a1", "User", std::nullopt, 1000});

  EXPECT_TRUE(repo_->accountExists("a1"));
  EXPECT_FALSE(repo_->accountExists("nonexistent"));
}

TEST_F(AccountRepositoryTest, AccountExistsByName) {
  repo_->createAccount({"a1", "John", std::nullopt, 1000});

  EXPECT_TRUE(repo_->accountExistsByName("John"));
  EXPECT_FALSE(repo_->accountExistsByName("Jane"));
}

// ============================================================
// Account Property CRUD
// ============================================================
TEST_F(AccountRepositoryTest, SetAndGetProperty) {
  repo_->createAccount({"a1", "User", std::nullopt, 1000});

  repo_->setProperty("a1", "theme", "dark");

  auto prop = repo_->getProperty("a1", "theme");
  ASSERT_TRUE(prop.has_value());
  EXPECT_EQ(prop->account_id, "a1");
  EXPECT_EQ(prop->key, "theme");
  EXPECT_EQ(prop->value, "dark");
  EXPECT_FALSE(prop->description.has_value());
}

TEST_F(AccountRepositoryTest, SetPropertyWithDescription) {
  repo_->createAccount({"a1", "User", std::nullopt, 1000});

  repo_->setProperty("a1", "theme", "dark", "User's preferred theme");

  auto prop = repo_->getProperty("a1", "theme");
  ASSERT_TRUE(prop.has_value());
  ASSERT_TRUE(prop->description.has_value());
  EXPECT_EQ(*prop->description, "User's preferred theme");
}

TEST_F(AccountRepositoryTest, SetPropertyUsingStruct) {
  repo_->createAccount({"a1", "User", std::nullopt, 1000});

  Entities::AccountProperty property{"a1", "key", "value", "desc"};
  repo_->setProperty(property);

  auto retrieved = repo_->getProperty("a1", "key");
  ASSERT_TRUE(retrieved.has_value());
  EXPECT_EQ(retrieved->value, "value");
  EXPECT_EQ(retrieved->description, "desc");
}

TEST_F(AccountRepositoryTest, SetPropertyUpdatesExisting) {
  repo_->createAccount({"a1", "User", std::nullopt, 1000});

  repo_->setProperty("a1", "key", "value1");
  repo_->setProperty("a1", "key", "value2");

  auto prop = repo_->getProperty("a1", "key");
  ASSERT_TRUE(prop.has_value());
  EXPECT_EQ(prop->value, "value2");
}

TEST_F(AccountRepositoryTest, GetPropertyNotFound) {
  repo_->createAccount({"a1", "User", std::nullopt, 1000});

  auto prop = repo_->getProperty("a1", "nonexistent");
  EXPECT_FALSE(prop.has_value());
}

TEST_F(AccountRepositoryTest, GetPropertyValue) {
  repo_->createAccount({"a1", "User", std::nullopt, 1000});
  repo_->setProperty("a1", "key", "value");

  auto value = repo_->getPropertyValue("a1", "key");
  ASSERT_TRUE(value.has_value());
  EXPECT_EQ(*value, "value");
}

TEST_F(AccountRepositoryTest, GetPropertyValueNotFound) {
  repo_->createAccount({"a1", "User", std::nullopt, 1000});

  auto value = repo_->getPropertyValue("a1", "nonexistent");
  EXPECT_FALSE(value.has_value());
}

TEST_F(AccountRepositoryTest, GetProperties) {
  repo_->createAccount({"a1", "User", std::nullopt, 1000});
  repo_->setProperty("a1", "key1", "value1");
  repo_->setProperty("a1", "key2", "value2");
  repo_->setProperty("a1", "key3", "value3");

  auto props = repo_->getProperties("a1");
  ASSERT_EQ(props.size(), 3u);
  // Ordered by key
  EXPECT_EQ(props[0].key, "key1");
  EXPECT_EQ(props[1].key, "key2");
  EXPECT_EQ(props[2].key, "key3");
}

TEST_F(AccountRepositoryTest, GetPropertiesEmpty) {
  repo_->createAccount({"a1", "User", std::nullopt, 1000});

  auto props = repo_->getProperties("a1");
  EXPECT_TRUE(props.empty());
}

TEST_F(AccountRepositoryTest, GetPropertiesByPrefix) {
  repo_->createAccount({"a1", "User", std::nullopt, 1000});
  repo_->setProperty("a1", "ui.theme", "dark");
  repo_->setProperty("a1", "ui.lang", "en");
  repo_->setProperty("a1", "api.key", "secret");

  auto uiProps = repo_->getPropertiesByPrefix("a1", "ui.");
  ASSERT_EQ(uiProps.size(), 2u);
  EXPECT_EQ(uiProps[0].key, "ui.lang");
  EXPECT_EQ(uiProps[1].key, "ui.theme");
}

TEST_F(AccountRepositoryTest, PropertyExists) {
  repo_->createAccount({"a1", "User", std::nullopt, 1000});
  repo_->setProperty("a1", "key", "value");

  EXPECT_TRUE(repo_->propertyExists("a1", "key"));
  EXPECT_FALSE(repo_->propertyExists("a1", "other"));
}

TEST_F(AccountRepositoryTest, RemoveProperty) {
  repo_->createAccount({"a1", "User", std::nullopt, 1000});
  repo_->setProperty("a1", "key", "value");

  ASSERT_TRUE(repo_->propertyExists("a1", "key"));

  repo_->removeProperty("a1", "key");
  EXPECT_FALSE(repo_->propertyExists("a1", "key"));
}

TEST_F(AccountRepositoryTest, RemovePropertiesByPrefix) {
  repo_->createAccount({"a1", "User", std::nullopt, 1000});
  repo_->setProperty("a1", "ui.theme", "dark");
  repo_->setProperty("a1", "ui.lang", "en");
  repo_->setProperty("a1", "api.key", "secret");

  repo_->removePropertiesByPrefix("a1", "ui.");

  EXPECT_FALSE(repo_->propertyExists("a1", "ui.theme"));
  EXPECT_FALSE(repo_->propertyExists("a1", "ui.lang"));
  EXPECT_TRUE(repo_->propertyExists("a1", "api.key"));
}

TEST_F(AccountRepositoryTest, ClearProperties) {
  repo_->createAccount({"a1", "User", std::nullopt, 1000});
  repo_->setProperty("a1", "key1", "value1");
  repo_->setProperty("a1", "key2", "value2");

  repo_->clearProperties("a1");

  EXPECT_EQ(repo_->countProperties("a1"), 0);
}

// ============================================================
// Count
// ============================================================
TEST_F(AccountRepositoryTest, CountAccounts) {
  EXPECT_EQ(repo_->countAccounts(), 0);

  repo_->createAccount({"a1", "User1", std::nullopt, 1000});
  EXPECT_EQ(repo_->countAccounts(), 1);

  repo_->createAccount({"a2", "User2", std::nullopt, 2000});
  EXPECT_EQ(repo_->countAccounts(), 2);
}

TEST_F(AccountRepositoryTest, CountProperties) {
  repo_->createAccount({"a1", "User", std::nullopt, 1000});
  EXPECT_EQ(repo_->countProperties("a1"), 0);

  repo_->setProperty("a1", "key1", "value1");
  EXPECT_EQ(repo_->countProperties("a1"), 1);

  repo_->setProperty("a1", "key2", "value2");
  EXPECT_EQ(repo_->countProperties("a1"), 2);
}

// ============================================================
// Edge Cases
// ============================================================
TEST_F(AccountRepositoryTest, UnicodeAccountName) {
  repo_->createAccount({"a1", "用户名", std::nullopt, 1000});

  auto account = repo_->getAccountByName("用户名");
  ASSERT_TRUE(account.has_value());
  EXPECT_EQ(account->name, "用户名");
}

TEST_F(AccountRepositoryTest, UnicodePropertyValue) {
  repo_->createAccount({"a1", "User", std::nullopt, 1000});
  repo_->setProperty("a1", "greeting", "こんにちは", "Japanese greeting");

  auto prop = repo_->getProperty("a1", "greeting");
  ASSERT_TRUE(prop.has_value());
  EXPECT_EQ(prop->value, "こんにちは");
}

TEST_F(AccountRepositoryTest, LargePasswordHash) {
  std::vector<uint8_t> largeHash(1024, 0xAB);
  repo_->createAccount({"a1", "User", largeHash, 1000});

  auto account = repo_->getAccount("a1");
  ASSERT_TRUE(account.has_value());
  ASSERT_TRUE(account->password_hash.has_value());
  EXPECT_EQ(account->password_hash->size(), 1024u);
}

TEST_F(AccountRepositoryTest, MultipleAccountsWithProperties) {
  repo_->createAccount({"a1", "Alice", std::nullopt, 1000});
  repo_->createAccount({"a2", "Bob", std::nullopt, 2000});

  repo_->setProperty("a1", "role", "admin");
  repo_->setProperty("a2", "role", "user");

  auto aliceRole = repo_->getPropertyValue("a1", "role");
  auto bobRole = repo_->getPropertyValue("a2", "role");

  ASSERT_TRUE(aliceRole.has_value());
  ASSERT_TRUE(bobRole.has_value());
  EXPECT_EQ(*aliceRole, "admin");
  EXPECT_EQ(*bobRole, "user");
}

// ============================================================
// Typical Use Cases
// ============================================================
TEST_F(AccountRepositoryTest, UserAccountWithCredentials) {
  // Simulate a user account with hashed password
  std::vector<uint8_t> hashedPassword = {0x5e, 0x88, 0x48, 0x98, 0xda, 0x28,
                                         0x04, 0x71, 0x51, 0xd0, 0xe5, 0x6f,
                                         0x8d, 0xc6, 0x29, 0x27};

  Entities::Account user;
  user.id = "user-uuid-123";
  user.name = "john@example.com";
  user.password_hash = hashedPassword;
  user.created_at = 1704067200000;

  repo_->createAccount(user);

  // User login lookup
  auto account = repo_->getAccountByName("john@example.com");
  ASSERT_TRUE(account.has_value());
  ASSERT_TRUE(account->password_hash.has_value());
  EXPECT_EQ(*account->password_hash, hashedPassword);
}

TEST_F(AccountRepositoryTest, ServiceAccountWithApiKey) {
  Entities::Account service;
  service.id = "svc-trading-bot";
  service.name = "Trading Bot";
  service.created_at = 1704067200000;

  repo_->createAccount(service);
  repo_->setProperty("svc-trading-bot", "api.key", "sk-live-xxx", "API key");
  repo_->setProperty("svc-trading-bot", "api.secret", "secret123",
                     "API secret");
  repo_->setProperty("svc-trading-bot", "permissions", "read,trade",
                     "Allowed actions");

  auto props = repo_->getPropertiesByPrefix("svc-trading-bot", "api.");
  EXPECT_EQ(props.size(), 2u);
}

TEST_F(AccountRepositoryTest, ExchangeAccount) {
  Entities::Account exchange;
  exchange.id = "exchange-binance";
  exchange.name = "Binance";
  exchange.created_at = 1704067200000;

  repo_->createAccount(exchange);
  repo_->setProperty("exchange-binance", "type", "crypto");
  repo_->setProperty("exchange-binance", "url", "https://api.binance.com");
  repo_->setProperty("exchange-binance", "rate_limit", "1200");

  auto url = repo_->getPropertyValue("exchange-binance", "url");
  ASSERT_TRUE(url.has_value());
  EXPECT_EQ(*url, "https://api.binance.com");
}

}  // namespace
}  // namespace Gateways::Repositories::Sqlite3
