# 20261114_180053 — kịch bản và kết quả

Đường dẫn: `docs/ketqua/projectltm_import/K8/20261114_180053`.

## Kịch bản đo

Tạm dừng PX4 rồi phục hồi để đo timeout; đọc offline event và restore_probe. Timeout gốc 3 giây không áp dụng trực tiếp cho timeout 5 giây hiện tại.

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
| [K8_20261114_180053_gateway_ingress.csv](K8_20261114_180053_gateway_ingress.csv) | 0 | Không có mẫu |
| [K8_20261114_180053_gateway_ingress_bursts.csv](K8_20261114_180053_gateway_ingress_bursts.csv) | 0 | Không có mẫu |
| [K8_20261114_180053_gateway_ingress_loss_seconds.csv](K8_20261114_180053_gateway_ingress_loss_seconds.csv) | 0 | Không có mẫu |
| [K8_20261114_180053_gateway_ingress_msgid.csv](K8_20261114_180053_gateway_ingress_msgid.csv) | 0 | Không có mẫu |
| [restore_probe.csv](restore_probe.csv) | 1 | seconds=5.0; frames=244 |
| [result.csv](result.csv) | 1 | run_id=20261114_180053; px4_pid=2385; action=SIGSTOP_then_SIGCONT; stop_epoch_s=1794654054.381; threshold_s=3 |

## Bằng chứng bổ trợ

- [gateway.log](gateway.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [restore.log](restore.log): Log thao tác, counter, lỗi hoặc kết quả test.
