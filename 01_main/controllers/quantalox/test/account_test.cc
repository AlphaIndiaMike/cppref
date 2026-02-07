// demo_controller_test.cc
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "qx_controller.h"

using ::testing::_;
using ::testing::HasSubstr;
using ::testing::Return;

// --- Mock repository ---

class MockAccountRepository : public UseCases::IAccountRepository {
 public:
  MOCK_METHOD(void, createAccount, (const Entities::Account&), (override));
  MOCK_METHOD(std::optional<Entities::Account>, getAccount,
              (const std::string&), (override));
  MOCK_METHOD(std::optional<Entities::Account>, getAccountByName,
              (const std::string&), (override));
  MOCK_METHOD(std::vector<Entities::Account>, getAllAccounts, (), (override));
  MOCK_METHOD(void, updateAccount, (const Entities::Account&), (override));
  MOCK_METHOD(void, deleteAccount, (const std::string&), (override));
  MOCK_METHOD(bool, accountExists, (const std::string&), (override));
  MOCK_METHOD(bool, accountExistsByName, (const std::string&), (override));

  MOCK_METHOD(void, setProperty,
              (const std::string&, const std::string&, const std::string&,
               const std::optional<std::string>&),
              (override));
  MOCK_METHOD(void, setProperty, (const Entities::AccountProperty&),
              (override));
  MOCK_METHOD(std::optional<Entities::AccountProperty>, getProperty,
              (const std::string&, const std::string&), (override));
  MOCK_METHOD(std::optional<std::string>, getPropertyValue,
              (const std::string&, const std::string&), (override));
  MOCK_METHOD(std::vector<Entities::AccountProperty>, getProperties,
              (const std::string&), (override));
  MOCK_METHOD(std::vector<Entities::AccountProperty>, getPropertiesByPrefix,
              (const std::string&, const std::string&), (override));
  MOCK_METHOD(bool, propertyExists, (const std::string&, const std::string&),
              (override));
  MOCK_METHOD(void, removeProperty, (const std::string&, const std::string&),
              (override));
  MOCK_METHOD(void, removePropertiesByPrefix,
              (const std::string&, const std::string&), (override));
  MOCK_METHOD(void, clearProperties, (const std::string&), (override));

  MOCK_METHOD(int64_t, countAccounts, (), (override));
  MOCK_METHOD(int64_t, countProperties, (const std::string&), (override));
};

// --- Fixture ---

class DemoControllerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    controller_ = std::make_unique<presenter::DemoController>(mock_repo_);
  }

  presenter::Request MakeRequest(const std::string& command,
                                 const std::vector<std::string>& args = {}) {
    return presenter::Request{command, args};
  }

  // Helper to build a mock account
  Entities::Account MakeAccount(const std::string& id,
                                const std::string& name) {
    Entities::Account a;
    a.id = id;
    a.name = name;
    a.created_at = 1000;
    return a;
  }

  ::testing::NiceMock<MockAccountRepository> mock_repo_;
  std::unique_ptr<presenter::DemoController> controller_;
};

// =====================================================================
// Discovery
// =====================================================================

TEST_F(DemoControllerTest, GetAvailableCommands_ReturnsAllRegistered) {
  auto commands = controller_->GetAvailableCommands();
  EXPECT_EQ(commands.size(), 6u);

  // All commands present in "name - description" format
  std::vector<std::string> names;
  for (const auto& cmd : commands) {
    auto pos = cmd.find(" - ");
    ASSERT_NE(pos, std::string::npos);
    names.push_back(cmd.substr(0, pos));
  }

  EXPECT_THAT(names, ::testing::UnorderedElementsAre(
                         "create_account", "get_account", "list_accounts",
                         "delete_account", "set_property", "get_property"));
}

// =====================================================================
// Routing
// =====================================================================

TEST_F(DemoControllerTest, UnknownCommand_Fails) {
  auto r = controller_->HandleRequest(MakeRequest("bogus"));

  EXPECT_FALSE(r.success);
  EXPECT_THAT(r.message, HasSubstr("Unknown command"));
  EXPECT_THAT(r.message, HasSubstr("bogus"));
}

// =====================================================================
// create_account
// =====================================================================

TEST_F(DemoControllerTest, CreateAccount_NoArgs_Fails) {
  auto r = controller_->HandleRequest(MakeRequest("create_account"));

  EXPECT_FALSE(r.success);
  EXPECT_THAT(r.message, HasSubstr("Usage"));
}

TEST_F(DemoControllerTest, CreateAccount_OneArg_Fails) {
  auto r =
      controller_->HandleRequest(MakeRequest("create_account", {"id_only"}));

  EXPECT_FALSE(r.success);
  EXPECT_THAT(r.message, HasSubstr("Usage"));
}

