cmake_minimum_required(VERSION 3.16)
project(AerStreamer VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(AERSTREAMER_ENABLE_GSTREAMER "Enable GStreamer-backed media pipeline" ON)

find_package(Boost 1.70 REQUIRED COMPONENTS system thread)
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json REQUIRED)

if(AERSTREAMER_ENABLE_GSTREAMER)
  find_package(PkgConfig QUIET)
  if(PkgConfig_FOUND)
    pkg_check_modules(GSTREAMER QUIET gstreamer-1.0)
  endif()
endif()

add_executable(aerstreamer_server
  server/main.cpp
  network/rtp_server.cpp
  network/json_server.cpp
  core/session_manager.cpp
  core/stream_router.cpp
  core/json_router.cpp
  core/recording_manager.cpp
  media/gst_pipeline.cpp
  media/codec_detector.cpp
  recording/stream_recorder.cpp
  storage/minio_client.cpp
  storage/upload_worker.cpp
  web_gateway/http_server.cpp
  web_gateway/websocket_server.cpp
)

target_include_directories(aerstreamer_server PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${Boost_INCLUDE_DIRS}
)

if(GSTREAMER_FOUND)
  target_compile_definitions(aerstreamer_server PRIVATE AERSTREAMER_HAS_GSTREAMER=1)
  target_include_directories(aerstreamer_server PRIVATE ${GSTREAMER_INCLUDE_DIRS})
  target_link_libraries(aerstreamer_server PRIVATE ${GSTREAMER_LIBRARIES})
else()
  target_compile_definitions(aerstreamer_server PRIVATE AERSTREAMER_HAS_GSTREAMER=0)
endif()

target_link_libraries(aerstreamer_server PRIVATE
  Boost::system
  Boost::thread
  OpenSSL::SSL
  OpenSSL::Crypto
  nlohmann_json::nlohmann_json
)

add_executable(aerstreamer_publisher publisher/publisher.cpp)
add_executable(rgb_publisher publisher/video_publisher.cpp)
add_executable(thermal_publisher publisher/thermal_publisher.cpp)
add_executable(json_publisher publisher/json_publisher.cpp)

target_link_libraries(aerstreamer_publisher PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(rgb_publisher PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(thermal_publisher PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(json_publisher PRIVATE nlohmann_json::nlohmann_json)
