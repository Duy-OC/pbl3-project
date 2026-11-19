# 20261114_172116 — kịch bản và kết quả

Đường dẫn: `docs/ketqua/projectltm_import/K0/20261114_172116`.

## Kịch bản đo

100000 gói tổng hợp/lượt, 3 seed/cấu hình; kiểm loss/cụm mất và tái lập, không có PX4.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

Nguồn: projectltm.rar. Không nhập proxy từ archive; số đo phụ thuộc proxy gốc chỉ là bằng chứng tham khảo. Báo cáo gốc có liên kết nhóm D/F không có trong archive, không dùng các liên kết đó làm bằng chứng đạt.

## Số liệu CSV

Số dòng không gồm header. Giá trị dưới đây chỉ thuộc **bản ghi cuối**, không phải tổng mọi UAV. Với summary.csv đọc metric/mean/std và đơn vị gốc; file chỉ có header không có mẫu đo.

| File | Dòng dữ liệu | Giá trị cuối tiêu biểu |
|---|---:|---|
| [result.csv](result.csv) | 12 | case=K4; repeat=3; seed=3003; packets=100000; loss_target_pct=10 |
| [summary.csv](summary.csv) | 12 | metric=burst_max_packets; mean=32.3333; std=6.6583 |

## Bằng chứng bổ trợ

- [run.log](run.log): Log thao tác, counter, lỗi hoặc kết quả test.
