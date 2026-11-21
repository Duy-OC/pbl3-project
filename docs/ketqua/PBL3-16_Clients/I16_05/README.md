# I16_05 — kịch bản và kết quả

Đường dẫn: `docs/ketqua/PBL3-16_Clients/I16_05`.

## Kịch bản đo

Thử riêng reconnect, client không đọc, giới hạn 10 client và tải 1/5/10 client. Đọc số client active, ACK, slow client, CRC và CPU/RAM theo từng lượt. Năm client, warm-up 10 giây và đo 600 giây; theo dõi frame/CRC từng client và tài nguyên.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

## Nhận xét gốc

- [summary.md](summary.md)

## Cấu hình của lượt

[I16_LOAD5_I16_05_config.txt](I16_LOAD5_I16_05_config.txt)

```text
scenario=I16_LOAD5
run_id=I16_05
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
| [I16_LOAD5_I16_05_gateway_egress.csv](I16_LOAD5_I16_05_gateway_egress.csv) | 120 | sysid=1; compid=1; t_s=600.000; frames=33943; frame_bytes=1375492 |
| [I16_LOAD5_I16_05_gateway_egress_msgid.csv](I16_LOAD5_I16_05_gateway_egress_msgid.csv) | 3718 | sysid=1; compid=1; t_s=600.000; frames=556; frame_bytes=20572 |
| [I16_LOAD5_I16_05_gateway_ingress.csv](I16_LOAD5_I16_05_gateway_ingress.csv) | 120 | sysid=1; compid=1; t_s=600.000; frames=33943; frame_bytes=1375492 |
| [I16_LOAD5_I16_05_gateway_ingress_msgid.csv](I16_LOAD5_I16_05_gateway_ingress_msgid.csv) | 3718 | sysid=1; compid=1; t_s=600.000; frames=556; frame_bytes=20572 |
| [I16_LOAD5_I16_05_states.csv](I16_LOAD5_I16_05_states.csv) | 2 | sysid=1 |
| [resources.csv](resources.csv) | 22 | timestamp=2026-11-13T20:59:25.3626038+07:00; process=multi_client_load; id=11560; cpu_seconds=4.15625; working_set_bytes=1609728 |

## Bằng chứng bổ trợ

- [client_load.log](client_load.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [gateway.log](gateway.log): Log thao tác, counter, lỗi hoặc kết quả test.
