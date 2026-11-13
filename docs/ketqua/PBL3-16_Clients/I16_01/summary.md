# PBL3-16 — 20 lần reconnect, run I16_01

Nguồn: `reconnect.log`, `gateway.log`, `resources.csv` và sáu file Gateway trong cùng thư mục. Một UAV SYSID 1 tại `172.26.162.42:14600`, Markov OFF (`urb=0`). Mỗi `route_probe` tạo một TCP connection mới, gửi `command=512` yêu cầu message, nhận ACK rồi đóng.

- Probe: 20 dòng `cycle=1..20`, 20 ACK `sysid=1 command=512 result=0`, 20 exit code 0, không timeout.
- Gateway: 20 `Client #... CONNECTED`, 20 `DISCONNECTED`, 20 ROUTE tới UAV 1 và 20 ACK từ UAV 1. STATS cuối `Clients=0`, `Client RX=20`, `UDP TX frames=20`, CRC/signature errors=0, BLOCK=0, UDP dropped/cancelled=0, Slow clients=0.
- Tài nguyên: 5 mẫu Gateway từ vòng 1 đến 20; CPU tích lũy 0.0625→0.125 s, working set 7069696–7094272 byte, handles giữ 94 ở cả năm mẫu. Cửa sổ ngắn (~5.84 s) nên không suy CPU ổn định dài hạn từ các mẫu này.

**Kết luận:** PASS 20 reconnect có route/ACK và không rò số client/handle trong lượt ngắn. Chưa thử client không đọc, tải 1/5/10 và kết nối thứ 11; các lượt đó vẫn thuộc PBL3-16.
