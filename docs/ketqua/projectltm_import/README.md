# projectltm_import — kịch bản và kết quả

Đường dẫn: `docs/ketqua/projectltm_import`.

## Kịch bản đo

Bằng chứng gốc từ projectltm.rar; chọn nhóm K0–K8 hoặc đọc báo cáo gốc để biết điều kiện đo.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

Nguồn: projectltm.rar. Không nhập proxy từ archive; số đo phụ thuộc proxy gốc chỉ là bằng chứng tham khảo. Báo cáo gốc có liên kết nhóm D/F không có trong archive, không dùng các liên kết đó làm bằng chứng đạt.

## Nhận xét gốc

- [bao_cao_ket_qua_do_lai.md](bao_cao_ket_qua_do_lai.md)
- [huong_dan_do_K0_K7.md](huong_dan_do_K0_K7.md)
- [moi_truong_do_K.md](moi_truong_do_K.md)

## Các nhóm/lượt con

- [charts](charts/README.md)
- [K0](K0/README.md)
- [K1](K1/README.md)
- [K2](K2/README.md)
- [K3](K3/README.md)
- [K4](K4/README.md)
- [K5](K5/README.md)
- [K6](K6/README.md)
- [K7](K7/README.md)
- [K8](K8/README.md)
