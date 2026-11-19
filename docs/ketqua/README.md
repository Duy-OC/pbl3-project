# ketqua — kịch bản và kết quả

Đường dẫn: `docs/ketqua`.

## Kịch bản đo

Chọn nhóm hoặc lượt bên dưới; đọc kịch bản của lượt trước khi dùng số liệu. Đây là thư mục tổng hợp, không phải một phép đo riêng.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

## Các nhóm/lượt con

- [00_TONG_HOP](00_TONG_HOP/README.md)
- [PBL3-01_A1](PBL3-01_A1/README.md)
- [PBL3-02_UDP](PBL3-02_UDP/README.md)
- [PBL3-03_A2](PBL3-03_A2/README.md)
- [PBL3-04_Parser](PBL3-04_Parser/README.md)
- [PBL3-05_Integrity](PBL3-05_Integrity/README.md)
- [PBL3-06_TCP](PBL3-06_TCP/README.md)
- [PBL3-07_Fanout](PBL3-07_Fanout/README.md)
- [PBL3-08_Route](PBL3-08_Route/README.md)
- [PBL3-09_Recovery](PBL3-09_Recovery/README.md)
- [PBL3-10_Build](PBL3-10_Build/README.md)
- [PBL3-11_Stats](PBL3-11_Stats/README.md)
- [PBL3-12_QoS](PBL3-12_QoS/README.md)
- [PBL3-13_Heartbeat](PBL3-13_Heartbeat/README.md)
- [PBL3-14_E0](PBL3-14_E0/README.md)
- [PBL3-16_Clients](PBL3-16_Clients/README.md)
- [PBL3-17_Queue](PBL3-17_Queue/README.md)
- [PBL3-18_Markov](PBL3-18_Markov/README.md)
- [PBL3-19_GatewayMarkov](PBL3-19_GatewayMarkov/README.md)
- [projectltm_import](projectltm_import/README.md)
- [tich_hop_projectltm](tich_hop_projectltm/README.md)
