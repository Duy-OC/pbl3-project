# PX4/Gazebo và MAVLink dùng cho E0_02

Nguồn: người đo cung cấp lệnh đã dùng và bản chụp `ver all`, `param show MAV_SYS_ID`, `mavlink status` sau run trong hội thoại. Người đo xác nhận UAV 1 **đứng trên mặt đất** trong E0_02; không có bản ghi rõ trạng thái Armed/Disarmed.

## Lệnh người đo cung cấp

```bash
cd ~/PX4-Autopilot
PX4_SYS_AUTOSTART=4001 PX4_GZ_MODEL_POSE="0,0,0,0,0,0" ./build/px4_sitl_default/bin/px4 -i 0
```

Lệnh tạo luồng MAVLink trên UAV 1:

```text
mavlink start -u 14600 -o 14550 -t 172.26.160.1 -r 100000
```

Trong môi trường hai UAV của các phép thử khác, người đo còn dùng:

```bash
PX4_SYS_AUTOSTART=4001 PX4_GZ_MODEL_POSE="2,0,0,0,0,0" ./build/px4_sitl_default/bin/px4 -i 1
```

```text
mavlink start -u 14601 -o 14550 -t 172.26.160.1 -r 100000
```

E0_02 chỉ đo **UAV 1**; không đưa UAV 2 vào điều kiện baseline.

## Phiên bản và stream

`ver all` sau run: PX4 SITL, Git `b3e343c1532fea4608cd373da3dcdb0fcc3fa763`, version `1.18.0 80 (17956992)`, branch `main`, build `Sep 28 2026 15:55:21`, Linux `6.18.33`, toolchain GCC `13.3.0`. `param show MAV_SYS_ID` cho giá trị `1`.

`mavlink status` sau run, instance #4: MAVLink 2, mode Normal, Forwarding Off, FTP No, local UDP 14600 → remote UDP 14550, TX max 100000 B/s, TX quan sát 2622.5 B/s, RX loss 0%. Bản chụp status này không hiện partner IP; lệnh khởi tạo ghi Windows IP `172.26.160.1` và log Gateway trong run học nguồn `172.26.162.42:14600`.
