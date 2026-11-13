# I17_04 — lỗi `sendto` và phục hồi ở lệnh kế tiếp

Nguồn: `gateway_build.log`, `probe_first.log`, `probe_second.log`, `gateway.log` và sáu file Gateway thô trong thư mục này. PX4 UAV 1 SYSID 1 tại `172.26.162.42:14600`; Gateway Markov OFF, `test_hold_udp_ms=0`, `test_sendto_invalid_addrlen_once=1`.

- Gateway build exit 0. Probe đầu gửi 44 byte MAV_CMD_REQUEST_MESSAGE=512, TIMEOUT sau 5 giây, exit 1. Gateway ghi `TEST SENDTO: first queued frame uses addrlen=0` rồi Winsock `sendto loi: 10014`.
- STATS sau lỗi: `Client RX=1`, `UDP TX frames=0`, `UDP queued=0 | UDP dropped/cancelled=1`; CRC/signature errors=0, `Blocked messages=0`; UAV 1 vẫn ONLINE.
- Probe thứ hai gửi cùng request, nhận `ACK: sysid=1 command=512 result=0`, exit 0. Gateway ghi `ACK UAV 1 | command=512 | result=0`; STATS tiếp theo `Client RX=2`, `UDP TX frames=1`, `UDP queued=0 | UDP dropped/cancelled=1`.

**Kết luận:** PASS nhánh lỗi `sendto` và phục hồi sau lỗi. Lỗi được kích hoạt bằng đối số địa chỉ không hợp lệ trong bản thử; kết quả chứng minh nhánh xử lý lời gọi Winsock thất bại, không đo xác suất lỗi mạng tự nhiên.
