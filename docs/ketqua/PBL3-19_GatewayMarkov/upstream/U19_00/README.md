# U19_00 — kịch bản và kết quả

Đường dẫn: `docs/ketqua/PBL3-19_GatewayMarkov/upstream/U19_00`.

## Kịch bản đo

PX4 → proxy upstream → gateway, gateway urb=0. Đo OFF/ON theo cấu hình từng run; kiểm pre=kept+dropped và missing/extra trên trace toàn phiên, không trừ hai tổng QoS khác cửa sổ. Preflight Markov OFF; đối chiếu trace toàn phiên; không phải bài đo đủ 600 giây.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

## Nhận xét gốc

- [summary.md](summary.md)

Chi tiết lượt gốc: **Kết luận: PASS cho bước kiểm kết nối.** Đây là preflight, không phải lượt đo Markov 600 giây.

## Cấu hình của lượt

[U19_PRE_U19_00_config.txt](U19_PRE_U19_00_config.txt)

```text
scenario=U19_PRE
run_id=U19_00
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

[U19_PRE_U19_00_gateway_config.txt](U19_PRE_U19_00_gateway_config.txt)

```text
scenario=U19_PRE
run_id=U19_00
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
| [U19_PRE_U19_00_gateway_egress.csv](U19_PRE_U19_00_gateway_egress.csv) | 40 | sysid=1; compid=1; t_s=201.246; frames=9502; frame_bytes=383805 |
| [U19_PRE_U19_00_gateway_egress_msgid.csv](U19_PRE_U19_00_gateway_egress_msgid.csv) | 1236 | sysid=1; compid=1; t_s=201.246; frames=156; frame_bytes=5772 |
| [U19_PRE_U19_00_gateway_ingress.csv](U19_PRE_U19_00_gateway_ingress.csv) | 40 | sysid=1; compid=1; t_s=201.246; frames=9502; frame_bytes=383805 |
| [U19_PRE_U19_00_gateway_ingress_msgid.csv](U19_PRE_U19_00_gateway_ingress_msgid.csv) | 1236 | sysid=1; compid=1; t_s=201.246; frames=156; frame_bytes=5772 |
| [U19_PRE_U19_00_gateway_ingress_trace.csv](U19_PRE_U19_00_gateway_ingress_trace.csv) | 10139 | sysid=1; compid=1 |
| [U19_PRE_U19_00_gateway_states.csv](U19_PRE_U19_00_gateway_states.csv) | 1 | sysid=1 |
| [U19_PRE_U19_00_post_trace.csv](U19_PRE_U19_00_post_trace.csv) | 10139 | sysid=1; compid=1 |
| [U19_PRE_U19_00_proxy_post.csv](U19_PRE_U19_00_proxy_post.csv) | 41 | sysid=1; compid=1; t_s=206.439; frames=9567; frame_bytes=386422 |
| [U19_PRE_U19_00_proxy_post_msgid.csv](U19_PRE_U19_00_proxy_post_msgid.csv) | 1271 | sysid=1; compid=1; t_s=206.439; frames=157; frame_bytes=5809 |
| [U19_PRE_U19_00_proxy_pre.csv](U19_PRE_U19_00_proxy_pre.csv) | 41 | sysid=1; compid=1; t_s=206.439; frames=9567; frame_bytes=386422 |
| [U19_PRE_U19_00_proxy_pre_msgid.csv](U19_PRE_U19_00_proxy_pre_msgid.csv) | 1271 | sysid=1; compid=1; t_s=206.439; frames=157; frame_bytes=5809 |
| [U19_PRE_U19_00_states.csv](U19_PRE_U19_00_states.csv) | 1 | sysid=1 |

## Bằng chứng bổ trợ

- [gateway.log](gateway.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [gateway_build.log](gateway_build.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [proxy.log](proxy.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [proxy_build.log](proxy_build.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [trace_compare.log](trace_compare.log): Log thao tác, counter, lỗi hoặc kết quả test.
