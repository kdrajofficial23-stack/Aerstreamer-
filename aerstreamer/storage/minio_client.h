#pragma once

#include <filesystem>
#include <string>

namespace aerstreamer::storage {

class MinioClient {
 public:
  bool uploadFile(const std::string& bucket_name, const std::filesystem::path& local_path,
                  const std::string& object_path) const;
};

}  // namespace aerstreamer::storage
