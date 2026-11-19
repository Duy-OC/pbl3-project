# tich_hop_projectltm — kịch bản và kết quả

Đường dẫn: `docs/ketqua/tich_hop_projectltm`.

## Kịch bản đo

Kiểm thử bằng UAV giả: parser/CRC, telemetry, sequence, Markov, khóa/ACK/retry và QGC. Các file là lần chạy tích hợp trước khi bỏ xuất file; không phải phép đo PX4. Build.log và sequence.log rỗng đã xóa; sequence.csv là kết quả sequence thực tế.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

## Số liệu CSV

Số dòng không gồm header. Giá trị dưới đây chỉ thuộc **bản ghi cuối**, không phải tổng mọi UAV. Với summary.csv đọc metric/mean/std và đơn vị gốc; file chỉ có header không có mẫu đo.

| File | Dòng dữ liệu | Giá trị cuối tiêu biểu |
|---|---:|---|
| [sequence.csv](sequence.csv) | 11 | scenario=silence_no_false_gap; expected=0; actual=0; result=PASS |
| [stage1_framing_results.csv](stage1_framing_results.csv) | 5 | status=PASS |
| [stage2_telemetry_results.csv](stage2_telemetry_results.csv) | 6 | status=PASS |

## Bằng chứng bổ trợ

- [control.log](control.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [framing.log](framing.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [markov.log](markov.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [qgc.log](qgc.log): Log thao tác, counter, lỗi hoặc kết quả test.
- [stage1_framing_log.txt](stage1_framing_log.txt): Metadata hoặc ghi chép gốc.
- [telemetry.log](telemetry.log): Log thao tác, counter, lỗi hoặc kết quả test.
