// Interactive CLI Shell (REPL)
#ifndef PRESENTER_CLI_CLI_SHELL_H_
#define PRESENTER_CLI_CLI_SHELL_H_

#include <iostream>
#include <memory>
#include <string>

#include "req_response_interface.h"

namespace presenter {

// Interactive command-line shell
class CliShell {
 public:
  // Constructor with custom streams (for testing)
  explicit CliShell(std::istream& input = std::cin,
                    std::ostream& output = std::cout);

  // Disable copy and move
  CliShell(const CliShell&) = delete;
  CliShell& operator=(const CliShell&) = delete;
  CliShell(CliShell&&) = delete;
  CliShell& operator=(CliShell&&) = delete;

  ~CliShell() = default;

  // Set the controller for handling custom commands
  void SetController(std::shared_ptr<RequestResponseInterface> controller);

  // Run the interactive shell (returns exit code)
  int Run();

 private:
  std::istream& input_;
  std::ostream& output_;
  std::shared_ptr<RequestResponseInterface> controller_;

  // Built-in command handlers
  void ShowPrompt();
  void ShowHelp();
  bool ProcessCommand(const std::string& line);
  Request ParseLine(const std::string& line);
};

}  // namespace presenter

#endif  // PRESENTER_CLI_CLI_SHELL_H_
