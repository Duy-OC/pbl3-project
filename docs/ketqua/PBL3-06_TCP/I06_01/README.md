# I06_01 — kịch bản và kết quả

Đường dẫn: `docs/ketqua/PBL3-06_TCP/I06_01`.

## Kịch bản đo

PX4 → gateway → client TCP; kiểm telemetry liên tục, frame/byte và lỗi CRC. Counter client và gateway có mốc bắt đầu riêng.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

## Nhận xét gốc

- [observation.md](observation.md)

Chi tiết lượt gốc: Nguồn: người đo cung cấp STATS/QOS Gateway và xác nhận QGC hiển thị độ cao, tốc độ cùng các thông số khác. Đây là ca kiểm tra kết nối, không phải run QoS 600 giây. Sau khi Gateway dừng, sáu file thô đã được chuyển vào cùng thư mục này.

## Cấu hình của lượt

[I06_TCP_I06_01_config.txt](I06_TCP_I06_01_config.txt)

```text
scenario=I06_TCP
run_id=I06_01
urb=0
dur_s=2
slot_ms=100
lossgood=0
lossbad=1
seed=1
warmup_s=10
measure_s=600
route_timeout_s=5
ingress_timeout_s=5
egress_timeout_s=3
```

## Số liệu CSV

Số dòng không gồm header. Giá trị dưới đây chỉ thuộc **bản ghi cuối**, không phải tổng mọi UAV. Với summary.csv đọc metric/mean/std và đơn vị gốc; file chỉ có header không có mẫu đo.

| File | Dòng dữ liệu | Giá trị cuối tiêu biểu |
|---|---:|---|
| [I06_TCP_I06_01_gateway_egress.csv](I06_TCP_I06_01_gateway_egress.csv) | 75 | sysid=1; compid=1; t_s=376.286; frames=22045; frame_bytes=893441 |
| [I06_TCP_I06_01_gateway_egress_msgid.csv](I06_TCP_I06_01_gateway_egress_msgid.csv) | 3064 | sysid=1; compid=1; t_s=376.286; frames=359; frame_bytes=13283 |
| [I06_TCP_I06_01_gateway_ingress.csv](I06_TCP_I06_01_gateway_ingress.csv) | 75 | sysid=1; compid=1; t_s=376.286; frames=22045; frame_bytes=893441 |
| [I06_TCP_I06_01_gateway_ingress_msgid.csv](I06_TCP_I06_01_gateway_ingress_msgid.csv) | 3064 | sysid=1; compid=1; t_s=376.286; frames=359; frame_bytes=13283 |
| [I06_TCP_I06_01_states.csv](I06_TCP_I06_01_states.csv) | 1 | sysid=1 |

## Bằng chứng bổ trợ

- [gateway_excerpt.log](gateway_excerpt.log): Log thao tác, counter, lỗi hoặc kết quả test.
