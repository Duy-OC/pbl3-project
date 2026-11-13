# A2 - Khảo sát server UDP trực tiếp với PX4 (60 giây)

Nguồn: kết quả `A2 RESULT` do người đo cung cấp. Đây là lần khảo sát 60 giây, chưa phải một trong ba lần đo chính 600 giây.

| Chỉ số | Giá trị |
|---|---:|
| Warm-up | 10 s |
| Cửa sổ đo | 60 s |
| MAVLink frame hợp lệ | 3490 |
| MAVLink byte đã serialize | 141001 |
| Frame/s | 58.17 |
| Byte/s | 2350.02 |
| Byte/frame | 40.40 |
| CRC lỗi | 0 |
| Chữ ký lỗi | 0 |
| Nguồn UDP | `172.26.162.42:14600` |
| SYSID / COMPID | `1 / 1` |
| HEARTBEAT | 57 |
| Khoảng cách HEARTBEAT trung bình | 1049.26 ms |
| Jitter HEARTBEAT | 30.19 ms |

Tổng 31 nhóm MSGID bên dưới bằng đúng 3490 frame đã ghi nhận.

## Số frame theo MSGID

| MSGID | Frame | MSGID | Frame | MSGID | Frame |
|---:|---:|---:|---:|---:|---:|
| 0 | 57 | 1 | 57 | 4 | 6 |
| 8 | 57 | 24 | 286 | 29 | 57 |
| 30 | 858 | 31 | 572 | 32 | 57 |
| 33 | 286 | 36 | 57 | 42 | 57 |
| 74 | 229 | 83 | 114 | 85 | 86 |
| 141 | 57 | 147 | 29 | 230 | 29 |
| 241 | 6 | 242 | 29 | 245 | 57 |
| 290 | 57 | 291 | 57 | 380 | 28 |
| 411 | 19 | 436 | 29 | 441 | 57 |
| 514 | 29 | 515 | 57 | 12901 | 57 |
| 12904 | 57 | | | | |

## Đối chiếu với A1

A1 ghi instance #0 có UDP local port `18570`, remote port `14550`, `tx` khoảng 15656–15688 B/s. A2 nhận UDP từ cổng nguồn `14600` và đo `2350.02 B/s`. Chưa xác nhận A2 đang nhận đúng cùng MAVLink instance và stream như A1. Không dùng hai tốc độ này để kết luận gateway hay đường truyền làm mất dữ liệu. Cần chạy `mavlink status` khi A2 đang nhận và đối chiếu local port, remote port, partner IP trước các lần đo chính.

Chưa có lệnh `mavlink start`, thời điểm chạy, phiên bản PX4 hoặc mô tả trạng thái UAV để gắn với run này.
