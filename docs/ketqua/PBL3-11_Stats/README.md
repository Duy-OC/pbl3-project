# PBL3-11_Stats — kịch bản và kết quả

Đường dẫn: `docs/ketqua/PBL3-11_Stats`.

## Kịch bản đo

Gửi một HEARTBEAT CRC hỏng qua UDP và một qua TCP; STATS kỳ vọng CRC tổng tăng 2, UDP=1, TCP=1; không tính frame sai là telemetry hợp lệ.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

## Nhận xét gốc

- [summary.md](summary.md)

Chi tiết lượt gốc: **Kết luận:** PASS việc tách các nhóm lỗi trong STATS qua những lượt kích hoạt riêng. Bộ đếm `UDP dropped/cancelled` gộp nhiều nguyên nhân, nên phải dùng log nguyên nhân của từng lượt để phân tích chi tiết.

## Các nhóm/lượt con

- [I11_01](I11_01/README.md)
