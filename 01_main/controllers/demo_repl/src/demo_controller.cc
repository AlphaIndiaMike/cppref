#include "demo_controller.h"

#include <sstream>

namespace presenter {

DemoController::DemoController() {
  // Register all available commands
  RegisterCommand("add", "Add new text item",
                  [this](const auto& args) { return HandleAdd(args); });
  RegisterCommand("delete", "Delete all text items",
                  [this](const auto& args) { return HandleDelete(args); });
  RegisterCommand("list", "List all text items",
                  [this](const auto& args) { return HandleList(args); });
}

Response DemoController::HandleRequest(const Request& request) {
  // Find the command handler
  auto it = command_handlers_.find(request.command);

  if (it == command_handlers_.end()) {
    return Response{false, "Unknown command: " + request.command};
  }

  // Execute the handler
  return it->second.handler(request.arguments);
}

std::vector<std::string> DemoController::GetAvailableCommands() const {
  std::vector<std::string> commands;
  for (const auto& [name, handler] : command_handlers_) {
    std::string formatted = name + " - " + handler.description;
    commands.push_back(formatted);
  }
  return commands;
}

void DemoController::RegisterCommand(const std::string& name,
                                     const std::string& description,
                                     CommandHandler handler) {
  command_handlers_[name] = {description, std::move(handler)};
}

Response DemoController::HandleAdd(const std::vector<std::string>& args) {
  if (args.empty()) {
    return Response{false, "Add requires at least one argument"};
  }

  // Join all arguments into a single string
  std::ostringstream oss;
  for (size_t i = 0; i < args.size(); ++i) {
    if (i > 0) oss << " ";
    oss << args[i];
  }

  std::string item = oss.str();
  items_.push_back(item);

  return Response{true, "Added: \"" + item + "\""};
}

Response DemoController::HandleDelete(const std::vector<std::string>& args) {
  size_t count = items_.size();
  items_.clear();

  return Response{true, "Deleted " + std::to_string(count) + " item(s)"};
}

Response DemoController::HandleList(const std::vector<std::string>& args) {
  if (items_.empty()) {
    return Response{true, "No items stored"};
  }

  std::ostringstream oss;
  oss << "Stored items (" << items_.size() << "):\n";

  for (size_t i = 0; i < items_.size(); ++i) {
    oss << "  " << (i + 1) << ". " << items_[i] << "\n";
  }

  return Response{true, oss.str()};
}

}  // namespace presenter
