#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace aerstreamer::media {

enum class CodecType { H264, H265, MJPEG, UNKNOWN };

class CodecDetector {
 public:
  static CodecType detect(const std::vector<uint8_t>& packet);
  static std::string toString(CodecType codec);
};

}  // namespace aerstreamer::media
