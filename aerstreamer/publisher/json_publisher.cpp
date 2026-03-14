#include <iostream>

#include <nlohmann/json.hpp>

int main() {
  nlohmann::json example = {
      {"stream_name", "telemetry_uav_17"},
      {"stream_type", "JSON_DATA"},
      {"payload", {{"lat", 12.34}, {"lon", 56.78}}},
  };
  std::cout << example.dump() << '\n';
  return 0;
}
