#include <iostream>
#include <thread>
#include <vector>

#include <boost/asio.hpp>
#include <nlohmann/json.hpp>

#include "core/json_router.h"
#include "core/recording_manager.h"
#include "core/session_manager.h"
#include "core/stream_router.h"
#include "media/codec_detector.h"
#include "network/json_server.h"
#include "network/rtp_server.h"
#include "recording/stream_recorder.h"
#include "server/config.h"
#include "storage/upload_worker.h"
#include "web_gateway/http_server.h"

int main() {
  using namespace aerstreamer;

  boost::asio::io_context ioc;
  core::SessionManager sessions;
  recording::StreamRecorder recorder;
  storage::UploadWorker uploader;
  uploader.start();

  core::StreamRouter stream_router(recorder);
  core::JsonRouter json_router(recorder);
  core::RecordingManager recording_manager(sessions, recorder, uploader, config::kRecordingBaseDir);

  network::RtpServer rtp_server(
      ioc,
      {boost::asio::ip::make_address(config::kBindAddress), config::kRtpPort},
      [&](const std::string& stream_name, std::vector<uint8_t> payload) {
        sessions.upsertStream(stream_name, core::StreamType::RGB_VIDEO, true);
        auto stream = sessions.getStream(stream_name);
        stream_router.onRtpPacket(stream_name, payload, stream && stream->recording_state);
      });

  network::JsonServer json_server(
      ioc,
      {boost::asio::ip::make_address(config::kBindAddress), config::kJsonWsPort},
      [&](const nlohmann::json& message) {
        const auto stream_name = message.value("stream_name", "");
        const auto type_name = message.value("stream_type", "JSON_DATA");
        sessions.upsertStream(stream_name, core::SessionManager::streamTypeFromString(type_name), true);
        auto stream = sessions.getStream(stream_name);
        json_router.route(stream_name, message, stream && stream->recording_state);
      });

  web_gateway::HttpServer http_server(
      ioc, {boost::asio::ip::make_address(config::kBindAddress), config::kHttpPort}, sessions,
      recording_manager);

  rtp_server.start();
  json_server.start();
  http_server.start();

  std::cout << "AerStreamer started on RTP:" << config::kRtpPort << " JSON:" << config::kJsonWsPort
            << " HTTP:" << config::kHttpPort << "\n";

  std::vector<std::thread> threads;
  const unsigned int count = std::max(2u, std::thread::hardware_concurrency());
  for (unsigned int i = 0; i < count; ++i) {
    threads.emplace_back([&ioc] { ioc.run(); });
  }

  for (auto& t : threads) t.join();
  uploader.stop();
  return 0;
}
