# U19_01 — baseline UAV → Gateway

**PASS.** Proxy và Gateway đều ghi đủ 600 giây sau 10 giây khởi động. Proxy nhận 33890 frame trong cửa sổ đo, giữ và gửi đủ 33890; Markov bỏ 0, lỗi gửi/CRC/signature 0. Trace toàn phiên: proxy gửi 38612 frame, Gateway nhận 38612, `missing=0`, `extra=0`, `trace_exit=0`. Gateway học UAV 1 qua `127.0.0.1:14610`.

Gateway QoS ghi 33884 frame trong cửa sổ riêng; chênh 6 frame với proxy QoS do mốc bắt đầu khác nhau, không phải mất gói. File thô Gateway đã chuyển vào thư mục run; config/state Gateway được thêm `_gateway_` để tránh trùng file proxy.

**Tiếp theo:** U19_02 giả lập mất sóng theo đợt trung bình 2 giây.
