# PBL3-14 — baseline E0_02, 600 giây

Nguồn: `gateway.log`, `client.log`, `metadata.log`, `resources.csv`, config/Markov/CSV Gateway và hai CSV client trong cùng thư mục. Gateway Markov OFF (`urb=0`), seed 1, warm-up 10 s, cửa sổ đo 600 s. Dữ liệu chỉ có SYSID 1; Gateway ghi 135 mẫu `UAV online=1 | UAV da hoc=1 | Clients=1`, không có mẫu hai UAV online.

| Điểm | Mẫu QoS | Mốc cuối | Frame | Byte frame | HEARTBEAT | Offline | Seq gap |
|---|---:|---:|---:|---:|---:|---:|---:|
| Gateway ingress | 120 | 600.000 s | 34074 | 1376377 | 558 | 0 | 0 |
| Gateway egress | 120 | 600.000 s | 34074 | 1376377 | 558 | 0 | 0 |
| TCP client RX | 120 | 600.000 s | 34053 | 1375463 | 558 | 0 | 0 |

Ingress và egress khớp frame/byte tại cả 120 mốc. Mỗi điểm có 0 mốc lệch khi cộng frame/byte theo MSGID (`3716` hàng MSGID ingress, `3716` egress, `3718` client). Client bắt đầu cửa sổ riêng nên chênh 21 frame với Gateway không được tính là mất gói.

Markov summary ghi `frames_in=34074`, `frames_kept=34074`, `frames_dropped_emu=0`, ingress/egress QoS đều 34074, `run_bad_slots=0`. Config 175 byte có đầy đủ `scenario=E0`, `run_id=E0_02`, `urb=0`, `measure_s=600`, timeout route/ingress 5 s, egress 3 s. Log Gateway/client ghi CRC và signature errors=0; STATS không có BLOCK, dropped/cancelled hay slow client.

Tài nguyên: 11 mẫu mỗi tiến trình trên 600.355 s. Gateway CPU tích lũy tăng 4.781 s (~0.796% một lõi trung bình), working set 6.672–6.773 MiB, 95 handles. TCP client CPU tăng 1.594 s (~0.265% một lõi), working set 6.430–6.504 MiB, 90 handles. Đây là CPU của tiến trình do Windows báo, không phải CPU toàn máy.

`metadata.log` lưu Git HEAD và SHA-256 của `gateway.cpp`, `tcpclient.cpp`, `qos.h`, `emulator.h`. Bản ghi phiên bản PX4, lệnh khởi động và commit/hash toàn bộ source sẽ hoàn thiện ở PBL3-15.

**Kết luận:** PASS baseline E0_02 cho một UAV/một client, Markov OFF, ingress=egress, không có lỗi CRC/drop và đủ log/tài nguyên. Không dùng E0_01 config trống làm bản baseline chính.
