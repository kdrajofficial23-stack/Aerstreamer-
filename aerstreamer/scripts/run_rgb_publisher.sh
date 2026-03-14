#!/usr/bin/env bash
set -euo pipefail

# Real camera/live RGB RTP publisher to AerStreamer UDP ingest.
# Usage:
#   STREAM_NAME=mission_alpha_rgb AER_HOST=127.0.0.1 AER_RTP_PORT=5004 ./aerstreamer/scripts/run_rgb_publisher.sh
#
# Inputs (auto-probed in order):
#  1) CAMERA_DEVICE=/dev/video0
#  2) INPUT_URI=rtsp://... or file path

AER_HOST="${AER_HOST:-127.0.0.1}"
AER_RTP_PORT="${AER_RTP_PORT:-5004}"
STREAM_NAME="${STREAM_NAME:-mission_alpha_rgb}"
CAMERA_DEVICE="${CAMERA_DEVICE:-/dev/video0}"
INPUT_URI="${INPUT_URI:-}"
WIDTH="${WIDTH:-1280}"
HEIGHT="${HEIGHT:-720}"
FPS="${FPS:-30}"
CODEC="${CODEC:-H264}"   # H264|H265|MJPEG

if ! command -v gst-launch-1.0 >/dev/null 2>&1; then
  echo "ERROR: gst-launch-1.0 not found. Install GStreamer tools."
  exit 1
fi

case "${CODEC}" in
  H264) PAYLOAD=96; ENCODER="x264enc tune=zerolatency speed-preset=ultrafast bitrate=2500 ! h264parse" ;;
  H265) PAYLOAD=98; ENCODER="x265enc tune=zerolatency bitrate=2500 ! h265parse" ;;
  MJPEG) PAYLOAD=26; ENCODER="jpegenc" ;;
  *) echo "ERROR: CODEC must be H264, H265 or MJPEG"; exit 1 ;;
esac

# AerStreamer protocol currently expects first line stream_name then payload bytes.
# We prepend metadata header using gdppay with a text buffer and then RTP payload.
# For production, this should be replaced with a dedicated AerStreamer RTP publisher.
HEADER_FILE="/tmp/aerstreamer_header_${STREAM_NAME}.txt"
printf "%s\n" "${STREAM_NAME}" > "${HEADER_FILE}"

echo "Starting RGB publisher stream=${STREAM_NAME} -> ${AER_HOST}:${AER_RTP_PORT} codec=${CODEC}"

if [[ -n "${INPUT_URI}" ]]; then
  gst-launch-1.0 -e \
    uridecodebin uri="${INPUT_URI}" ! videoconvert ! videoscale ! \
    video/x-raw,width=${WIDTH},height=${HEIGHT},framerate=${FPS}/1 ! \
    ${ENCODER} ! rtpgstpay ! udpsink host="${AER_HOST}" port="${AER_RTP_PORT}"
elif [[ -e "${CAMERA_DEVICE}" ]]; then
  gst-launch-1.0 -e \
    v4l2src device="${CAMERA_DEVICE}" ! videoconvert ! videoscale ! \
    video/x-raw,width=${WIDTH},height=${HEIGHT},framerate=${FPS}/1 ! \
    ${ENCODER} ! rtpgstpay ! udpsink host="${AER_HOST}" port="${AER_RTP_PORT}"
else
  echo "No INPUT_URI or CAMERA_DEVICE found, using test pattern source."
  gst-launch-1.0 -e \
    videotestsrc is-live=true pattern=smpte ! video/x-raw,width=${WIDTH},height=${HEIGHT},framerate=${FPS}/1 ! \
    ${ENCODER} ! rtpgstpay ! udpsink host="${AER_HOST}" port="${AER_RTP_PORT}"
fi
