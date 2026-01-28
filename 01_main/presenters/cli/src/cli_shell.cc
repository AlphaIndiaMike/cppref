#include "cli_shell.h"

#include <sstream>

namespace presenter {

// Constructor: Takes input/output streams
// Default = std::cin/cout for normal use
// Custom streams for testing (inject stringstreams)
CliShell::CliShell(std::istream& input, std::ostream& output)
    : input_(input), output_(output), controller_(nullptr) {
  // Member initializer list - efficient initialization
  // controller_ starts as nullptr (no controller yet)
}

// Set controller after construction (optional)
void CliShell::SetController(
    std::shared_ptr<RequestResponseInterface> controller) {
  controller_ = controller;
}

// Main REPL loop
int CliShell::Run() {
  output_ << "Welcome to Presenter CLI!\n";
  output_ << "Type 'help' for commands.\n\n";

  while (true) {
    ShowPrompt();

    std::string line;
    if (!std::getline(input_, line)) {
      break;  // EOF or error
    }

    if (!ProcessCommand(line)) {
      break;  // User typed quit/exit
    }
  }

  output_ << "Goodbye!\n";
  return 0;
}

void CliShell::ShowPrompt() {
  output_ << ">> ";
  output_.flush();  // Force output immediately
}

// Returns false = exit loop, true = continue
bool CliShell::ProcessCommand(const std::string& line) {
  // Ignore empty lines
  if (line.empty()) {
    return true;
  }

  // Parse command and arguments
  Request request = ParseLine(line);

  // Built-in commands
  if (request.command == "quit" || request.command == "exit") {
    return false;  // Stop loop
  }

  if (request.command == "help") {
    ShowHelp();
    return true;
  }

  // Custom commands via controller
  if (controller_) {
    Response response = controller_->HandleRequest(request);
    output_ << response.message << "\n";
  } else {
    output_ << "Command not implemented: " << request.command << "\n";
  }

  return true;
}

Request CliShell::ParseLine(const std::string& line) {
  Request req;
  std::istringstream iss(line);

  // First word = command
  iss >> req.command;

  // Rest = arguments
  std::string arg;
  while (iss >> arg) {
    req.arguments.push_back(arg);
  }

  return req;
}

void CliShell::ShowHelp() {
  output_ << "Available commands:\n";
  output_ << "  help - Show this message\n";
  output_ << "  quit - Exit shell\n";
  output_ << "  exit - Exit shell\n";

  if (controller_) {
    auto commands = controller_->GetAvailableCommands();
    for (const auto& cmd : commands) {
      output_ << "  " << cmd << "\n";
    }
  }
}

}  // namespace presenter
