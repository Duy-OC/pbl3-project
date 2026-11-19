# PBL3-14_E0 — kịch bản và kết quả

Đường dẫn: `docs/ketqua/PBL3-14_E0`.

## Kịch bản đo

Baseline PX4, một client, urb=0; phiên bản đo cũ warm-up 10 giây rồi đo 600 giây. Đối soát frame ingress/egress, MSGID, CRC, jitter, offline và tài nguyên.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

## Các nhóm/lượt con

- [E0_01](E0_01/README.md)
- [E0_02](E0_02/README.md)
