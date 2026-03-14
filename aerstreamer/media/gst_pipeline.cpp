#include "media/gst_pipeline.h"

#include <iostream>

namespace aerstreamer::media {

bool GstPipeline::start(const std::string& stream_name, const std::string& output_file) {
#if AERSTREAMER_HAS_GSTREAMER
  std::cout << "[GST] starting pipeline for " << stream_name << " -> " << output_file << "\n";
#else
  std::cout << "[GST] disabled at compile-time; using placeholder recorder for " << stream_name
            << "\n";
#endif
  return true;
}

void GstPipeline::stop(const std::string& stream_name) {
  std::cout << "[GST] stopping pipeline for " << stream_name << "\n";
}

}  // namespace aerstreamer::media
