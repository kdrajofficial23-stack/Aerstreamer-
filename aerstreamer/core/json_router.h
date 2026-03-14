#pragma once

#include <functional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "recording/stream_recorder.h"

namespace aerstreamer::core {

class JsonRouter {
 public:
  using ViewerCallback = std::function<void(const std::string&, const nlohmann::json&)>;

  explicit JsonRouter(recording::StreamRecorder& recorder);
  void addViewerSink(ViewerCallback cb);
  void route(const std::string& stream_name, const nlohmann::json& payload, bool recording_active);

 private:
  recording::StreamRecorder& recorder_;
  std::vector<ViewerCallback> sinks_;
};

}  // namespace aerstreamer::core
