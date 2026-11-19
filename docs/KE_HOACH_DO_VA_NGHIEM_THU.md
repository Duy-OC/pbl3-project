# Kế hoạch đo và nghiệm thu PBL3

Tài liệu này gộp `huong_dan_do (1).md` với tiêu chí trong GitHub Issues của `Duy-OC/pbl3-project`. **Theo dõi bước đang làm và thứ tự cố định ở `docs/TRINH_TU_KIEM_THU_THEO_ISSUE.md`**. Phạm vi Markov là tuyến UAV → Gateway theo [mô hình và điểm đo](MARKOV_UAV_GATEWAY.md). Chỉ tạo kết quả khi người đo tự chạy lệnh; không dùng số tham chiếu cũ làm số liệu của run mới. Một run bị lỗi phải giữ nguyên và đánh dấu `FAIL`/`CHƯA ĐỦ BẰNG CHỨNG`.

## 1. Quy tắc chung cho mọi run

1. Ghi ngày giờ, run ID duy nhất, ca đo, SYSID, số client, trạng thái UAV, câu lệnh PX4, Gateway và cấu hình Markov, phiên bản PX4/MAVLink, commit/hash nguồn. Ghi `chưa biết` nếu thiếu.
2. Dùng cùng PX4 stream, trạng thái UAV và máy đo cho các run cần so sánh. Không so trực tiếp A1 `instance #0:18570` với A2 `instance #4:14600` vì đó là hai MAVLink instance khác nhau.
3. Dùng 10 giây warm-up và 600 giây đo chính, trừ nơi ghi khác. Giữ dữ liệu thô và log. Sau mỗi run kiểm tra thời lượng, tổng MSGID, CRC, số HEARTBEAT, `frames_in = kept + dropped`, và điều kiện nghiệm thu riêng.
4. Ingress và egress của Gateway dùng chung cửa sổ. Client RX tự mở cửa sổ riêng; không lấy hiệu số client và Gateway để tính mất gói. `seq gap` là ước lượng, không phải tỷ lệ mất mạng chính xác.
5. Tắt QGC khi một chương trình Windows khác cần bind UDP 14550. QGC nối TCP Gateway ở `127.0.0.1:5760` khi ca đo yêu cầu QGC qua Gateway.
6. Tách `PASS`, `FAIL`, `CHƯA THỬ` và `CHƯA ĐỦ BẰNG CHỨNG`. Issue đã đóng trong GitHub không tự chứng minh run mới sau khi mã nguồn hoặc stream thay đổi.

## 2. Các ca theo thứ tự

