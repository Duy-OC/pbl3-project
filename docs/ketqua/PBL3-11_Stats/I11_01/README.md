# I11_01 — kịch bản và kết quả

Đường dẫn: `docs/ketqua/PBL3-11_Stats/I11_01`.

## Kịch bản đo

Gửi một HEARTBEAT CRC hỏng qua UDP và một qua TCP; STATS kỳ vọng CRC tổng tăng 2, UDP=1, TCP=1; không tính frame sai là telemetry hợp lệ.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

## Nhận xét gốc

- [summary.md](summary.md)

Chi tiết lượt gốc: Nguồn: `probe.log`, `gateway.log` và sáu file cấu hình/CSV của run `I11_01`.

## Cấu hình của lượt

[I11_STATS_I11_01_config.txt](I11_STATS_I11_01_config.txt)

```text
scenario=I11_STATS
run_id=I11_01
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
| [I11_STATS_I11_01_gateway_egress.csv](I11_STATS_I11_01_gateway_egress.csv) | 0 | Không có mẫu |
| [I11_STATS_I11_01_gateway_egress_msgid.csv](I11_STATS_I11_01_gateway_egress_msgid.csv) | 0 | Không có mẫu |
| [I11_STATS_I11_01_gateway_ingress.csv](I11_STATS_I11_01_gateway_ingress.csv) | 0 | Không có mẫu |
| [I11_STATS_I11_01_gateway_ingress_msgid.csv](I11_STATS_I11_01_gateway_ingress_msgid.csv) | 0 | Không có mẫu |
| [I11_STATS_I11_01_states.csv](I11_STATS_I11_01_states.csv) | 0 | Không có mẫu |

## Bằng chứng bổ trợ

- [gateway.log](gateway.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [probe.log](probe.log): Log thao tác, counter, lỗi hoặc kết quả test.
