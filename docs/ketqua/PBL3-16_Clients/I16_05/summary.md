# PBL3-16 — tải năm TCP client, run I16_05

- Helper: năm client đều active tới `t=600 s`, mỗi client 33938–33939 frame, 1375307–1375347 byte, 556 HEARTBEAT; tổng 169691 frame/6876575 byte/2780 HEARTBEAT. CRC/signature errors=0, disconnected=0, `pass=1`, exit 0.
- Tài nguyên: 11 mẫu trên 600.2 s. Gateway CPU tích lũy tăng 8.188 s, handles giữ 99. Helper tăng 4.125 s, handles giữ 92. Working set Gateway 0.98–6.74 MiB, helper 0.73–6.26 MiB; Windows đã thu gọn working set giữa run, nên không diễn giải giá trị nhỏ là mức bộ nhớ cấp phát ổn định. Private bytes và log gốc giữ trong `resources.csv`.

So với I16_04, số frame ingress gần tương đương (33943 so với 33989), còn Gateway CPU tăng từ 4.531 lên 8.188 giây trong ~600 s. Đây là so sánh mô tả của hai run; chưa đủ để suy quan hệ tuyến tính tải theo số client.

**Kết luận:** PASS mức năm client trong 600 s, mỗi client nhận telemetry và không bị ngắt. Còn mức 10 client để chốt ma trận tải PBL3-16.
