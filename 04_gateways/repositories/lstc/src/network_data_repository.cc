// network_data_repository.cc
#include "network_data_repository.h"

#include <algorithm>
#include <stdexcept>

#include "json.hpp"

namespace Gateways::Repositories::Network {

LsTcRepository::LsTcRepository(Gateways::Network::IHttpClient& client)
    : m_client(client) {
  m_client.setDefaultHeaders({
      {"User-Agent",
       "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36"},
      {"Accept", "application/json, text/plain, */*"},
      {"Accept-Language", "en-US,en;q=0.9"},
  });
  m_client.setConnectTimeout(10);
  m_client.setReadTimeout(30);
}

std::vector<Entities::TimeSeriesPoint> LsTcRepository::fetchTimeSeriesData(
    const std::string& instrument_id) {
  auto params = buildQueryParams(instrument_id);

  try {
    auto response = m_client.get(kBaseUrl, params);
    return parseResponse(instrument_id, response->body());
  } catch (const Gateways::Network::NetworkException&) {
    throw;  // Let network exceptions propagate as-is
  } catch (const std::exception& e) {
    throw Gateways::Network::NetworkException(
        "Failed to fetch data for instrument: " + instrument_id + " - " +
        e.what());
  }
}

Gateways::Network::QueryParams LsTcRepository::buildQueryParams(
    const std::string& instrument_id) const {
  return {
      {"instrumentId", instrument_id},  {"marketId", kDefaultMarketId},
      {"quotetype", kDefaultQuoteType}, {"series", kDefaultSeries},
      {"localeId", kDefaultLocaleId},
  };
}

std::vector<Entities::TimeSeriesPoint> LsTcRepository::parseResponse(
    const std::string& instrument_id, const std::string& json_body) const {
  auto json = nlohmann::json::parse(json_body);

  auto& data = json.at("series").at("history").at("data");

  std::vector<Entities::TimeSeriesPoint> points;
  points.reserve(data.size());

  for (const auto& data_point : data) {
    // Each data_point is [timestamp, price]
    if (data_point.size() < 2) continue;

    Entities::TimeSeriesPoint point;
    point.asset_id = instrument_id;
    point.timestamp_ms = data_point[0].get<int64_t>() * 1000;  // s -> ms
    point.unit_id = "";  // Not provided by this API
    point.value = data_point[1].get<double>();

    points.push_back(std::move(point));
  }

  std::sort(points.begin(), points.end(),
            [](const Entities::TimeSeriesPoint& a,
               const Entities::TimeSeriesPoint& b) {
              return a.timestamp_ms < b.timestamp_ms;
            });

  return points;
}

}  // namespace Gateways::Repositories::Network
