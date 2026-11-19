# 20261114_173811_r2 — kịch bản và kết quả

Đường dẫn: `docs/ketqua/projectltm_import/K1/20261114_173811_r2`.

## Kịch bản đo

Baseline PX4 trực tiếp, không proxy; 3 lượt × 120 giây, ổn định 5 giây/lượt; HEARTBEAT 1 Hz, ATTITUDE 10 Hz.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

Nguồn: projectltm.rar. Không nhập proxy từ archive; số đo phụ thuộc proxy gốc chỉ là bằng chứng tham khảo. Báo cáo gốc có liên kết nhóm D/F không có trong archive, không dùng các liên kết đó làm bằng chứng đạt.

## Số liệu CSV

Số dòng không gồm header. Giá trị dưới đây chỉ thuộc **bản ghi cuối**, không phải tổng mọi UAV. Với summary.csv đọc metric/mean/std và đơn vị gốc; file chỉ có header không có mẫu đo.

| File | Dòng dữ liệu | Giá trị cuối tiêu biểu |
|---|---:|---|
| [K1_20261114_173811_r2_gateway_ingress.csv](K1_20261114_173811_r2_gateway_ingress.csv) | 24 | sysid=1; compid=1; t_s=120.000; frames=6165; frame_bytes=250048; crc_errors=0 |
| [K1_20261114_173811_r2_gateway_ingress_bursts.csv](K1_20261114_173811_r2_gateway_ingress_bursts.csv) | 0 | Không có mẫu |
| [K1_20261114_173811_r2_gateway_ingress_loss_seconds.csv](K1_20261114_173811_r2_gateway_ingress_loss_seconds.csv) | 120 | sysid=1; compid=1 |
| [result.csv](result.csv) | 1 | seconds=120; frames=6165; crc_errors=0 |

## Bằng chứng bổ trợ

- [gateway.log](gateway.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [run.log](run.log): Log thao tác, counter, lỗi hoặc kết quả test.
