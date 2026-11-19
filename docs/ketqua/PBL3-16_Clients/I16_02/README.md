# I16_02 — kịch bản và kết quả

Đường dẫn: `docs/ketqua/PBL3-16_Clients/I16_02`.

## Kịch bản đo

Thử riêng reconnect, client không đọc, giới hạn 10 client và tải 1/5/10 client. Đọc số client active, ACK, slow client, CRC và CPU/RAM theo từng lượt. Một client đọc bình thường; client thứ hai receive buffer 1024 byte, không recv trong 600 giây; kiểm client chậm bị ngắt và client thường tiếp tục.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

## Nhận xét gốc

- [summary.md](summary.md)

Chi tiết lượt gốc: Nguồn: `gateway.log`, `normal_client.log`, `slow_client.log`, `resources.csv`, config/Markov/CSV Gateway và hai CSV client thường trong thư mục này. Một UAV SYSID 1, Gateway Markov OFF, một client thường đọc dữ liệu và một client TCP có receive buffer 1024 byte không gọi `recv` trong 600 s.

## Cấu hình của lượt

[I16_SLOW_I16_02_config.txt](I16_SLOW_I16_02_config.txt)

```text
scenario=I16_SLOW
run_id=I16_02
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
| [E0_1794575875249_client_rx.csv](E0_1794575875249_client_rx.csv) | 120 | sysid=1; compid=1; t_s=600.000; frames=33906; frame_bytes=1373929 |
| [E0_1794575875249_client_rx_msgid.csv](E0_1794575875249_client_rx_msgid.csv) | 3718 | sysid=1; compid=1; t_s=600.000; frames=556; frame_bytes=20572 |
| [I16_SLOW_I16_02_gateway_egress.csv](I16_SLOW_I16_02_gateway_egress.csv) | 120 | sysid=1; compid=1; t_s=600.000; frames=33913; frame_bytes=1374226 |
| [I16_SLOW_I16_02_gateway_egress_msgid.csv](I16_SLOW_I16_02_gateway_egress_msgid.csv) | 3718 | sysid=1; compid=1; t_s=600.000; frames=556; frame_bytes=20572 |
| [I16_SLOW_I16_02_gateway_ingress.csv](I16_SLOW_I16_02_gateway_ingress.csv) | 120 | sysid=1; compid=1; t_s=600.000; frames=33913; frame_bytes=1374226 |
| [I16_SLOW_I16_02_gateway_ingress_msgid.csv](I16_SLOW_I16_02_gateway_ingress_msgid.csv) | 3718 | sysid=1; compid=1; t_s=600.000; frames=556; frame_bytes=20572 |
| [I16_SLOW_I16_02_markov.csv](I16_SLOW_I16_02_markov.csv) | 1 | sysid=1 |
| [I16_SLOW_I16_02_states.csv](I16_SLOW_I16_02_states.csv) | 2 | sysid=1 |
| [resources.csv](resources.csv) | 22 | timestamp=2026-11-13T20:28:26.8799777+07:00; process=tcpclient; id=10804; cpu_seconds=1.578125; working_set_bytes=6832128 |

## Bằng chứng bổ trợ

- [gateway.log](gateway.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [normal_client.log](normal_client.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [slow_client.log](slow_client.log): Log thao tác, counter, lỗi hoặc kết quả test.
