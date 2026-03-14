#pragma once

#include <string>

namespace aerstreamer::media {

class GstPipeline {
 public:
  bool start(const std::string& stream_name, const std::string& output_file);
  void stop(const std::string& stream_name);
};

}  // namespace aerstreamer::media
