#include "core/session_manager.h"

#include <algorithm>

namespace aerstreamer::core {

void SessionManager::upsertStream(const std::string& stream_name, StreamType stream_type,
                                  bool publisher_status) {
  std::unique_lock lock(mutex_);
  auto& stream = sessions_[stream_name];
  stream.stream_name = stream_name;
  stream.stream_type = stream_type;
  stream.publisher_status = publisher_status;
  stream.last_seen_timestamp = std::chrono::system_clock::now();
}

void SessionManager::setPublisherStatus(const std::string& stream_name, bool active) {
  std::unique_lock lock(mutex_);
  auto it = sessions_.find(stream_name);
  if (it == sessions_.end()) {
    return;
  }
  it->second.publisher_status = active;
  it->second.last_seen_timestamp = std::chrono::system_clock::now();
}

void SessionManager::addViewer(const std::string& stream_name, const std::string& viewer_name) {
  std::unique_lock lock(mutex_);
  auto it = sessions_.find(stream_name);
  if (it == sessions_.end()) {
    return;
  }
  auto& viewers = it->second.viewer_list;
  if (std::find(viewers.begin(), viewers.end(), viewer_name) == viewers.end()) {
    viewers.push_back(viewer_name);
  }
}

void SessionManager::removeViewer(const std::string& stream_name, const std::string& viewer_name) {
  std::unique_lock lock(mutex_);
  auto it = sessions_.find(stream_name);
  if (it == sessions_.end()) {
    return;
  }
  auto& viewers = it->second.viewer_list;
  viewers.erase(std::remove(viewers.begin(), viewers.end(), viewer_name), viewers.end());
}

bool SessionManager::startRecording(const std::string& stream_name, const std::string& recording_id,
                                    const std::string& local_dir) {
  std::unique_lock lock(mutex_);
  auto it = sessions_.find(stream_name);
  if (it == sessions_.end()) {
    return false;
  }
  it->second.recording_state = true;
  it->second.active_recording_id = recording_id;
  it->second.local_recording_directory = local_dir;
  return true;
}

bool SessionManager::stopRecording(const std::string& stream_name) {
  std::unique_lock lock(mutex_);
  auto it = sessions_.find(stream_name);
  if (it == sessions_.end()) {
    return false;
  }
  it->second.recording_state = false;
  it->second.active_recording_id.reset();
  return true;
}

std::optional<StreamSession> SessionManager::getStream(const std::string& stream_name) const {
  std::shared_lock lock(mutex_);
  auto it = sessions_.find(stream_name);
  if (it == sessions_.end()) {
    return std::nullopt;
  }
  return it->second;
}

std::vector<StreamSession> SessionManager::listStreams() const {
  std::shared_lock lock(mutex_);
  std::vector<StreamSession> streams;
  streams.reserve(sessions_.size());
  for (const auto& [_, stream] : sessions_) {
    streams.push_back(stream);
  }
  return streams;
}

std::string SessionManager::streamTypeToString(StreamType type) {
  switch (type) {
    case StreamType::RGB_VIDEO:
      return "RGB_VIDEO";
    case StreamType::THERMAL_VIDEO:
      return "THERMAL_VIDEO";
    case StreamType::JSON_DATA:
      return "JSON_DATA";
    default:
      return "UNKNOWN";
  }
}

StreamType SessionManager::streamTypeFromString(const std::string& type) {
  if (type == "RGB_VIDEO") return StreamType::RGB_VIDEO;
  if (type == "THERMAL_VIDEO") return StreamType::THERMAL_VIDEO;
  if (type == "JSON_DATA") return StreamType::JSON_DATA;
  return StreamType::UNKNOWN;
}

nlohmann::json SessionManager::toJson(const StreamSession& session) {
  return {
      {"stream_name", session.stream_name},
      {"stream_type", streamTypeToString(session.stream_type)},
      {"publisher_status", session.publisher_status},
      {"viewer_list", session.viewer_list},
      {"recording_state", session.recording_state},
      {"active_recording_id", session.active_recording_id.value_or("")},
      {"local_recording_directory", session.local_recording_directory},
      {"last_seen_timestamp",
       std::chrono::duration_cast<std::chrono::milliseconds>(
           session.last_seen_timestamp.time_since_epoch())
           .count()},
  };
}

}  // namespace aerstreamer::core
