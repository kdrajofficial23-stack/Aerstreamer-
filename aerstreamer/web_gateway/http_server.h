#pragma once

#include <memory>

#include <boost/asio.hpp>
#include <boost/beast/http.hpp>

#include "core/recording_manager.h"
#include "core/session_manager.h"

namespace aerstreamer::web_gateway {

class HttpServer {
 public:
  HttpServer(boost::asio::io_context& ioc, boost::asio::ip::tcp::endpoint endpoint,
             aerstreamer::core::SessionManager& sessions,
             aerstreamer::core::RecordingManager& recordings);
  void start();

 private:
  class Session;
  boost::beast::http::response<boost::beast::http::string_body> handleRequest(
      const boost::beast::http::request<boost::beast::http::string_body>& req);
  void doAccept();

  boost::asio::ip::tcp::acceptor acceptor_;
  aerstreamer::core::SessionManager& sessions_;
  aerstreamer::core::RecordingManager& recordings_;
};

}  // namespace aerstreamer::web_gateway
