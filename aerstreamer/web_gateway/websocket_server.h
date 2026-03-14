#pragma once

#include <mutex>
#include <string>
#include <vector>

#include <boost/asio.hpp>
#include <boost/beast/websocket.hpp>
#include <nlohmann/json.hpp>

namespace aerstreamer::web_gateway {

class WebsocketServer {
 public:
  explicit WebsocketServer(boost::asio::io_context& ioc);

  void publish(const std::string& stream_name, const nlohmann::json& payload);

 private:
  boost::asio::io_context& ioc_;
  std::mutex mutex_;
  std::vector<std::weak_ptr<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>>> clients_;
};

}  // namespace aerstreamer::web_gateway
