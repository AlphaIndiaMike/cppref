// httplib_network_connector.h
#ifndef GATEWAYS_NETWORK_HTTPLIB_NETWORK_CONNECTOR_H_
#define GATEWAYS_NETWORK_HTTPLIB_NETWORK_CONNECTOR_H_

#include <memory>
#include <string>

// #define CPPHTTPLIB_OPENSSL_SUPPORT

#include "httplib.h"
#include "network_connector.h"

namespace Gateways::Network {

// ============================================================
// HttplibResponse
// ============================================================

class HttplibResponse : public IHttpResponse {
 public:
  HttplibResponse(int status_code, std::string body, Headers headers);
  ~HttplibResponse() override = default;

  int statusCode() const override;
  std::string body() const override;
  Headers headers() const override;
  std::string header(const std::string& name) const override;

 private:
  int status_code_;
  std::string body_;
  Headers headers_;
};

// ============================================================
// HttplibClient
// ============================================================

class HttplibClient : public IHttpClient {
 public:
  HttplibClient() = default;
  ~HttplibClient() override = default;

  HttplibClient(const HttplibClient&) = delete;
  HttplibClient& operator=(const HttplibClient&) = delete;
  HttplibClient(HttplibClient&&) = delete;
  HttplibClient& operator=(HttplibClient&&) = delete;

  // Configuration
  void setDefaultHeaders(const Headers& headers) override;
  void setConnectTimeout(int seconds) override;
  void setReadTimeout(int seconds) override;

  // HTTP methods
  std::unique_ptr<IHttpResponse> get(const std::string& url,
                                     const QueryParams& params = {}) override;

  std::unique_ptr<IHttpResponse> post(
      const std::string& url, const std::string& body,
      const std::string& content_type = "application/json") override;

 private:
  // Parse URL into scheme+host and path components
  struct ParsedUrl {
    std::string scheme_host;  // e.g., "https://www.ls-tc.de"
    std::string path;         // e.g., "/_rpc/json/instrument/chart/data"
  };

  static ParsedUrl parseUrl(const std::string& url);
  static std::string buildQueryString(const QueryParams& params);
  httplib::Headers toHttplibHeaders() const;

  Headers default_headers_;
  int connect_timeout_sec_ = 10;
  int read_timeout_sec_ = 30;
};

}  // namespace Gateways::Network

#endif  // GATEWAYS_NETWORK_HTTPLIB_NETWORK_CONNECTOR_H_
