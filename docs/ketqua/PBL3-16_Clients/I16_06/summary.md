# PBL3-16 — tải mười TCP client, run I16_06

Nguồn: `client_load.log`, `gateway.log`, `resources.csv` và bảy file Gateway trong cùng thư mục. Một PX4 UAV SYSID 1, Markov OFF, helper mười client, warm-up 10 s, đo 600 s; cùng executable helper với I16_04/05.

- Helper: 10 client đều active tới `t=600 s`, mỗi client 34078–34079 frame, 1381049–1381089 byte, 558 HEARTBEAT; tổng 340781 frame/13810530 byte/5580 HEARTBEAT. CRC/signature errors=0, disconnected=0, `pass=1`, exit 0.
- Gateway: ingress=egress ở mốc 600 s: 34084 frame/1381283 byte, 558 HEARTBEAT; Markov `frames_in=frames_kept=34084`, drop=0. Có 122 mẫu STATS `Clients=10`, 10 CONNECTED và 10 DISCONNECTED sau khi helper kết thúc. CRC/BLOCK/Slow clients=0. TCP TX cuối log 14048020 byte.
- Tài nguyên: 11 mẫu trên 600.2 s. Gateway CPU tích lũy tăng 11.328 s, working set 6.67–6.74 MiB, handles giữ 104. Helper CPU tăng 7.688 s, working set 6.23–6.32 MiB, handles giữ 97.

**Kết luận:** PASS mức tải 10 client trong 600 s. Kết hợp I16_01 (20 reconnect), I16_02 (slow client), I16_03 (giới hạn 10/11) và I16_04/05 (1/5 client): PBL3-16 PASS trong điều kiện PX4 SITL này.
