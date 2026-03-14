#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <nlohmann/json.hpp>

namespace websocket = boost::beast::websocket;
using tcp = boost::asio::ip::tcp;

int main(int argc, char** argv) {
  const std::string host = (argc > 1) ? argv[1] : "127.0.0.1";
  const std::string port = (argc > 2) ? argv[2] : "9001";
  const std::string stream_name = (argc > 3) ? argv[3] : "telemetry_uav_17";
  const int hz = (argc > 4) ? std::max(1, std::atoi(argv[4])) : 10;

  try {
    boost::asio::io_context ioc;
    tcp::resolver resolver{ioc};
    websocket::stream<tcp::socket> ws{ioc};

    auto const results = resolver.resolve(host, port);
    boost::asio::connect(ws.next_layer(), results.begin(), results.end());
    ws.handshake(host + ":" + port, "/");

    std::cout << "Connected JSON publisher to ws://" << host << ':' << port
              << " stream_name=" << stream_name << " rate=" << hz << "Hz\n";

    int seq = 0;
    const auto sleep_ms = std::chrono::milliseconds(1000 / hz);
    while (true) {
      const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                              std::chrono::system_clock::now().time_since_epoch())
                              .count();

      // Production-like telemetry envelope accepted by AerStreamer JSON ingest.
      nlohmann::json msg = {
          {"stream_name", stream_name},
          {"stream_type", "JSON_DATA"},
          {"publisher", "json_publisher_cpp"},
          {"timestamp_ms", now_ms},
          {"seq", seq++},
          {"payload",
           {
               {"vehicle_id", "uav-17"},
               {"lat", 24.7136 + (seq % 50) * 0.00001},
               {"lon", 46.6753 + (seq % 50) * 0.00001},
               {"alt_m", 121.5 + (seq % 10) * 0.1},
               {"ground_speed_mps", 15.2},
               {"heading_deg", (seq * 3) % 360},
               {"battery_pct", std::max(0, 95 - (seq / 40))},
           }},
      };

      ws.write(boost::asio::buffer(msg.dump()));
      std::this_thread::sleep_for(sleep_ms);
    }
  } catch (const std::exception& ex) {
    std::cerr << "json_publisher error: " << ex.what() << '\n';
    return 1;
  }
}
