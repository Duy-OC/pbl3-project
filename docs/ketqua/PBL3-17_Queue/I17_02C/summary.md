# I17_02C — hủy hàng đợi khi route UAV hết hiệu lực

- Cả Gateway và probe build exit 0. Probe gửi 44 byte MAV_CMD_REQUEST_MESSAGE=512, giữ kết nối 180 giây, sau đó TIMEOUT/exit 1 như dự kiến vì gói không được gửi đến UAV.
- Gateway ghi `Client #1 CONNECTED`, `QUEUE: target_system=1 hold_ms=300000 queued=1`; mẫu STATS trước khi route hết hạn có `Clients=1`, `UDP queued=1 | UDP dropped/cancelled=0`, `UDP TX frames=0`.
- Luồng UAV dừng ở `UDP RX datagrams=1536 | UDP RX bytes=61982`. Gateway ghi `UAV 1 OFFLINE` rồi ngay sau đó `QUEUE CANCEL: reason=route_offline sysid=1 count=1`. Mẫu STATS kế tiếp: `Clients=1`, `UDP queued=0 | UDP dropped/cancelled=1`, `UDP TX frames=0`, `Blocked messages=0`, CRC/signature errors=0.
- `Client #1 DISCONNECTED` chỉ xuất hiện về sau, khi queue đã trống. Do đó việc hủy một gói thuộc nhánh route OFFLINE, không phải nhánh client ngắt.

**Kết luận:** PASS I17_02C. Kết hợp I17_01, đã kiểm hai nguyên nhân hủy queue. Các nhánh queue đầy và `sendto` lỗi vẫn chưa đo.
