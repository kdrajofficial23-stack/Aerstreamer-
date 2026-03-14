#!/usr/bin/env bash
set -euo pipefail
cmake -S aerstreamer -B build
cmake --build build -j
./build/aerstreamer_server
