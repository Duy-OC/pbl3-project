"""Gateway integration check: lock ownership, ACK routing, retry and disconnect."""

import socket
import subprocess
import time
import unittest
from pathlib import Path

from pymavlink import mavutil
from pymavlink.dialects.v20 import common as mavlink2


ROOT = Path(__file__).resolve().parents[1]


def line(sock, timeout=3):
    sock.settimeout(timeout)
    data = b""
    while not data.endswith(b"\n"):
        data += sock.recv(1)
    return data.decode().strip()


class ControlProtocolTest(unittest.TestCase):
    def test_lock_ack_retry(self):
        gateway = subprocess.Popen(
            [str(ROOT / "bin" / "gateway.exe"), "--scenario", "control_test", "--udp-port", "14558"],
            cwd=ROOT,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0),
        )
        uav = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        uav.bind(("127.0.0.1", 0))
        uav.settimeout(3)
        one = two = None
        try:
            for _ in range(30):
                try:
                    one = socket.create_connection(("127.0.0.1", 5760), timeout=0.2)
                    break
                except OSError:
                    time.sleep(0.1)
            self.assertIsNotNone(one)
            two = socket.create_connection(("127.0.0.1", 5760))
            one.sendall(b"ACQUIRE\n")
            self.assertEqual(line(one), "OK ACQUIRE")
            for altitude in ("1e999", "-1e999", "nan", "inf", "abc", "0", "101", "10 extra"):
                one.sendall(f"COMMAND 1 TAKEOFF {altitude}\n".encode())
                self.assertEqual(line(one), "ERR ALTITUDE 1..100", altitude)
            two.sendall(b"ACQUIRE\nCOMMAND 1 ARM\n")
            self.assertEqual(line(two), "ERR LOCKED")
            self.assertEqual(line(two), "ERR NOT_OWNER")
            two.sendall(b"SUBSCRIBE 6\nSUBSCRIBE 2\n")
            self.assertEqual(line(two), "ERR USAGE SUBSCRIBE 1..5")
            self.assertEqual(line(two), "OK SUBSCRIBE 2")
            self.assertEqual(line(two), "DETAIL NONE")
            two.sendall(b"UNSUBSCRIBE\n")
            self.assertEqual(line(two), "OK UNSUBSCRIBE")

            mav = mavutil.mavlink.MAVLink(None)
            mav.srcSystem = 1
            mav.srcComponent = 1
            heartbeat = mav.heartbeat_encode(
                mavutil.mavlink.MAV_TYPE_QUADROTOR,
                mavutil.mavlink.MAV_AUTOPILOT_PX4, 0, 0,
                mavutil.mavlink.MAV_STATE_ACTIVE)
            uav.sendto(heartbeat.pack(mav), ("127.0.0.1", 14558))
            one.sendall(b"GET_DETAIL 1\nCOMMAND 1 ARM\n")
            self.assertTrue(line(one).startswith("DETAIL {"))
            self.assertEqual(line(one), "OK COMMAND_PENDING 1 400")

            parser = mavutil.mavlink.MAVLink(None)
            seen = []
            gcs_heartbeats = 0
            deadline = time.monotonic() + 2.5
            while time.monotonic() < deadline and len(seen) < 2:
                packet = uav.recv(4096)
                for byte in packet:
                    message = parser.parse_char(bytes([byte]))
                    if message and message.get_type() == "COMMAND_LONG":
                        seen.append(message)
                    if message and message.get_type() == "HEARTBEAT" and message.get_srcSystem() == 255:
                        gcs_heartbeats += 1
            self.assertEqual(len(seen), 2)
            self.assertGreaterEqual(gcs_heartbeats, 1)
            self.assertEqual(seen[0].command, 400)
            self.assertEqual(seen[1].confirmation, 1)

            ack = mav.command_ack_encode(400, mavutil.mavlink.MAV_RESULT_ACCEPTED)
            for component, target_system, target_component in (
                (100, 255, 190), (1, 42, 190), (1, 255, 99),
            ):
                sender = mavlink2.MAVLink(None, srcSystem=1, srcComponent=component)
                wrong_ack = sender.command_ack_encode(
                    400, 0, 0, 0, target_system, target_component)
                uav.sendto(wrong_ack.pack(sender), ("127.0.0.1", 14558))
                one.settimeout(0.1)
                with self.assertRaises(socket.timeout):
                    one.recv(1)
            uav.sendto(ack.pack(mav), ("127.0.0.1", 14558))
            self.assertEqual(line(one), "RESULT 1 400 0")
            for command_text, command_id, altitude in (
                (b"COMMAND 1 TAKEOFF 10\n", 22, 10),
                (b"COMMAND 1 TAKEOFF\n", 22, 10),
                (b"COMMAND 1 TAKEOFF 1\n", 22, 1),
                (b"COMMAND 1 TAKEOFF 100\n", 22, 100),
                (b"COMMAND 1 LAND\n", 21, 0),
            ):
                uav.sendto(heartbeat.pack(mav), ("127.0.0.1", 14558))
                one.sendall(command_text)
                self.assertEqual(line(one), f"OK COMMAND_PENDING 1 {command_id}")
                deadline = time.monotonic() + 2
                received = None
                while time.monotonic() < deadline and received is None:
                    packet = uav.recv(4096)
                    for byte in packet:
                        message = parser.parse_char(bytes([byte]))
                        if message and message.get_type() == "COMMAND_LONG" and message.command == command_id:
                            received = message
                self.assertIsNotNone(received)
                self.assertAlmostEqual(received.param7, altitude)
                ack_sender = mavlink2.MAVLink(None, srcSystem=1, srcComponent=1)
                response = ack_sender.command_ack_encode(command_id, 0, 0, 0, 255, 190)
                uav.sendto(response.pack(ack_sender), ("127.0.0.1", 14558))
                self.assertEqual(line(one), f"RESULT 1 {command_id} 0")
            uav.sendto(heartbeat.pack(mav), ("127.0.0.1", 14558))
            one.sendall(b"COMMAND 1 DISARM\n")
            self.assertEqual(line(one), "OK COMMAND_PENDING 1 400")
            deadline = time.monotonic() + 2
            disarm = None
            while time.monotonic() < deadline and disarm is None:
                packet = uav.recv(4096)
                for byte in packet:
                    message = parser.parse_char(bytes([byte]))
                    if message and message.get_type() == "COMMAND_LONG" and message.param1 == 0:
                        disarm = message
            self.assertIsNotNone(disarm)
            self.assertEqual(disarm.command, 400)
            uav.sendto(ack.pack(mav), ("127.0.0.1", 14558))
            self.assertEqual(line(one), "RESULT 1 400 0")
            uav.sendto(ack.pack(mav), ("127.0.0.1", 14558))
            one.settimeout(0.3)
            with self.assertRaises(socket.timeout):
                one.recv(1)
            time.sleep(5.2)
            one.sendall(b"COMMAND 1 ARM\n")
            self.assertEqual(line(one), "ERR UAV_OFFLINE")
            one.close()
            one = None
            time.sleep(0.2)
            two.sendall(b"ACQUIRE\n")
            self.assertEqual(line(two), "OK ACQUIRE")
        finally:
            if one: one.close()
            if two: two.close()
            uav.close()
            gateway.terminate()
            gateway.wait(timeout=3)


if __name__ == "__main__":
    unittest.main()
