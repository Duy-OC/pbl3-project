# 20261114_175817_r1 — kịch bản và kết quả

Đường dẫn: `docs/ketqua/projectltm_import/K7/20261114_175817_r1`.

## Kịch bản đo

Không thả; xác suất 2% lật một bit payload chiều UAV → gateway; 2 lượt × 60 giây, warm-up 5 giây; đối chiếu CRC proxy/gateway.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

Nguồn: projectltm.rar. Không nhập proxy từ archive; số đo phụ thuộc proxy gốc chỉ là bằng chứng tham khảo. Báo cáo gốc có liên kết nhóm D/F không có trong archive, không dùng các liên kết đó làm bằng chứng đạt.

## Cấu hình của lượt

[K7_20261114_175817_r1_config.txt](K7_20261114_175817_r1_config.txt)

```text
scenario=K7
run_id=20261114_175817_r1
sysid=1
px4_listen_port=14550
gateway_source_port=14610
gateway_address=127.0.0.1:14551
urb=0
dur_s=2
slot_ms=100
markov_unit=time
burst_packets=4
lossgood=0
lossbad=1
seed=1001
flip_probability=0.02
delay_bad_min_ms=0
delay_bad_max_ms=0
reverse_urb=0
reverse_dur_s=2
reverse_slot_ms=100
reverse_markov_unit=time
reverse_burst_packets=4
reverse_lossgood=0
reverse_lossbad=1
reverse_seed=2654435920
warmup_s=5
measure_s=60
```

## Số liệu CSV

Số dòng không gồm header. Giá trị dưới đây chỉ thuộc **bản ghi cuối**, không phải tổng mọi UAV. Với summary.csv đọc metric/mean/std và đơn vị gốc; file chỉ có header không có mẫu đo.

| File | Dòng dữ liệu | Giá trị cuối tiêu biểu |
|---|---:|---|
| [K7_20261114_175817_r1_faults.csv](K7_20261114_175817_r1_faults.csv) | 71 | sysid=1; compid=1; t_s=64.8743 |
| [K7_20261114_175817_r1_gateway_ingress.csv](K7_20261114_175817_r1_gateway_ingress.csv) | 12 | sysid=1; compid=1; t_s=60.000; frames=3081; frame_bytes=124903; crc_errors=64 |
| [K7_20261114_175817_r1_gateway_ingress_bursts.csv](K7_20261114_175817_r1_gateway_ingress_bursts.csv) | 62 | sysid=1; compid=1; t_s=58.983 |
| [K7_20261114_175817_r1_gateway_ingress_loss_seconds.csv](K7_20261114_175817_r1_gateway_ingress_loss_seconds.csv) | 60 | sysid=1; compid=1 |
| [K7_20261114_175817_r1_post_trace.csv](K7_20261114_175817_r1_post_trace.csv) | 3501 | sysid=None; compid=None |
| [K7_20261114_175817_r1_proxy_post.csv](K7_20261114_175817_r1_proxy_post.csv) | 12 | sysid=1; compid=1; t_s=60.000; frames=3103; frame_bytes=125805; crc_errors=0 |
| [K7_20261114_175817_r1_proxy_post_trace.csv](K7_20261114_175817_r1_proxy_post_trace.csv) | 3103 | sysid=1; compid=1; t_s=59.9798 |
| [K7_20261114_175817_r1_proxy_pre.csv](K7_20261114_175817_r1_proxy_pre.csv) | 12 | sysid=1; compid=1; t_s=60.000; frames=3168; frame_bytes=128523; crc_errors=0 |
| [K7_20261114_175817_r1_proxy_pre_trace.csv](K7_20261114_175817_r1_proxy_pre_trace.csv) | 3168 | sysid=1; compid=1; t_s=59.9798 |
| [K7_20261114_175817_r1_reverse_states.csv](K7_20261114_175817_r1_reverse_states.csv) | 2 | sysid=1 |
| [K7_20261114_175817_r1_reverse_trace.csv](K7_20261114_175817_r1_reverse_trace.csv) | 63 | sysid=1 |
| [K7_20261114_175817_r1_states.csv](K7_20261114_175817_r1_states.csv) | 2 | sysid=1 |
| [K7_20261114_175817_r1_summary.csv](K7_20261114_175817_r1_summary.csv) | 1 | sysid=1; crc_errors=0 |
| [result.csv](result.csv) | 1 | seconds=60; frames=3081; crc_errors=64 |

## Bằng chứng bổ trợ

- [gateway.log](gateway.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [proxy.log](proxy.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [run.log](run.log): Log thao tác, counter, lỗi hoặc kết quả test.
