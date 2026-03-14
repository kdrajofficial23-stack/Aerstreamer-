# AerStreamer

AerStreamer is a C++17 dynamic streaming server for RTP video and JSON telemetry ingest with stream-name based session routing, local-first recording, and deferred MinIO upload.

## Features
- RTP ingest over UDP (dynamic `stream_name` extraction).
- Unified JSON ingest via a single WebSocket server.
- Thread-safe `SessionManager` with dynamic stream registration.
- HTTP control API: `GET /streams`, `POST /record/start`, `POST /record/stop`.
- Recording model writes local files first (`/var/aerstreamer/recordings/<stream_name>/<recording_id>/`).
- Asynchronous MinIO upload worker with retries after recording stop.
- Web viewer skeleton for metadata monitoring.

## Build
```bash
cmake -S aerstreamer -B build
cmake --build build -j
```

## Run
```bash
./build/aerstreamer_server
```

## API examples
```bash
curl http://localhost:8080/streams

curl -X POST http://localhost:8080/record/start \
  -H 'Content-Type: application/json' \
  -d '{
    "stream_name":"telemetry_uav_17",
    "bucket_name":"mission-bucket",
    "folder_path":"mission_42/day_1",
    "metadata":{"operator":"alpha"}
  }'

curl -X POST http://localhost:8080/record/stop \
  -H 'Content-Type: application/json' \
  -d '{"stream_name":"telemetry_uav_17"}'
```
