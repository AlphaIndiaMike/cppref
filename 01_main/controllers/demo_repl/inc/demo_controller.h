// DEMO_controller.h
#ifndef DEMO_CONTROLLER_H_
#define DEMO_CONTROLLER_H_

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "req_response_interface.h"

namespace presenter {

class DemoController : public RequestResponseInterface {
 public:
  DemoController();
  ~DemoController() override = default;

  // Disable copy and move
  DemoController(const DemoController&) =
      delete;  // C++11 syntax - don't allow new object - only one object shall
               // be created. Triggered by: DemoController b(a); or
               // DemoController b = a;
  DemoController& operator=(const DemoController&) =
      delete;  // = operator overload that takes reference to another
               // DemoController is not allowed. Triggered by: a = b; (where
               // both already exist).
  DemoController(DemoController&&) =
      delete;  // Move constructor is not allowed.  Triggered by: DemoController
               // b(std::move(a)); or returning by value; return object by value
  DemoController& operator=(DemoController&&) =
      delete;  // Move assignment operator is not allowed. Triggered by: a =
               // std::move(b); assigning from temporary or function return

  /* C++11 move semantics: The `&&` Syntax
        - `&` = lvalue reference (refers to a named object that has a persistent
                address)
        - `&&` = rvalue reference (refers to a temporary object about to be
                 destroyed)*/

  // Implement the interface methods
  Response HandleRequest(const Request& request) override;
  std::vector<std::string> GetAvailableCommands() const override;

 private:
  // Command handler type
  using CommandHandler =
      std::function<Response(const std::vector<std::string>&)>;

  struct CommandData {
    std::string description;
    CommandHandler handler;
  };

  // Map of command names to handler functions
  std::unordered_map<std::string, CommandData> command_handlers_;

  // Storage for text items
  std::vector<std::string> items_;

  // Individual command handlers
  Response HandleAdd(const std::vector<std::string>& args);
  Response HandleDelete(const std::vector<std::string>& args);
  Response HandleList(const std::vector<std::string>& args);

  // Helper to register commands
  void RegisterCommand(const std::string& name, const std::string& description,
                       CommandHandler handler);
};

}  // namespace presenter

#endif  // DEMO_CONTROLLER_H_
