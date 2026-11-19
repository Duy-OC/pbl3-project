# U19_02 — mất sóng theo đợt trung bình 2 giây

**PASS.** Proxy và Gateway đều ghi đủ 600 giây. Cấu hình proxy: `urb=0.5`, `dur_s=2`, `lossgood=0`, `lossbad=1`, seed 1; Gateway giữ `urb=0`.

Trong cửa sổ proxy: nhận 33458 frame = giữ/gửi 16546 + bỏ 16912; trạng thái G có 16546 frame và bỏ 0, trạng thái B có 16912 frame và bỏ cả 16912. Tỷ lệ thời gian B gồm warm-up là 3098/6100 slot = 0.507869; đợt B hoàn chỉnh trung bình 2.15845 giây. Lỗi CRC, chữ ký và gửi UDP đều bằng 0.

Trace toàn phiên: proxy gửi 19448 frame, Gateway nhận 19448, `missing=0`, `extra=0`, `trace_exit=0`. Gateway QoS ghi 15943 frame trong cửa sổ riêng; không so hiệu với 16546 frame của proxy vì thời điểm bắt đầu cửa sổ khác nhau và có đợt mất heartbeat.

File thô Gateway đã chuyển vào thư mục run; config và state Gateway được thêm `_gateway_` để không trùng file proxy. Tiếp theo là U19_03 với đợt mất sóng trung bình 5 giây.
