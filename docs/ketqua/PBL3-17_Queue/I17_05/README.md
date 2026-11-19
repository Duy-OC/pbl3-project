# I17_05 — kịch bản và kết quả

Đường dẫn: `docs/ketqua/PBL3-17_Queue/I17_05`.

## Kịch bản đo

Giữ lệnh bằng test_hold_udp_ms, rồi gây ngắt client/route, queue đầy hoặc lỗi sendto. Đối chiếu queued/dropped/cancelled với nguyên nhân trong log; timeout probe có thể là kết quả mong đợi. Giữ một lệnh trong queue; ngắt UAV rồi phục hồi cùng SYSID ở cổng mới. Kiểm lệnh cũ đã hủy, không gửi tới endpoint mới.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

## Nhận xét gốc

- [summary.md](summary.md)

Chi tiết lượt gốc: Nguồn: `gateway.log`, `old_probe.log` và sáu file Gateway thô trong thư mục này. Gateway Markov OFF, `test_hold_udp_ms=300000`. Probe gửi một MAV_CMD_REQUEST_MESSAGE=512 đến SYSID 1 và giữ TCP 180 giây.

## Cấu hình của lượt

[I17_ENDPOINT_CHANGE_I17_05_config.txt](I17_ENDPOINT_CHANGE_I17_05_config.txt)

```text
scenario=I17_ENDPOINT_CHANGE
run_id=I17_05
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
| [I17_ENDPOINT_CHANGE_I17_05_gateway_egress.csv](I17_ENDPOINT_CHANGE_I17_05_gateway_egress.csv) | 62 | sysid=1; compid=1; t_s=186.231; frames=6985; frame_bytes=282004 |
| [I17_ENDPOINT_CHANGE_I17_05_gateway_egress_msgid.csv](I17_ENDPOINT_CHANGE_I17_05_gateway_egress_msgid.csv) | 2017 | sysid=1; compid=1; t_s=186.231; frames=115; frame_bytes=4255 |
| [I17_ENDPOINT_CHANGE_I17_05_gateway_ingress.csv](I17_ENDPOINT_CHANGE_I17_05_gateway_ingress.csv) | 62 | sysid=1; compid=1; t_s=186.231; frames=6985; frame_bytes=282004 |
| [I17_ENDPOINT_CHANGE_I17_05_gateway_ingress_msgid.csv](I17_ENDPOINT_CHANGE_I17_05_gateway_ingress_msgid.csv) | 2017 | sysid=1; compid=1; t_s=186.231; frames=115; frame_bytes=4255 |
| [I17_ENDPOINT_CHANGE_I17_05_states.csv](I17_ENDPOINT_CHANGE_I17_05_states.csv) | 1 | sysid=1 |

## Bằng chứng bổ trợ

- [gateway.log](gateway.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [old_probe.log](old_probe.log): Log thao tác, counter, lỗi hoặc kết quả test.
