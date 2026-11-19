# PBL3-01_A1 — kịch bản và kết quả

Đường dẫn: `docs/ketqua/PBL3-01_A1`.

## Kịch bản đo

QGC trực tiếp PX4: quan sát online và cập nhật telemetry ít nhất 2 phút, ghi mavlink status. Chưa đủ ảnh/phiên bản thì chưa kết luận đạt toàn ca.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

## Nhận xét gốc

- [A1_QGC_truc_tiep.md](A1_QGC_truc_tiep.md)
