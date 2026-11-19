# L5 — kịch bản và kết quả

Đường dẫn: `docs/ketqua/projectltm_import/K6/L5`.

## Kịch bản đo

UAV giả trả ACK, 50 lệnh ARM/lượt, 2 lượt/mức; loss hai chiều, cụm trung bình 4 frame. Đọc số lần gửi, ACK thành công và RTT. Mức loss của thư mục này: 5%.

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
| [summary.csv](summary.csv) | 3 | metric=reverse_loss_pct; mean=2.812; std=1.1201 |

## Các nhóm/lượt con

- [20261114_180446_r1](20261114_180446_r1/README.md)
- [20261114_180554_r2](20261114_180554_r2/README.md)
