# Kế hoạch đo và nghiệm thu PBL3

Tài liệu này gộp `huong_dan_do (1).md` với tiêu chí trong GitHub Issues của `Duy-OC/pbl3-project`. **Theo dõi bước đang làm và thứ tự cố định ở `docs/TRINH_TU_KIEM_THU_THEO_ISSUE.md`**; phần E0 bên dưới chỉ là hướng dẫn chi tiết khi đến bước 14. Chỉ tạo kết quả khi người đo tự chạy lệnh; không dùng số tham chiếu cũ làm số liệu của run mới. Một run bị lỗi phải giữ nguyên và đánh dấu `FAIL`/`CHƯA ĐỦ BẰNG CHỨNG`.

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
| C1 | Markov độc lập, không PX4 | Kiểm `urb=0`, tham số sai, seed tái lập, lịch tick cố định, 3 seed × 180000 slot giả. Kiểm `seen=kept+dropped`, tỷ lệ B và độ dài đợt B. Test cũ đã được xóa; cần tạo lại harness trước ca này. | [PBL3-18](https://github.com/Duy-OC/pbl3-project/issues/21) |
| C2 | Ghép Markov vào Gateway | Chạy OFF, ON (`urb=0.5,dur=2`), OUTAGE (`urb=0.5,dur=5`), mỗi cấu hình giữ cùng PX4 stream; 600 giây/run, ON/OUTAGE dùng seed 1/2/3. Đối chiếu ingress/egress/summary và G/B; ingress vẫn online khi egress timeout ở OUTAGE; thêm client không đổi quyết định drop. | [PBL3-19](https://github.com/Duy-OC/pbl3-project/issues/22) |
| E1–E3 | Ma trận Markov | E1: `urb=0.1/0.5/0.6,dur=2,lossbad=1`; E2: `urb=0.5,dur=0.5/2/5,lossbad=1`; E3: `urb=0.5,dur=2,lossbad=0.5/1`. Mỗi cấu hình 3 seed, 600 giây/run. Cố định slot 100 ms, lossgood 0, cùng nguồn PX4. Có thể dùng lại ON run nếu điều kiện và phiên bản giống hệt, nhưng không tính trùng thành quan sát độc lập. | [PBL3-20](https://github.com/Duy-OC/pbl3-project/issues/23) |
| E4–E6 | Tải và độ bền | E4: Markov OFF, 1/5/10 client, 600 giây/mức; E5: client chậm + thường 600 giây; E6: `urb=0.1,dur=2`, tải cố định 1800 giây. Lưu tài nguyên, lỗi, trạng thái client và cấu hình. | [PBL3-20](https://github.com/Duy-OC/pbl3-project/issues/23) |
| R | Phân tích và bàn giao | Kiểm CSV/schema/run, ít nhất 4 biểu đồ ingress/egress/client, B/drop theo seed, outage/jitter và tải; báo cáo HTML nếu sử dụng; README build/chạy, slide/video demo; gắn mọi kết luận với raw run. | [PBL3-21](https://github.com/Duy-OC/pbl3-project/issues/24), [PBL3-22](https://github.com/Duy-OC/pbl3-project/issues/25) |

Các mở rộng M7 chỉ kiểm khi thực sự triển khai: [WebSocket](https://github.com/Duy-OC/pbl3-project/issues/26), [lệnh nhóm UAV](https://github.com/Duy-OC/pbl3-project/issues/27), [mất liên lạc hai chiều/failsafe](https://github.com/Duy-OC/pbl3-project/issues/28). Không gộp kết quả của chúng với Markov telemetry một chiều.

## 3. Tình trạng hiện tại

- A1: đã lưu ba mẫu `mavlink status` trong `docs/ketqua/PBL3-01_A1/A1_QGC_truc_tiep.md`; quan sát QGC 2 phút tạm hoãn theo yêu cầu người đo, chưa đánh dấu PASS.
- A2: đã lưu một run 60 giây và một run 600 giây tại `docs/ketqua/PBL3-03_A2/`. Run 600 giây có 34522 frame, 31 MSGID, 0 CRC lỗi. Đã xác nhận MAVLink instance #4 `14600 → 14550`. Người đo bỏ qua run A2 thứ hai.
- E0 run `E0_01` nằm trong `docs/ketqua/PBL3-14_E0/E0_01/`; CSV đủ 600 giây và ingress/egress đối soát đúng, nhưng file config trống và thiếu log/tài nguyên; xem `docs/ketqua/00_TONG_HOP/KIEM_KE_BANG_CHUNG_20261112.md`. Khi đến bước PBL3-14, chạy E0 mới bằng mã đã sửa, dùng run ID `E0_02`.
- T0 là bộ kiểm thử thành phần cần dựng lại để nghiệm thu toàn bộ code. Chương trình Gateway/client chỉ tạo CSV khi người đo tự khởi chạy.

## 4. Cách chạy B1/E0 tiếp theo

Thao tác trên Windows PowerShell tại thư mục gốc dự án, PX4 SITL vẫn dùng luồng `14600 → 14550`. Đóng QGC trực tiếp và chương trình đo A2 để giải phóng UDP 14550.

### 4.1 Chuẩn bị nguồn và build

Ghi vào sổ run: ngày giờ, phiên bản PX4, lệnh `mavlink start`/cấu hình stream, trạng thái UAV, `git rev-parse HEAD`, `git status --short`. Vì source hiện có thay đổi chưa commit, ghi thêm SHA-256 của `src/gateway.cpp`, `src/qos.h`, `src/emulator.h` và commit của submodule MAVLink. Không gán số E0 mới cho commit cũ nếu source đã sửa.

```powershell
git rev-parse HEAD
git status --short
git -C third_party/mavlink rev-parse HEAD
Get-FileHash src/gateway.cpp,src/qos.h,src/emulator.h -Algorithm SHA256
g++ -std=c++17 -O2 src/gateway.cpp -o gateway.exe -lws2_32
g++ -std=c++17 -O2 src/tcpclient.cpp -o tcpclient.exe -lws2_32
```

Nếu compiler báo lỗi, dừng để sửa build; không tính đó là run đo. Hai file nguồn đã qua kiểm tra cú pháp `g++ -std=c++17 -fsyntax-only`, chưa xác nhận liên kết/chạy thực tế trong kế hoạch này.

### 4.2 Chạy một run E0

Mở hai PowerShell riêng. Ở cửa sổ thứ nhất chạy Gateway (chọn run ID duy nhất):

```powershell
.\gateway.exe --scenario E0 --run-id E0_02 --urb 0 --seed 1
```

Ngay sau khi Gateway báo đang nghe cổng, ở cửa sổ thứ hai chạy client:

```powershell
.\tcpclient.exe
```

Đợi cả hai báo kết thúc 600 giây. Gateway bắt đầu 10 giây warm-up từ frame PX4 hợp lệ đầu tiên; client cũng warm-up riêng từ frame đầu tiên nó nhận. Giữ Gateway và client chạy thêm cho đến khi **cả hai** hoàn tất. Ghi `mavlink status` của instance `14600 → 14550` trong run. Lấy ít nhất hai mẫu `Get-Process gateway | Select-Object CPU,WorkingSet64,PrivateMemorySize64,HandleCount` cách nhau một khoảng thời gian đã ghi để tính CPU % một lõi.

Gateway hiện tự tạo CSV ingress, egress, state, summary và config trong `docs/ketqua/issue19/` với tiền tố `E0_E0_02`; client tạo CSV riêng trong `docs/ketqua/` và in tên file ra console. Sau khi cả hai kết thúc và đã kiểm tra file, chuyển toàn bộ file của run vào `docs/ketqua/PBL3-14_E0/E0_02/` như run `E0_01` hiện có. **Không đổi run ID giữa chừng hoặc ghi đè file cũ.**

### 4.3 Kiểm tra sau run

1. Tìm `E0_E0_02_markov.csv`: `frames_in = frames_kept`, `frames_dropped_emu = 0`, `ingress_qos_frames = egress_qos_frames` cho cùng SYSID. Kiểm `_config.txt` không trống ngay khi Gateway bắt đầu.
2. Trong CSV ingress/egress, dòng cuối có `t_s=600` (cho phép sai số định dạng), tổng `frames`/`frame_bytes` và từng MSGID khớp giữa hai điểm. Không lấy hiệu số client RX làm mất gói vì cửa sổ client khác.
3. Client có dòng cuối 600 giây, CRC bằng 0 ở console. Kiểm số HEARTBEAT, jitter, outage và sequence; ghi hiện tượng bất thường, không tự sửa dữ liệu thô.
4. Lưu STATS Gateway, tham số, source hash, mẫu CPU/RAM/handle và nhận xét `PASS`/`FAIL`/`CHƯA ĐỦ BẰNG CHỨNG`.

Sau khi E0 hợp lệ, tiến hành B2 và D1–D5, rồi C1/C2 và ma trận E1–E6. Không chạy hàng loạt trước khi một run ON ghép thật đã được đối soát.
