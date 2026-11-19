# U19_02 — kịch bản và kết quả

Đường dẫn: `docs/ketqua/PBL3-19_GatewayMarkov/upstream/U19_02`.

## Kịch bản đo

PX4 → proxy upstream → gateway, gateway urb=0. Đo OFF/ON theo cấu hình từng run; kiểm pre=kept+dropped và missing/extra trên trace toàn phiên, không trừ hai tổng QoS khác cửa sổ. ON, warm-up 10 giây và đo 600 giây; proxy urb=0.5, dur_s=2, seed=1; gateway urb=0.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

## Nhận xét gốc

- [summary.md](summary.md)

Chi tiết lượt gốc: **PASS.** Proxy và Gateway đều ghi đủ 600 giây. Cấu hình proxy: `urb=0.5`, `dur_s=2`, `lossgood=0`, `lossbad=1`, seed 1; Gateway giữ `urb=0`.

## Cấu hình của lượt

[U19_LOSS2_U19_02_config.txt](U19_LOSS2_U19_02_config.txt)

```text
scenario=U19_LOSS2
run_id=U19_02
sysid=1
px4_listen_port=14551
gateway_source_port=14610
gateway_address=127.0.0.1:14550
urb=0.5
dur_s=2
slot_ms=100
lossgood=0
lossbad=1
seed=1
warmup_s=10
measure_s=600
```

[U19_LOSS2_U19_02_gateway_config.txt](U19_LOSS2_U19_02_gateway_config.txt)

```text
scenario=U19_LOSS2
run_id=U19_02
urb=0
dur_s=2
slot_ms=100
lossgood=0
lossbad=1
seed=1
test_hold_udp_ms=0
test_sendto_invalid_addrlen_once=0
trace_ingress=1
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
| [U19_LOSS2_U19_02_gateway_egress.csv](U19_LOSS2_U19_02_gateway_egress.csv) | 119 | sysid=1; compid=1; t_s=600.000; frames=15943; frame_bytes=642733 |
| [U19_LOSS2_U19_02_gateway_egress_msgid.csv](U19_LOSS2_U19_02_gateway_egress_msgid.csv) | 3685 | sysid=1; compid=1; t_s=600.000; frames=276; frame_bytes=10212 |
| [U19_LOSS2_U19_02_gateway_ingress.csv](U19_LOSS2_U19_02_gateway_ingress.csv) | 119 | sysid=1; compid=1; t_s=600.000; frames=15943; frame_bytes=642733 |
| [U19_LOSS2_U19_02_gateway_ingress_msgid.csv](U19_LOSS2_U19_02_gateway_ingress_msgid.csv) | 3685 | sysid=1; compid=1; t_s=600.000; frames=276; frame_bytes=10212 |
| [U19_LOSS2_U19_02_gateway_ingress_trace.csv](U19_LOSS2_U19_02_gateway_ingress_trace.csv) | 19448 | sysid=1; compid=1 |
| [U19_LOSS2_U19_02_gateway_states.csv](U19_LOSS2_U19_02_gateway_states.csv) | 2 | sysid=1 |
| [U19_LOSS2_U19_02_markov.csv](U19_LOSS2_U19_02_markov.csv) | 1 | sysid=1 |
| [U19_LOSS2_U19_02_post_trace.csv](U19_LOSS2_U19_02_post_trace.csv) | 19448 | sysid=1; compid=1 |
| [U19_LOSS2_U19_02_proxy_post.csv](U19_LOSS2_U19_02_proxy_post.csv) | 120 | sysid=1; compid=1; t_s=600.000; frames=16546; frame_bytes=667838 |
| [U19_LOSS2_U19_02_proxy_post_msgid.csv](U19_LOSS2_U19_02_proxy_post_msgid.csv) | 3720 | sysid=1; compid=1; t_s=600.000; frames=277; frame_bytes=10249 |
| [U19_LOSS2_U19_02_proxy_pre.csv](U19_LOSS2_U19_02_proxy_pre.csv) | 120 | sysid=1; compid=1; t_s=600.000; frames=33458; frame_bytes=1351498 |
| [U19_LOSS2_U19_02_proxy_pre_msgid.csv](U19_LOSS2_U19_02_proxy_pre_msgid.csv) | 3720 | sysid=1; compid=1; t_s=600.000; frames=548; frame_bytes=20276 |
| [U19_LOSS2_U19_02_states.csv](U19_LOSS2_U19_02_states.csv) | 287 | sysid=1 |
| [U19_LOSS2_U19_02_summary.csv](U19_LOSS2_U19_02_summary.csv) | 1 | sysid=1; crc_errors=0 |

## Bằng chứng bổ trợ

- [gateway.log](gateway.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [proxy.log](proxy.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [trace_compare.log](trace_compare.log): Log thao tác, counter, lỗi hoặc kết quả test.
