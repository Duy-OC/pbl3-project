# Môi trường đo K0–K7 (14/11/2026)

| Nhóm | Giá trị đã xác minh | Nguồn |
|---|---|---|
| CPU | Intel Core i5-12450H, 8 nhân, 12 luồng | Windows WMI |
| RAM | 16.782.360.576 byte (khoảng 15,63 GiB) | Windows WMI |
| Ổ C: | 276.600.713.216 byte, trống 31.116.820.480 byte | Windows WMI |
| Ổ D: | 234.386.092.032 byte, trống 122.240.466.944 byte | Windows WMI |
| Windows | Windows 11 Pro, build 26200 | Windows WMI |
| WSL | WSL 3.0.5, distro Ubuntu 24.04 chạy WSL2 | `wsl --version`, `wsl --status` |
| Mạng WSL | NAT; WSL `172.26.162.42/20`, default gateway và IP phía Windows `172.26.160.1` | `ipconfig`, `wsl --exec ip route` |
| PX4 | SITL; MAVLink 2, mode Normal; UDP local 14600 → Windows `172.26.160.1:14550`, rate tối đa 100.000 B/s | PX4 shell `mavlink status` |
| PX4 stream | Probe 10 s nhận 531 frame, 10 HEARTBEAT, 92 ATTITUDE; nguồn `172.26.162.42:14600` | `scripts/probe_px4.py` |
| Phiên bản PX4 | 1.18.0 80, nhánh `main`, git `b3e343c1532fea4608cd373da3dcdb0fcc3fa763`, build 13/11/2026 13:32:26 | PX4 shell `ver all` |
| Gazebo | Gazebo Sim 8.15.0, world `default` | `gz sim --versions`, `pgrep -af gz` trong WSL |
| Model và giao diện | `x500_0` (mẫu x500), có GUI (`gz sim -g`) | `gz model --list`, `pgrep -af gz` trong WSL |
| MinGW g++ | GCC 6.3.0; build `-O2 -std=c++17` | `g++ --version`, lệnh build |
| pymavlink | 2.4.50 | Python package |
| Wireshark | Chưa dùng trong các lượt đo này | Nhật ký đo |
| Cổng | K1: PX4 → gateway UDP 14550; K2–K7: PX4 → proxy UDP 14550 → gateway UDP 14551; TCP gateway 5760 | Mã nguồn và lệnh chạy |
| Timeout UAV | 3 giây | `src/common/config.h` |
| SUBSCRIBE | 1–5 Hz theo từng client | Giao thức gateway |
| Hàng đợi TCP | Tối đa 256 KiB mỗi client; tối đa 10 client | `src/common/config.h` |
| Cửa sổ đo | K1–K2: 5 giây ổn định + 120 giây, 3 lượt; từ K3: 5 giây ổn định + 60 giây, 2 lượt | Lệnh chạy thực tế |

PX4, Gazebo, proxy và gateway chạy trên cùng máy. Độ biến thiên lịch trình của Windows/WSL2 và tải nền góp vào jitter nhận được; báo cáo dùng K1 làm mốc nền.

**Lưu ý về quy mô:** tại luồng khoảng 53 frame/s, một lượt 120 giây có khoảng 6.000–7.000 frame; một lượt 60 giây có khoảng 3.000–3.500 frame. K0 dùng đúng 100.000 gói tổng hợp *mỗi lượt*.