TEST_F(DemoControllerTest, CreateAccount_AlreadyExists_Fails) {
  EXPECT_CALL(mock_repo_, accountExists("acc_1")).WillOnce(Return(true));

  auto r = controller_->HandleRequest(
      MakeRequest("create_account", {"acc_1", "Alice"}));

  EXPECT_FALSE(r.success);
  EXPECT_THAT(r.message, HasSubstr("already exists"));
}

TEST_F(DemoControllerTest, CreateAccount_Success) {
  EXPECT_CALL(mock_repo_, accountExists("acc_1")).WillOnce(Return(false));
  EXPECT_CALL(mock_repo_, createAccount(_)).Times(1);

  auto r = controller_->HandleRequest(
      MakeRequest("create_account", {"acc_1", "Alice"}));

  EXPECT_TRUE(r.success);
  EXPECT_THAT(r.message, HasSubstr("acc_1"));
  EXPECT_THAT(r.message, HasSubstr("Alice"));
}

TEST_F(DemoControllerTest, CreateAccount_PassesCorrectFields) {
  Entities::Account captured;

  EXPECT_CALL(mock_repo_, accountExists("x")).WillOnce(Return(false));
  EXPECT_CALL(mock_repo_, createAccount(_))
      .WillOnce([&captured](const Entities::Account& a) { captured = a; });

  controller_->HandleRequest(MakeRequest("create_account", {"x", "Y"}));

  EXPECT_EQ(captured.id, "x");
  EXPECT_EQ(captured.name, "Y");
  EXPECT_GT(captured.created_at, 0);
}

// =====================================================================
// get_account
// =====================================================================

TEST_F(DemoControllerTest, GetAccount_NoArgs_Fails) {
  auto r = controller_->HandleRequest(MakeRequest("get_account"));

  EXPECT_FALSE(r.success);
  EXPECT_THAT(r.message, HasSubstr("Usage"));
}

TEST_F(DemoControllerTest, GetAccount_NotFound_Fails) {
  EXPECT_CALL(mock_repo_, getAccount("missing")).WillOnce(Return(std::nullopt));

  auto r = controller_->HandleRequest(MakeRequest("get_account", {"missing"}));

  EXPECT_FALSE(r.success);
  EXPECT_THAT(r.message, HasSubstr("not found"));
}

TEST_F(DemoControllerTest, GetAccount_Found_ReturnsDetails) {
  EXPECT_CALL(mock_repo_, getAccount("acc_1"))
      .WillOnce(Return(MakeAccount("acc_1", "Alice")));

  auto r = controller_->HandleRequest(MakeRequest("get_account", {"acc_1"}));

  EXPECT_TRUE(r.success);
  EXPECT_THAT(r.message, HasSubstr("acc_1"));
  EXPECT_THAT(r.message, HasSubstr("Alice"));
}

// =====================================================================
// list_accounts
// =====================================================================

TEST_F(DemoControllerTest, ListAccounts_Empty) {
  EXPECT_CALL(mock_repo_, getAllAccounts())
      .WillOnce(Return(std::vector<Entities::Account>{}));

  auto r = controller_->HandleRequest(MakeRequest("list_accounts"));

  EXPECT_TRUE(r.success);
  EXPECT_THAT(r.message, HasSubstr("No accounts"));
}

TEST_F(DemoControllerTest, ListAccounts_WithAccounts) {
  std::vector<Entities::Account> accounts = {
      MakeAccount("a1", "Alice"),
      MakeAccount("a2", "Bob"),
  };

  EXPECT_CALL(mock_repo_, getAllAccounts()).WillOnce(Return(accounts));

  auto r = controller_->HandleRequest(MakeRequest("list_accounts"));

  EXPECT_TRUE(r.success);
  EXPECT_THAT(r.message, HasSubstr("Accounts (2)"));
  EXPECT_THAT(r.message, HasSubstr("a1"));
  EXPECT_THAT(r.message, HasSubstr("Alice"));
  EXPECT_THAT(r.message, HasSubstr("a2"));
  EXPECT_THAT(r.message, HasSubstr("Bob"));
}

// =====================================================================
// delete_account
// =====================================================================

TEST_F(DemoControllerTest, DeleteAccount_NoArgs_Fails) {
  auto r = controller_->HandleRequest(MakeRequest("delete_account"));

  EXPECT_FALSE(r.success);
  EXPECT_THAT(r.message, HasSubstr("Usage"));
}

