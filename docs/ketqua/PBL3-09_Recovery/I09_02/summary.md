# PBL3-09 — UAV 2 phục hồi với endpoint mới, run I09_02

Nguồn: `gateway.log`, `probe_uav2_recovered.log`, sáu file CSV/config của run trong cùng thư mục.

Trong **một phiên Gateway**, log ghi đúng trình tự:

1. `UAV 2 ONLINE tai 172.26.162.42:14601`; STATS `UAV online=2`.
2. QoS egress ghi UAV 2 OFFLINE tại `t=25.8414 s`, ingress tại `t=27.8414 s`; Gateway ghi `UAV 2 OFFLINE`, STATS còn `UAV online=1`. Counter UAV 2 đứng ở RX=1852 trong nhiều mẫu, UAV 1 tiếp tục online.
3. `UAV 2 ONLINE tai 172.26.162.42:14602`; STATS trở lại `UAV online=2`.
4. `ROUTE: Client #1 -> UAV 2 -> 172.26.162.42:14602`, `ACK UAV 2 | command=512 | result=0`. Probe xác nhận ACK `sysid=2`, exit 0. Không có ROUTE mới tới 14601 sau khi phục hồi.

Cuối log: UDP RX=12679 datagram/513504 byte, `MAVLink UAV RX=12679`, UAV 1 RX=8076, UAV 2 RX=4537, CRC/signature errors=0, blocked=0. CSV ingress ở `t=130.544 s` ghi UAV 1 online, 7500 frame; UAV 2 nguồn cũ 14601 offline, 1328 frame, một offline event; UAV 2 nguồn mới 14602 online, 2685 frame. CSV bỏ warm-up nên counter khác STATS.

**Kết luận:** PASS phục hồi và đổi endpoint. Gộp với I09_01 đã PASS OFFLINE/BLOCK và UAV 1 vẫn phục vụ: PBL3-09 PASS. Kiểm queue cũ bị hủy thuộc PBL3-17, chưa suy từ lượt này.
