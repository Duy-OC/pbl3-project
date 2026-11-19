# I16_01 — kịch bản và kết quả

Đường dẫn: `docs/ketqua/PBL3-16_Clients/I16_01`.

## Kịch bản đo

Thử riêng reconnect, client không đọc, giới hạn 10 client và tải 1/5/10 client. Đọc số client active, ACK, slow client, CRC và CPU/RAM theo từng lượt. 20 vòng mở TCP → gửi command 512 → nhận ACK → đóng; kiểm 20 ACK và không rò kết nối.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

## Nhận xét gốc

- [summary.md](summary.md)

Chi tiết lượt gốc: Nguồn: `reconnect.log`, `gateway.log`, `resources.csv` và sáu file Gateway trong cùng thư mục. Một UAV SYSID 1 tại `172.26.162.42:14600`, Markov OFF (`urb=0`). Mỗi `route_probe` tạo một TCP connection mới, gửi `command=512` yêu cầu message, nhận ACK rồi đóng.

## Cấu hình của lượt

[I16_RECONNECT_I16_01_config.txt](I16_RECONNECT_I16_01_config.txt)

```text
scenario=I16_RECONNECT
run_id=I16_01
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
| [I16_RECONNECT_I16_01_gateway_egress.csv](I16_RECONNECT_I16_01_gateway_egress.csv) | 8 | sysid=1; compid=1; t_s=40.196; frames=2315; frame_bytes=93918 |
| [I16_RECONNECT_I16_01_gateway_egress_msgid.csv](I16_RECONNECT_I16_01_gateway_egress_msgid.csv) | 258 | sysid=1; compid=1; t_s=40.196; frames=37; frame_bytes=1369 |
| [I16_RECONNECT_I16_01_gateway_ingress.csv](I16_RECONNECT_I16_01_gateway_ingress.csv) | 8 | sysid=1; compid=1; t_s=40.196; frames=2315; frame_bytes=93918 |
| [I16_RECONNECT_I16_01_gateway_ingress_msgid.csv](I16_RECONNECT_I16_01_gateway_ingress_msgid.csv) | 258 | sysid=1; compid=1; t_s=40.196; frames=37; frame_bytes=1369 |
| [I16_RECONNECT_I16_01_states.csv](I16_RECONNECT_I16_01_states.csv) | 1 | sysid=1 |
| [resources.csv](resources.csv) | 5 | phase=cycle_20; time=2026-11-13T20:14:12.6040182+07:00; cpu_seconds=0.125; working_set_bytes=7069696; handles=94 |

## Bằng chứng bổ trợ

- [gateway.log](gateway.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [reconnect.log](reconnect.log): Log thao tác, counter, lỗi hoặc kết quả test.
