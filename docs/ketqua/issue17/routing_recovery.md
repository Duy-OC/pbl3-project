# Issue 17 — Hồ sơ routing và các nhánh còn thiếu bằng chứng

Ngày kiểm tra tài liệu: 07/11/2026.

Nguồn: `./routing_recovery.log` từ gói tải lên. Đây là log phục hồi cũ, không phải kết quả mới của toàn bộ issue 17. Log đồng thời phục vụ lịch sử issue 08–09; giữ một bản duy nhất và tham chiếu dùng chung.

## Bằng chứng có trong log

- Học UAV 1 tại 172.26.162.42:14600 và UAV 2 tại :14601.
- Có UAV 2 OFFLINE/ONLINE, học endpoint :14602.
- Có ROUTE tới endpoint mới và ACK từ UAV 2.
- Có ACK command=22/result=0 từ cả UAV 1 và UAV 2. ACK chấp nhận lệnh không thay thế video quan sát cất cánh.
- 111 dòng STATS có UDP dropped/cancelled=0, Blocked messages=0.
- Không tìm thấy dòng BLOCK:, Hang doi UDP day hoặc sendto loi.

## Trạng thái nghiệm thu

| Ca | Trạng thái từ gói hiện tại | Bằng chứng còn cần |
|---|---|---|
| Điều khiển đúng UAV Takeoff/Land | Chưa đủ | Video/quan sát không tác động nhầm UAV |
| Gửi request sau OFFLINE, bị BLOCK | Chưa có | OFFLINE rồi request đích cụ thể và no route tăng |
| Hủy request chờ khi đổi phiên route | Chưa có | Queue có request cũ, đổi phiên, log hủy |
| Hủy request chờ khi client đóng | Chưa có | Queue có request, disconnect, log hủy |
| Queue đầy / lỗi sendto có kiểm soát | Chưa có | Cách gây lỗi và log/counters |
| Phục hồi endpoint, route mới | Có bằng chứng lịch sử | Giữ log gốc, gắn source/run khi bổ sung |

**Chưa đủ bằng chứng đóng toàn bộ issue 17.** Blocked tăng lúc khởi tạo trong log issue 16 không thay thế ca gửi request tới UAV đã online rồi offline. Không coi counters hủy bằng 0 là đã kiểm tra nhánh hủy.
