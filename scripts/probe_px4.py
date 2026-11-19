"""Read-only UDP probe of the configured PX4 MAVLink stream."""

import collections
import csv
import socket
import sys
import time

from pymavlink import mavutil


sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
port = int(sys.argv[1]) if len(sys.argv) > 1 else 14550
duration = float(sys.argv[2]) if len(sys.argv) > 2 else 10.0
output_path = sys.argv[3] if len(sys.argv) > 3 else None
sock.bind(("0.0.0.0", port))
sock.settimeout(1)
parser = mavutil.mavlink.MAVLink(None)
counts = collections.Counter()
senders = set()
started = time.monotonic()
deadline = started + duration
while time.monotonic() < deadline:
    try:
        payload, sender = sock.recvfrom(65535)
    except socket.timeout:
        continue
    senders.add(sender)
    for byte in payload:
        message = parser.parse_char(bytes([byte]))
        if message:
            counts[message.get_type()] += 1
print("senders:", sorted(senders))
print("frames:", sum(counts.values()))
print("HEARTBEAT:", counts["HEARTBEAT"])
print("ATTITUDE:", counts["ATTITUDE"])
print("other:", sum(counts.values()) - counts["HEARTBEAT"] - counts["ATTITUDE"])
if output_path:
    result = {
        "seconds": duration,
        "frames": sum(counts.values()),
        "pps": round(sum(counts.values()) / duration, 3),
        "heartbeat": counts["HEARTBEAT"],
        "attitude": counts["ATTITUDE"],
        "attitude_hz": round(counts["ATTITUDE"] / duration, 3),
        "other": sum(counts.values()) - counts["HEARTBEAT"] - counts["ATTITUDE"],
        "senders": ";".join(f"{host}:{source_port}" for host, source_port in sorted(senders)),
    }
    with open(output_path, "w", newline="", encoding="utf-8") as stream:
        writer = csv.DictWriter(stream, fieldnames=result)
        writer.writeheader()
        writer.writerow(result)
