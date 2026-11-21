# PBL3-14 — baseline E0_02, 600 giây

| Điểm | Mẫu QoS | Mốc cuối | Frame | Byte frame | HEARTBEAT | Offline | Seq gap |
|---|---:|---:|---:|---:|---:|---:|---:|
| Gateway ingress | 120 | 600.000 s | 34074 | 1376377 | 558 | 0 | 0 |
| Gateway egress | 120 | 600.000 s | 34074 | 1376377 | 558 | 0 | 0 |
| TCP client RX | 120 | 600.000 s | 34053 | 1375463 | 558 | 0 | 0 |

Ingress và egress khớp frame/byte tại cả 120 mốc. Mỗi điểm có 0 mốc lệch khi cộng frame/byte theo MSGID (`3716` hàng MSGID ingress, `3716` egress, `3718` client). Client bắt đầu cửa sổ riêng nên chênh 21 frame với Gateway không được tính là mất gói.

Tài nguyên: 11 mẫu mỗi tiến trình trên 600.355 s. Gateway CPU tích lũy tăng 4.781 s (~0.796% một lõi trung bình), working set 6.672–6.773 MiB, 95 handles. TCP client CPU tăng 1.594 s (~0.265% một lõi), working set 6.430–6.504 MiB, 90 handles. Đây là CPU của tiến trình do Windows báo, không phải CPU toàn máy.

`metadata.log` lưu Git HEAD và SHA-256 của `gateway.cpp`, `tcpclient.cpp`, `qos.h`, `emulator.h`. Bản ghi phiên bản PX4, lệnh khởi động và commit/hash toàn bộ source sẽ hoàn thiện ở PBL3-15.
