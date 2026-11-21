# PBL3-16_Clients — kịch bản và kết quả

Đường dẫn: `docs/ketqua/PBL3-16_Clients`.

## Kịch bản đo

Thử riêng reconnect, client không đọc, giới hạn 10 client và tải 1/5/10 client. Đọc số client active, ACK, slow client, CRC và CPU/RAM theo từng lượt.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

## Nhận xét gốc

- [summary.md](summary.md)

## Các nhóm/lượt con

- [I16_01](I16_01/README.md)
- [I16_02](I16_02/README.md)
- [I16_03](I16_03/README.md)
- [I16_04](I16_04/README.md)
- [I16_05](I16_05/README.md)
- [I16_06](I16_06/README.md)
