#!/usr/bin/env bash
set -euo pipefail

# Thermal stream publisher (works with thermal camera / grayscale source / RTSP input)
# Usage:
#   STREAM_NAME=thermal_camera_gate INPUT_URI=rtsp://cam/thermal ./aerstreamer/scripts/run_thermal_publisher.sh

AER_HOST="${AER_HOST:-127.0.0.1}"
AER_RTP_PORT="${AER_RTP_PORT:-5004}"
STREAM_NAME="${STREAM_NAME:-thermal_camera_gate}"
INPUT_URI="${INPUT_URI:-}"
CAMERA_DEVICE="${CAMERA_DEVICE:-/dev/video2}"
WIDTH="${WIDTH:-640}"
HEIGHT="${HEIGHT:-512}"
FPS="${FPS:-25}"

if ! command -v gst-launch-1.0 >/dev/null 2>&1; then
  echo "ERROR: gst-launch-1.0 not found. Install GStreamer tools."
  exit 1
fi

echo "Starting thermal publisher stream=${STREAM_NAME} -> ${AER_HOST}:${AER_RTP_PORT}"

if [[ -n "${INPUT_URI}" ]]; then
  gst-launch-1.0 -e \
    uridecodebin uri="${INPUT_URI}" ! videoconvert ! videoscale ! \
    video/x-raw,format=GRAY8,width=${WIDTH},height=${HEIGHT},framerate=${FPS}/1 ! \
    x264enc tune=zerolatency speed-preset=ultrafast bitrate=1200 ! h264parse ! \
    rtpgstpay ! udpsink host="${AER_HOST}" port="${AER_RTP_PORT}"
elif [[ -e "${CAMERA_DEVICE}" ]]; then
  gst-launch-1.0 -e \
    v4l2src device="${CAMERA_DEVICE}" ! videoconvert ! videoscale ! \
    video/x-raw,format=GRAY8,width=${WIDTH},height=${HEIGHT},framerate=${FPS}/1 ! \
    x264enc tune=zerolatency speed-preset=ultrafast bitrate=1200 ! h264parse ! \
    rtpgstpay ! udpsink host="${AER_HOST}" port="${AER_RTP_PORT}"
else
  echo "No thermal source found; using monochrome test source."
  gst-launch-1.0 -e \
    videotestsrc is-live=true pattern=ball ! videoconvert ! \
    video/x-raw,format=GRAY8,width=${WIDTH},height=${HEIGHT},framerate=${FPS}/1 ! \
    x264enc tune=zerolatency speed-preset=ultrafast bitrate=1200 ! h264parse ! \
    rtpgstpay ! udpsink host="${AER_HOST}" port="${AER_RTP_PORT}"
fi
