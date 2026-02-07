// network_data_repository.h
#ifndef REPOSITORIES_NETWORK_DATA_REPOSITORY_H_
#define REPOSITORIES_NETWORK_DATA_REPOSITORY_H_

#include <string>
#include <vector>

#include "entities.h"
#include "network_connector.h"
#ifndef UNIT_TEST
#include "i_network_data_repository.h"
#define OVERRIDE override
#define USE_CASE : public UseCases::INetworkDataRepository
#else
#define OVERRIDE
#define USE_CASE
#endif

namespace Gateways::Repositories::Network {

class LsTcRepository USE_CASE {
 public:
  explicit LsTcRepository(Gateways::Network::IHttpClient& client);

  std::vector<Entities::TimeSeriesPoint> fetchTimeSeriesData(
      const std::string& instrument_id) OVERRIDE;

 private:
  // Build query params for the ls-tc.de API
  Gateways::Network::QueryParams buildQueryParams(
      const std::string& instrument_id) const;

  // Parse JSON response body into entities
  std::vector<Entities::TimeSeriesPoint> parseResponse(
      const std::string& instrument_id, const std::string& json_body) const;

  Gateways::Network::IHttpClient& m_client;

  static constexpr const char* kBaseUrl =
      "https://www.ls-tc.de/_rpc/json/instrument/chart/dataForInstrument";
  static constexpr const char* kDefaultMarketId = "1";
  static constexpr const char* kDefaultQuoteType = "last";
  static constexpr const char* kDefaultSeries = "intraday";
  static constexpr const char* kDefaultLocaleId = "2";
};

}  // namespace Gateways::Repositories::Network

#endif  // REPOSITORIES_NETWORK_DATA_REPOSITORY_H_