TEST_F(DemoControllerTest, DeleteAccount_NotFound_Fails) {
  EXPECT_CALL(mock_repo_, accountExists("missing")).WillOnce(Return(false));

  auto r =
      controller_->HandleRequest(MakeRequest("delete_account", {"missing"}));

  EXPECT_FALSE(r.success);
  EXPECT_THAT(r.message, HasSubstr("not found"));
}

TEST_F(DemoControllerTest, DeleteAccount_Success) {
  EXPECT_CALL(mock_repo_, accountExists("acc_1")).WillOnce(Return(true));
  EXPECT_CALL(mock_repo_, deleteAccount("acc_1")).Times(1);

  auto r = controller_->HandleRequest(MakeRequest("delete_account", {"acc_1"}));

  EXPECT_TRUE(r.success);
  EXPECT_THAT(r.message, HasSubstr("deleted"));
  EXPECT_THAT(r.message, HasSubstr("acc_1"));
}

// =====================================================================
// set_property
// =====================================================================

TEST_F(DemoControllerTest, SetProperty_TooFewArgs_Fails) {
  auto r =
      controller_->HandleRequest(MakeRequest("set_property", {"acc_1", "key"}));

  EXPECT_FALSE(r.success);
  EXPECT_THAT(r.message, HasSubstr("Usage"));
}

TEST_F(DemoControllerTest, SetProperty_AccountNotFound_Fails) {
  EXPECT_CALL(mock_repo_, accountExists("missing")).WillOnce(Return(false));

  auto r = controller_->HandleRequest(
      MakeRequest("set_property", {"missing", "k", "v"}));

  EXPECT_FALSE(r.success);
  EXPECT_THAT(r.message, HasSubstr("not found"));
}

TEST_F(DemoControllerTest, SetProperty_Success) {
  EXPECT_CALL(mock_repo_, accountExists("acc_1")).WillOnce(Return(true));
  EXPECT_CALL(mock_repo_, setProperty("acc_1", "color", "blue", _)).Times(1);

  auto r = controller_->HandleRequest(
      MakeRequest("set_property", {"acc_1", "color", "blue"}));

  EXPECT_TRUE(r.success);
  EXPECT_THAT(r.message, HasSubstr("color"));
  EXPECT_THAT(r.message, HasSubstr("blue"));
}

// =====================================================================
// get_property
// =====================================================================

TEST_F(DemoControllerTest, GetProperty_TooFewArgs_Fails) {
  auto r = controller_->HandleRequest(MakeRequest("get_property", {"acc_1"}));

  EXPECT_FALSE(r.success);
  EXPECT_THAT(r.message, HasSubstr("Usage"));
}

TEST_F(DemoControllerTest, GetProperty_NotFound_Fails) {
  EXPECT_CALL(mock_repo_, getPropertyValue("acc_1", "missing"))
      .WillOnce(Return(std::nullopt));

  auto r = controller_->HandleRequest(
      MakeRequest("get_property", {"acc_1", "missing"}));

  EXPECT_FALSE(r.success);
  EXPECT_THAT(r.message, HasSubstr("not found"));
}

TEST_F(DemoControllerTest, GetProperty_Found_ReturnsValue) {
  EXPECT_CALL(mock_repo_, getPropertyValue("acc_1", "color"))
      .WillOnce(Return(std::string("blue")));

  auto r = controller_->HandleRequest(
      MakeRequest("get_property", {"acc_1", "color"}));

  EXPECT_TRUE(r.success);
  EXPECT_THAT(r.message, HasSubstr("color"));
  EXPECT_THAT(r.message, HasSubstr("blue"));
}

// =====================================================================
// Skeleton workflow - simulates app behavior through mocked repository
// =====================================================================

TEST_F(DemoControllerTest, Workflow_CreateThenGetThenDelete) {
  // Create
  EXPECT_CALL(mock_repo_, accountExists("w1"))
      .WillOnce(Return(false))  // create check
      .WillOnce(Return(true));  // delete check
  EXPECT_CALL(mock_repo_, createAccount(_)).Times(1);

  auto r1 = controller_->HandleRequest(
      MakeRequest("create_account", {"w1", "Workflow"}));
  EXPECT_TRUE(r1.success);

  // Get
  EXPECT_CALL(mock_repo_, getAccount("w1"))
      .WillOnce(Return(MakeAccount("w1", "Workflow")));

  auto r2 = controller_->HandleRequest(MakeRequest("get_account", {"w1"}));
  EXPECT_TRUE(r2.success);
  EXPECT_THAT(r2.message, HasSubstr("Workflow"));

  // Delete
  EXPECT_CALL(mock_repo_, deleteAccount("w1")).Times(1);

  auto r3 = controller_->HandleRequest(MakeRequest("delete_account", {"w1"}));
  EXPECT_TRUE(r3.success);
}
