#!/usr/bin/env bash
set -euo pipefail

# Send JSON telemetry over WebSocket to AerStreamer.
# Usage:
#   STREAM_NAME=telemetry_uav_17 AER_HOST=127.0.0.1 AER_JSON_PORT=9001 ./aerstreamer/scripts/run_json_publisher.sh

AER_HOST="${AER_HOST:-127.0.0.1}"
AER_JSON_PORT="${AER_JSON_PORT:-9001}"
STREAM_NAME="${STREAM_NAME:-telemetry_uav_17}"
RATE_HZ="${RATE_HZ:-10}"

if [[ -x ./build/json_publisher ]]; then
  exec ./build/json_publisher "${AER_HOST}" "${AER_JSON_PORT}" "${STREAM_NAME}" "${RATE_HZ}"
else
  echo "ERROR: ./build/json_publisher not found. Build first: cmake -S aerstreamer -B build && cmake --build build -j"
  exit 1
fi
