#include <memory>

#include "cli_shell.h"
#include "demo_controller.h"

int main() {
  presenter::CliShell shell;
  presenter::DemoController ctl;  // Create on stack
  shell.SetController(std::make_shared<presenter::DemoController>());
  return shell.Run();
}
