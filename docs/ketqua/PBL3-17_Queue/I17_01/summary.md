# I17_01 — hủy hàng đợi khi client ngắt

Nguồn: `build.log`, `probe.log`, `gateway.log` và sáu file Gateway thô trong thư mục này. Một PX4 UAV SYSID 1 tại `172.26.162.42:14600`; Gateway Markov OFF, `test_hold_udp_ms=10000`. Đây là lượt kích hoạt nhánh hàng đợi, không phải phép đo QoS 600 giây.

- Build Gateway: exit 0. Probe gửi 44 byte MAV_CMD_REQUEST_MESSAGE=512 đến SYSID 1, hết hạn sau 5 giây và exit 1 như dự kiến khi lệnh còn trong hàng đợi.
- Gateway: `Client RX=1`; log `QUEUE: target_system=1 hold_ms=10000 queued=1`, sau đó `Client #1 DISCONNECTED` và `QUEUE CANCEL: reason=client_disconnected count=1`.
- Mẫu STATS trước hủy: `UDP queued=1 | UDP dropped/cancelled=0`. Mẫu sau hủy: `UDP queued=0 | UDP dropped/cancelled=1`; `UDP TX frames=0`, `Blocked messages=0`, CRC/signature errors=0. UAV 1 vẫn ONLINE.
- CSV ingress và egress cùng ghi 2267 frame/91915 byte ở mốc cuối `t=40.208 s`; luồng UAV không bị emulator làm rơi. Mốc này chỉ là dữ liệu phụ cho lượt nhánh, không chứng minh độ ổn định 600 giây.

**Kết luận:** PASS I17_01, hủy đúng một gói của client đã ngắt. Nhánh route offline và queue đầy vẫn cần lượt riêng.
