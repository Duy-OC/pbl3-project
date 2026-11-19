# PBL3-02_UDP — kịch bản và kết quả

Đường dẫn: `docs/ketqua/PBL3-02_UDP`.

## Kịch bản đo

UDP echo: gửi payload đã biết bằng udp_echo_check, đối chiếu số byte và nội dung nhận lại, đọc exit code và lỗi Winsock.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

## Nhận xét gốc

- [udp_echo_result.md](udp_echo_result.md)

## Bằng chứng bổ trợ

- [metrics_02.log](metrics_02.log): Log thao tác, counter, lỗi hoặc kết quả test.
