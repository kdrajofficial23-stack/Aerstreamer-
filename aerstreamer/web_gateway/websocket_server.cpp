#include "web_gateway/websocket_server.h"

namespace aerstreamer::web_gateway {

WebsocketServer::WebsocketServer(boost::asio::io_context& ioc) : ioc_(ioc) {}

void WebsocketServer::publish(const std::string& stream_name, const nlohmann::json& payload) {
  const auto message = nlohmann::json{{"stream_name", stream_name}, {"payload", payload}}.dump();

  std::lock_guard lock(mutex_);
  for (auto it = clients_.begin(); it != clients_.end();) {
    if (auto session = it->lock()) {
      boost::system::error_code ec;
      session->write(boost::asio::buffer(message), ec);
      ++it;
    } else {
      it = clients_.erase(it);
    }
  }
}

}  // namespace aerstreamer::web_gateway
