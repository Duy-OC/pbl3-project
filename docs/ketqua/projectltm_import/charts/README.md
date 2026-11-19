# charts — kịch bản và kết quả

Đường dẫn: `docs/ketqua/projectltm_import/charts`.

## Kịch bản đo

Biểu đồ từ CSV gốc K0–K8; đọc cùng báo cáo và dữ liệu số, không thay CSV bằng giá trị ước lượng trên ảnh.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

Nguồn: projectltm.rar. Không nhập proxy từ archive; số đo phụ thuộc proxy gốc chỉ là bằng chứng tham khảo. Báo cáo gốc có liên kết nhóm D/F không có trong archive, không dùng các liên kết đó làm bằng chứng đạt.

## Bằng chứng bổ trợ

- [H1_loss.png](H1_loss.png): Hình minh họa từ dữ liệu gốc.
- [H2_bursts.png](H2_bursts.png): Hình minh họa từ dữ liệu gốc.
- [H3_loss_time.png](H3_loss_time.png): Hình minh họa từ dữ liệu gốc.
- [H4_jitter.png](H4_jitter.png): Hình minh họa từ dữ liệu gốc.
- [H5_commands.png](H5_commands.png): Hình minh họa từ dữ liệu gốc.
- [H6_pps.png](H6_pps.png): Hình minh họa từ dữ liệu gốc.
