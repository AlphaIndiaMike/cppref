// Copyright 2025
// Request-Response Interface for command handling
// Following Google C++ Style Guide

#ifndef PRESENTER_CLI_REQUEST_RESPONSE_INTERFACE_H_
#define PRESENTER_CLI_REQUEST_RESPONSE_INTERFACE_H_

#include <string>
#include <vector>

namespace presenter {

// Request structure for commands
struct Request {
  std::string command;
  std::vector<std::string> arguments;
};

// Response structure from command execution
struct Response {
  bool success = false;
  std::string message;
};

// Virtual interface for command controllers
class RequestResponseInterface {
 public:
  virtual ~RequestResponseInterface() = default;

  // Handle a command request
  virtual Response HandleRequest(const Request& request) = 0;

  // Get list of available commands
  virtual std::vector<std::string> GetAvailableCommands() const = 0;
};

}  // namespace presenter

#endif  // PRESENTER_CLI_REQUEST_RESPONSE_INTERFACE_H_
