# Kiểm thử TCP client — Issue 16

Ngày cập nhật báo cáo: 07/11/2026.

Báo cáo được sửa từ client_tests.md và đối chiếu với client_tests_gateway.log trong docs.rar. CSV và log gốc được giữ nguyên. Số dòng dưới đây tính theo nội dung log đã giải mã UTF-16, bắt đầu từ dòng 1.

## 1. Cấu hình và phạm vi

- Một UAV PX4 SITL trong Gazebo, SYSID=1, endpoint 172.26.162.42:14600.
- Gateway UDP 14550; TCP 127.0.0.1:5760.
- MAX_CLIENTS=10; MAX_TCP_PENDING=262144 byte theo cấu hình đã cung cấp.
- QGC và console là các client bình thường theo mô tả người thực hiện.
- Commit/code version của chính lần thử: chưa ghi nhận.
- Ngày/giờ bắt đầu thực tế: chưa được nhập riêng. Tên CSV gateway chứa mốc thời gian hệ thống 1794011910944; không dùng nó thay hồ sơ thời gian nếu đồng hồ máy chưa được xác minh.
- **Số client nền thực tế trong bài B/C/D là 3**, không phải 2 như mẫu hướng dẫn ban đầu. Có thể là QGC và hai console; mapping PID/ID chưa được ghi đầy đủ nên không khẳng định riêng từng ID là QGC hay console nào.
- Log: client_tests_gateway.log. Đầu log mở CSV ingress 1794011910944.

## 2. A — Fan-out, đóng và kết nối lại console

- [x] Có ba client đồng thời theo log và quan sát người thực hiện.
- [x] Client #3 CONNECTED, tổng 3 — dòng 451.
- [x] Client #3 DISCONNECTED — dòng 675; STATS sau đó về 2.
- [x] Client #4 CONNECTED, tổng trở lại 3 — dòng 803.
- [x] Gateway tiếp tục nhận UDP và gửi TCP; client #1/#2 không bị đóng trong đoạn này.
- [x] Người thực hiện đã xác nhận client còn lại tiếp tục nhận trong mẫu báo cáo ban đầu.

**Kết luận:** đạt fan-out và disconnect/reconnect cơ bản. Kết hợp log phía Gateway với quan sát người thực hiện; log Gateway một mình không chứng minh từng frame đã được ứng dụng client xử lý.

## 3. B — Reconnect 20 lần

- [x] Có đúng 20 cặp CONNECTED/DISCONNECTED, client #5 đến #24, dòng 1210–1305.
- [x] Mỗi client thử nâng tổng từ 3 lên 4 rồi bị dọn.
- [x] Sau khi thử, số client trở về **3**, đúng số nền trước thử. Không cần về 2 khi vẫn giữ client #4.
- [x] Hai số đo tài nguyên do người thực hiện ghi có cùng PID 21372.

| Chỉ số | Trước | Sau | Thay đổi |
|---|---:|---:|---:|
| PID | 21372 | 21372 | Không đổi |
| CPU time, giây tích lũy | 3.609375 | 5.968750 | +2.359375 s |
| HandleCount | 94 | 94 | 0 |
| Working set RAM | 6.65 MB | 6.62 MB | -0.03 MB |
| Private memory | 1.13 MB | 1.05 MB | -0.08 MB |

**Nhận xét:** không thấy tăng tích lũy handle/bộ nhớ giữa hai mẫu. CPU là tổng thời gian dùng CPU, không phải phần trăm tải và không đủ suy ra CPU trung bình khi thiếu khoảng thời gian lấy mẫu. Hai mẫu không chứng minh tuyệt đối không rò rỉ bộ nhớ trong mọi điều kiện.

**Kết luận:** đạt reconnect 20 lần trong phạm vi thử.

## 4. C — Client không đọc

- [x] Client #25 CONNECTED, tổng 4 — dòng 1894.
- [x] Có `Client qua cham` — dòng 2282.
- [x] Client #25 DISCONNECTED, TCP TX=73752 byte — dòng 2283.
- [x] Slow clients tăng 0→1; STATS sau đó tổng client về 3.
- [x] Các client nền không có log disconnect; tổng TCP TX và UDP RX tiếp tục tăng.
- [x] Người thực hiện đã xác nhận các client bình thường không bị ảnh hưởng.

Thời gian chờ chính xác trước khi bị ngắt: **chưa ghi nhận** vì từng sự kiện log không có timestamp trực tiếp. Không điền số giây suy đoán.

**Kết luận:** đạt kích hoạt policy ngắt client chậm. TCP TX=73752 là byte send() đã chấp nhận ở socket client đó; không phải số byte ứng dụng client đã đọc. Không kết luận pending chỉ có 73752 byte, vì đây là hai bộ đếm khác nhau.

