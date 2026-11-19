# PBL3-10_Build — kịch bản và kết quả

Đường dẫn: `docs/ketqua/PBL3-10_Build`.

## Kịch bản đo

Build riêng gateway/client, đọc compiler, exit code và hash trong build.log. Executable lịch sử đã xóa; hash trong log không phải hash binary hiện tại. Build hiện tại dùng build.bat.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

## Các nhóm/lượt con

- [I10_01](I10_01/README.md)
