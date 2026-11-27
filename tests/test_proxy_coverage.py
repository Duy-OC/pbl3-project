"""Exercise the proxy coverage gate with simulated PX4 and gateway UDP sockets."""

import argparse
import binascii
import json
from pathlib import Path
import socket
import struct
import subprocess
import tempfile
import time
import unittest


ROOT = Path(__file__).resolve().parents[1]
LOOPBACK = "127.0.0.1"
EXECUTABLE = ROOT / "bin/udp_proxy.exe"


def local_position(x, sequence):
    # MAVLink 1 LOCAL_POSITION_NED (message 32, CRC extra 185).
    payload = struct.pack("<I6f", sequence * 100, x, 0, -100, 0, 0, 0)
    body = bytes((len(payload), sequence, 1, 1, 32)) + payload
    # MAVLink uses reflected X.25, so reflect every input octet and the result.
    reverse = bytes(int(f"{value:08b}"[::-1], 2) for value in range(256))
    crc = binascii.crc_hqx((body + bytes((185,))).translate(reverse), 0xFFFF)
    checksum = bytes((reverse[crc >> 8], reverse[crc & 0xFF]))
    return b"\xfe" + body + checksum


class CoverageTest(unittest.TestCase):
    def udp(self):
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.addCleanup(sock.close)
        sock.bind((LOOPBACK, 0))
        sock.settimeout(0.2)
        return sock

    def check_gateway(self, gateway_id, boundary, outside):
        gateway, px4 = self.udp(), self.udp()
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as reservation:
            reservation.bind((LOOPBACK, 0))
            target = reservation.getsockname()
        config = json.loads((ROOT / "config/proxy.json").read_text(encoding="utf-8"))
        config["px4"].update(listen_address=LOOPBACK, listen_port=target[1])
        config["position"].update(stale_timeout_ms=700)
        config["position"]["transform"].update(
            world_origin_of_ned_m=[0, 0, 0], world_yaw_of_ned_rad=0, altitude_offset_m=0,
        )
        config["coverage"]["zones_file"] = str(ROOT / "config/zones.json")
        for endpoint in config["gateways"]:
            if endpoint["id"] == gateway_id:
                endpoint["port"] = gateway.getsockname()[1]

        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder) / "proxy.json"
            path.write_text(json.dumps(config), encoding="utf-8")
            process = subprocess.Popen([str(EXECUTABLE), "--config", str(path), "--gateway", gateway_id],
                                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            try:
                time.sleep(0.3)
                self.assertIsNone(process.poll())
                px4.sendto(b"no position", target)
                with self.assertRaises(socket.timeout):
                    gateway.recvfrom(1024)

                def exchange(x, sequence):
                    frame = local_position(x, sequence)
                    px4.sendto(frame, target)
                    received, source = gateway.recvfrom(1024)
                    self.assertEqual(received, frame)
                    gateway.sendto(b"reply", source)
                    self.assertEqual(px4.recvfrom(1024), (b"reply", target))
                    return source

                source = exchange(boundary, 1)
                px4.sendto(local_position(outside, 2), target)
                with self.assertRaises(socket.timeout):
                    gateway.recvfrom(1024)
                gateway.sendto(b"out of range reply", source)
                with self.assertRaises(socket.timeout):
                    px4.recvfrom(1024)
                exchange(30, 3)  # Overlap of A and B.
                time.sleep(0.8)
                px4.sendto(b"stale", target)
                gateway.sendto(b"stale reply", source)
                for sock in (gateway, px4):
                    with self.assertRaises(socket.timeout):
                        sock.recvfrom(1024)
                exchange(boundary, 4)
            finally:
                if process.poll() is None:
                    process.terminate()
                process.wait(timeout=5)

    def test_gateway_a(self):
        self.check_gateway("A", boundary=40, outside=41)

    def test_gateway_b(self):
        self.check_gateway("B", boundary=20, outside=19)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", type=Path, default=EXECUTABLE)
    args = parser.parse_args()
    EXECUTABLE = args.exe.resolve()
    unittest.main(argv=[__file__], verbosity=2)
