#include "core/recording_manager.h"

#include <chrono>
#include <sstream>

namespace aerstreamer::core {

RecordingManager::RecordingManager(SessionManager& sessions, recording::StreamRecorder& recorder,
                                   storage::UploadWorker& uploader, std::string base_dir)
    : sessions_(sessions), recorder_(recorder), uploader_(uploader), base_dir_(std::move(base_dir)) {}

bool RecordingManager::startRecording(const std::string& stream_name, const std::string& bucket_name,
                                      const std::string& folder_path, const nlohmann::json& metadata) {
  auto stream = sessions_.getStream(stream_name);
  if (!stream) {
    return false;
  }

  const auto recording_id = makeRecordingId();
  const bool json_only = (stream->stream_type == StreamType::JSON_DATA);
  if (!recorder_.start(stream_name, recording_id, base_dir_, json_only)) {
    return false;
  }

  // Metadata is persisted along with stream jsonl so it can be reconstructed later.
  recorder_.writeJson(stream_name, nlohmann::json{{"recording_metadata", metadata}});
  sessions_.startRecording(stream_name, recording_id, (std::filesystem::path(base_dir_) / stream_name / recording_id).string());

  std::lock_guard lock(mutex_);
  upload_targets_[stream_name] = Target{bucket_name, folder_path};
  return true;
}

bool RecordingManager::stopRecording(const std::string& stream_name) {
  auto stream = sessions_.getStream(stream_name);
  if (!stream || !stream->active_recording_id.has_value()) {
    return false;
  }

  const auto local_dir = recorder_.stop(stream_name);

  Target target;
  {
    std::lock_guard lock(mutex_);
    auto it = upload_targets_.find(stream_name);
    if (it == upload_targets_.end()) return false;
    target = it->second;
  }

  uploader_.enqueue(storage::UploadTask{
      .stream_name = stream_name,
      .recording_id = *stream->active_recording_id,
      .bucket_name = target.bucket_name,
      .folder_path = target.folder_path,
      .local_dir = local_dir,
  });

  return sessions_.stopRecording(stream_name);
}

std::string RecordingManager::makeRecordingId() {
  const auto now = std::chrono::system_clock::now().time_since_epoch();
  const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
  std::ostringstream oss;
  oss << "rec_" << ms;
  return oss.str();
}

}  // namespace aerstreamer::core
