
#ifndef QX_CONTROLLER_H_
#define QX_CONTROLLER_H_

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "i_account_repository.h"
#include "i_request_response.h"

namespace presenter {

class DemoController : public RequestResponseInterface {
 public:
  explicit DemoController(UseCases::IAccountRepository& repository);
  ~DemoController() override = default;

  DemoController(const DemoController&) = delete;
  DemoController& operator=(const DemoController&) = delete;
  DemoController(DemoController&&) = delete;
  DemoController& operator=(DemoController&&) = delete;

  Response HandleRequest(const Request& request) override;
  std::vector<std::string> GetAvailableCommands() const override;

 private:
  using CommandHandler =
      std::function<Response(const std::vector<std::string>&)>;

  struct CommandData {
    std::string description;
    CommandHandler handler;
  };

  std::unordered_map<std::string, CommandData> command_handlers_;

  UseCases::IAccountRepository& repository_;

  // Command handlers - each one routes to repository interface
  Response HandleCreateAccount(const std::vector<std::string>& args);
  Response HandleGetAccount(const std::vector<std::string>& args);
  Response HandleListAccounts(const std::vector<std::string>& args);
  Response HandleDeleteAccount(const std::vector<std::string>& args);
  Response HandleSetProperty(const std::vector<std::string>& args);
  Response HandleGetProperty(const std::vector<std::string>& args);

  void RegisterCommand(const std::string& name, const std::string& description,
                       CommandHandler handler);
};

}  // namespace presenter

#endif  // QX_CONTROLLER_H_
