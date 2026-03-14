#pragma once

#include <chrono>
#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>

namespace aerstreamer::core {

enum class StreamType { RGB_VIDEO, THERMAL_VIDEO, JSON_DATA, UNKNOWN };

struct StreamSession {
  std::string stream_name;
  StreamType stream_type{StreamType::UNKNOWN};
  bool publisher_status{false};
  std::vector<std::string> viewer_list;
  bool recording_state{false};
  std::chrono::system_clock::time_point last_seen_timestamp{};
  std::optional<std::string> active_recording_id;
  std::string local_recording_directory;
};

class SessionManager {
 public:
  void upsertStream(const std::string& stream_name, StreamType stream_type, bool publisher_status);
  void setPublisherStatus(const std::string& stream_name, bool active);
  void addViewer(const std::string& stream_name, const std::string& viewer_name);
  void removeViewer(const std::string& stream_name, const std::string& viewer_name);
  bool startRecording(const std::string& stream_name, const std::string& recording_id,
                      const std::string& local_dir);
  bool stopRecording(const std::string& stream_name);
  std::optional<StreamSession> getStream(const std::string& stream_name) const;
  std::vector<StreamSession> listStreams() const;

  static std::string streamTypeToString(StreamType type);
  static StreamType streamTypeFromString(const std::string& type);
  static nlohmann::json toJson(const StreamSession& session);

 private:
  mutable std::shared_mutex mutex_;
  std::unordered_map<std::string, StreamSession> sessions_;
};

}  // namespace aerstreamer::core
