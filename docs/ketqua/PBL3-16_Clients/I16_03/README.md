# I16_03 — kịch bản và kết quả

Đường dẫn: `docs/ketqua/PBL3-16_Clients/I16_03`.

## Kịch bản đo

Thử riêng reconnect, client không đọc, giới hạn 10 client và tải 1/5/10 client. Đọc số client active, ACK, slow client, CRC và CPU/RAM theo từng lượt. Giữ 10 client đọc rồi kết nối client thứ 11; kiểm từ chối client mới và 10 client cũ tiếp tục nhận.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

## Nhận xét gốc

- [summary.md](summary.md)

## Cấu hình của lượt

[I16_CAP_I16_03_config.txt](I16_CAP_I16_03_config.txt)

```text
scenario=I16_CAP
run_id=I16_03
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
| [I16_CAP_I16_03_gateway_egress.csv](I16_CAP_I16_03_gateway_egress.csv) | 7 | sysid=1; compid=1; t_s=35.146; frames=1981; frame_bytes=80274 |
| [I16_CAP_I16_03_gateway_egress_msgid.csv](I16_CAP_I16_03_gateway_egress_msgid.csv) | 217 | sysid=1; compid=1; t_s=35.146; frames=32; frame_bytes=1184 |
| [I16_CAP_I16_03_gateway_ingress.csv](I16_CAP_I16_03_gateway_ingress.csv) | 7 | sysid=1; compid=1; t_s=35.146; frames=1981; frame_bytes=80274 |
| [I16_CAP_I16_03_gateway_ingress_msgid.csv](I16_CAP_I16_03_gateway_ingress_msgid.csv) | 217 | sysid=1; compid=1; t_s=35.146; frames=32; frame_bytes=1184 |
| [I16_CAP_I16_03_states.csv](I16_CAP_I16_03_states.csv) | 1 | sysid=1 |

## Bằng chứng bổ trợ

- [cap11.log](cap11.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [gateway.log](gateway.log): Log thao tác, counter, lỗi hoặc kết quả test.
