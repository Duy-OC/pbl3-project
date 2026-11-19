# PBL3-18_Markov — kịch bản và kết quả

Đường dẫn: `docs/ketqua/PBL3-18_Markov`.

## Kịch bản đo

Markov độc lập, 3 seed × 180000 slot, thêm chế độ OFF và cấu hình sai. Đọc bad fraction, thời gian B trung bình, seen=kept+dropped và tái lập seed; không dùng PX4.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

## Các nhóm/lượt con

- [I18_01](I18_01/README.md)
