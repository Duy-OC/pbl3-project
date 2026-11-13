# PBL3-08 — hai UAV qua Gateway, run I08_01

Nguồn: `gateway.log`, sáu file Gateway CSV/config trong thư mục này và xác nhận của người đo rằng QGC thấy hai thiết bị. Cửa sổ CSV kết thúc ở `t=45.193 s` sau warm-up 10 s; đây là ca kiểm định tuyến ngắn, không phải phép đo 600 s.

| UAV | SYSID | Endpoint Gateway học | Ingress frame/byte | Egress frame/byte | HEARTBEAT | Ước lượng seq gap |
|---|---:|---|---:|---:|---:|---:|
| 1 | 1 | 172.26.162.42:14600 | 2697 / 108316 | 2697 / 108316 | 43 | 0 |
| 2 | 2 | 172.26.162.42:14601 | 2691 / 108042 | 2691 / 108042 | 43 | 0 |

Gateway ghi `UAV online=2` ở các mẫu STATS liên tiếp, một QGC client kết nối qua TCP, CRC UDP/TCP = 0, signature errors = 0. Không thấy `CONFLICT`, `BLOCK` hay `OFFLINE` trong log.

Log có 36 dòng ROUTE tới mỗi endpoint. Có 31 ACK `command=512,result=0` từ mỗi UAV, 4 ACK `command=512,result=2` từ mỗi UAV và 1 ACK `command=401,result=0` từ mỗi UAV. `512` là yêu cầu message; `401` là arm/disarm. Log hiện chưa ghi tham số hoặc thao tác cụ thể của người đo, nên không dùng các ACK này để khẳng định tác động chọn riêng từng UAV. `result=2` là ACK từ chối, cần giữ nguyên trong báo cáo.

**Kết luận:** đạt kiểm tra hai SYSID/endpoint, telemetry tới QGC và route/ACK đến từng cổng. Còn thiếu ca thao tác chọn riêng UAV 1 rồi UAV 2 với quan sát trạng thái UAV còn lại; thực hiện ở I08_02 trước khi đánh dấu toàn PBL3-08 PASS.
