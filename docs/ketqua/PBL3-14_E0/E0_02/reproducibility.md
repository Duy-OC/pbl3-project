# PBL3-15 — dấu vết tái lập baseline E0_02

Nguồn: `metadata.log` ghi trước E0_02 lúc 14:53:57 +07, `source_manifest.log` ghi sau run lúc 15:10:14 +07, `E0_E0_02_config.txt`, log/CSV, `PBL3-10_Build/I10_01/build.log`, và `ver all`/`param show MAV_SYS_ID`/`mavlink status` do người đo gửi sau run.

| Trường | Giá trị đã xác nhận | Giới hạn |
|---|---|---|
| Gateway run | `scenario=E0`, `run_id=E0_02`, `urb=0`, `seed=1`, warm-up 10 s, đo 600 s | Config trong thư mục run |
| Gateway network | UDP `0.0.0.0:14550`, TCP `127.0.0.1:5760` | Gateway log |
| UAV/Client | Một UAV SYSID 1 nguồn `172.26.162.42:14600`, một TCP client | Gateway log/CSV; 135 mẫu STATS Clients=1 |
| Git HEAD | `79fd39fd73371dfea43d092ee4a78994e046d607` | Working tree có thay đổi chưa commit; không dùng HEAD đơn lẻ để xác định source |
| MAVLink submodule | `6a7760569ce6d46c56f93163703543d4345e940a` | `source_manifest.log` sau run |
| Source | SHA-256 của 12 file `src` trong `source_manifest.log`; hash bốn file chính ở `metadata.log` trước run khớp manifest sau run | Cả 12 file có LastWriteTime trước lúc đo; timestamp không thay cho hash lịch sử của tám header còn lại |
| Compiler build Gateway/client | MinGW.org GCC 6.3.0, C++17 `-O2`, hai executable build riêng, exit 0 | `PBL3-10_Build/I10_01/build.log` |
| PX4 | PX4 SITL 1.18.0 80, Git `b3e343c1532fea4608cd373da3dcdb0fcc3fa763`, branch main, build 2026-09-28 15:55:21, GCC 13.3.0, Linux 6.18.33 | `ver all` do người đo gửi **sau** run; chưa có bản chụp phiên bản ngay trong run |
| PX4 SYSID | `MAV_SYS_ID=1` | `param show` sau run; run Gateway cũng nhận SYSID 1 |
| PX4 stream | instance #4, MAVLink 2, Normal, Forwarding Off, FTP No, local UDP 14600 → remote UDP 14550, TX max 100000 B/s; Gateway học nguồn 14600 trong run | `mavlink status` sau run; IP partner không hiện ở bản chụp này, bản chụp trước đó ghi Windows `172.26.160.1` |

Người đo đã bổ sung lệnh khởi động PX4/Gazebo và lệnh `mavlink start` gốc, lưu trong `px4_environment.md`. UAV 1 đứng trên mặt đất trong E0_02 theo xác nhận của người đo. **Trạng thái Armed/Disarmed chưa được ghi nhận**; không suy trường này từ ảnh chụp sau run. Các trường nguồn, build, stream và số client còn lại đủ để chạy lại baseline với điều kiện tương đương.
