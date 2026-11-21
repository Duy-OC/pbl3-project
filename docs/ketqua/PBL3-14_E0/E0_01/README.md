# E0_01 — kịch bản và kết quả

Đường dẫn: `docs/ketqua/PBL3-14_E0/E0_01`.

## Kịch bản đo

Baseline PX4, một client, urb=0; phiên bản đo cũ warm-up 10 giây rồi đo 600 giây. Đối soát frame ingress/egress, MSGID, CRC, jitter, offline và tài nguyên.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

## Cấu hình của lượt

[E0_E0_01_config.txt](E0_E0_01_config.txt)

```text
(File rỗng: thiếu metadata gốc; giữ để chứng minh thiếu sót.)
```

## Số liệu CSV

Số dòng không gồm header. Giá trị dưới đây chỉ thuộc **bản ghi cuối**, không phải tổng mọi UAV. Với summary.csv đọc metric/mean/std và đơn vị gốc; file chỉ có header không có mẫu đo.

| File | Dòng dữ liệu | Giá trị cuối tiêu biểu |
|---|---:|---|
| [E0_1794462221839_client_rx.csv](E0_1794462221839_client_rx.csv) | 120 | sysid=1; compid=1; t_s=600.000; frames=34818; frame_bytes=1406906 |
| [E0_1794462221839_client_rx_msgid.csv](E0_1794462221839_client_rx_msgid.csv) | 3718 | sysid=1; compid=1; t_s=600.000; frames=571; frame_bytes=21127 |
| [E0_E0_01_gateway_egress.csv](E0_E0_01_gateway_egress.csv) | 120 | sysid=1; compid=1; t_s=600.000; frames=34900; frame_bytes=1410290 |
| [E0_E0_01_gateway_egress_msgid.csv](E0_E0_01_gateway_egress_msgid.csv) | 3720 | sysid=1; compid=1; t_s=600.000; frames=572; frame_bytes=21164 |
| [E0_E0_01_gateway_ingress.csv](E0_E0_01_gateway_ingress.csv) | 120 | sysid=1; compid=1; t_s=600.000; frames=34900; frame_bytes=1410290 |
| [E0_E0_01_gateway_ingress_msgid.csv](E0_E0_01_gateway_ingress_msgid.csv) | 3720 | sysid=1; compid=1; t_s=600.000; frames=572; frame_bytes=21164 |
| [E0_E0_01_states.csv](E0_E0_01_states.csv) | 2 | sysid=1 |
