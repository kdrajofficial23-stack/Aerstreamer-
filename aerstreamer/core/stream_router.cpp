#include "core/stream_router.h"

namespace aerstreamer::core {

StreamRouter::StreamRouter(recording::StreamRecorder& recorder) : recorder_(recorder) {}

void StreamRouter::onRtpPacket(const std::string& stream_name, const std::vector<uint8_t>& packet,
                               bool recording_active) {
  if (recording_active) {
    recorder_.writePacket(stream_name, packet);
  }
}

}  // namespace aerstreamer::core
