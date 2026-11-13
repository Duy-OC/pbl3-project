# PBL3-09 — ngắt UAV 2, kiểm BLOCK và UAV 1 tiếp tục

Nguồn: `gateway.log`, `probe_uav2_offline.log`, `probe_uav1_online.log` và sáu file thô trong thư mục run. Cấu hình `I09_ROUTE`, `I09_01`, `urb=0`.

- Gateway học UAV 1 tại `172.26.162.42:14600`, UAV 2 tại `172.26.162.42:14601`; trước ngắt có nhiều STATS `UAV online=2`.
- QoS egress ghi UAV 2 OFFLINE tại `t=33.595 s`, QoS ingress tại `t=35.595 s`; Gateway ghi `UAV 2 OFFLINE`, STATS chuyển `UAV online=1`. Ở mẫu cuối, UAV 1 vẫn ONLINE, RX=8140; UAV 2 OFFLINE, RX dừng ở 2409 (counter Gateway tính cả warm-up).
- Probe tới SYSID 2 gửi 44 byte `command=512`, hết 5 giây không có ACK, exit 1. Gateway ghi `BLOCK: target_system=2 offline/chua hoc | MSGID=76` và `Blocked messages=1 (no route=1, unknown metadata=0)`.
- Probe tới SYSID 1 gửi 44 byte `command=512`, nhận `ACK: sysid=1 command=512 result=0`, exit 0. Gateway ghi `ROUTE: Client #2 -> UAV 1 -> 172.26.162.42:14600` và ACK từ UAV 1.
- Cuối run, `CRC errors=0`, `Signature errors=0`, `UDP dropped/cancelled=0`. CSV ingress tại `t=130.586 s` có UAV 1: 7578 frame/307266 byte, online, 0 offline event; UAV 2: 1825 frame/73990 byte, offline, 1 offline event, offline 94.991 s. CSV bỏ warm-up 10 s nên các tổng này khác counter Gateway.

**Kết luận:** PASS giai đoạn offline/không có route và cô lập UAV 1. Chưa đo phục hồi UAV 2 hoặc đổi endpoint; làm ở I09_02.
