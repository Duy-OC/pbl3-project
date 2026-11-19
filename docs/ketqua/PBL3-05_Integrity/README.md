# PBL3-05_Integrity — kịch bản và kết quả

Đường dẫn: `docs/ketqua/PBL3-05_Integrity`.

## Kịch bản đo

Chạy mavlink_integrity_check: serialize/parse, payload nhị phân và CRC hỏng. attempt_02_pass là phép thử hợp lệ; giữ attempt_01_invalid_test để giải thích lỗi thiết kế phép thử đầu.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

## Nhận xét gốc

- [attempt_01_invalid_test.md](attempt_01_invalid_test.md)
- [attempt_02_pass.md](attempt_02_pass.md)

## Bằng chứng bổ trợ

- [metrics_03.log](metrics_03.log): Log thao tác, counter, lỗi hoặc kết quả test.
