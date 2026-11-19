# U19_00 — kiểm đường UAV → proxy → Gateway

**Kết luận: PASS cho bước kiểm kết nối.** Đây là preflight, không phải lượt đo Markov 600 giây.

| Điểm kiểm | Bằng chứng |
|---|---|
| Build | `gateway_build_exit=0`, `proxy_build_exit=0`. |
| Cấu hình | Proxy SYSID 1 nghe UDP 14551, gửi từ 127.0.0.1:14610 tới Gateway UDP 14550; cả proxy và Gateway `urb=0`, `trace_ingress=1`. |
| Nguồn UAV | Proxy học `172.26.162.42:14600`; Gateway báo `UAV 1 ONLINE tai 127.0.0.1:14610`. |
| Markov OFF | Bộ đếm cuối của proxy trong cửa sổ đang chạy: `pre=9567`, `kept=9567`, `post_sent=9567`, `dropped_markov=0`, `send_errors=0`, `crc_errors=0`. |
| Giao nhận cả phiên | Trace proxy gửi **10139 frame**, Gateway nhận **10139 frame**; `missing=0`, `extra=0`, `pass=1`, exit 0. Gateway báo `UDP RX datagrams=10139`, `CRC errors=0`, `Signature errors=0`. |

Hai file QoS của proxy có 9567 frame ở mốc cuối; Gateway ingress có 9502 frame vì cửa sổ đo bắt đầu khác thời điểm. Đối chiếu trace toàn phiên mới là bằng chứng giao nhận ở bước này. Sau khi dừng PX4, cả proxy và Gateway ghi trạng thái OFFLINE theo timeout; đó là kết quả của thao tác dừng nguồn cuối preflight, không phải mất sóng Markov.

Các file Gateway thô đã được chuyển từ `docs/ketqua/issue19/` vào thư mục này sau khi tiến trình dừng. Hai file trùng tên với proxy được đặt tên `_gateway_config.txt` và `_gateway_states.csv`. Chưa có file summary 600 giây vì lượt này dừng trước mốc 600 giây.

Chưa lưu bản `mavlink status` của PX4 trong thư mục run; địa chỉ `172.26.162.42:14600` được xác nhận từ `proxy.log`. Bổ sung status nếu còn giữ, nhưng đối chiếu trace đủ để xác nhận đường nhận của preflight.

**Bước tiếp:** U19_01, Markov OFF, một UAV, 600 giây để lấy baseline trên đúng tuyến này.
