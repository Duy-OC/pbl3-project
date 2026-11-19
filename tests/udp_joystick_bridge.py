import select
import socket

UDP_PORT = 14560
GATEWAY = ("127.0.0.1", 5760)

with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as udp:
    udp.bind(("0.0.0.0", UDP_PORT))

    with socket.create_connection(GATEWAY, timeout=3) as tcp:
        tcp.settimeout(1)
        print(f"Joystick UDP :{UDP_PORT} -> Gateway TCP {GATEWAY[1]}")

        while True:
            ready, _, _ = select.select([udp, tcp], [], [])

            if tcp in ready:
                # Đọc telemetry để Gateway không xếp hàng cho client chậm.
                if not tcp.recv(65536):
                    print("Gateway đã đóng kết nối.")
                    break

            if udp in ready:
                packet, _ = udp.recvfrom(65535)
                if packet:
                    tcp.sendall(packet)