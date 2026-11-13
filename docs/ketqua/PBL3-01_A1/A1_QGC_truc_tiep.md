# A1 - QGC nối trực tiếp với PX4

Nguồn: ba kết quả `mavlink status` do người đo cung cấp. Chưa có thời điểm lấy mẫu, lệnh `mavlink start`, ảnh chụp hoặc mô tả quan sát QGC.

## Instance #0 - đường GCS cần theo dõi

| Chỉ số | Mẫu 1 | Mẫu 2 | Mẫu 3 |
|---|---:|---:|---:|
| `tx` (B/s) | 15688.2 | 15656.4 | 15656.4 |
| `txerr` (B/s) | 0.0 | 0.0 | 0.0 |
| `rx` (B/s) | 20.9 | 20.9 | 20.9 |
| `rx loss` | 0.0% | 0.0% | 0.0% |
| GCS messages total | 81 | 116 | 127 |
| GCS messages lost | 0 | 0 | 0 |
| Ping mean (ms) | 2.50 | 0.31 | 0.16 |
| Ping max (ms) | 8.00 | 8.00 | 8.00 |
| Ping dropped packets | 0 | 0 | 0 |

- `GCS heartbeat valid` ở cả ba mẫu.
- MAVLink 2, UDP local port `18570`, remote port `14550`, partner IP `172.26.160.1` ở cả ba mẫu.
- Mẫu 2 và 3 ghi nhận HEARTBEAT từ GCS khoảng `1.1 Hz` trong `Received Messages`.
- Instance #1: Onboard, UDP `14580 → 14540`, `tx` lần lượt `26002.0`, `26043.8`, `25757.0 B/s`.
- Instance #2: Onboard, UDP `14280 → 14030`, `tx` lần lượt `4436.3`, `4650.4`, `4341.6 B/s`.
- Instance #3: Gimbal, UDP `13030 → 13280`, `tx` lần lượt `1411.4`, `1333.7`, `1333.7 B/s`.

## Nhận xét và phần còn thiếu

Instance #0 cho thấy PX4 đang kết nối với GCS, có heartbeat và không báo lỗi truyền trong ba lần lấy mẫu. `rx loss` đo chiều PX4 nhận, không chứng minh chiều PX4 → QGC không mất gói. `tx` của instance #0 là tốc độ PX4 báo tại thời điểm lấy mẫu, không phải byte/s đo độc lập ở phía nhận.

Chưa thể xác nhận đủ tiêu chí A1 (QGC hiển thị UAV và cập nhật liên tục trong 2 phút) vì chưa có mô tả quan sát. Khi có thông tin đó, bổ sung vào đây; không suy đoán từ `mavlink status`.
