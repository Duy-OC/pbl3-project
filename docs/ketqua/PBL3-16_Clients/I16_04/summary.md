# PBL3-16 — tải một TCP client, run I16_04

- Helper: `frames=33986`, `bytes=1377189`, `heartbeats=557`, CRC/signature errors=0, `disconnected=0`, `pass=1`, exit 0. Mỗi mẫu 5 giây có `active=1` đến `t=600`.
- `resources.csv`: Gateway 11 mẫu trên 600.3 s, CPU tích lũy tăng 4.531 s, working set 6.68–6.76 MiB, handle 94–95. Helper có 10 mẫu trên 540.3 s trước khi tiến trình kết thúc, CPU tăng 1.125 s, working set 6.59–6.69 MiB, handle 96.

Helper và Gateway bắt đầu cửa sổ đo riêng, nên chênh 3 frame giữa tổng hai điểm không dùng làm gói mất. Đây là mốc 1 client dùng để so cùng cách đếm với mức 5 và 10 client.

**Kết luận:** PASS mức tải 1 client trong 600 s. PBL3-16 còn mức 5 và 10 client.
