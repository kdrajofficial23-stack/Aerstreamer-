# AerStreamer

AerStreamer is a C++17 dynamic streaming server for RTP video and JSON telemetry ingest with stream-name based session routing, local-first recording, and deferred MinIO upload.

## Features
- RTP ingest over UDP with dynamic `stream_name` sessions.
- Single JSON ingest server over WebSocket.
- Thread-safe `SessionManager` and stream lifecycle model.
- HTTP control API:
  - `GET /streams`
  - `POST /record/start`
  - `POST /record/stop`
- Local-first recording at `/var/aerstreamer/recordings/<stream_name>/<recording_id>/`.
- Asynchronous MinIO upload worker with retry.
- Web viewer for metadata.

---

## Build
```bash
cmake -S aerstreamer -B build
cmake --build build -j
```

## Run server
```bash
./build/aerstreamer_server
```

---

## Real publisher usage

### 1) RGB live camera publisher (bash)
```bash
STREAM_NAME=mission_alpha_rgb \
AER_HOST=127.0.0.1 \
AER_RTP_PORT=5004 \
CAMERA_DEVICE=/dev/video0 \
./aerstreamer/scripts/run_rgb_publisher.sh
```

Optional RTSP input instead of local camera:
```bash
STREAM_NAME=mission_alpha_rgb INPUT_URI=rtsp://user:pass@cam/stream ./aerstreamer/scripts/run_rgb_publisher.sh
```

### 2) Thermal live publisher (bash)
```bash
STREAM_NAME=thermal_camera_gate \
AER_HOST=127.0.0.1 \
AER_RTP_PORT=5004 \
CAMERA_DEVICE=/dev/video2 \
./aerstreamer/scripts/run_thermal_publisher.sh
```

### 3) JSON telemetry publisher (C++ binary via bash)
```bash
STREAM_NAME=telemetry_uav_17 RATE_HZ=10 ./aerstreamer/scripts/run_json_publisher.sh
```

---

## FastAPI + MAVLink telemetry bridge example (drone stack)

If your drone backend is FastAPI, use the provided bridge:

```bash
pip install fastapi uvicorn websockets pymavlink
python aerstreamer/publisher/fastapi_mavlink_telemetry_publisher.py \
  --mavlink udp:127.0.0.1:14550 \
  --aer-ws ws://127.0.0.1:9001 \
  --stream telemetry_uav_17 \
  --rate-hz 20
```

This bridge:
- reads MAVLink (`GLOBAL_POSITION_INT`, `GPS_RAW_INT`, `SYS_STATUS`, `HEARTBEAT`)
- builds JSON telemetry payload
- sends to AerStreamer JSON WebSocket ingest
- exposes FastAPI endpoints:
  - `GET /health`
  - `GET /telemetry/latest`

---

## HTTP API examples
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
