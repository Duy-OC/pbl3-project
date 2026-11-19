# upstream — kịch bản và kết quả

Đường dẫn: `docs/ketqua/PBL3-19_GatewayMarkov/upstream`.

## Kịch bản đo

PX4 → proxy upstream → gateway, gateway urb=0. Đo OFF/ON theo cấu hình từng run; kiểm pre=kept+dropped và missing/extra trên trace toàn phiên, không trừ hai tổng QoS khác cửa sổ.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

## Các nhóm/lượt con

- [U19_00](U19_00/README.md)
- [U19_01](U19_01/README.md)
- [U19_02](U19_02/README.md)
