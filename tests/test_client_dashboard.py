"""Verify the real gateway/client with two UAVs and no runtime file output."""
import socket
import subprocess
import tempfile
import time
import threading
import unittest
from pathlib import Path
from pymavlink import mavutil

ROOT = Path(__file__).resolve().parents[1]

class DashboardTest(unittest.TestCase):
    def test_two_uavs_without_files(self):
        flags = getattr(subprocess, 'CREATE_NO_WINDOW', 0)
        with tempfile.TemporaryDirectory() as directory:
            gateway = subprocess.Popen([str(ROOT/'bin/gateway.exe'), '--udp-port', '14557'], cwd=directory,
                                       stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, creationflags=flags)
            client = None
            try:
                for _ in range(40):
                    try:
                        with socket.create_connection(('127.0.0.1', 5760), timeout=.1):
                            break
                    except OSError:
                        time.sleep(.1)
                client = subprocess.Popen([str(ROOT/'bin/tcpclient.exe')], cwd=directory,
                                          stdout=subprocess.PIPE, stderr=subprocess.PIPE, creationflags=flags)
                chunks = []
                reader = threading.Thread(target=lambda: chunks.extend(iter(lambda: client.stdout.read(1024), b"")), daemon=True)
                reader.start()
                time.sleep(.3)
                with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as udp:
                    for sysid, voltage in [(1, 12340), (2, 15670)]:
                        mav = mavutil.mavlink.MAVLink(None)
                        mav.srcSystem = sysid
                        mav.srcComponent = 1
                        heartbeat = mav.heartbeat_encode(2, 12, 0, 0, 4)
                        status = mav.sys_status_encode(0, 0, 0, 0, voltage, 250, 75, 0, 0, 0, 0, 0, 0)
                        for message in [heartbeat, status]:
                            udp.sendto(message.pack(mav), ('127.0.0.1', 14557))
                # Includes online display followed by timeout display for both UAVs.
                time.sleep(6.3)
                client.terminate()
                client.wait(timeout=3)
                reader.join(timeout=3)
                errors = client.stderr.read()
                client.stdout.close()
                client.stderr.close()
                output = b"".join(chunks).decode(errors="replace")
                self.assertIn('SYSID: 1', output)
                self.assertIn('SYSID: 2', output)
                self.assertIn('12.34 V', output)
                self.assertIn('15.67 V', output)
                self.assertIn('[ONLINE]', output)
                self.assertIn('[OFFLINE/TIMEOUT]', output)
                self.assertEqual(errors, b'')
                self.assertEqual(list(Path(directory).rglob('*')), [])
                self.assertIsNone(gateway.poll())
            finally:
                if client and client.poll() is None:
                    client.terminate()
                    client.communicate(timeout=3)
                gateway.terminate()
                gateway.communicate(timeout=3)

if __name__ == '__main__':
    unittest.main()
