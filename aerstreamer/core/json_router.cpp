#include "core/json_router.h"

namespace aerstreamer::core {

JsonRouter::JsonRouter(recording::StreamRecorder& recorder) : recorder_(recorder) {}

void JsonRouter::addViewerSink(ViewerCallback cb) { sinks_.push_back(std::move(cb)); }

void JsonRouter::route(const std::string& stream_name, const nlohmann::json& payload,
                       bool recording_active) {
  if (recording_active) {
    recorder_.writeJson(stream_name, payload);
  }

  for (const auto& sink : sinks_) {
    sink(stream_name, payload);
  }
}

}  // namespace aerstreamer::core
