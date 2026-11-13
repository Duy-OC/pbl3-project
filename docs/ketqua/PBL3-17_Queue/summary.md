# PBL3-17 — route, hủy hàng đợi và lỗi gửi

- D1–D3: I09_01 chứng minh BLOCK/no-route khi UAV 2 offline và UAV 1 tiếp tục ACK; I09_02 chứng minh route phục hồi ở endpoint mới.
- D4: [I17_01](I17_01/summary.md) hủy 1 gói khi client ngắt; [I17_02C](I17_02C/summary.md) hủy 1 gói khi route OFFLINE; [I17_05](I17_05/summary.md) xác nhận gói cũ không được gửi sau khi UAV xuất hiện ở cổng 14602.
- D5: [I17_03](I17_03/summary.md) chạm giới hạn 512 gói và hủy đúng 512 gói đã xếp; [I17_04](I17_04/summary.md) ghi lỗi Winsock `sendto` 10014, tăng dropped 1, rồi lệnh kế tiếp được ACK.

**Kết luận:** PASS PBL3-17 trong điều kiện PX4 SITL và các chế độ thử queue/lỗi gửi đã ghi rõ trong config từng run. Lỗi `sendto` được gây có kiểm soát bằng đối số không hợp lệ; không đại diện cho tỷ lệ lỗi mạng tự nhiên.
