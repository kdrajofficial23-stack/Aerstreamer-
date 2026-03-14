#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

int main() {
  const std::string host = std::getenv("AER_HOST") ? std::getenv("AER_HOST") : "127.0.0.1";
  const std::string port = std::getenv("AER_RTP_PORT") ? std::getenv("AER_RTP_PORT") : "5004";
  const std::string device = std::getenv("CAMERA_DEVICE") ? std::getenv("CAMERA_DEVICE") : "/dev/video0";

  std::ostringstream cmd;
  cmd << "gst-launch-1.0 -e v4l2src device=" << device
      << " ! videoconvert ! x264enc tune=zerolatency speed-preset=ultrafast bitrate=2500 "
      << "! h264parse ! rtpgstpay ! udpsink host=" << host << " port=" << port;

  std::cout << "Launching RGB publisher:\n" << cmd.str() << '\n';
  return std::system(cmd.str().c_str());
}
