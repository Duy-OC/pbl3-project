# PBL3-12_QoS — kịch bản và kết quả

Đường dẫn: `docs/ketqua/PBL3-12_QoS`.

## Kịch bản đo

Đọc CSV E0_01, ghép theo t_s/sysid/compid, cộng frame/byte theo MSGID rồi so hàng tổng tại từng điểm. Kiểm 120 mẫu tới 600 giây; không so hiệu hai cửa sổ khác mốc thành mất gói.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

## Nhận xét gốc

- [summary.md](summary.md)

Chi tiết lượt gốc: Nguồn dữ liệu đo do người đo chạy trước đó tại `docs/ketqua/PBL3-14_E0/E0_01/`. Kiểm toán đọc-only bằng `Import-Csv`, ghép mỗi mẫu theo `t_s`, `sysid`, `compid`, cộng `frames` và `frame_bytes` của mọi MSGID rồi so với hàng tổng cùng thời điểm. Không tạo lượt đo mới.
