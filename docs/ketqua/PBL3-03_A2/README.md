# PBL3-03_A2 — kịch bản và kết quả

Đường dẫn: `docs/ketqua/PBL3-03_A2`.

## Kịch bản đo

PX4 gửi UDP 14550 trực tiếp; tách lượt khảo sát 60 giây và lượt 600 giây. Đọc frame, byte, HEARTBEAT, MSGID và CRC/signature.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

## Nhận xét gốc

- [A2_600_run1.md](A2_600_run1.md)
- [A2_khao_sat_60s.md](A2_khao_sat_60s.md)
