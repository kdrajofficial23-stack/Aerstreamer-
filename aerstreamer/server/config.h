#pragma once

#include <chrono>
#include <string>

namespace aerstreamer::config {

constexpr const char* kBindAddress = "0.0.0.0";
constexpr uint16_t kRtpPort = 5004;
constexpr uint16_t kJsonWsPort = 9001;
constexpr uint16_t kHttpPort = 8080;
constexpr uint16_t kViewerWsPort = 9002;
constexpr const char* kRecordingBaseDir = "/var/aerstreamer/recordings";
constexpr std::chrono::seconds kUploadRetryDelay{3};
constexpr int kUploadMaxRetries = 5;

}  // namespace aerstreamer::config
