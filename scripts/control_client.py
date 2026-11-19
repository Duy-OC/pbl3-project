"""Interactive line-protocol client for the gateway control port."""

import argparse
import socket
import threading
from datetime import datetime
from pathlib import Path


def stamp():
    return datetime.now().astimezone().isoformat(timespec="milliseconds")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("host", nargs="?", default="127.0.0.1")
    parser.add_argument("port", nargs="?", type=int, default=5760)
    parser.add_argument("--log-file", type=Path)
    args = parser.parse_args()

    with socket.create_connection((args.host, args.port)) as conn:
        log = args.log_file.open("w", encoding="utf-8") if args.log_file else None
        log_lock = threading.Lock()

        def emit(message):
            with log_lock:
                print(message, flush=True)
                if log:
                    log.write(message + "\n")
                    log.flush()

        conn.sendall(b"GET_DETAIL\n")
        emit("Commands: GET_DETAIL [sysid], SUBSCRIBE 1..5, UNSUBSCRIBE,"
             " ACQUIRE, RELEASE, COMMAND sysid ARM|TAKEOFF|LAND [altitude_m]")

        def receive():
            buffer = b""
            try:
                while True:
                    chunk = conn.recv(4096)
                    if not chunk:
                        emit("Gateway disconnected")
                        return
                    buffer += chunk
                    while b"\n" in buffer:
                        line, buffer = buffer.split(b"\n", 1)
                        emit(f"{stamp()} RX {line.decode('utf-8', errors='replace')}")
            except OSError:
                pass

        receiver = threading.Thread(target=receive, daemon=True)
        receiver.start()
        try:
            while True:
                line = input("> ").strip()
                if line.upper() in ("QUIT", "EXIT"):
                    break
                if line:
                    emit(f"{stamp()} TX {line}")
                    conn.sendall((line + "\n").encode("ascii"))
        except (EOFError, KeyboardInterrupt, OSError):
            pass
        finally:
            try:
                conn.shutdown(socket.SHUT_RDWR)
            except OSError:
                pass
            receiver.join(timeout=1)
            if log:
                log.close()


if __name__ == "__main__":
    main()
