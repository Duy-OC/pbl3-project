# PBL3-11 — STATS tách CRC UDP/TCP

Nguồn: `probe.log`, `gateway.log` và sáu file cấu hình/CSV của run `I11_01`.

Probe tạo một HEARTBEAT `sysid=42`, 21 byte, sửa một byte CRC rồi gửi 21 byte qua UDP 14550 và 21 byte qua TCP 5760; exit 0 chỉ xác nhận gửi. Trước probe, Gateway ghi `CRC errors=0 (UDP=0, TCP=0)`. Sau probe, STATS ghi `UDP RX datagrams=1`, `UDP RX bytes=21`, `MAVLink UAV RX=0`, `Client RX=0`, `CRC errors=2 (UDP=1, TCP=1)`. Các mẫu STATS sau đó giữ nguyên kết quả. Không có UAV hợp lệ được học.

Trong cùng STATS, `Blocked messages=0`, `UDP dropped/cancelled=0`, `Slow clients=0`. I09_01 đã kích hoạt riêng nhánh `Blocked messages=1 (no route=1)` bằng probe tới UAV offline. Counter queue và slow client vẫn phải kích hoạt ở PBL3-17 và PBL3-16; số 0 ở lượt này chỉ cho thấy CRC sai không bị tính lẫn sang các nhóm đó.

**Kết luận:** PASS tách và đếm CRC UDP/TCP, loại hai frame sai; STATS tách khỏi no-route. Nhánh queue/slow client chưa nghiệm thu trong lượt này.
