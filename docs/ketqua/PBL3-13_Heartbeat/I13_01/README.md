# I13_01 — kịch bản và kết quả

Đường dẫn: `docs/ketqua/PBL3-13_Heartbeat/I13_01`.

## Kịch bản đo

Ngắt/khôi phục nguồn telemetry UAV 1, giữ UAV còn lại hoạt động; đọc online/offline, resync và tuổi dữ liệu. Timeout ingress/egress lịch sử khác nhau.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

## Nhận xét gốc

- [summary.md](summary.md)

Chi tiết lượt gốc: Nguồn: `gateway.log`, `client.log`, sáu file Gateway và hai CSV client trong cùng thư mục; người đo còn gửi `mavlink status` sau khôi phục xác nhận instance #4 local 14600 → Windows 172.26.160.1:14550, TX 2269.9 B/s, RX loss 0%.

## Cấu hình của lượt

[I13_HB_I13_01_config.txt](I13_HB_I13_01_config.txt)

```text
scenario=I13_HB
run_id=I13_01
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
| [E0_1794469577016_client_rx.csv](E0_1794469577016_client_rx.csv) | 38 | sysid=2; compid=1; t_s=95.416; frames=5426; frame_bytes=219272 |
| [E0_1794469577016_client_rx_msgid.csv](E0_1794469577016_client_rx_msgid.csv) | 1192 | sysid=2; compid=1; t_s=95.416; frames=89; frame_bytes=3293 |
| [I13_HB_I13_01_gateway_egress.csv](I13_HB_I13_01_gateway_egress.csv) | 42 | sysid=2; compid=1; t_s=105.261; frames=5963; frame_bytes=240883 |
| [I13_HB_I13_01_gateway_egress_msgid.csv](I13_HB_I13_01_gateway_egress_msgid.csv) | 1314 | sysid=2; compid=1; t_s=105.261; frames=98; frame_bytes=3626 |
| [I13_HB_I13_01_gateway_ingress.csv](I13_HB_I13_01_gateway_ingress.csv) | 42 | sysid=2; compid=1; t_s=105.261; frames=5963; frame_bytes=240883 |
| [I13_HB_I13_01_gateway_ingress_msgid.csv](I13_HB_I13_01_gateway_ingress_msgid.csv) | 1314 | sysid=2; compid=1; t_s=105.261; frames=98; frame_bytes=3626 |
| [I13_HB_I13_01_states.csv](I13_HB_I13_01_states.csv) | 2 | sysid=1 |

## Bằng chứng bổ trợ

- [client.log](client.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [gateway.log](gateway.log): Log thao tác, counter, lỗi hoặc kết quả test.
