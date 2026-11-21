# I17_05 — lệnh cũ không sang endpoint mới

- Gateway học UAV 1 từ `172.26.162.42:14600`, nhận một lệnh (`UDP queued=1`, `UDP TX frames=0`). Khi luồng cũ dừng, Gateway ghi `UAV 1 OFFLINE` rồi `QUEUE CANCEL: reason=route_offline sysid=1 count=1`.
- STATS ngay sau hủy: `Clients=1`, `UDP queued=0 | UDP dropped/cancelled=1`, `UDP TX frames=0`. UAV 1 trở lại trong cùng phiên Gateway tại `172.26.162.42:14602`; các STATS tiếp theo khi client cũ vẫn kết nối giữ queue=0 và UDP TX=0.
- Probe cũ TIMEOUT sau 180 giây, exit 1, không nhận ACK. Gateway chỉ ghi `Client #1 DISCONNECTED` sau các mẫu STATS tại endpoint mới. CRC/signature errors=0 và `Blocked messages=0` trong lượt này.

**Kết luận:** PASS điều kiện request cũ đã bị hủy, không chuyển tiếp sang endpoint mới. Bằng chứng cổng mới là log Gateway; lượt này không có file `mavlink status` riêng từ PX4.
