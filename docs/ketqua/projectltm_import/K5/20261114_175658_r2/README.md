# 20261114_175658_r2 — kịch bản và kết quả

Đường dẫn: `docs/ketqua/projectltm_import/K5/20261114_175658_r2`.

## Kịch bản đo

Không thả; trạng thái B thêm delay 20–80 ms; 2 lượt × 60 giây, warm-up 5 giây; slot 100 ms, urb=0.1, dur_s=0.4.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

Nguồn: projectltm.rar. Không nhập proxy từ archive; số đo phụ thuộc proxy gốc chỉ là bằng chứng tham khảo. Báo cáo gốc có liên kết nhóm D/F không có trong archive, không dùng các liên kết đó làm bằng chứng đạt.

## Cấu hình của lượt

[K5_20261114_175658_r2_config.txt](K5_20261114_175658_r2_config.txt)

```text
scenario=K5
run_id=20261114_175658_r2
sysid=1
px4_listen_port=14550
gateway_source_port=14610
gateway_address=127.0.0.1:14551
urb=0.1
dur_s=0.4
slot_ms=100
markov_unit=time
burst_packets=4
lossgood=0
lossbad=0
seed=1002
flip_probability=0
delay_bad_min_ms=20
delay_bad_max_ms=80
reverse_urb=0.1
reverse_dur_s=0.4
reverse_slot_ms=100
reverse_markov_unit=time
reverse_burst_packets=4
reverse_lossgood=0
reverse_lossbad=0
reverse_seed=2654435923
warmup_s=5
measure_s=60
```

## Số liệu CSV

Số dòng không gồm header. Giá trị dưới đây chỉ thuộc **bản ghi cuối**, không phải tổng mọi UAV. Với summary.csv đọc metric/mean/std và đơn vị gốc; file chỉ có header không có mẫu đo.

| File | Dòng dữ liệu | Giá trị cuối tiêu biểu |
|---|---:|---|
| [K5_20261114_175658_r2_delay.csv](K5_20261114_175658_r2_delay.csv) | 336 | t_pre_ms=59936; t_sent_ms=60025; delay_set_ms=79; delay_observed_ms=88.529; seq=50 |
| [K5_20261114_175658_r2_gateway_ingress.csv](K5_20261114_175658_r2_gateway_ingress.csv) | 12 | sysid=1; compid=1; t_s=60.000; frames=3144; frame_bytes=127569; crc_errors=0 |
| [K5_20261114_175658_r2_gateway_ingress_bursts.csv](K5_20261114_175658_r2_gateway_ingress_bursts.csv) | 66 | sysid=1; compid=1; t_s=54.097 |
| [K5_20261114_175658_r2_gateway_ingress_loss_seconds.csv](K5_20261114_175658_r2_gateway_ingress_loss_seconds.csv) | 60 | sysid=1; compid=1 |
| [K5_20261114_175658_r2_proxy_post.csv](K5_20261114_175658_r2_proxy_post.csv) | 12 | sysid=1; compid=1; t_s=60.000; frames=3167; frame_bytes=128457; crc_errors=0 |
| [K5_20261114_175658_r2_proxy_pre.csv](K5_20261114_175658_r2_proxy_pre.csv) | 12 | sysid=1; compid=1; t_s=60.000; frames=3167; frame_bytes=128457; crc_errors=0 |
| [K5_20261114_175658_r2_proxy_pre_trace.csv](K5_20261114_175658_r2_proxy_pre_trace.csv) | 3167 | sysid=1; compid=1; t_s=59.9905 |
| [K5_20261114_175658_r2_reverse_states.csv](K5_20261114_175658_r2_reverse_states.csv) | 38 | sysid=1 |
| [K5_20261114_175658_r2_reverse_trace.csv](K5_20261114_175658_r2_reverse_trace.csv) | 63 | sysid=1 |
| [K5_20261114_175658_r2_states.csv](K5_20261114_175658_r2_states.csv) | 32 | sysid=1 |
| [K5_20261114_175658_r2_summary.csv](K5_20261114_175658_r2_summary.csv) | 1 | sysid=1; crc_errors=0 |
| [result.csv](result.csv) | 1 | seconds=60; frames=3144; crc_errors=0 |

## Bằng chứng bổ trợ

- [gateway.log](gateway.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [proxy.log](proxy.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [run.log](run.log): Log thao tác, counter, lỗi hoặc kết quả test.
