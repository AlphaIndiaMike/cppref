#ifndef USE_CASES_CREATE_ACCOUNT_H_
#define USE_CASES_CREATE_ACCOUNT_H_

#include <stdexcept>
#include <string>

#include "entities.h"
#include "i_account_repository.h"

namespace UseCases {

class CreateAccountError : public std::runtime_error {
 public:
  explicit CreateAccountError(const std::string& msg)
      : std::runtime_error(msg) {}
};

struct CreateAccountRequest {
  std::string id;
  std::string name;
  std::optional<std::vector<uint8_t>> password_hash;
  int64_t created_at;
};

struct CreateAccountResponse {
  std::string id;
  std::string name;
  int64_t created_at;
};

class CreateAccountInteractor {
 public:
  explicit CreateAccountInteractor(IAccountRepository& repository);

  CreateAccountResponse execute(const CreateAccountRequest& request);

 private:
  IAccountRepository& m_repository;
};

}  // namespace UseCases

#endif  // USE_CASES_CREATE_ACCOUNT_H_
