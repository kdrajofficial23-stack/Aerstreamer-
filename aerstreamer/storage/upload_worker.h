#pragma once

#include <atomic>
#include <condition_variable>
#include <filesystem>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <thread>

#include "storage/minio_client.h"

namespace aerstreamer::storage {

struct UploadTask {
  std::string stream_name;
  std::string recording_id;
  std::string bucket_name;
  std::string folder_path;
  std::filesystem::path local_dir;
};

class UploadWorker {
 public:
  UploadWorker();
  ~UploadWorker();

  void start();
  void stop();
  void enqueue(UploadTask task);

 private:
  void run();
  bool processTask(const UploadTask& task);

  MinioClient minio_;
  std::atomic<bool> running_{false};
  std::mutex mutex_;
  std::condition_variable cv_;
  std::queue<UploadTask> queue_;
  std::thread worker_;
};

}  // namespace aerstreamer::storage
