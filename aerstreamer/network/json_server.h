#pragma once

#include <functional>
#include <memory>
#include <string>

#include <boost/asio.hpp>
#include <boost/beast/websocket.hpp>
#include <nlohmann/json.hpp>

namespace aerstreamer::network {

class JsonServer {
 public:
  using JsonCallback = std::function<void(const nlohmann::json&)>;

  JsonServer(boost::asio::io_context& ioc, boost::asio::ip::tcp::endpoint endpoint, JsonCallback cb);
  void start();

 private:
  class Session;
  void doAccept();

  boost::asio::ip::tcp::acceptor acceptor_;
  JsonCallback callback_;
};

}  // namespace aerstreamer::network
