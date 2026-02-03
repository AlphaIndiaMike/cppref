#ifndef DOMAIN_ENTITIES_H_
#define DOMAIN_ENTITIES_H_

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace Entities {

// ============================================================
// Time Series Entities
// ============================================================

struct Asset {
  std::string id;  // UUID
  std::string name;
  std::string description;
  std::string source;
};

struct Unit {
  std::string id;      // e.g., "degC", "EUR", "USD"
  std::string symbol;  // e.g., "°C", "€", "$"
  std::string name;    // e.g., "Degrees Celsius", "Euro"
};

struct UnitConversion {
  std::string from_unit_id;
  std::string to_unit_id;
  double factor;  // to = from * factor
};

struct TimeSeriesPoint {
  std::string asset_id;
  int64_t timestamp_ms;  // Unix milliseconds
  std::string unit_id;
  double value;
};

// ============================================================
// Key-Value Storage Entities
// ============================================================

struct Setting {
  std::string key;
  std::string value;
  std::optional<std::string> description;
};

// ============================================================
// Account Entities
// ============================================================

struct Account {
  std::string id;  // UUID or user-provided
  std::string name;
  std::optional<std::vector<uint8_t>> password_hash;
  int64_t created_at;  // Unix milliseconds
};

struct AccountProperty {
  std::string account_id;
  std::string key;
  std::string value;
  std::optional<std::string> description;
};

}  // namespace Entities

#endif  // DOMAIN_ENTITIES_H_
