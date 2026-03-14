#pragma once

#include <optional>
#include <string>

#include <nlohmann/json.hpp>

#include "core/session_manager.h"
#include "recording/stream_recorder.h"
#include "storage/upload_worker.h"

namespace aerstreamer::core {

class RecordingManager {
 public:
  RecordingManager(SessionManager& sessions, recording::StreamRecorder& recorder,
                   storage::UploadWorker& uploader, std::string base_dir);

  bool startRecording(const std::string& stream_name, const std::string& bucket_name,
                      const std::string& folder_path, const nlohmann::json& metadata);
  bool stopRecording(const std::string& stream_name);

 private:
  struct Target {
    std::string bucket_name;
    std::string folder_path;
  };

  std::string makeRecordingId();

  SessionManager& sessions_;
  recording::StreamRecorder& recorder_;
  storage::UploadWorker& uploader_;
  std::string base_dir_;
  std::mutex mutex_;
  std::unordered_map<std::string, Target> upload_targets_;
};

}  // namespace aerstreamer::core
