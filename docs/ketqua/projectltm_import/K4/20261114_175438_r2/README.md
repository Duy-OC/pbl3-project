# 20261114_175438_r2 — kịch bản và kết quả

Đường dẫn: `docs/ketqua/projectltm_import/K4/20261114_175438_r2`.

## Kịch bản đo

Loss 10%, cụm 4 frame; 2 lượt × 60 giây + warm-up 5 giây; p=0.027778, r=0.25.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

Nguồn: projectltm.rar. Không nhập proxy từ archive; số đo phụ thuộc proxy gốc chỉ là bằng chứng tham khảo. Báo cáo gốc có liên kết nhóm D/F không có trong archive, không dùng các liên kết đó làm bằng chứng đạt.

## Cấu hình của lượt

[K4_20261114_175438_r2_config.txt](K4_20261114_175438_r2_config.txt)

```text
scenario=K4
run_id=20261114_175438_r2
sysid=1
px4_listen_port=14550
gateway_source_port=14610
gateway_address=127.0.0.1:14551
urb=0.1
dur_s=2
slot_ms=100
markov_unit=frame
burst_packets=4
lossgood=0
lossbad=1
seed=1002
flip_probability=0
delay_bad_min_ms=0
delay_bad_max_ms=0
reverse_urb=0.1
reverse_dur_s=2
reverse_slot_ms=100
reverse_markov_unit=frame
reverse_burst_packets=4
reverse_lossgood=0
reverse_lossbad=1
reverse_seed=2654435923
warmup_s=5
measure_s=60
```

## Số liệu CSV

Số dòng không gồm header. Giá trị dưới đây chỉ thuộc **bản ghi cuối**, không phải tổng mọi UAV. Với summary.csv đọc metric/mean/std và đơn vị gốc; file chỉ có header không có mẫu đo.

| File | Dòng dữ liệu | Giá trị cuối tiêu biểu |
|---|---:|---|
| [common.csv](common.csv) | 1 | common_s=58.93; proxy_pre=3123; proxy_drop=321; proxy_loss_pct=10.279; gateway_rx=2802 |
| [K4_20261114_175438_r2_decision_trace.csv](K4_20261114_175438_r2_decision_trace.csv) | 3484 | sysid=1; compid=1 |
| [K4_20261114_175438_r2_gateway_ingress.csv](K4_20261114_175438_r2_gateway_ingress.csv) | 12 | sysid=1; compid=1; t_s=60.000; frames=2846; frame_bytes=115404; crc_errors=0 |
| [K4_20261114_175438_r2_gateway_ingress_bursts.csv](K4_20261114_175438_r2_gateway_ingress_bursts.csv) | 71 | sysid=1; compid=1; t_s=59.431 |
| [K4_20261114_175438_r2_gateway_ingress_loss_seconds.csv](K4_20261114_175438_r2_gateway_ingress_loss_seconds.csv) | 60 | sysid=1; compid=1 |
| [K4_20261114_175438_r2_gateway_ingress_trace.csv](K4_20261114_175438_r2_gateway_ingress_trace.csv) | 2846 | sysid=1; compid=1; t_s=59.7871 |
| [K4_20261114_175438_r2_proxy_post.csv](K4_20261114_175438_r2_proxy_post.csv) | 12 | sysid=1; compid=1; t_s=60.000; frames=2830; frame_bytes=114796; crc_errors=0 |
| [K4_20261114_175438_r2_proxy_pre.csv](K4_20261114_175438_r2_proxy_pre.csv) | 12 | sysid=1; compid=1; t_s=60.000; frames=3174; frame_bytes=128743; crc_errors=0 |
| [K4_20261114_175438_r2_proxy_pre_trace.csv](K4_20261114_175438_r2_proxy_pre_trace.csv) | 3174 | sysid=1; compid=1; t_s=59.9648 |
| [K4_20261114_175438_r2_reverse_states.csv](K4_20261114_175438_r2_reverse_states.csv) | 6 | sysid=1 |
| [K4_20261114_175438_r2_reverse_trace.csv](K4_20261114_175438_r2_reverse_trace.csv) | 62 | sysid=1 |
| [K4_20261114_175438_r2_states.csv](K4_20261114_175438_r2_states.csv) | 160 | sysid=1 |
| [K4_20261114_175438_r2_summary.csv](K4_20261114_175438_r2_summary.csv) | 1 | sysid=1; crc_errors=0 |
| [result.csv](result.csv) | 1 | seconds=60; frames=2846; crc_errors=0 |

## Bằng chứng bổ trợ

- [gateway.log](gateway.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [proxy.log](proxy.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [run.log](run.log): Log thao tác, counter, lỗi hoặc kết quả test.
