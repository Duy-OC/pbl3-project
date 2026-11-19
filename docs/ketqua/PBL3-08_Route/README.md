# PBL3-08_Route — kịch bản và kết quả

Đường dẫn: `docs/ketqua/PBL3-08_Route`.

## Kịch bản đo

Hai UAV có SYSID và endpoint riêng; chọn từng SYSID để gửi yêu cầu, đối chiếu ROUTE/ACK đúng đích. ACK yêu cầu thông tin không chứng minh thao tác bay thành công.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

## Nhận xét gốc

- [I08_targeted_from_chat.md](I08_targeted_from_chat.md)
- [preflight_px4.md](preflight_px4.md)

## Các nhóm/lượt con

- [I08_01](I08_01/README.md)
- [I08_legacy_215s](I08_legacy_215s/README.md)
