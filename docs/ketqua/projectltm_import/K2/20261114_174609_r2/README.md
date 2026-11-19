# 20261114_174609_r2 — kịch bản và kết quả

Đường dẫn: `docs/ketqua/projectltm_import/K2/20261114_174609_r2`.

## Kịch bản đo

Loss 5%, cụm 4 frame, hai chiều; 3 lượt × 120 giây + warm-up 5 giây; Markov theo frame p=0.013158, r=0.25.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

Nguồn: projectltm.rar. Không nhập proxy từ archive; số đo phụ thuộc proxy gốc chỉ là bằng chứng tham khảo. Báo cáo gốc có liên kết nhóm D/F không có trong archive, không dùng các liên kết đó làm bằng chứng đạt.

## Cấu hình của lượt

[K2_20261114_174609_r2_config.txt](K2_20261114_174609_r2_config.txt)

```text
scenario=K2
run_id=20261114_174609_r2
sysid=1
px4_listen_port=14550
gateway_source_port=14610
gateway_address=127.0.0.1:14551
urb=0.05
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
reverse_urb=0.05
reverse_dur_s=2
reverse_slot_ms=100
reverse_markov_unit=frame
reverse_burst_packets=4
reverse_lossgood=0
reverse_lossbad=1
reverse_seed=2654435923
warmup_s=5
measure_s=120
```

## Số liệu CSV

Số dòng không gồm header. Giá trị dưới đây chỉ thuộc **bản ghi cuối**, không phải tổng mọi UAV. Với summary.csv đọc metric/mean/std và đơn vị gốc; file chỉ có header không có mẫu đo.

| File | Dòng dữ liệu | Giá trị cuối tiêu biểu |
|---|---:|---|
| [K2_20261114_174609_r2_decision_trace.csv](K2_20261114_174609_r2_decision_trace.csv) | 6470 | sysid=1; compid=1 |
| [K2_20261114_174609_r2_gateway_ingress.csv](K2_20261114_174609_r2_gateway_ingress.csv) | 24 | sysid=1; compid=1; t_s=120.000; frames=5933; frame_bytes=240587; crc_errors=0 |
| [K2_20261114_174609_r2_gateway_ingress_bursts.csv](K2_20261114_174609_r2_gateway_ingress_bursts.csv) | 64 | sysid=1; compid=1; t_s=117.994 |
| [K2_20261114_174609_r2_gateway_ingress_loss_seconds.csv](K2_20261114_174609_r2_gateway_ingress_loss_seconds.csv) | 120 | sysid=1; compid=1 |
| [K2_20261114_174609_r2_proxy_post.csv](K2_20261114_174609_r2_proxy_post.csv) | 24 | sysid=1; compid=1; t_s=120.000; frames=5916; frame_bytes=239812; crc_errors=0 |
| [K2_20261114_174609_r2_proxy_pre.csv](K2_20261114_174609_r2_proxy_pre.csv) | 24 | sysid=1; compid=1; t_s=120.000; frames=6171; frame_bytes=250342; crc_errors=0 |
| [K2_20261114_174609_r2_reverse_states.csv](K2_20261114_174609_r2_reverse_states.csv) | 7 | sysid=1 |
| [K2_20261114_174609_r2_reverse_trace.csv](K2_20261114_174609_r2_reverse_trace.csv) | 122 | sysid=1 |
| [K2_20261114_174609_r2_states.csv](K2_20261114_174609_r2_states.csv) | 140 | sysid=1 |
| [K2_20261114_174609_r2_summary.csv](K2_20261114_174609_r2_summary.csv) | 1 | sysid=1; crc_errors=0 |
| [result.csv](result.csv) | 1 | seconds=120; frames=5933; crc_errors=0 |

## Bằng chứng bổ trợ

- [gateway.log](gateway.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [proxy.log](proxy.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [run.log](run.log): Log thao tác, counter, lỗi hoặc kết quả test.
