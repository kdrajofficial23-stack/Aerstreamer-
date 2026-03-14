#include "web_gateway/http_server.h"

#include <boost/beast/core.hpp>
#include <nlohmann/json.hpp>

namespace aerstreamer::web_gateway {
namespace http = boost::beast::http;
using tcp = boost::asio::ip::tcp;

class HttpServer::Session : public std::enable_shared_from_this<HttpServer::Session> {
 public:
  Session(tcp::socket socket, HttpServer& server) : socket_(std::move(socket)), server_(server) {}

  void run() { read(); }

 private:
  void read() {
    http::async_read(socket_, buffer_, req_, [self = shared_from_this()](boost::system::error_code ec, std::size_t) {
      if (ec) return;
      self->write();
    });
  }

  void write() {
    auto response = std::make_shared<http::response<http::string_body>>(server_.handleRequest(req_));
    http::async_write(socket_, *response,
                      [self = shared_from_this(), response](boost::system::error_code, std::size_t) {
                        self->socket_.shutdown(tcp::socket::shutdown_send);
                      });
  }

  tcp::socket socket_;
  boost::beast::flat_buffer buffer_;
  http::request<http::string_body> req_;
  HttpServer& server_;
};

HttpServer::HttpServer(boost::asio::io_context& ioc, tcp::endpoint endpoint,
                       aerstreamer::core::SessionManager& sessions,
                       aerstreamer::core::RecordingManager& recordings)
    : acceptor_(ioc, endpoint), sessions_(sessions), recordings_(recordings) {}

void HttpServer::start() { doAccept(); }

void HttpServer::doAccept() {
  acceptor_.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
    if (!ec) {
      std::make_shared<Session>(std::move(socket), *this)->run();
    }
    doAccept();
  });
}

http::response<http::string_body> HttpServer::handleRequest(const http::request<http::string_body>& req) {
  http::response<http::string_body> res{http::status::ok, req.version()};
  res.set(http::field::content_type, "application/json");

  if (req.method() == http::verb::get && req.target() == "/streams") {
    nlohmann::json payload = nlohmann::json::array();
    for (const auto& stream : sessions_.listStreams()) {
      payload.push_back(aerstreamer::core::SessionManager::toJson(stream));
    }
    res.body() = payload.dump();
  } else if (req.method() == http::verb::post && req.target() == "/record/start") {
    const auto body = nlohmann::json::parse(req.body(), nullptr, false);
    if (body.is_discarded()) {
      res.result(http::status::bad_request);
      res.body() = R"({"error":"invalid json"})";
    } else {
      const bool ok = recordings_.startRecording(body.value("stream_name", ""), body.value("bucket_name", ""),
                                                 body.value("folder_path", ""), body.value("metadata", nlohmann::json::object()));
      res.result(ok ? http::status::ok : http::status::bad_request);
      res.body() = nlohmann::json{{"ok", ok}}.dump();
    }
  } else if (req.method() == http::verb::post && req.target() == "/record/stop") {
    const auto body = nlohmann::json::parse(req.body(), nullptr, false);
    if (body.is_discarded()) {
      res.result(http::status::bad_request);
      res.body() = R"({"error":"invalid json"})";
    } else {
      const bool ok = recordings_.stopRecording(body.value("stream_name", ""));
      res.result(ok ? http::status::ok : http::status::bad_request);
      res.body() = nlohmann::json{{"ok", ok}}.dump();
    }
  } else {
    res.result(http::status::not_found);
    res.body() = R"({"error":"not found"})";
  }

  res.prepare_payload();
  return res;
}

}  // namespace aerstreamer::web_gateway
