# Tổng hợp PBL3-16 — client và tải

| Run | Ca | Kết quả chính |
|---|---|---|
| I16_01 | 20 reconnect | 20/20 route+ACK, 20 CONNECTED/DISCONNECTED, Clients cuối=0 |
| I16_02 | Client không đọc | Slow clients=1; client thường tiếp tục nhận đến 600 s |
| I16_03 | 10 client và thứ 11 | 10 client nhận tiếp, thứ 11 bị từ chối, handles 104→94 sau đóng |
| I16_04 | 1 client, 600 s | Mỗi client 33986 frame, 557 HB, 0 lỗi/ngắt |
| I16_05 | 5 client, 600 s | Mỗi client 33938–33939 frame, 556 HB, 0 lỗi/ngắt |
| I16_06 | 10 client, 600 s | Mỗi client 34078–34079 frame, 558 HB, 0 lỗi/ngắt |

**Kết luận:** PASS PBL3-16 trong điều kiện mô phỏng và mức tải đã đo. File raw và nhận xét chi tiết nằm ở từng thư mục I16_01–06.
