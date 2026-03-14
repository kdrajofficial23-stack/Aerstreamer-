#include "storage/minio_client.h"

#include <iostream>

namespace aerstreamer::storage {

bool MinioClient::uploadFile(const std::string& bucket_name, const std::filesystem::path& local_path,
                             const std::string& object_path) const {
  // In production this should call S3-compatible APIs (e.g. aws-sdk-cpp/minio-cpp).
  std::cout << "[UPLOAD] bucket=" << bucket_name << " local=" << local_path
            << " object=" << object_path << "\n";
  return std::filesystem::exists(local_path);
}

}  // namespace aerstreamer::storage