QoS ingress của lần chạy đã kết thúc ở 600 giây; sự kiện client chậm nằm sau đó trong log. Vì vậy không lấy CSV QoS đã dừng để chứng minh liên tục phía nhận trong toàn bộ đoạn client chậm; bằng chứng hiện tại là log Gateway và xác nhận quan sát người thực hiện.

## 5. D — Giới hạn 10 client

- [x] Trước bài thử có 3 client nền.
- [x] Client #26 đến #32 được nhận: thêm 7, tổng đạt 10 — dòng 2804–2810.
- [x] Có đúng hai dòng `Da du so client` — dòng 2811–2812.
- [x] STATS đạt 10, không ghi nhận lớn hơn 10.
- [x] Bảy client #26–#32 được đóng — dòng 2865–2871.
- [x] STATS trở lại 3, đúng số nền; Gateway tiếp tục xử lý dữ liệu sau đó.

**Kết luận:** đạt giới hạn kết nối và từ chối quá mức. Lần thử mở 9 kết nối bổ sung khi đã có 3 client, nên nhận 7 và từ chối 2 là đúng, không phải lỗi thiếu slot.

## 6. Counters và bất thường

- CRC UDP/TCP=0 trong toàn bộ các STATS có trong log.
- Signature errors=0; không suy ra đã có xác thực khóa.
- UDP dropped/cancelled=0 trong các STATS có trong log.
- Slow clients cuối các STATS đầy đủ=1.
- Không có sự kiện UAV OFFLINE trong log bài client.
- Blocked messages tăng 0→5→10→15 khi UAV online=0/chưa học route, sau đó giữ 15 khi UAV đã ONLINE. Đây là các request chưa có đích online trong giai đoạn khởi tạo, không phải CRC error hoặc mất gói mạng. Log không ghi payload từng request nên chưa xác định được message cụ thể.
- Cuối log bị dừng giữa một khối STATS: chỉ dùng khối đầy đủ cuối ở dòng 3198 trở đi (trước đoạn cuối bị cắt), không tự điền các counters còn thiếu.

## 7. Dữ liệu CSV của bài client

| Điểm/lần chạy | Dòng | t_s cuối | Frames cuối | Tình trạng |
|---|---:|---:|---:|---|
| Ingress 1794011910944 | 120 | 600.000 | 34825 | Đủ cửa sổ QoS |
| Client 1794011939330 | 120 | 600.000 | 34825 | Đủ cửa sổ QoS |
| Client 1794012053592 | 13 | 65.266 | 3792 | Đo một phần, phù hợp phiên client bị đóng |
| Client 1794012180678 | 120 | 600.000 | 34806 | Đủ cửa sổ QoS, phiên mới sau reconnect |

Mỗi điểm có file `_msgid.csv` đi kèm. Tất cả cặp tổng hợp/msgid đối soát khớp, thời gian/counters frame-byte tăng đúng. Các cột sequence và outage đều 0 trong những cửa sổ có ghi. Không so tổng giữa các phiên có mốc bắt đầu khác nhau để tính mất gói.

Hai file của client 1794012053592 không bị hỏng: chỉ chưa chạy đủ 600 giây. Chúng là dữ liệu thử disconnect, không phải một baseline hoàn chỉnh. Log không in tên file CSV ở từng lần accept nên mapping từng phiên CSV với client ID chỉ là suy luận theo thứ tự khởi động, không ghi thành định danh tuyệt đối.

## 8. Kết luận issue 16

- [x] A: fan-out và reconnect console cơ bản.
- [x] B: 20 lần reconnect; số client nền phục hồi đúng; hai mẫu tài nguyên không tăng bất thường.
- [x] C: client chậm bị ngắt, Slow clients tăng, Gateway/client nền tiếp tục hoạt động theo log và xác nhận người thực hiện.
- [x] D: nhận tối đa 10 client, từ chối 2 yêu cầu vượt mức, cleanup về 3 client nền.

**Có thể đóng issue 16 trong phạm vi các bài trên**, kèm log và bảng tài nguyên. Chưa coi đây là E4/E5/E6 đủ ma trận thí nghiệm, bài đo CPU phần trăm, kiểm chứng partial send bị ép hoặc chứng minh không rò rỉ trong mọi tải.

**Không cần chạy lại chỉ để số client nền về 2.** Chỉ cần ghi đúng nền=3 như log. Có thể bổ sung timestamp sự kiện/thu số liệu phía client trong lần thử sau để bằng chứng mạnh hơn.
