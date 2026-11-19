# U19_01 — kịch bản và kết quả

Đường dẫn: `docs/ketqua/PBL3-19_GatewayMarkov/upstream/U19_01`.

## Kịch bản đo

PX4 → proxy upstream → gateway, gateway urb=0. Đo OFF/ON theo cấu hình từng run; kiểm pre=kept+dropped và missing/extra trên trace toàn phiên, không trừ hai tổng QoS khác cửa sổ. OFF, warm-up 10 giây và đo 600 giây; cả proxy và gateway urb=0.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

## Nhận xét gốc

- [summary.md](summary.md)

Chi tiết lượt gốc: **PASS.** Proxy và Gateway đều ghi đủ 600 giây sau 10 giây khởi động. Proxy nhận 33890 frame trong cửa sổ đo, giữ và gửi đủ 33890; Markov bỏ 0, lỗi gửi/CRC/signature 0. Trace toàn phiên: proxy gửi 38612 frame, Gateway nhận 38612, `missing=0`, `extra=0`, `trace_exit=0`. Gateway học UAV 1 qua `127.0.0.1:14610`.

## Cấu hình của lượt

[U19_OFF_U19_01_config.txt](U19_OFF_U19_01_config.txt)

```text
scenario=U19_OFF
run_id=U19_01
sysid=1
px4_listen_port=14551
gateway_source_port=14610
gateway_address=127.0.0.1:14550
urb=0
dur_s=2
slot_ms=100
lossgood=0
lossbad=1
seed=1
warmup_s=10
measure_s=600
```

[U19_OFF_U19_01_gateway_config.txt](U19_OFF_U19_01_gateway_config.txt)

```text
scenario=U19_OFF
run_id=U19_01
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
| [U19_OFF_U19_01_gateway_egress.csv](U19_OFF_U19_01_gateway_egress.csv) | 120 | sysid=1; compid=1; t_s=600.000; frames=33884; frame_bytes=1368663 |
| [U19_OFF_U19_01_gateway_egress_msgid.csv](U19_OFF_U19_01_gateway_egress_msgid.csv) | 3716 | sysid=1; compid=1; t_s=600.000; frames=555; frame_bytes=20535 |
| [U19_OFF_U19_01_gateway_ingress.csv](U19_OFF_U19_01_gateway_ingress.csv) | 120 | sysid=1; compid=1; t_s=600.000; frames=33884; frame_bytes=1368663 |
| [U19_OFF_U19_01_gateway_ingress_msgid.csv](U19_OFF_U19_01_gateway_ingress_msgid.csv) | 3716 | sysid=1; compid=1; t_s=600.000; frames=555; frame_bytes=20535 |
| [U19_OFF_U19_01_gateway_ingress_trace.csv](U19_OFF_U19_01_gateway_ingress_trace.csv) | 38612 | sysid=1; compid=1 |
| [U19_OFF_U19_01_gateway_states.csv](U19_OFF_U19_01_gateway_states.csv) | 2 | sysid=1 |
| [U19_OFF_U19_01_markov.csv](U19_OFF_U19_01_markov.csv) | 1 | sysid=1 |
| [U19_OFF_U19_01_post_trace.csv](U19_OFF_U19_01_post_trace.csv) | 38612 | sysid=1; compid=1 |
| [U19_OFF_U19_01_proxy_post.csv](U19_OFF_U19_01_proxy_post.csv) | 120 | sysid=1; compid=1; t_s=600.000; frames=33890; frame_bytes=1368948 |
| [U19_OFF_U19_01_proxy_post_msgid.csv](U19_OFF_U19_01_proxy_post_msgid.csv) | 3720 | sysid=1; compid=1; t_s=600.000; frames=555; frame_bytes=20535 |
| [U19_OFF_U19_01_proxy_pre.csv](U19_OFF_U19_01_proxy_pre.csv) | 120 | sysid=1; compid=1; t_s=600.000; frames=33890; frame_bytes=1368948 |
| [U19_OFF_U19_01_proxy_pre_msgid.csv](U19_OFF_U19_01_proxy_pre_msgid.csv) | 3720 | sysid=1; compid=1; t_s=600.000; frames=555; frame_bytes=20535 |
| [U19_OFF_U19_01_states.csv](U19_OFF_U19_01_states.csv) | 2 | sysid=1 |
| [U19_OFF_U19_01_summary.csv](U19_OFF_U19_01_summary.csv) | 1 | sysid=1; crc_errors=0 |

## Bằng chứng bổ trợ

- [gateway.log](gateway.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [proxy.log](proxy.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [trace_compare.log](trace_compare.log): Log thao tác, counter, lỗi hoặc kết quả test.
