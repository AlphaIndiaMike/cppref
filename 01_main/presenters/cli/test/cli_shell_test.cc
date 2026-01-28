// Copyright 2025
// Tests for interactive CLI shell
// Following Google C++ Style Guide

#include "cli_shell.h"

#include <gtest/gtest.h>

#include <sstream>
#include <string>

#include "req_response_interface.h"

namespace presenter {
namespace {

// ============================================================================
// Test Fixtures
// ============================================================================

class CliShellTest : public ::testing::Test {
 protected:
  void SetUp() override {
    input_stream_.str("");
    output_stream_.str("");
  }

  std::istringstream input_stream_;
  std::ostringstream output_stream_;
};

// ============================================================================
// Basic Shell Tests
// ============================================================================

TEST_F(CliShellTest, ShowsPrompt) {
  input_stream_.str("quit\n");

  CliShell shell(input_stream_, output_stream_);
  shell.Run();

  std::string output = output_stream_.str();
  EXPECT_TRUE(output.find(">>") != std::string::npos);
}

TEST_F(CliShellTest, ExitsOnQuitCommand) {
  input_stream_.str("quit\n");

  CliShell shell(input_stream_, output_stream_);
  int exit_code = shell.Run();

  EXPECT_EQ(exit_code, 0);
}

TEST_F(CliShellTest, ExitsOnExitCommand) {
  input_stream_.str("exit\n");

  CliShell shell(input_stream_, output_stream_);
  int exit_code = shell.Run();

  EXPECT_EQ(exit_code, 0);
}

TEST_F(CliShellTest, ShowsHelpCommand) {
  input_stream_.str("help\nquit\n");

  CliShell shell(input_stream_, output_stream_);
  shell.Run();

  std::string output = output_stream_.str();
  EXPECT_TRUE(output.find("help") != std::string::npos);
  EXPECT_TRUE(output.find("quit") != std::string::npos);
}

TEST_F(CliShellTest, HandlesEmptyInput) {
  input_stream_.str("\n\nquit\n");

  CliShell shell(input_stream_, output_stream_);
  int exit_code = shell.Run();

  EXPECT_EQ(exit_code, 0);
}

TEST_F(CliShellTest, ShowsUnknownCommandMessage) {
  input_stream_.str("unknown_command\nquit\n");

  CliShell shell(input_stream_, output_stream_);
  shell.Run();

  std::string output = output_stream_.str();
  EXPECT_TRUE(output.find("Unknown command") != std::string::npos ||
              output.find("not implemented") != std::string::npos);
}

// ============================================================================
// Controller Integration Tests
// ============================================================================

// Mock controller for testing
class MockController : public RequestResponseInterface {
 public:
  Response HandleRequest(const Request& request) override {
    if (request.command == "test") {
      Response resp;
      resp.success = true;
      resp.message = "Test command executed";
      return resp;
    }

    Response resp;
    resp.success = false;
    resp.message = "Command not implemented";
    return resp;
  }

  std::vector<std::string> GetAvailableCommands() const override {
    return {"test"};
  }
};

TEST_F(CliShellTest, ExecutesCustomCommandThroughController) {
  input_stream_.str("test\nquit\n");

  auto controller = std::make_shared<MockController>();
  CliShell shell(input_stream_, output_stream_);
  shell.SetController(controller);
  shell.Run();

  std::string output = output_stream_.str();
  EXPECT_TRUE(output.find("Test command executed") != std::string::npos);
}

TEST_F(CliShellTest, WorksWithoutController) {
  input_stream_.str("custom_command\nquit\n");

  CliShell shell(input_stream_, output_stream_);
  shell.Run();

  std::string output = output_stream_.str();
  EXPECT_TRUE(output.find("not implemented") != std::string::npos);
}

TEST_F(CliShellTest, HelpShowsControllerCommands) {
  input_stream_.str("help\nquit\n");

  auto controller = std::make_shared<MockController>();
  CliShell shell(input_stream_, output_stream_);
  shell.SetController(controller);
  shell.Run();

  std::string output = output_stream_.str();
  EXPECT_TRUE(output.find("test") != std::string::npos);
}

}  // namespace
}  // namespace presenter
