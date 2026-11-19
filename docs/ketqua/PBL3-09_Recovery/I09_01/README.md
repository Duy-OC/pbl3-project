# I09_01 — kịch bản và kết quả

Đường dẫn: `docs/ketqua/PBL3-09_Recovery/I09_01`.

## Kịch bản đo

Giữ UAV 1 hoạt động, ngắt UAV 2 rồi gửi yêu cầu tới cả hai. Khôi phục UAV 2 bằng endpoint mới; kiểm BLOCK khi offline và route mới sau phục hồi.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

## Nhận xét gốc

- [summary.md](summary.md)

Chi tiết lượt gốc: Nguồn: `gateway.log`, `probe_uav2_offline.log`, `probe_uav1_online.log` và sáu file thô trong thư mục run. Cấu hình `I09_ROUTE`, `I09_01`, `urb=0`.

## Cấu hình của lượt

[I09_ROUTE_I09_01_config.txt](I09_ROUTE_I09_01_config.txt)

```text
scenario=I09_ROUTE
run_id=I09_01
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
| [I09_ROUTE_I09_01_gateway_egress.csv](I09_ROUTE_I09_01_gateway_egress.csv) | 52 | sysid=2; compid=1; t_s=130.586; frames=1825; frame_bytes=73990 |
| [I09_ROUTE_I09_01_gateway_egress_msgid.csv](I09_ROUTE_I09_01_gateway_egress_msgid.csv) | 1628 | sysid=2; compid=1; t_s=130.586; frames=30; frame_bytes=1110 |
| [I09_ROUTE_I09_01_gateway_ingress.csv](I09_ROUTE_I09_01_gateway_ingress.csv) | 52 | sysid=2; compid=1; t_s=130.586; frames=1825; frame_bytes=73990 |
| [I09_ROUTE_I09_01_gateway_ingress_msgid.csv](I09_ROUTE_I09_01_gateway_ingress_msgid.csv) | 1628 | sysid=2; compid=1; t_s=130.586; frames=30; frame_bytes=1110 |
| [I09_ROUTE_I09_01_states.csv](I09_ROUTE_I09_01_states.csv) | 2 | sysid=2 |

## Bằng chứng bổ trợ

- [gateway.log](gateway.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [probe_uav1_online.log](probe_uav1_online.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [probe_uav2_offline.log](probe_uav2_offline.log): Log thao tác, counter, lỗi hoặc kết quả test.
