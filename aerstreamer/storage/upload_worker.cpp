#include "storage/upload_worker.h"

#include <chrono>
#include <iostream>

#include "server/config.h"

namespace aerstreamer::storage {

UploadWorker::UploadWorker() = default;

UploadWorker::~UploadWorker() { stop(); }

void UploadWorker::start() {
  running_ = true;
  worker_ = std::thread([this] { run(); });
}

void UploadWorker::stop() {
  running_ = false;
  cv_.notify_all();
  if (worker_.joinable()) worker_.join();
}

void UploadWorker::enqueue(UploadTask task) {
  {
    std::lock_guard lock(mutex_);
    queue_.push(std::move(task));
  }
  cv_.notify_one();
}

void UploadWorker::run() {
  while (running_) {
    std::optional<UploadTask> next;
    {
      std::unique_lock lock(mutex_);
      cv_.wait(lock, [this] { return !running_ || !queue_.empty(); });
      if (!running_) break;
      next = queue_.front();
      queue_.pop();
    }

    if (!processTask(*next)) {
      std::cerr << "[UPLOAD] task failed for stream=" << next->stream_name << "\n";
    }
  }
}

bool UploadWorker::processTask(const UploadTask& task) {
  if (!std::filesystem::exists(task.local_dir)) return false;

  for (const auto& entry : std::filesystem::directory_iterator(task.local_dir)) {
    if (!entry.is_regular_file()) continue;

    auto object_path = task.folder_path + "/" + task.stream_name + "/" + task.recording_id + "/" +
                       entry.path().filename().string();

    bool uploaded = false;
    for (int attempt = 0; attempt < config::kUploadMaxRetries && !uploaded; ++attempt) {
      uploaded = minio_.uploadFile(task.bucket_name, entry.path(), object_path);
      if (!uploaded) {
        std::this_thread::sleep_for(config::kUploadRetryDelay);
      }
    }
    if (!uploaded) return false;
  }
  return true;
}

}  // namespace aerstreamer::storage
