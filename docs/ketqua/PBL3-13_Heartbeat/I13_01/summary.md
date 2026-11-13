# PBL3-13 — ngắt và khôi phục HEARTBEAT UAV 1

Nguồn: `gateway.log`, `client.log`, sáu file Gateway và hai CSV client trong cùng thư mục; người đo còn gửi `mavlink status` sau khôi phục xác nhận instance #4 local 14600 → Windows 172.26.160.1:14550, TX 2269.9 B/s, RX loss 0%.

Gateway egress ghi `UAV 1 OFFLINE` tại `t=49.088 s`, ingress tại `t=51.088 s`; route Gateway ghi OFFLINE và STATS chuyển từ hai UAV online còn một. Cả ingress/egress ghi ONLINE lại tại `t=72.0244 s`, route Gateway học lại `172.26.162.42:14600`, STATS trở lại hai UAV online. Khoảng cách đúng 2 s giữa egress và ingress phản ánh ngưỡng timeout cấu hình 3 s và 5 s.

Client QoS ghi `UAV 1 OFFLINE tai t=37.2635 s`, `ONLINE tai t=60.2 s`. Đồng hồ `t_s` của client bắt đầu độc lập với Gateway nên không lấy hai timestamp tuyệt đối để tính độ trễ truyền.

| Điểm QoS, SYSID 1 | Số mẫu | Mẫu đầu tiên offline | Mốc cuối | Frame cuối | Offline events | Offline time | Seq gap | Seq resync | Uncertain gaps |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Gateway ingress | 21 | 55.137 s | 105.261 s | 4487 | 1 | 20.936 s | 0 | 1 | 0 |
| Gateway egress | 21 | 50.129 s | 105.261 s | 4487 | 1 | 22.936 s | 0 | 1 | 0 |
| TCP client RX | 19 | 40.157 s | 95.416 s | 3954 | 1 | 22.936 s | 0 | 1 | 0 |

Mỗi điểm ghi một lần resync sequence khi khởi động lại MAVLink instance. Không diễn giải resync đó thành mất một gói; `seq_gap_estimate` vẫn bằng 0. Client bắt đầu đo muộn hơn Gateway nên tổng frame cuối khác nhau.

**Kết luận:** PASS phát hiện OFFLINE/ONLINE và ghi cột QoS/sequence tại ba điểm trong ca ngắt có kiểm soát. Lượt này không đo mất gói ngẫu nhiên hay độ trễ một chiều.
