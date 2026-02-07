// httplib_network_connector.cc
#include "httplib_network_connector.h"

#include <sstream>

namespace Gateways::Network {

// ============================================================
// HttplibResponse
// ============================================================

HttplibResponse::HttplibResponse(int status_code, std::string body,
                                 Headers headers)
    : status_code_(status_code),
      body_(std::move(body)),
      headers_(std::move(headers)) {}

int HttplibResponse::statusCode() const { return status_code_; }

std::string HttplibResponse::body() const { return body_; }

Headers HttplibResponse::headers() const { return headers_; }

std::string HttplibResponse::header(const std::string& name) const {
  auto it = headers_.find(name);
  if (it == headers_.end()) return "";
  return it->second;
}

// ============================================================
// HttplibClient
// ============================================================

void HttplibClient::setDefaultHeaders(const Headers& headers) {
  default_headers_ = headers;
}

void HttplibClient::setConnectTimeout(int seconds) {
  connect_timeout_sec_ = seconds;
}

void HttplibClient::setReadTimeout(int seconds) { read_timeout_sec_ = seconds; }

std::unique_ptr<IHttpResponse> HttplibClient::get(const std::string& url,
                                                  const QueryParams& params) {
  auto parsed = parseUrl(url);

  httplib::Client client(parsed.scheme_host);
  client.set_connection_timeout(connect_timeout_sec_, 0);
  client.set_read_timeout(read_timeout_sec_, 0);

  auto hdrs = toHttplibHeaders();

  std::string path = parsed.path;
  if (!params.empty()) {
    path += "?" + buildQueryString(params);
  }

  auto result = client.Get(path, hdrs);

  if (!result) {
    auto err = result.error();
    if (err == httplib::Error::ConnectionTimeout ||
        err == httplib::Error::Timeout) {
      throw TimeoutException("Request timed out: " + url);
    }
    throw ConnectionException("Failed to connect: " + url);
  }

  if (result->status < 200 || result->status >= 300) {
    throw HttpException(result->status, result->body);
  }

  Headers response_headers;
  for (const auto& [key, value] : result->headers) {
    response_headers[key] = value;
  }

  return std::make_unique<HttplibResponse>(result->status, result->body,
                                           std::move(response_headers));
}

std::unique_ptr<IHttpResponse> HttplibClient::post(
    const std::string& url, const std::string& body,
    const std::string& content_type) {
  auto parsed = parseUrl(url);

  httplib::Client client(parsed.scheme_host);
  client.set_connection_timeout(connect_timeout_sec_, 0);
  client.set_read_timeout(read_timeout_sec_, 0);

  auto hdrs = toHttplibHeaders();

  auto result = client.Post(parsed.path, hdrs, body, content_type);

  if (!result) {
    auto err = result.error();
    if (err == httplib::Error::ConnectionTimeout ||
        err == httplib::Error::Timeout) {
      throw TimeoutException("Request timed out: " + url);
    }
    throw ConnectionException("Failed to connect: " + url);
  }

  if (result->status < 200 || result->status >= 300) {
    throw HttpException(result->status, result->body);
  }

  Headers response_headers;
  for (const auto& [key, value] : result->headers) {
    response_headers[key] = value;
  }

  return std::make_unique<HttplibResponse>(result->status, result->body,
                                           std::move(response_headers));
}

HttplibClient::ParsedUrl HttplibClient::parseUrl(const std::string& url) {
  ParsedUrl parsed;

  // Find scheme end
  auto scheme_end = url.find("://");
  if (scheme_end == std::string::npos) {
    throw NetworkException("Invalid URL (missing scheme): " + url);
  }

  // Find path start (first '/' after scheme)
  auto path_start = url.find('/', scheme_end + 3);
  if (path_start == std::string::npos) {
    parsed.scheme_host = url;
    parsed.path = "/";
  } else {
    parsed.scheme_host = url.substr(0, path_start);
    parsed.path = url.substr(path_start);
  }

  return parsed;
}

std::string HttplibClient::buildQueryString(const QueryParams& params) {
  std::ostringstream oss;
  bool first = true;
  for (const auto& [key, value] : params) {
    if (!first) oss << "&";
    oss << key << "=" << value;
    first = false;
  }
  return oss.str();
}

httplib::Headers HttplibClient::toHttplibHeaders() const {
  httplib::Headers hdrs;
  for (const auto& [key, value] : default_headers_) {
    hdrs.emplace(key, value);
  }
  return hdrs;
}

}  // namespace Gateways::Network
