#pragma once

#include <array>
#include <functional>
#include <string>

#include <boost/asio.hpp>

namespace aerstreamer::network {

class RtpServer {
 public:
  using PacketCallback = std::function<void(const std::string&, std::vector<uint8_t>)>;

  RtpServer(boost::asio::io_context& io, const boost::asio::ip::udp::endpoint& endpoint,
            PacketCallback callback);
  void start();

 private:
  void doReceive();

  boost::asio::ip::udp::socket socket_;
  boost::asio::ip::udp::endpoint remote_;
  std::array<uint8_t, 65535> buffer_{};
  PacketCallback callback_;
};

}  // namespace aerstreamer::network
