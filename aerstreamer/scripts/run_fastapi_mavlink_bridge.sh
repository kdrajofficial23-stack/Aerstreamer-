#!/usr/bin/env bash
set -euo pipefail

# Example launcher for FastAPI MAVLink telemetry bridge.
# Requires: pip install fastapi uvicorn websockets pymavlink

MAVLINK_URI="${MAVLINK_URI:-udp:127.0.0.1:14550}"
AER_WS="${AER_WS:-ws://127.0.0.1:9001}"
STREAM_NAME="${STREAM_NAME:-telemetry_uav_17}"
RATE_HZ="${RATE_HZ:-20}"
API_PORT="${API_PORT:-8100}"

exec python aerstreamer/publisher/fastapi_mavlink_telemetry_publisher.py \
  --mavlink "${MAVLINK_URI}" \
  --aer-ws "${AER_WS}" \
  --stream "${STREAM_NAME}" \
  --rate-hz "${RATE_HZ}" \
  --api-port "${API_PORT}"
