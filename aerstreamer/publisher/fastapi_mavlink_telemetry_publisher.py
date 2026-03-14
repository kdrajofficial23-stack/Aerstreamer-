#!/usr/bin/env python3
"""
FastAPI telemetry bridge for AerStreamer JSON ingest.

- Reads vehicle telemetry from MAVLink (serial/udp/tcp) using pymavlink.
- Broadcasts telemetry to AerStreamer JSON WebSocket ingest server.
- Exposes a simple REST endpoint for health check / latest telemetry.

Run:
  pip install fastapi uvicorn websockets pymavlink
  python aerstreamer/publisher/fastapi_mavlink_telemetry_publisher.py \
      --mavlink udp:127.0.0.1:14550 --aer-ws ws://127.0.0.1:9001 --stream telemetry_uav_17
"""

import argparse
import asyncio
import json
import time
from dataclasses import asdict, dataclass
from typing import Optional

import uvicorn
import websockets
from fastapi import FastAPI
from pymavlink import mavutil


@dataclass
class TelemetryState:
  ts_ms: int = 0
  lat: float = 0.0
  lon: float = 0.0
  alt_m: float = 0.0
  ground_speed_mps: float = 0.0
  heading_deg: float = 0.0
  battery_pct: int = 0
  mode: str = "UNKNOWN"
  armed: bool = False
  fix_type: int = 0


class MavlinkAerPublisher:
  def __init__(self, mavlink_conn: str, aer_ws: str, stream_name: str, rate_hz: int) -> None:
    self.mavlink_conn = mavlink_conn
    self.aer_ws = aer_ws
    self.stream_name = stream_name
    self.rate_hz = max(1, rate_hz)
    self.state = TelemetryState()
    self._stop = asyncio.Event()

  async def run(self) -> None:
    while not self._stop.is_set():
      try:
        await self._run_once()
      except Exception as ex:
        print(f"[telemetry] bridge error: {ex}; reconnecting in 2s")
        await asyncio.sleep(2)

  async def _run_once(self) -> None:
    print(f"[telemetry] connecting MAVLink: {self.mavlink_conn}")
    mav = mavutil.mavlink_connection(self.mavlink_conn)
    mav.wait_heartbeat(timeout=30)
    print("[telemetry] MAVLink heartbeat received")

    print(f"[telemetry] connecting AerStreamer WS: {self.aer_ws}")
    async with websockets.connect(self.aer_ws, ping_interval=20, ping_timeout=20) as ws:
      next_emit = time.time()
      seq = 0
      while not self._stop.is_set():
        msg = mav.recv_match(blocking=False)
        if msg is not None:
          self._update_state(msg)

        now = time.time()
        if now >= next_emit:
          payload = {
              "stream_name": self.stream_name,
              "stream_type": "JSON_DATA",
              "publisher": "fastapi_mavlink_bridge",
              "timestamp_ms": int(now * 1000),
              "seq": seq,
              "payload": asdict(self.state),
          }
          await ws.send(json.dumps(payload))
          seq += 1
          next_emit = now + 1.0 / self.rate_hz

        await asyncio.sleep(0.005)

  def _update_state(self, msg) -> None:
    msg_type = msg.get_type()
    self.state.ts_ms = int(time.time() * 1000)

    if msg_type == "GLOBAL_POSITION_INT":
      self.state.lat = msg.lat / 1e7
      self.state.lon = msg.lon / 1e7
      self.state.alt_m = msg.relative_alt / 1000.0
      self.state.heading_deg = msg.hdg / 100.0 if msg.hdg != 65535 else self.state.heading_deg
      self.state.ground_speed_mps = ((msg.vx ** 2 + msg.vy ** 2) ** 0.5) / 100.0
    elif msg_type == "GPS_RAW_INT":
      self.state.fix_type = int(msg.fix_type)
    elif msg_type == "SYS_STATUS":
      self.state.battery_pct = int(msg.battery_remaining)
    elif msg_type == "HEARTBEAT":
      self.state.armed = bool(msg.base_mode & mavutil.mavlink.MAV_MODE_FLAG_SAFETY_ARMED)
      self.state.mode = mavutil.mode_string_v10(msg)

  def latest(self) -> TelemetryState:
    return self.state


def create_app(publisher: MavlinkAerPublisher) -> FastAPI:
  app = FastAPI(title="AerStreamer MAVLink Telemetry Bridge")

  @app.get("/health")
  async def health():
    return {"ok": True, "stream_name": publisher.stream_name}

  @app.get("/telemetry/latest")
  async def telemetry_latest():
    return asdict(publisher.latest())

  return app


async def main_async(args) -> None:
  publisher = MavlinkAerPublisher(args.mavlink, args.aer_ws, args.stream, args.rate_hz)
  app = create_app(publisher)

  config = uvicorn.Config(app, host=args.api_host, port=args.api_port, log_level="info")
  server = uvicorn.Server(config)

  await asyncio.gather(
      publisher.run(),
      server.serve(),
  )


def parse_args():
  parser = argparse.ArgumentParser()
  parser.add_argument("--mavlink", default="udp:127.0.0.1:14550", help="pymavlink connection string")
  parser.add_argument("--aer-ws", default="ws://127.0.0.1:9001", help="AerStreamer JSON WebSocket URL")
  parser.add_argument("--stream", default="telemetry_uav_17", help="stream_name for AerStreamer")
  parser.add_argument("--rate-hz", default=10, type=int, help="publish rate")
  parser.add_argument("--api-host", default="0.0.0.0")
  parser.add_argument("--api-port", default=8100, type=int)
  return parser.parse_args()


if __name__ == "__main__":
  asyncio.run(main_async(parse_args()))
