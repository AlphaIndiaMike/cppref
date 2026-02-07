// i_network_data_repository.h
#ifndef I_NETWORK_DATA_REPOSITORY_H_
#define I_NETWORK_DATA_REPOSITORY_H_

#include <string>
#include <vector>

#include "entities.h"

namespace UseCases {

class INetworkDataRepository {
 public:
  virtual ~INetworkDataRepository() = default;

  // Fetch time series data for an asset from a network source
  virtual std::vector<Entities::TimeSeriesPoint> fetchTimeSeriesData(
      const std::string& instrument_id) = 0;
};

}  // namespace UseCases

#endif  // I_NETWORK_DATA_REPOSITORY_H_
