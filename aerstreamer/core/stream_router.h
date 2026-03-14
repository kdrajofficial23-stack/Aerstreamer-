#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "recording/stream_recorder.h"

namespace aerstreamer::core {

class StreamRouter {
 public:
  explicit StreamRouter(recording::StreamRecorder& recorder);
  void onRtpPacket(const std::string& stream_name, const std::vector<uint8_t>& packet, bool recording_active);

 private:
  recording::StreamRecorder& recorder_;
};

}  // namespace aerstreamer::core
