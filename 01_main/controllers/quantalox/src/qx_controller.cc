// demo_controller.cc
#include "qx_controller.h"

#include <chrono>
#include <sstream>

namespace presenter {

DemoController::DemoController(UseCases::IAccountRepository& repository)
    : repository_(repository) {
  RegisterCommand(
      "create_account", "Create account (create_account <id> <name>)",
      [this](const auto& args) { return HandleCreateAccount(args); });
  RegisterCommand("get_account", "Get account by id (get_account <id>)",
                  [this](const auto& args) { return HandleGetAccount(args); });
  RegisterCommand(
      "list_accounts", "List all accounts",
      [this](const auto& args) { return HandleListAccounts(args); });
  RegisterCommand(
      "delete_account", "Delete account (delete_account <id>)",
      [this](const auto& args) { return HandleDeleteAccount(args); });
  RegisterCommand("set_property",
                  "Set property (set_property <account_id> <key> <value>)",
                  [this](const auto& args) { return HandleSetProperty(args); });
  RegisterCommand("get_property",
                  "Get property (get_property <account_id> <key>)",
                  [this](const auto& args) { return HandleGetProperty(args); });
}

Response DemoController::HandleRequest(const Request& request) {
  auto it = command_handlers_.find(request.command);

  if (it == command_handlers_.end()) {
    return Response{false, "Unknown command: " + request.command};
  }

  return it->second.handler(request.arguments);
}

std::vector<std::string> DemoController::GetAvailableCommands() const {
  std::vector<std::string> commands;
  for (const auto& [name, data] : command_handlers_) {
    commands.push_back(name + " - " + data.description);
  }
  return commands;
}

void DemoController::RegisterCommand(const std::string& name,
                                     const std::string& description,
                                     CommandHandler handler) {
  command_handlers_[name] = {description, std::move(handler)};
}

Response DemoController::HandleCreateAccount(
    const std::vector<std::string>& args) {
  if (args.size() < 2) {
    return Response{false, "Usage: create_account <id> <name>"};
  }

  if (repository_.accountExists(args[0])) {
    return Response{false, "Account already exists: " + args[0]};
  }

  Entities::Account account;
  account.id = args[0];
  account.name = args[1];
  account.created_at = std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::system_clock::now().time_since_epoch())
                           .count();

  repository_.createAccount(account);

  return Response{
      true, "Account created: id=" + account.id + ", name=" + account.name};
}

Response DemoController::HandleGetAccount(
    const std::vector<std::string>& args) {
  if (args.empty()) {
    return Response{false, "Usage: get_account <id>"};
  }

  auto account = repository_.getAccount(args[0]);

  if (!account.has_value()) {
    return Response{false, "Account not found: " + args[0]};
  }

  return Response{true, "id=" + account->id + ", name=" + account->name};
}

Response DemoController::HandleListAccounts(
    const std::vector<std::string>& args) {
  auto accounts = repository_.getAllAccounts();

  if (accounts.empty()) {
    return Response{true, "No accounts"};
  }

  std::ostringstream oss;
  oss << "Accounts (" << accounts.size() << "):\n";
  for (size_t i = 0; i < accounts.size(); ++i) {
    oss << "  " << (i + 1) << ". " << accounts[i].id << " - "
        << accounts[i].name << "\n";
  }

  return Response{true, oss.str()};
}

Response DemoController::HandleDeleteAccount(
    const std::vector<std::string>& args) {
  if (args.empty()) {
    return Response{false, "Usage: delete_account <id>"};
  }

  if (!repository_.accountExists(args[0])) {
    return Response{false, "Account not found: " + args[0]};
  }

  repository_.deleteAccount(args[0]);

  return Response{true, "Account deleted: " + args[0]};
}

Response DemoController::HandleSetProperty(
    const std::vector<std::string>& args) {
  if (args.size() < 3) {
    return Response{false, "Usage: set_property <account_id> <key> <value>"};
  }

  if (!repository_.accountExists(args[0])) {
    return Response{false, "Account not found: " + args[0]};
  }

  repository_.setProperty(args[0], args[1], args[2]);

  return Response{true, "Property set: " + args[1] + "=" + args[2] +
                            " on account " + args[0]};
}

Response DemoController::HandleGetProperty(
    const std::vector<std::string>& args) {
  if (args.size() < 2) {
    return Response{false, "Usage: get_property <account_id> <key>"};
  }

  auto value = repository_.getPropertyValue(args[0], args[1]);

  if (!value.has_value()) {
    return Response{
        false, "Property not found: " + args[1] + " on account " + args[0]};
  }

  return Response{true, args[1] + "=" + value.value()};
}

}  // namespace presenter
