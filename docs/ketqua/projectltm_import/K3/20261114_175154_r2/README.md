# 20261114_175154_r2 — kịch bản và kết quả

Đường dẫn: `docs/ketqua/projectltm_import/K3/20261114_175154_r2`.

## Kịch bản đo

Loss 5%, cụm 1 frame; 2 lượt × 60 giây + warm-up 5 giây; Markov theo frame p=0.052632, r=1.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

Nguồn: projectltm.rar. Không nhập proxy từ archive; số đo phụ thuộc proxy gốc chỉ là bằng chứng tham khảo. Báo cáo gốc có liên kết nhóm D/F không có trong archive, không dùng các liên kết đó làm bằng chứng đạt.

## Cấu hình của lượt

[K3_20261114_175154_r2_config.txt](K3_20261114_175154_r2_config.txt)

```text
scenario=K3
run_id=20261114_175154_r2
sysid=1
px4_listen_port=14550
gateway_source_port=14610
gateway_address=127.0.0.1:14551
urb=0.05
dur_s=2
slot_ms=100
markov_unit=frame
burst_packets=1
lossgood=0
lossbad=1
seed=1002
flip_probability=0
delay_bad_min_ms=0
delay_bad_max_ms=0
reverse_urb=0.05
reverse_dur_s=2
reverse_slot_ms=100
reverse_markov_unit=frame
reverse_burst_packets=1
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
| [common.csv](common.csv) | 1 | common_s=59.452; proxy_pre=3066; proxy_drop=126; proxy_loss_pct=4.11; gateway_rx=2939 |
| [K3_20261114_175154_r2_decision_trace.csv](K3_20261114_175154_r2_decision_trace.csv) | 3403 | sysid=None; compid=None |
| [K3_20261114_175154_r2_gateway_ingress.csv](K3_20261114_175154_r2_gateway_ingress.csv) | 12 | sysid=1; compid=1; t_s=60.000; frames=2957; frame_bytes=120005; crc_errors=0 |
| [K3_20261114_175154_r2_gateway_ingress_bursts.csv](K3_20261114_175154_r2_gateway_ingress_bursts.csv) | 127 | sysid=1; compid=1; t_s=59.707 |
| [K3_20261114_175154_r2_gateway_ingress_loss_seconds.csv](K3_20261114_175154_r2_gateway_ingress_loss_seconds.csv) | 60 | sysid=1; compid=1 |
| [K3_20261114_175154_r2_gateway_ingress_trace.csv](K3_20261114_175154_r2_gateway_ingress_trace.csv) | 2957 | sysid=1; compid=1; t_s=59.9992 |
| [K3_20261114_175154_r2_proxy_post.csv](K3_20261114_175154_r2_proxy_post.csv) | 12 | sysid=1; compid=1; t_s=60.000; frames=2958; frame_bytes=119992; crc_errors=0 |
| [K3_20261114_175154_r2_proxy_pre.csv](K3_20261114_175154_r2_proxy_pre.csv) | 12 | sysid=1; compid=1; t_s=60.000; frames=3084; frame_bytes=125096; crc_errors=0 |
| [K3_20261114_175154_r2_proxy_pre_trace.csv](K3_20261114_175154_r2_proxy_pre_trace.csv) | 3084 | sysid=1; compid=1; t_s=59.9671 |
| [K3_20261114_175154_r2_reverse_states.csv](K3_20261114_175154_r2_reverse_states.csv) | 8 | sysid=1 |
| [K3_20261114_175154_r2_reverse_trace.csv](K3_20261114_175154_r2_reverse_trace.csv) | 64 | sysid=1 |
| [K3_20261114_175154_r2_states.csv](K3_20261114_175154_r2_states.csv) | 296 | sysid=1 |
| [K3_20261114_175154_r2_summary.csv](K3_20261114_175154_r2_summary.csv) | 1 | sysid=1; crc_errors=0 |
| [result.csv](result.csv) | 1 | seconds=60; frames=2957; crc_errors=0 |

## Bằng chứng bổ trợ

- [gateway.log](gateway.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [proxy.log](proxy.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [run.log](run.log): Log thao tác, counter, lỗi hoặc kết quả test.