| Bước | Phép đo/kiểm thử | Cách làm và bằng chứng tối thiểu | Issue |
|---|---|---|---|
| A1 | PX4/Gazebo/QGC trực tiếp | Quan sát QGC 2 phút; 3 mẫu `mavlink status`; ảnh UAV, lệnh PX4, cổng/IP. | [PBL3-01](https://github.com/Duy-OC/pbl3-project/issues/4) |
| A2 | PX4 → UDP receiver | QGC/Gateway đóng; 60 giây khảo sát, ít nhất 1 run 600 giây; frame/byte/s, MSGID, HEARTBEAT, CRC, SYSID/COMPID, nguồn UDP. Lặp thêm nếu cần độ phân tán; GitHub Issue không bắt buộc A2 ba lần. | [PBL3-03](https://github.com/Duy-OC/pbl3-project/issues/6), [PBL3-04](https://github.com/Duy-OC/pbl3-project/issues/7) |
| P0 | Chốt môi trường và build | Lưu lệnh build, `git rev-parse HEAD`, `git status --short`, hash file nguồn đã sửa, phiên bản PX4, MAVLink, compiler, lệnh stream; build riêng Gateway/client. | [PBL3-10](https://github.com/Duy-OC/pbl3-project/issues/13), [PBL3-15](https://github.com/Duy-OC/pbl3-project/issues/18), [PBL3-22](https://github.com/Duy-OC/pbl3-project/issues/25) |
| T0 | Kiểm thử thành phần cơ bản | Dựng lại harness nhỏ cho UDP gửi/nhận ký tự, parser MAVLink frame chia/gộp, payload byte 0 cuối, frame sửa payload gây CRC lỗi, và serialize giữ nguyên frame/signature. Ghi lệnh chạy và PASS/FAIL; test cũ đã xóa nên ca này chưa thực hiện trong phiên hiện tại. | [PBL3-02](https://github.com/Duy-OC/pbl3-project/issues/5), [PBL3-04](https://github.com/Duy-OC/pbl3-project/issues/7), [PBL3-05](https://github.com/Duy-OC/pbl3-project/issues/8) |
| B1 / E0 | Baseline Gateway, `urb=0` | 1 UAV + 1 client, 600 giây. Lưu ingress, egress, client, Markov summary, cấu hình, STATS, CPU/RAM/handle. Ingress = egress cùng cửa sổ; drop = 0; tổng MSGID khớp. CRC bằng 0. Ghi rõ client có cửa sổ riêng. | [PBL3-05](https://github.com/Duy-OC/pbl3-project/issues/8), [PBL3-06](https://github.com/Duy-OC/pbl3-project/issues/9), [PBL3-11](https://github.com/Duy-OC/pbl3-project/issues/14), [PBL3-12](https://github.com/Duy-OC/pbl3-project/issues/15), [PBL3-13](https://github.com/Duy-OC/pbl3-project/issues/16), [PBL3-14](https://github.com/Duy-OC/pbl3-project/issues/17) |
| B1b | QGC qua Gateway TCP | Sau run E0, chạy một ca riêng với QGC nối TCP `127.0.0.1:5760`; xác nhận UAV hiển thị, telemetry cập nhật và Gateway ghi client kết nối. Nếu thử command, ghi ACK và quan sát trong Gazebo; không trộn với run 1-client E0. | [PBL3-06](https://github.com/Duy-OC/pbl3-project/issues/9) |
| B2 | Nhiều client và client chậm | 1/5/10 client; 20 lần reconnect; client không đọc; thử kết nối thứ 11. Lưu log chấp nhận/từ chối/ngắt, frame client nền, pending, CPU/RAM/handle. Client thường tiếp tục nhận; sau dọn kết nối tài nguyên trở về gần mức nền. | [PBL3-07](https://github.com/Duy-OC/pbl3-project/issues/10), [PBL3-16](https://github.com/Duy-OC/pbl3-project/issues/19), E4/E5 của [PBL3-20](https://github.com/Duy-OC/pbl3-project/issues/23) |
| D1–D5 | Định tuyến và phục hồi 2 UAV | Hai SYSID/endpoint khác nhau; command/ACK riêng, video đúng UAV; ngắt UAV 2, thử BLOCK/no-route, giữ UAV 1; khởi động lại/đổi cổng; hủy queue khi route đổi hoặc client ngắt; thử queue đầy/sendto lỗi có kiểm soát. Mỗi nhánh phải có log thật, không suy từ counter bằng 0. | [PBL3-08](https://github.com/Duy-OC/pbl3-project/issues/11), [PBL3-09](https://github.com/Duy-OC/pbl3-project/issues/12), [PBL3-17](https://github.com/Duy-OC/pbl3-project/issues/20) |
| C1 | Kiểm mô hình mất sóng độc lập | I18_01 đã PASS cho G giữ hết/B bỏ hết, ba seed, chế độ OFF và độ dài đợt B. Kết quả này kiểm mã mô hình, chưa dùng lưu lượng PX4. | [PBL3-18](https://github.com/Duy-OC/pbl3-project/issues/21) |
| C2 | Mất sóng trên tuyến PX4 → Gateway | Proxy UDP đặt trước Gateway: U19_00 kiểm đường đi, U19_01 bình thường, U19_02 mất sóng theo đợt 2 giây, U19_03 đợt 5 giây. Ba run chính 600 giây, một seed cố định; so proxy nhận/bỏ/gửi và frame Gateway nhận. Gateway giữ `urb=0`. Xem [trình tự đơn giản](MARKOV_UAV_GATEWAY.md). | [PBL3-19](https://github.com/Duy-OC/pbl3-project/issues/22) |
| E1–E3 | Mở rộng sau phép đo mất sóng cơ bản | Chỉ làm sau U19_01–03 nếu cần so nhiều mức độ mất sóng hoặc độ dao động theo seed. Đổi tham số ở proxy trên tuyến UAV → Gateway, Gateway giữ `urb=0`; mỗi cấu hình dùng một run riêng 600 giây. Không đưa ma trận này vào bước đo hiện tại. | [PBL3-20](https://github.com/Duy-OC/pbl3-project/issues/23) |
| E4–E6 | Tải và độ bền hệ thống | E4: Markov OFF, 1/5/10 client, 600 giây/mức; E5: client chậm + thường 600 giây. Đây là kiểm tải Gateway riêng, không dùng để ước lượng mất gói Markov. E6: đặt Markov ở proxy UAV → Gateway với `urb=0.1,dur=2`, tải cố định 1800 giây; đối chiếu proxy trước/sau và Gateway ingress. | [PBL3-20](https://github.com/Duy-OC/pbl3-project/issues/23) |
| R | Phân tích và bàn giao | Kiểm CSV/schema/run; với Markov vẽ `proxy_pre`, `proxy_post`, Gateway ingress, G/B và drop theo seed/cấu hình. Tách biểu đồ client/tải của các Issue khác. Gắn mọi kết luận với raw run; README build/chạy, slide/video demo. | [PBL3-21](https://github.com/Duy-OC/pbl3-project/issues/24), [PBL3-22](https://github.com/Duy-OC/pbl3-project/issues/25) |

Các mở rộng M7 chỉ kiểm khi thực sự triển khai: [WebSocket](https://github.com/Duy-OC/pbl3-project/issues/26), [lệnh nhóm UAV](https://github.com/Duy-OC/pbl3-project/issues/27), [mất liên lạc hai chiều/failsafe](https://github.com/Duy-OC/pbl3-project/issues/28). Không gộp kết quả của chúng với Markov telemetry một chiều.

## 3. Tình trạng và bước tiếp theo

- A1 vẫn tạm hoãn theo yêu cầu người đo; các kết quả chức năng, baseline E0 và tải client đã có thư mục riêng trong `docs/ketqua/`.
- **PBL3-18:** [I18_01](ketqua/PBL3-18_Markov/I18_01/summary.md) PASS cho mô hình G giữ hết/B bỏ hết dùng để giả lập mất sóng.
- **PBL3-19:** [U19_00 đã PASS](ketqua/PBL3-19_GatewayMarkov/upstream/U19_00/summary.md) cho kết nối PX4 → proxy → Gateway; 10139 frame gửi/nhận khớp. [U19_01 đã PASS](ketqua/PBL3-19_GatewayMarkov/upstream/U19_01/summary.md) cho baseline 600 giây. [U19_02 đã PASS](ketqua/PBL3-19_GatewayMarkov/upstream/U19_02/summary.md) cho mất sóng ngắn. **Bước đang làm: U19_03** mất sóng dài; sau đó U19_02 mất sóng ngắn, U19_03 mất sóng dài. Mỗi run chính đo 600 giây sau 10 giây khởi động. Xem [hướng dẫn](MARKOV_UAV_GATEWAY.md).
- **PBL3-20:** E1–E3 và E6 dùng cùng proxy Markov trước Gateway. E4/E5 kiểm tải client của Gateway riêng, không đưa kết quả đó vào thống kê mất gói UAV → Gateway.

Mỗi lần chỉ người đo chạy một bước và gửi log; kiểm xong bằng chứng mới chuyển sang bước kế tiếp. U19_00 chỉ là preflight, chưa thay thế baseline 600 giây U19_01. Không gộp dữ liệu mô phỏng I18 với dữ liệu lưu lượng PX4 của U19.
