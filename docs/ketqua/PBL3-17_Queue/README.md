# PBL3-17_Queue — kịch bản và kết quả

Đường dẫn: `docs/ketqua/PBL3-17_Queue`.

## Kịch bản đo

Giữ lệnh bằng test_hold_udp_ms, rồi gây ngắt client/route, queue đầy hoặc lỗi sendto. Đối chiếu queued/dropped/cancelled với nguyên nhân trong log; timeout probe có thể là kết quả mong đợi.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

## Nhận xét gốc

- [summary.md](summary.md)

Chi tiết lượt gốc: **Kết luận:** PASS PBL3-17 trong điều kiện PX4 SITL và các chế độ thử queue/lỗi gửi đã ghi rõ trong config từng run. Lỗi `sendto` được gây có kiểm soát bằng đối số không hợp lệ; không đại diện cho tỷ lệ lỗi mạng tự nhiên.

## Các nhóm/lượt con

- [I17_01](I17_01/README.md)
- [I17_02C](I17_02C/README.md)
- [I17_03](I17_03/README.md)
- [I17_04](I17_04/README.md)
- [I17_05](I17_05/README.md)
