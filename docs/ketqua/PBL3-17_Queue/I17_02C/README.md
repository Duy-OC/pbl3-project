# I17_02C — kịch bản và kết quả

Đường dẫn: `docs/ketqua/PBL3-17_Queue/I17_02C`.

## Kịch bản đo

Giữ lệnh bằng test_hold_udp_ms, rồi gây ngắt client/route, queue đầy hoặc lỗi sendto. Đối chiếu queued/dropped/cancelled với nguyên nhân trong log; timeout probe có thể là kết quả mong đợi. hold=300000 ms, probe giữ TCP 180 giây; dừng UAV để route timeout trước lúc client đóng.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

## Nhận xét gốc

- [summary.md](summary.md)

Chi tiết lượt gốc: Nguồn: `gateway_build.log`, `probe_build.log`, `probe.log`, `gateway.log` và sáu file Gateway thô trong thư mục này. PX4 UAV 1 SYSID 1 tại `172.26.162.42:14600`; Gateway Markov OFF, `test_hold_udp_ms=300000`. Đây là phép thử nhánh route timeout, không phải kết quả QoS 600 giây.

## Cấu hình của lượt

[I17_CANCEL_ROUTE_I17_02C_config.txt](I17_CANCEL_ROUTE_I17_02C_config.txt)

```text
scenario=I17_CANCEL_ROUTE
run_id=I17_02C
urb=0
dur_s=2
slot_ms=100
lossgood=0
lossbad=1
seed=1
test_hold_udp_ms=300000
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
| [I17_CANCEL_ROUTE_I17_02C_gateway_egress.csv](I17_CANCEL_ROUTE_I17_02C_gateway_egress.csv) | 38 | sysid=1; compid=1; t_s=193.326; frames=915; frame_bytes=36954 |
| [I17_CANCEL_ROUTE_I17_02C_gateway_egress_msgid.csv](I17_CANCEL_ROUTE_I17_02C_gateway_egress_msgid.csv) | 1174 | sysid=1; compid=1; t_s=193.326; frames=15; frame_bytes=555 |
| [I17_CANCEL_ROUTE_I17_02C_gateway_ingress.csv](I17_CANCEL_ROUTE_I17_02C_gateway_ingress.csv) | 38 | sysid=1; compid=1; t_s=193.326; frames=915; frame_bytes=36954 |
| [I17_CANCEL_ROUTE_I17_02C_gateway_ingress_msgid.csv](I17_CANCEL_ROUTE_I17_02C_gateway_ingress_msgid.csv) | 1174 | sysid=1; compid=1; t_s=193.326; frames=15; frame_bytes=555 |
| [I17_CANCEL_ROUTE_I17_02C_states.csv](I17_CANCEL_ROUTE_I17_02C_states.csv) | 1 | sysid=1 |

## Bằng chứng bổ trợ

- [gateway.log](gateway.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [gateway_build.log](gateway_build.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [probe.log](probe.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [probe_build.log](probe_build.log): Log thao tác, counter, lỗi hoặc kết quả test.
