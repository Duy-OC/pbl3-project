# PBL3-16 — client không đọc, run I16_02

- Gateway ghi Client #1 và #2 CONNECTED, nhiều STATS `Clients=2`. Sau `QOS t=160.699 s`, ghi `Client qua cham`, `Client #2 DISCONNECTED | TCP TX=67661 bytes`; STATS kế tiếp `Clients=1`, `Slow clients=1`. Đây là client không đọc theo thứ tự kết nối của lượt thử.
- Client thường tiếp tục in HEARTBEAT và tăng `Frames` từ trước sự kiện tới cuối log (`Frames=38619`, TCP RX=1564923 byte ở mẫu cuối đọc được), CRC/signature errors=0. CSV client thường đủ 120 mốc đến 600 s, 33906 frame/1373929 byte, 556 HEARTBEAT.
- `resources.csv` có 11 mẫu mỗi tiến trình trong 600.3 s. Gateway CPU tích lũy tăng 4.359 s, working set 6.66–7.20 MiB, handle 95–96; client thường CPU tăng 1.453 s, working set 6.45–6.54 MiB, handle 90.

**Kết luận:** PASS phát hiện/ngắt client không đọc và cô lập client thường. Chênh frame giữa Gateway và client thường là do cửa sổ đo khởi động độc lập, không dùng làm tỷ lệ mất gói. Tải 1/5/10 và kết nối thứ 11 còn phải đo.
