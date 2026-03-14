#include "recording/stream_recorder.h"

#include <chrono>

namespace aerstreamer::recording {

bool StreamRecorder::start(const std::string& stream_name, const std::string& recording_id,
                           const std::string& base_dir, bool json_only) {
  std::lock_guard lock(mutex_);
  auto dir = std::filesystem::path(base_dir) / stream_name / recording_id;
  std::filesystem::create_directories(dir);

  RecorderContext context;
  context.dir = dir;
  context.json_only = json_only;
  context.json_file.open(dir / "stream.jsonl", std::ios::out | std::ios::app);
  if (!context.json_only) {
    context.packet_file.open(dir / "video.rtp.bin", std::ios::out | std::ios::binary | std::ios::app);
  }

  if (!context.json_file.is_open() || (!context.json_only && !context.packet_file.is_open())) {
    return false;
  }

  active_[stream_name] = std::move(context);
  return true;
}

void StreamRecorder::writeJson(const std::string& stream_name, const nlohmann::json& message) {
  std::lock_guard lock(mutex_);
  auto it = active_.find(stream_name);
  if (it == active_.end()) return;
  it->second.json_file << message.dump() << '\n';
  it->second.json_file.flush();
}

void StreamRecorder::writePacket(const std::string& stream_name, const std::vector<uint8_t>& packet) {
  std::lock_guard lock(mutex_);
  auto it = active_.find(stream_name);
  if (it == active_.end() || it->second.json_only) return;
  it->second.packet_file.write(reinterpret_cast<const char*>(packet.data()),
                               static_cast<std::streamsize>(packet.size()));
  it->second.packet_file.flush();
}

std::filesystem::path StreamRecorder::stop(const std::string& stream_name) {
  std::lock_guard lock(mutex_);
  auto it = active_.find(stream_name);
  if (it == active_.end()) return {};
  auto dir = it->second.dir;
  if (it->second.json_file.is_open()) it->second.json_file.close();
  if (it->second.packet_file.is_open()) it->second.packet_file.close();
  active_.erase(it);
  return dir;
}

}  // namespace aerstreamer::recording
