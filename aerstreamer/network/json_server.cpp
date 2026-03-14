#include "network/json_server.h"

#include <iostream>

#include <boost/beast/core.hpp>

namespace aerstreamer::network {
namespace websocket = boost::beast::websocket;
using tcp = boost::asio::ip::tcp;

class JsonServer::Session : public std::enable_shared_from_this<JsonServer::Session> {
 public:
  Session(tcp::socket socket, JsonCallback cb) : ws_(std::move(socket)), callback_(std::move(cb)) {}

  void run() {
    ws_.async_accept([self = shared_from_this()](boost::system::error_code ec) {
      if (ec) return;
      self->readLoop();
    });
  }

 private:
  void readLoop() {
    ws_.async_read(buffer_, [self = shared_from_this()](boost::system::error_code ec, std::size_t) {
      if (ec) return;
      try {
        const auto data = boost::beast::buffers_to_string(self->buffer_.data());
        self->callback_(nlohmann::json::parse(data));
      } catch (const std::exception& ex) {
        std::cerr << "[JSON] parse error: " << ex.what() << "\n";
      }
      self->buffer_.consume(self->buffer_.size());
      self->readLoop();
    });
  }

  websocket::stream<tcp::socket> ws_;
  boost::beast::flat_buffer buffer_;
  JsonCallback callback_;
};

JsonServer::JsonServer(boost::asio::io_context& ioc, tcp::endpoint endpoint, JsonCallback cb)
    : acceptor_(ioc, endpoint), callback_(std::move(cb)) {}

void JsonServer::start() { doAccept(); }

void JsonServer::doAccept() {
  acceptor_.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
    if (!ec) {
      std::make_shared<Session>(std::move(socket), callback_)->run();
    }
    doAccept();
  });
}

}  // namespace aerstreamer::network
