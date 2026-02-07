#include "create_account.h"

namespace UseCases {

CreateAccountInteractor::CreateAccountInteractor(IAccountRepository& repository)
    : m_repository(repository) {}

CreateAccountResponse CreateAccountInteractor::execute(
    const CreateAccountRequest& request) {
  if (request.id.empty()) {
    throw CreateAccountError("Account ID cannot be empty");
  }

  if (request.name.empty()) {
    throw CreateAccountError("Account name cannot be empty");
  }

  if (m_repository.accountExists(request.id)) {
    throw CreateAccountError("Account with this ID already exists");
  }

  if (m_repository.accountExistsByName(request.name)) {
    throw CreateAccountError("Account with this name already exists");
  }

  Entities::Account account;
  account.id = request.id;
  account.name = request.name;
  account.password_hash = request.password_hash;
  account.created_at = request.created_at;

  m_repository.createAccount(account);

  return CreateAccountResponse{
      .id = account.id,
      .name = account.name,
      .created_at = account.created_at,
  };
}

}  // namespace UseCases
