# 00_TONG_HOP — kịch bản và kết quả

Đường dẫn: `docs/ketqua/00_TONG_HOP`.

## Kịch bản đo

Kiểm kê bằng chứng theo ngày, chỉ mục và danh sách file đã xóa. Không phải một lượt đo; trạng thái trong tài liệu cũ phản ánh thời điểm ghi.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

## Nhận xét gốc

- [KIEM_KE_BANG_CHUNG_20261112.md](KIEM_KE_BANG_CHUNG_20261112.md)

## Bằng chứng bổ trợ

- [FILES_DA_XOA_20261116.txt](FILES_DA_XOA_20261116.txt): Metadata hoặc ghi chép gốc.

## Lần dọn mới nhất

[Dọn bằng chứng ngày 16/11/2026](DON_DEP_20261116.md).
