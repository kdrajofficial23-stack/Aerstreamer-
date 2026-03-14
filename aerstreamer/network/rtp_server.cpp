#include "network/rtp_server.h"

#include <iostream>

namespace aerstreamer::network {

RtpServer::RtpServer(boost::asio::io_context& io, const boost::asio::ip::udp::endpoint& endpoint,
                     PacketCallback callback)
    : socket_(io, endpoint), callback_(std::move(callback)) {}

void RtpServer::start() { doReceive(); }

void RtpServer::doReceive() {
  socket_.async_receive_from(
      boost::asio::buffer(buffer_), remote_,
      [this](boost::system::error_code ec, std::size_t bytes) {
        if (!ec && bytes > 0) {
          // Protocol: first line is stream name, then binary payload.
          std::string stream_name = "default";
          std::size_t split = 0;
          while (split < bytes && buffer_[split] != '\n') ++split;
          if (split < bytes) {
            stream_name = std::string(reinterpret_cast<char*>(buffer_.data()), split);
            std::vector<uint8_t> payload(buffer_.begin() + static_cast<long>(split + 1),
                                         buffer_.begin() + static_cast<long>(bytes));
            callback_(stream_name, std::move(payload));
          }
        } else {
          std::cerr << "[RTP] receive error: " << ec.message() << "\n";
        }
        doReceive();
      });
}

}  // namespace aerstreamer::network
