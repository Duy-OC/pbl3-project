# PBL3-16 — giới hạn 10 client, run I16_03

- Mười TCP client đầu đều kết nối và đọc telemetry. Mỗi client đọc 11881–11925 byte trước lần thử thứ 11, rồi tiếp tục đọc 15611–15655 byte sau đó.
- Kết nối thứ 11 bắt tay TCP nhưng `11th_read=0`; Gateway ghi `Da du so client`, vẫn `Clients=10` và chỉ có Client #1–#10 CONNECTED. Đây là từ chối ở tầng ứng dụng sau TCP handshake.
- Sau khi đóng mười client, Gateway ghi đủ 10 DISCONNECTED và STATS `Clients=0`. CRC/signature errors=0, BLOCK=0, Slow clients=0. Gateway handles ở mẫu khi đủ 10 client là 104; sau đóng còn 94. Working set 7098368→7020544 byte trong hai mẫu.

**Kết luận:** PASS giới hạn 10 client và cô lập lần thử thứ 11; mười client hiện hữu vẫn nhận dữ liệu. Cần đo riêng 1/5/10 client trong cửa sổ 600 s để so tải và tài nguyên.
