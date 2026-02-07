#include "create_account.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "i_account_repository.h"

namespace UseCases {
namespace {

using ::testing::_;
using ::testing::Return;

// Mock repository for testing interactor in isolation
class MockAccountRepository : public IAccountRepository {
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

class CreateAccountInteractorTest : public ::testing::Test {
 protected:
  MockAccountRepository mockRepo_;
  UseCases::CreateAccountInteractor interactor_{mockRepo_};
};

// ============================================================
// Success Cases
// ============================================================
TEST_F(CreateAccountInteractorTest, CreateAccountSuccess) {
  UseCases::CreateAccountRequest request{
      .id = "acc-123",
      .name = "John Doe",
      .password_hash = std::nullopt,
      .created_at = 1704067200000,
  };

  EXPECT_CALL(mockRepo_, accountExists("acc-123")).WillOnce(Return(false));
  EXPECT_CALL(mockRepo_, accountExistsByName("John Doe"))
      .WillOnce(Return(false));
  EXPECT_CALL(mockRepo_, createAccount(_)).Times(1);

  auto response = interactor_.execute(request);

  EXPECT_EQ(response.id, "acc-123");
  EXPECT_EQ(response.name, "John Doe");
  EXPECT_EQ(response.created_at, 1704067200000);
}

TEST_F(CreateAccountInteractorTest, CreateAccountWithPassword) {
  std::vector<uint8_t> hash = {0x01, 0x02, 0x03};

  UseCases::CreateAccountRequest request{
      .id = "acc-123",
      .name = "John Doe",
      .password_hash = hash,
      .created_at = 1704067200000,
  };

  EXPECT_CALL(mockRepo_, accountExists(_)).WillOnce(Return(false));
  EXPECT_CALL(mockRepo_, accountExistsByName(_)).WillOnce(Return(false));
  EXPECT_CALL(mockRepo_, createAccount(_)).Times(1);

  auto response = interactor_.execute(request);

  EXPECT_EQ(response.id, "acc-123");
}

// ============================================================
// Validation Errors
// ============================================================
TEST_F(CreateAccountInteractorTest, EmptyIdThrows) {
  UseCases::CreateAccountRequest request{
      .id = "",
      .name = "John Doe",
      .password_hash = std::nullopt,
      .created_at = 1704067200000,
  };

  EXPECT_THROW(interactor_.execute(request), UseCases::CreateAccountError);
}

TEST_F(CreateAccountInteractorTest, EmptyNameThrows) {
  UseCases::CreateAccountRequest request{
      .id = "acc-123",
      .name = "",
      .password_hash = std::nullopt,
      .created_at = 1704067200000,
  };

  EXPECT_THROW(interactor_.execute(request), UseCases::CreateAccountError);
}

// ============================================================
// Duplicate Errors
// ============================================================
TEST_F(CreateAccountInteractorTest, DuplicateIdThrows) {
  UseCases::CreateAccountRequest request{
      .id = "acc-123",
      .name = "John Doe",
      .password_hash = std::nullopt,
      .created_at = 1704067200000,
  };

  EXPECT_CALL(mockRepo_, accountExists("acc-123")).WillOnce(Return(true));

  EXPECT_THROW(interactor_.execute(request), UseCases::CreateAccountError);
}

TEST_F(CreateAccountInteractorTest, DuplicateNameThrows) {
  UseCases::CreateAccountRequest request{
      .id = "acc-123",
      .name = "John Doe",
      .password_hash = std::nullopt,
      .created_at = 1704067200000,
  };

  EXPECT_CALL(mockRepo_, accountExists("acc-123")).WillOnce(Return(false));
  EXPECT_CALL(mockRepo_, accountExistsByName("John Doe"))
      .WillOnce(Return(true));

  EXPECT_THROW(interactor_.execute(request), UseCases::CreateAccountError);
}

// ============================================================
// Error Message Verification
// ============================================================
TEST_F(CreateAccountInteractorTest, EmptyIdErrorMessage) {
  UseCases::CreateAccountRequest request{
      .id = "", .name = "John", .created_at = 0};

  try {
    interactor_.execute(request);
    FAIL() << "Expected CreateAccountError";
  } catch (const UseCases::CreateAccountError& e) {
    EXPECT_STREQ(e.what(), "Account ID cannot be empty");
  }
}

TEST_F(CreateAccountInteractorTest, EmptyNameErrorMessage) {
  UseCases::CreateAccountRequest request{
      .id = "123", .name = "", .created_at = 0};

  try {
    interactor_.execute(request);
    FAIL() << "Expected CreateAccountError";
  } catch (const UseCases::CreateAccountError& e) {
    EXPECT_STREQ(e.what(), "Account name cannot be empty");
  }
}

TEST_F(CreateAccountInteractorTest, DuplicateIdErrorMessage) {
  UseCases::CreateAccountRequest request{
      .id = "123", .name = "John", .created_at = 0};

  EXPECT_CALL(mockRepo_, accountExists("123")).WillOnce(Return(true));

  try {
    interactor_.execute(request);
    FAIL() << "Expected CreateAccountError";
  } catch (const UseCases::CreateAccountError& e) {
    EXPECT_STREQ(e.what(), "Account with this ID already exists");
  }
}

TEST_F(CreateAccountInteractorTest, DuplicateNameErrorMessage) {
  UseCases::CreateAccountRequest request{
      .id = "123", .name = "John", .created_at = 0};

  EXPECT_CALL(mockRepo_, accountExists(_)).WillOnce(Return(false));
  EXPECT_CALL(mockRepo_, accountExistsByName("John")).WillOnce(Return(true));

  try {
    interactor_.execute(request);
    FAIL() << "Expected CreateAccountError";
  } catch (const UseCases::CreateAccountError& e) {
    EXPECT_STREQ(e.what(), "Account with this name already exists");
  }
}

// ============================================================
// Repository Interaction Verification
// ============================================================
TEST_F(CreateAccountInteractorTest, RepositoryReceivesCorrectData) {
  std::vector<uint8_t> hash = {0xAB, 0xCD};

  UseCases::CreateAccountRequest request{
      .id = "test-id",
      .name = "Test Name",
      .password_hash = hash,
      .created_at = 999,
  };

  Entities::Account capturedAccount;

  EXPECT_CALL(mockRepo_, accountExists(_)).WillOnce(Return(false));
  EXPECT_CALL(mockRepo_, accountExistsByName(_)).WillOnce(Return(false));
  EXPECT_CALL(mockRepo_, createAccount(_))
      .WillOnce([&capturedAccount](const Entities::Account& acc) {
        capturedAccount = acc;
      });

  interactor_.execute(request);

  EXPECT_EQ(capturedAccount.id, "test-id");
  EXPECT_EQ(capturedAccount.name, "Test Name");
  ASSERT_TRUE(capturedAccount.password_hash.has_value());
  EXPECT_EQ(*capturedAccount.password_hash, hash);
  EXPECT_EQ(capturedAccount.created_at, 999);
}

}  // namespace
}  // namespace UseCases
