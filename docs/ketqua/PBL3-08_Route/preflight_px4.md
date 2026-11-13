# PBL3-08 — kiểm tra hai PX4 trước khi chạy Gateway

Nguồn: hai bản `param show MAV_SYS_ID` và `mavlink status` người đo gửi trong hội thoại. Chưa có log Gateway hoặc QGC của run I08_01; đây là bằng chứng cấu hình PX4, không phải kết quả định tuyến.

| PX4 | MAV_SYS_ID | MAVLink instance phục vụ phép đo | Endpoint nguồn | Đích | Tốc độ TX tại lúc chụp |
|---|---:|---:|---|---|---:|
| UAV 1 | 1 | #4, Normal, Forwarding Off | UDP 14600 | 172.26.160.1:14550 | 2306.8 B/s |
| UAV 2 | 2 | #4, Normal, Forwarding Off | UDP 14601 | 172.26.160.1:14550 | 2302.8 B/s |

UAV 1 instance #4 còn ghi `dropped packets: 101` trong phần ping statistics. Chỉ số này thuộc ping, chưa đủ để kết luận 101 frame telemetry bị mất. Khi đo Gateway cần kiểm `CRC errors`, `rx loss`, frame ingress và endpoint học thực tế.

Hai PX4 đều còn có instance #0 hướng tới UDP 14550: UAV 1 local 18570, UAV 2 local 18571. Vì Gateway học endpoint từ HEARTBEAT đầu tiên của một SYSID và bỏ qua endpoint khác cùng SYSID khi route còn online, trước khi phát lệnh phải kiểm log `UAV 1 ONLINE tai ...:14600` và `UAV 2 ONLINE tai ...:14601`. Nếu log học 18570/18571 hoặc có `CONFLICT`, dừng phép đo định tuyến và xử lý luồng trùng trước.

**Trạng thái:** đạt phần kiểm SYSID và cấu hình MAVLink nguồn; chưa kiểm Gateway thấy hai UAV online, QGC thấy hai UAV, ROUTE hoặc ACK.
