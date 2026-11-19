# I18_01 — kịch bản và kết quả

Đường dẫn: `docs/ketqua/PBL3-18_Markov/I18_01`.

## Kịch bản đo

Markov độc lập, 3 seed × 180000 slot, thêm chế độ OFF và cấu hình sai. Đọc bad fraction, thời gian B trung bình, seen=kept+dropped và tái lập seed; không dùng PX4.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

## Nhận xét gốc

- [summary.md](summary.md)

Chi tiết lượt gốc: Nguồn: `build.log` và `model.log`. Helper đọc trực tiếp `src/emulator.h`; thời gian được mô phỏng, không dùng PX4 hay Gateway. Cấu hình ON: `urb=0.5`, `dur_s=2`, `slot_ms=100`, `lossgood=0`, `lossbad=1`; mỗi seed 180000 slot và một frame mỗi slot.

## Bằng chứng bổ trợ

- [build.log](build.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [model.log](model.log): Log thao tác, counter, lỗi hoặc kết quả test.
