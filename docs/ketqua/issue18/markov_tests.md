# Issue 18 — Kiểm thử module Markov hai trạng thái

Ngày lập báo cáo: 07/11/2026.

## 1. Phạm vi và nguồn bằng chứng

Đối chiếu trực tiếp `./markov_test.log` trong gói docs(1).rar. Log có M01–M09 PASS và kết thúc bằng `ALL TESTS PASS`. Báo cáo ghi nhận lần chạy do người thực hiện cung cấp; không phải lần chạy lại trong môi trường kiểm tra tài liệu.

Module được kiểm thử độc lập bằng thời gian giả, chưa chứng minh việc ghép emulator vào Gateway/PX4. Gói docs không có `src/emulator.h` và `tests/markov_test.cpp`, nên chưa đối chiếu source thực tế với log.

## 2. Cấu hình bài test

Theo kịch bản/code được cung cấp trong trao đổi: urb=0.5; dur_s=2; slot_ms=100; lossgood=0; lossbad=1; UAV thử sysid=1; seeds=1,2,3; mỗi seed 180000 khe, một frame giữa mỗi khe. Thời gian giả tương ứng 18000 giây (5 giờ) mỗi seed. Đây là cấu hình theo kịch bản, log hiện không in đầy đủ cấu hình nên chưa xác minh độc lập mọi tham số từ log.

Dung sai theo kịch bản: tỷ lệ B ±0.03 (±3 điểm phần trăm); độ dài B trung bình ±0.2 giây. Đây là ngưỡng thực nghiệm cho bài test, không phải bảo đảm mọi seed đều đạt.

- Lệnh build đã hướng dẫn: `g++ -std=c++11 -O2 -Wall -Wextra tests/markov_test.cpp -o markov_test.exe`.
- Compiler thực tế: chưa có output `g++ --version` kèm run. Đường dẫn lỗi build trước đó có MinGW GCC 6.3.0, chưa dùng thay xác nhận của run thành công.
- Commit/hash source của run: chưa cung cấp.
- Ngày/giờ chạy chính xác: chưa có timestamp trong log.
- Exit code: chưa có bản ghi; log có ALL TESTS PASS.

## 3. Kết quả các ca

| Ca | Nội dung theo kịch bản | Kết quả log |
|---|---|---|
| M01 | Tắt emulator, giữ mọi frame | PASS |
| M02 | Từ chối cấu hình sai | PASS |
| M03 | Cùng seed/lịch đầu vào tái lập kết quả | PASS |
| M04 | Tick từng khe và cập nhật bù khớp nhau | PASS |
| M05 | Kênh vẫn tiến triển khi không có frame | PASS |
| M06 | RNG trạng thái và RNG drop riêng | PASS |
| M07 | Các UAV độc lập | PASS |
| M08 | Đối soát frame và biên loss=0/1 | PASS |
| M09 | Thống kê dài hạn qua ba seed | PASS |

## 4. Số liệu M09

| Seed | Khe | Tỷ lệ B | Đợt B trung bình (s) | Seen | Kept | Dropped | Initial partial | Final open |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 180000 | 0.495972 | 1.95879 | 180000 | 90725 | 89275 | 0 | 1 |
| 2 | 180000 | 0.513806 | 2.04665 | 180000 | 87515 | 92485 | 1 | 1 |
| 3 | 180000 | 0.503644 | 1.98249 | 180000 | 89344 | 90656 | 1 | 1 |

Cả ba seed có seen=kept+dropped=180000. Tỷ lệ B và độ dài trung bình đợt B đều nằm trong dung sai. Tỷ lệ B lần lượt là 49.5972%, 51.3806%, 50.3644%.

`initial_partial=1` nghĩa là bắt đầu khi kênh đã ở B; `final_open=1` nghĩa là khi kết thúc vẫn ở B. Theo thiết kế đã hướng dẫn, đợt đầu bị cắt và đợt cuối chưa hoàn chỉnh không được đưa vào trung bình đợt B hoàn chỉnh. Log không phải lỗi do các cờ này bằng 1.

## 5. Kết luận và việc cần bổ sung

- [x] Có log test M01–M09 PASS.
- [x] Đối soát lại số liệu ba seed và dung sai từ log.
- [x] Có báo cáo kiểm thử.
- [ ] Ghi compiler thực tế bằng `g++ --version`.
- [ ] Gắn commit chứa source/test đã dùng và lưu source trong repo.

Phần kiểm thử độc lập issue 18 đạt theo log đã cung cấp. Có thể chốt issue sau khi gắn đúng code/test và bằng chứng vào Git; gói docs không chứng minh trạng thái commit. Ghép Gateway, QoS egress và kiểm thử PX4 thuộc issue 19.
