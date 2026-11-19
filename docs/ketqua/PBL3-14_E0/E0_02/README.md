# E0_02 — kịch bản và kết quả

Đường dẫn: `docs/ketqua/PBL3-14_E0/E0_02`.

## Kịch bản đo

Baseline PX4, một client, urb=0; phiên bản đo cũ warm-up 10 giây rồi đo 600 giây. Đối soát frame ingress/egress, MSGID, CRC, jitter, offline và tài nguyên.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

## Nhận xét gốc

- [px4_environment.md](px4_environment.md)
- [reproducibility.md](reproducibility.md)
- [summary.md](summary.md)

Chi tiết lượt gốc: Nguồn: `gateway.log`, `client.log`, `metadata.log`, `resources.csv`, config/Markov/CSV Gateway và hai CSV client trong cùng thư mục. Gateway Markov OFF (`urb=0`), seed 1, warm-up 10 s, cửa sổ đo 600 s. Dữ liệu chỉ có SYSID 1; Gateway ghi 135 mẫu `UAV online=1 | UAV da hoc=1 | Clients=1`, không có mẫu hai UAV online.

## Cấu hình của lượt

[E0_E0_02_config.txt](E0_E0_02_config.txt)

```text
scenario=E0
run_id=E0_02
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
| [E0_1794470087479_client_rx.csv](E0_1794470087479_client_rx.csv) | 120 | sysid=1; compid=1; t_s=600.000; frames=34053; frame_bytes=1375463 |
| [E0_1794470087479_client_rx_msgid.csv](E0_1794470087479_client_rx_msgid.csv) | 3718 | sysid=1; compid=1; t_s=600.000; frames=558; frame_bytes=20646 |
| [E0_E0_02_gateway_egress.csv](E0_E0_02_gateway_egress.csv) | 120 | sysid=1; compid=1; t_s=600.000; frames=34074; frame_bytes=1376377 |
| [E0_E0_02_gateway_egress_msgid.csv](E0_E0_02_gateway_egress_msgid.csv) | 3716 | sysid=1; compid=1; t_s=600.000; frames=558; frame_bytes=20646 |
| [E0_E0_02_gateway_ingress.csv](E0_E0_02_gateway_ingress.csv) | 120 | sysid=1; compid=1; t_s=600.000; frames=34074; frame_bytes=1376377 |
| [E0_E0_02_gateway_ingress_msgid.csv](E0_E0_02_gateway_ingress_msgid.csv) | 3716 | sysid=1; compid=1; t_s=600.000; frames=558; frame_bytes=20646 |
| [E0_E0_02_markov.csv](E0_E0_02_markov.csv) | 1 | sysid=1 |
| [E0_E0_02_states.csv](E0_E0_02_states.csv) | 2 | sysid=1 |
| [resources.csv](resources.csv) | 22 | timestamp=2026-11-12T15:05:21.7915223+07:00; process=tcpclient; id=19776; cpu_seconds=1.6875; working_set_bytes=6782976 |

## Bằng chứng bổ trợ

- [client.log](client.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [gateway.log](gateway.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [metadata.log](metadata.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [source_manifest.log](source_manifest.log): Log thao tác, counter, lỗi hoặc kết quả test.
