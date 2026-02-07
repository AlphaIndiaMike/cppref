// network_connector.h
#ifndef GATEWAYS_NETWORK_NETWORK_CONNECTOR_H_
#define GATEWAYS_NETWORK_NETWORK_CONNECTOR_H_

#include <cstdint>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace Gateways::Network {

// ============================================================
// Type Definitions
// ============================================================

using Headers = std::map<std::string, std::string>;
using QueryParams = std::map<std::string, std::string>;

// ============================================================
// Exceptions
// ============================================================

class NetworkException : public std::runtime_error {
 public:
  explicit NetworkException(const std::string& msg) : std::runtime_error(msg) {}
};

class ConnectionException : public NetworkException {
 public:
  explicit ConnectionException(const std::string& msg)
      : NetworkException("Connection error: " + msg) {}
};

class HttpException : public NetworkException {
 public:
  HttpException(int status_code, const std::string& msg)
      : NetworkException("HTTP " + std::to_string(status_code) + ": " + msg),
        status_code_(status_code) {}

  int statusCode() const { return status_code_; }

 private:
  int status_code_;
};

class TimeoutException : public NetworkException {
 public:
  explicit TimeoutException(const std::string& msg)
      : NetworkException("Timeout: " + msg) {}
};

// ============================================================
// IHttpResponse Interface
// ============================================================

class IHttpResponse {
 public:
  virtual ~IHttpResponse() = default;

  virtual int statusCode() const = 0;
  virtual std::string body() const = 0;
  virtual Headers headers() const = 0;
  virtual std::string header(const std::string& name) const = 0;
};

// ============================================================
// IHttpClient Interface
// ============================================================

class IHttpClient {
 public:
  virtual ~IHttpClient() = default;

  // Configuration
  virtual void setDefaultHeaders(const Headers& headers) = 0;
  virtual void setConnectTimeout(int seconds) = 0;
  virtual void setReadTimeout(int seconds) = 0;

  // HTTP methods
  virtual std::unique_ptr<IHttpResponse> get(
      const std::string& url, const QueryParams& params = {}) = 0;

  virtual std::unique_ptr<IHttpResponse> post(
      const std::string& url, const std::string& body,
      const std::string& content_type = "application/json") = 0;
};

}  // namespace Gateways::Network

#endif  // GATEWAYS_NETWORK_NETWORK_CONNECTOR_H_
