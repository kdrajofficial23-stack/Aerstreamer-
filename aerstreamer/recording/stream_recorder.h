#pragma once

#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>

namespace aerstreamer::recording {

class StreamRecorder {
 public:
  bool start(const std::string& stream_name, const std::string& recording_id,
             const std::string& base_dir, bool json_only);
  void writeJson(const std::string& stream_name, const nlohmann::json& message);
  void writePacket(const std::string& stream_name, const std::vector<uint8_t>& packet);
  std::filesystem::path stop(const std::string& stream_name);

 private:
  struct RecorderContext {
    std::filesystem::path dir;
    std::ofstream json_file;
    std::ofstream packet_file;
    bool json_only{false};
  };

  std::mutex mutex_;
  std::unordered_map<std::string, RecorderContext> active_;
};

}  // namespace aerstreamer::recording
