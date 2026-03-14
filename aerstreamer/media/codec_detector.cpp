#include "media/codec_detector.h"

namespace aerstreamer::media {

CodecType CodecDetector::detect(const std::vector<uint8_t>& packet) {
  if (packet.size() < 4) {
    return CodecType::UNKNOWN;
  }

  // Lightweight heuristic for incoming RTP payload preambles.
  if ((packet[0] & 0x1F) == 7 || (packet[0] & 0x1F) == 5) {
    return CodecType::H264;
  }
  if (((packet[0] >> 1) & 0x3F) == 32 || ((packet[0] >> 1) & 0x3F) == 19) {
    return CodecType::H265;
  }
  if (packet[0] == 0xFF && packet[1] == 0xD8) {
    return CodecType::MJPEG;
  }
  return CodecType::UNKNOWN;
}

std::string CodecDetector::toString(CodecType codec) {
  switch (codec) {
    case CodecType::H264:
      return "H264";
    case CodecType::H265:
      return "H265";
    case CodecType::MJPEG:
      return "MJPEG";
    default:
      return "UNKNOWN";
  }
}

}  // namespace aerstreamer::media
