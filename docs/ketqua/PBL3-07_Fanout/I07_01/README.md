# I07_01 — kịch bản và kết quả

Đường dẫn: `docs/ketqua/PBL3-07_Fanout/I07_01`.

## Kịch bản đo

Mở ba client, cho nhận telemetry rồi ngắt một client. Kiểm các client còn lại tiếp tục đọc; đây là lượt ngắn, không thay bài tải 600 giây.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

## Nhận xét gốc

- [summary.md](summary.md)

Chi tiết lượt gốc: Nguồn: `gateway.log`, `client_A.log`, `client_B.log`, sáu file Gateway và bốn CSV client trong cùng thư mục. Đây là ca kết nối ngắn, không phải bài tải 600 giây PBL3-16/E4.

## Cấu hình của lượt

[I07_FANOUT_I07_01_config.txt](I07_FANOUT_I07_01_config.txt)

```text
scenario=I07_FANOUT
run_id=I07_01
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
| [E0_1794466025196_client_rx.csv](E0_1794466025196_client_rx.csv) | 24 | sysid=1; compid=1; t_s=120.494; frames=6978; frame_bytes=281558 |
| [E0_1794466025196_client_rx_msgid.csv](E0_1794466025196_client_rx_msgid.csv) | 888 | sysid=1; compid=1; t_s=120.494; frames=113; frame_bytes=4181 |
| [E0_1794466039813_client_rx.csv](E0_1794466039813_client_rx.csv) | 12 | sysid=1; compid=1; t_s=60.322; frames=3549; frame_bytes=142662 |
| [E0_1794466039813_client_rx_msgid.csv](E0_1794466039813_client_rx_msgid.csv) | 433 | sysid=1; compid=1; t_s=60.322; frames=57; frame_bytes=2109 |
| [I07_FANOUT_I07_01_gateway_egress.csv](I07_FANOUT_I07_01_gateway_egress.csv) | 29 | sysid=1; compid=1; t_s=145.612; frames=8409; frame_bytes=339560 |
| [I07_FANOUT_I07_01_gateway_egress_msgid.csv](I07_FANOUT_I07_01_gateway_egress_msgid.csv) | 1041 | sysid=1; compid=1; t_s=145.612; frames=136; frame_bytes=5032 |
| [I07_FANOUT_I07_01_gateway_ingress.csv](I07_FANOUT_I07_01_gateway_ingress.csv) | 29 | sysid=1; compid=1; t_s=145.612; frames=8409; frame_bytes=339560 |
| [I07_FANOUT_I07_01_gateway_ingress_msgid.csv](I07_FANOUT_I07_01_gateway_ingress_msgid.csv) | 1041 | sysid=1; compid=1; t_s=145.612; frames=136; frame_bytes=5032 |
| [I07_FANOUT_I07_01_states.csv](I07_FANOUT_I07_01_states.csv) | 1 | sysid=1 |

## Bằng chứng bổ trợ

- [client_A.log](client_A.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [client_B.log](client_B.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [gateway.log](gateway.log): Log thao tác, counter, lỗi hoặc kết quả test.
