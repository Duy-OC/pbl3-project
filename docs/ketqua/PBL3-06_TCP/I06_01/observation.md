# PBL3-06 — QGC nhận telemetry qua Gateway TCP

Nguồn: người đo cung cấp STATS/QOS Gateway và xác nhận QGC hiển thị độ cao, tốc độ cùng các thông số khác. Đây là ca kiểm tra kết nối, không phải run QoS 600 giây. Sau khi Gateway dừng, sáu file thô đã được chuyển vào cùng thư mục này.

## Quan sát người đo

- QGC hiển thị độ cao, tốc độ và các thông số telemetry.
- Gateway báo `Clients=1`, `UAV online=1` tại các STATS quanh mốc QOS `t=120.432`, `125.445`, `130.469`, `135.488 s`.
- Tại mốc cuối người đo gửi: `UDP RX datagrams=8630`, `UDP RX bytes=350218`, `MAVLink UAV RX=8630`, `Client RX=172`, `TCP TX bytes=266239`, `UDP TX frames=172`, `CRC errors=0 (UDP=0, TCP=0)`, `Signature errors=0`, `Blocked messages=0`, `Slow clients=0`, `UAV 1: ONLINE`.
- Hai dòng QOS ở `t=135.488 s` đều ghi `frames=8036` (ingress và egress trong cấu hình Markov OFF).
- File cấu hình ghi `scenario=I06_TCP`, `run_id=I06_01`, `urb=0`, `dur_s=2`, `slot_ms=100`, `lossgood=0`, `lossbad=1`, `seed=1`, warm-up 10 s và cửa sổ mục tiêu 600 s.
- CSV thực tế có 75 mốc, dòng cuối `t=376.286 s`: ingress=egress=22045 frame, 893441 byte, 359 HEARTBEAT; 42 MSGID ở mỗi điểm và tổng MSGID cuối khớp 22045 frame. Run dừng trước 600 s, nên không có Markov summary hoàn tất.

## Kết luận

**PASS trong phạm vi PBL3-06:** Gateway có một client TCP, gửi byte tới client, QGC hiển thị và cập nhật telemetry quá 2 phút; CSV client console 600 giây của E0 trước đó bổ sung bằng chứng TCP client nhận HEARTBEAT/frame.

`Client RX` trong STATS là frame từ QGC đi vào Gateway, **không phải** số frame QGC nhận. `TCP TX bytes` là byte Gateway đã gửi, còn quan sát màn hình QGC chứng minh ứng dụng nhận và giải mã dữ liệu. Không dùng ca 2 phút này làm baseline QoS 600 giây.

Chưa có ảnh chụp QGC hoặc dòng `Client #... CONNECTED` riêng trong thông tin người đo gửi. Không có log terminal đầy đủ của ca này; đoạn STATS/QOS người đo đã chép được giữ nguyên trong `gateway_excerpt.log`.
