# I17_03 — giới hạn hàng đợi UDP 512 gói

- Gateway/probe build exit 0. Probe gửi 513 frame/22572 byte MAV_CMD_REQUEST_MESSAGE tới SYSID 1, nhận `peer_closed=1`, exit 0.
- Gateway xếp lần lượt tới `queued=512`; frame thứ 513 gặp `QUEUE FULL: client=1 queued=512 incoming=1 limit=512`. Ngay sau đó Gateway ghi `Client #1 DISCONNECTED` và `QUEUE CANCEL: reason=client_disconnected count=512`.
- Mẫu STATS sau hủy: `Client RX=513`, `UDP queued=0 | UDP dropped/cancelled=512`, `UDP TX frames=0`, `Blocked messages=0`, CRC/signature errors=0. UAV 1 vẫn ONLINE. 512 gói đã xếp bị hủy; frame thứ 513 bị từ chối trước khi vào queue.

**Kết luận:** PASS giới hạn 512 và xử lý queue đầy. Nhánh `sendto` lỗi vẫn cần lượt riêng.
