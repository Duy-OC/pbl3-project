"""Check multi-client TCP handling and 100 simultaneous lock races."""

import csv
import socket
import subprocess
import threading
import time
from datetime import datetime
from pathlib import Path

from pymavlink import mavutil


ROOT = Path(__file__).resolve().parents[1]


def line(sock, timeout=3):
    sock.settimeout(timeout)
    data = bytearray()
    while not data.endswith(b"\n"):
        part = sock.recv(1)
        if not part:
            raise ConnectionError("TCP closed")
        data.extend(part)
    return data.decode(errors="replace").strip()


def until(sock, predicate, timeout=3):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        reply = line(sock, max(0.1, deadline - time.monotonic()))
        if predicate(reply):
            return reply
    raise TimeoutError("Expected TCP response did not arrive")


def main():
    run_id = datetime.now().strftime("%Y%m%d_%H%M%S")
    folder = ROOT / "docs/ketqua/F/gateway_functions" / run_id
    folder.mkdir(parents=True, exist_ok=False)
    relative = folder.relative_to(ROOT).as_posix()
    log = (folder / "gateway.log").open("w", encoding="utf-8")
    flags = getattr(subprocess, "CREATE_NO_WINDOW", 0)
    gateway = subprocess.Popen([
        str(ROOT / "bin/gateway.exe"), "--scenario", "F",
        "--run-id", run_id,
        "--udp-port", "14553",
    ], cwd=ROOT, stdout=log, stderr=subprocess.STDOUT,
        creationflags=flags)
    sender = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sender.bind(("127.0.0.1", 0))
    stop = threading.Event()
    clients = []
    checks = []
    mav = mavutil.mavlink.MAVLink(None)
    mav.srcSystem = 1
    mav.srcComponent = 1

    def feed():
        while not stop.wait(0.5):
            heartbeat = mav.heartbeat_encode(
                mavutil.mavlink.MAV_TYPE_QUADROTOR,
                mavutil.mavlink.MAV_AUTOPILOT_PX4, 0, 0,
                mavutil.mavlink.MAV_STATE_ACTIVE)
            sender.sendto(heartbeat.pack(mav), ("127.0.0.1", 14553))

    feeder = threading.Thread(target=feed, daemon=True)
    feeder.start()
    try:
        deadline = time.monotonic() + 5
        while time.monotonic() < deadline:
            try:
                probe = socket.create_connection(("127.0.0.1", 5760), timeout=0.3)
                probe.close()
                break
            except OSError:
                time.sleep(0.1)
        else:
            raise RuntimeError("Gateway TCP unavailable")
        time.sleep(0.7)
        clients = [socket.create_connection(("127.0.0.1", 5760)) for _ in range(3)]
        for client in clients:
            client.sendall(b"SUBSCRIBE 5\n")
            checks.append(("subscribe_ack", until(client, lambda x: x == "OK SUBSCRIBE 5") == "OK SUBSCRIBE 5"))
        for client in clients:
            checks.append(("three_clients_receive_detail",
                           until(client, lambda x: x.startswith("DETAIL {"), 2).startswith("DETAIL {")))
        for client in clients:
            client.close()
        clients = []

        a = socket.create_connection(("127.0.0.1", 5760))
        b = socket.create_connection(("127.0.0.1", 5760))
        clients = [a, b]
        for byte in b"GET_DETAIL 1\n":
            a.sendall(bytes([byte]))
        checks.append(("tcp_split_bytes", line(a).startswith("DETAIL {")))
        a.sendall(b"SUBSCRIBE 6\n")
        checks.append(("malformed_subscribe", line(a) == "ERR USAGE SUBSCRIBE 1..5"))
        a.sendall(b"ACQUIRE\nRELEASE\n")
        checks.append(("coalesced_commands", line(a) == "OK ACQUIRE" and line(a) == "OK RELEASE"))

        wins = 0
        for _ in range(100):
            barrier = threading.Barrier(3)

            def send_acquire(client):
                barrier.wait()
                client.sendall(b"ACQUIRE\n")

            ta = threading.Thread(target=send_acquire, args=(a,))
            tb = threading.Thread(target=send_acquire, args=(b,))
            ta.start()
            tb.start()
            barrier.wait()
            ta.join(timeout=1)
            tb.join(timeout=1)
            responses = (line(a), line(b))
            if sorted(responses) == ["ERR LOCKED", "OK ACQUIRE"]:
                wins += 1
                owner = a if responses[0] == "OK ACQUIRE" else b
                owner.sendall(b"RELEASE\n")
                if line(owner) != "OK RELEASE":
                    break
        checks.append(("100_lock_races_one_winner", wins == 100))

        a.sendall(b"ACQUIRE\n")
        checks.append(("acquire_before_disconnect", line(a) == "OK ACQUIRE"))
        a.close()
        clients = [b]
        time.sleep(0.2)
        b.sendall(b"ACQUIRE\n")
        checks.append(("release_on_disconnect", line(b) == "OK ACQUIRE"))

        slow = socket.create_connection(("127.0.0.1", 5760))
        clients.append(slow)
        slow.sendall(b"SUBSCRIBE 5\n")
        b.sendall(b"GET_DETAIL 1\n")
        checks.append(("slow_client_does_not_block_other",
                       line(b).startswith("DETAIL {")))
        with (folder / "checks.csv").open("w", newline="", encoding="utf-8") as stream:
            writer = csv.DictWriter(stream, fieldnames=("check", "result"))
            writer.writeheader()
            writer.writerows({"check": name, "result": "PASS" if passed else "FAIL"}
                             for name, passed in checks)
        total = len(checks)
        passed = sum(passed for _, passed in checks)
        result = {"checks": total, "passed": passed,
                  "lock_races": 100, "one_winner": wins,
                  "result": "PASS" if passed == total else "FAIL"}
        with (folder / "result.csv").open("w", newline="", encoding="utf-8") as stream:
            writer = csv.DictWriter(stream, fieldnames=result)
            writer.writeheader()
            writer.writerow(result)
        print(folder / "result.csv", result)
        if passed != total:
            raise AssertionError("Some gateway function checks failed")
    finally:
        for client in clients:
            client.close()
        stop.set()
        feeder.join(timeout=1)
        sender.close()
        if gateway.poll() is None:
            gateway.terminate()
            gateway.wait(timeout=5)
        log.close()


if __name__ == "__main__":
    main()
