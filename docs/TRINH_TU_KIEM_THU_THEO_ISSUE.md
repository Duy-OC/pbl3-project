# Trình tự kiểm thử PBL3 theo Issue

> Ghi chú: Các lệnh và số liệu đo dưới đây thuộc phiên bản lịch sử. Gateway hiện tại chỉ in thống kê trên console; xem README để build và chạy.
Đây là **danh sách điều hành** cho lần kiểm thử lại từ đầu. Đi từ PBL3-01 đến PBL3-22; PBL3-23 đến PBL3-25 là mở rộng tùy chọn chưa triển khai. Mỗi bước trả lời một câu hỏi và chỉ đánh dấu `ĐẠT` sau khi có bằng chứng của **phiên hiện tại**. Issue GitHub đã đóng là trạng thái lịch sử, không tự thay cho lần kiểm thử mới.

## Thư mục kết quả

- `docs/ketqua/PBL3-01_A1/`: ba mẫu A1 đã ghi; còn thiếu quan sát QGC.
- `docs/ketqua/PBL3-02_UDP/`: output UDP echo và nhận xét giới hạn.
- `docs/ketqua/PBL3-03_A2/`: khảo sát 60 s và run chính 600 s của A2.
- `docs/ketqua/PBL3-04_Parser/`: output parser frame chia/gộp.
- `docs/ketqua/PBL3-05_Integrity/`: hai lần thử integrity; lần đầu helper sai giả định, lần hai PASS.
- `docs/ketqua/PBL3-06_TCP/I06_01/`: quan sát QGC qua TCP và sáu file CSV/config của ca này, kết thúc tại `t=376.286 s`.
- `docs/ketqua/PBL3-07_Fanout/I07_01/`: ba log terminal, mười file thô và `summary.md` của ca fan-out.
- `docs/ketqua/PBL3-14_E0/E0_01/`: chín file thô E0 hiện có, gồm Gateway và client. Run mới sẽ có thư mục riêng `E0_02/` sau khi đo.
- `docs/ketqua/00_TONG_HOP/`: kiểm kê bằng chứng chung. Khi có kết quả PBL3-02, lưu vào `docs/ketqua/PBL3-02_UDP/`.

Giữ một thư mục cho mỗi Issue và một thư mục con cho mỗi run khi có nhiều run. **Mỗi run lưu số liệu thô, log terminal, lệnh/cấu hình và kết luận PASS/FAIL**; chỉ ghi PASS/FAIL là chưa đủ hồ sơ. Cách vừa xem terminal vừa ghi log nằm ở `docs/GHI_LOG_KIEM_THU.md`. CSV do chương trình tạo ra ở đường dẫn cũ sẽ được chuyển vào đúng thư mục **sau khi chương trình đã kết thúc**; không di chuyển file đang được ghi.

## Thứ tự và mục đích

| Thứ tự | Issue / câu hỏi cần trả lời | Cách kiểm và bằng chứng cần giữ | Tình trạng phiên hiện tại |
|---|---|---|---|
| 01 | [PBL3-01](https://github.com/Duy-OC/pbl3-project/issues/4): PX4, Gazebo, QGC trực tiếp có hoạt động? | Quan sát UAV trong QGC 2 phút, ảnh, lệnh PX4, ba `mavlink status`. | **TẠM HOÃN**: đã có ba status, thiếu quan sát QGC. |
| 02 | [PBL3-02](https://github.com/Duy-OC/pbl3-project/issues/5): WinSock UDP có gửi, nhận và trả lại đúng dữ liệu? | Chạy `udp_echo_check` trên loopback, thấy `UDP ECHO PASS`; lưu lệnh build/chạy và output. | **PASS + METRICS**: `PBL3-02_UDP/metrics_02.log`, 14 byte gửi/nhận/trả lại, exit 0. |
| 03 | [PBL3-03](https://github.com/Duy-OC/pbl3-project/issues/6): PX4 WSL có gửi tới Windows UDP 14550? | Đối chiếu IP/cổng của A2 với `mavlink status`; Gateway nhận đúng nguồn. | **PASS cho đường nhận UDP**: A2 `172.26.162.42:14600`, PX4 instance #4 `14600 → 14550`, E0 ingress cùng nguồn. Metadata stream/phiên bản sẽ bổ sung ở bước 15. |
| 04 | [PBL3-04](https://github.com/Duy-OC/pbl3-project/issues/7): parser có nhận đúng MAVLink frame/SYSID/COMPID/MSGID, kể cả frame chia/gộp? | Tổng MSGID khớp frame, thêm test parser byte stream chia/gộp. | **PASS + METRICS**: `PBL3-04_Parser/metrics_02.log`, 21+21 byte, chia tại byte 10, ghép 42 byte nhận 2 frame, exit 0. |
| 05 | [PBL3-05](https://github.com/Duy-OC/pbl3-project/issues/8): Gateway có giữ nguyên frame và phát hiện CRC hỏng? | So ingress/egress OFF; test payload byte 0 cuối, frame bị sửa, signature. | **PASS + METRICS**: `PBL3-05_Integrity/metrics_03.log`, 1 frame hợp lệ nhận, 1 frame hỏng CRC bị loại, 13 byte signature giữ nguyên, exit 0; chưa xác thực chữ ký bằng khóa. |
| 06 | [PBL3-06](https://github.com/Duy-OC/pbl3-project/issues/9): TCP client và QGC có nhận telemetry qua Gateway? | Client nhận HEARTBEAT/CSV; QGC nối TCP `127.0.0.1:5760` hiện UAV 2 phút. | **PASS**: `PBL3-06_TCP/I06_01/observation.md`, QGC có độ cao/tốc độ, Clients=1, TCP TX tăng, CRC=0 quá 135 s; client console E0 có CSV 600 s. |
| 07 | [PBL3-07](https://github.com/Duy-OC/pbl3-project/issues/10): một frame có phát cho nhiều client độc lập? | Mở 2–3 client cùng lúc, đối chiếu HEARTBEAT; đóng một client, còn lại tiếp tục. | **PASS**: `PBL3-07_Fanout/I07_01/summary.md`, Clients 3→2, client còn lại tiếp tục nhận; chưa thử stress PBL3-16. |
| 08 | [PBL3-08](https://github.com/Duy-OC/pbl3-project/issues/11): lệnh có tới đúng UAV đích? | Hai SYSID; gửi riêng lệnh có ACK tới từng UAV; log route và quan sát Gazebo. | **PASS**: I08_01 xác nhận hai endpoint/QGC; thao tác riêng Arm/Disarm có 4 ROUTE/ACK đúng UAV và người đo xác nhận bình thường (`PBL3-08_Route/I08_targeted_from_chat.md`). Chưa lưu video/log đầy đủ của lượt thao tác. |
| 09 | [PBL3-09](https://github.com/Duy-OC/pbl3-project/issues/12): route có offline, phục hồi, đổi endpoint đúng? | Ngắt UAV 2, giữ UAV 1; khởi động lại/đổi cổng; log OFFLINE→ONLINE→ROUTE/ACK. | **PASS**: I09_01 OFFLINE/BLOCK và UAV 1 vẫn ACK; I09_02 OFFLINE→ONLINE ở 14602, ROUTE/ACK cổng mới (`PBL3-09_Recovery/I09_01/summary.md`, `I09_02/summary.md`). Queue cũ thuộc PBL3-17. |
| 10 | [PBL3-10](https://github.com/Duy-OC/pbl3-project/issues/13): code chia module có build được từ source? | Build Gateway và client riêng bằng `g++`, lưu lệnh, exit code, phiên bản compiler. | **PASS**: `PBL3-10_Build/I10_01/build.log` và `summary.md`, hai exit 0, kích thước/hash đối chiếu khớp; working tree chưa commit sẽ được chốt ở bước 15. |
| 11 | [PBL3-11](https://github.com/Duy-OC/pbl3-project/issues/14): STATS có tách lỗi dữ liệu, route và hàng đợi? | Lưu log STATS thực; kiểm UDP/TCP CRC, BLOCK, dropped/cancelled, client chậm là các bộ đếm riêng. | **PASS**: [tổng hợp](ketqua/PBL3-11_Stats/summary.md) từ I11_01 CRC, I09_01 no-route, I16_02 slow client và các lượt I17 queue/sendto; mỗi nguyên nhân có log riêng. |
| 12 | [PBL3-12](https://github.com/Duy-OC/pbl3-project/issues/15): QoS ingress/client có ghi đủ 600 s và MSGID? | CSV 120 mốc đến 600 s; tổng MSGID = tổng frame/byte ở mỗi điểm. | **PASS**: kiểm toán `PBL3-12_QoS/summary.md` từ E0_01; 120 mẫu/điểm tới 600 s, 0 mẫu lệch tổng MSGID frame/byte. |
| 13 | [PBL3-13](https://github.com/Duy-OC/pbl3-project/issues/16): sequence/outage có ghi đúng khi mất heartbeat? | Kiểm cột CSV; tạo mất heartbeat có kiểm soát, xem OFFLINE/ONLINE, timeout 5/3 s. | **PASS**: `PBL3-13_Heartbeat/I13_01/summary.md`, ingress/egress/client mỗi điểm 1 offline event và ONLINE lại; timeout 5/3/3 s, seq resync=1 do restart. |
| 14 | [PBL3-14](https://github.com/Duy-OC/pbl3-project/issues/17): baseline E0 600 s có nhất quán? | 1 UAV + 1 client, `urb=0`, ingress=egress, drop=0; lưu config, CSV, STATS, tài nguyên. | **PASS**: `PBL3-14_E0/E0_02/summary.md`, 34074 frame/1376377 byte ingress=egress, 0 drop, 120 mốc, log/config/tài nguyên đầy đủ. |
| 15 | [PBL3-15](https://github.com/Duy-OC/pbl3-project/issues/18): có tái lập đúng source và điều kiện đo? | Commit/hash source, submodule MAVLink, phiên bản PX4, lệnh stream, trạng thái UAV, số client, tên run. | **CÓ HỒ SƠ, CÒN GIỚI HẠN**: `E0_02/reproducibility.md`, 12 hash source, MAVLink/PX4 commit, lệnh stream/launch, một UAV đứng trên mặt đất, một client; Armed/Disarmed không được ghi trong run. |
| 16 | [PBL3-16](https://github.com/Duy-OC/pbl3-project/issues/19): nhiều client, reconnect, client chậm có ổn? | 20 reconnect; client không đọc; mức 1/5/10 và kết nối thứ 11; log, CPU/RAM/handle. | **PASS**: `PBL3-16_Clients/summary.md`, I16_01–06 đủ reconnect/slow/cap và tải 1/5/10 trong 600 s/mức. |
| 17 | [PBL3-17](https://github.com/Duy-OC/pbl3-project/issues/20): route offline có BLOCK và hủy queue đúng? | D1–D5: no-route/BLOCK, queue bị hủy khi route đổi/client ngắt, queue đầy/sendto lỗi; log từng nhánh. | **PASS**: [tổng hợp](ketqua/PBL3-17_Queue/summary.md) từ I09_01/02 và I17_01/02C/03/04/05; lệnh cũ bị hủy trước khi endpoint 14602 ONLINE, UDP TX vẫn 0. |
| 21 | [PBL3-21](https://github.com/Duy-OC/pbl3-project/issues/24): số liệu có được phân tích và vẽ lại? | Script đọc raw CSV, ≥4 biểu đồ; ghi đơn vị, mẫu, seed, phân tán; không suy latency một chiều. | **CHƯA LÀM**. |
| 22 | [PBL3-22](https://github.com/Duy-OC/pbl3-project/issues/25): người khác có build/chạy và kiểm chứng được? | README đầy đủ, bảng run ↔ raw file, kết luận/giới hạn, slide, video demo 3–5 phút. | **CHƯA ĐỦ**: README còn rất ngắn. |

**Mở rộng tùy chọn, sau bước 22 nếu đề cương yêu cầu:** [PBL3-23 WebSocket](https://github.com/Duy-OC/pbl3-project/issues/26), [PBL3-24 lệnh nhóm](https://github.com/Duy-OC/pbl3-project/issues/27), [PBL3-25 mất liên lạc hai chiều/failsafe](https://github.com/Duy-OC/pbl3-project/issues/28). Chưa triển khai nên chưa có phép đo hợp lệ.

## Quy tắc chuyển bước

- Mỗi lần chỉ làm **một bước đang làm**; lưu bằng chứng rồi mới cập nhật trạng thái và chuyển sang số kế tiếp. Bước đã có dữ liệu thì kiểm phần còn thiếu; lần chạy lại PBL3-02/04/05 ở đây nhằm bổ sung METRICS theo yêu cầu người đo, không xoá kết quả cũ.
- `CHƯA THỬ LẠI` không có nghĩa tính năng chắc chắn hỏng. `CÓ BẰNG CHỨNG` không có nghĩa toàn Issue đã nghiệm thu nếu cột bằng chứng còn thiếu.
- A1 vẫn tạm hoãn theo yêu cầu; khi muốn quay lại, hoàn thành quan sát QGC trước khi đánh dấu bước 01 đạt.

## Bước đã hoàn thành: PBL3-08

**Mục đích:** xác nhận Gateway học hai UAV có SYSID/endpoint riêng và chuyển lệnh đúng đích. Giai đoạn đầu chỉ kiểm hai nguồn online; chưa gửi Takeoff/Land.

1. Giữ PX4/Gazebo đầu tiên đã dùng ở A2/E0 (UAV 1, UDP source 14600) và mở PX4 SITL thứ hai với instance khác. Trong `pxh>` của UAV 2, kiểm SYSID=2 và cấu hình một MAVLink UDP local port 14601 tới Windows UDP 14550. Ghi lệnh và `mavlink status` của cả hai. Nếu IP Windows từ WSL đã đổi, dùng IP mới thay vì số cũ.
2. Tạo thư mục run `docs/ketqua/PBL3-08_Route/I08_01/`, khởi động Gateway `--scenario I08_ROUTE --run-id I08_01` qua `Tee-Object` để lưu `gateway.log`. Kết nối QGC TCP `127.0.0.1:5760`.
3. Kiểm Gateway STATS có `UAV online=2`, hai dòng UAV 1/2 và QGC thấy hai UAV. Gửi đoạn log đó; giữ run đang chạy để bước sau kiểm lệnh riêng từng UAV. Chưa phát lệnh bay ở giai đoạn chuẩn bị này.

## Bước đã hoàn thành: PBL3-09

**Mục đích:** kiểm Gateway đánh dấu riêng UAV 2 offline sau khi mất HEARTBEAT, chặn lệnh tới UAV 2, vẫn phục vụ UAV 1, rồi học lại UAV 2 ở endpoint mới.

I09_01 đã PASS phần offline/BLOCK. Bước cần làm tiếp là I09_02: cho Gateway học UAV 2 ở `14601`, ngắt tới OFFLINE, rồi trong cùng tiến trình Gateway cho UAV 2 trở lại ở `14602` và dùng `route_probe.exe 2` kiểm ACK/ROUTE tới cổng mới. Lệnh chi tiết ở [hướng dẫn log](GHI_LOG_KIEM_THU.md).

1. **I09_01 đã xong:** Gateway ghi `UAV 2 OFFLINE`, probe 2 bị BLOCK/no-route và probe 1 nhận ACK. Bằng chứng tại `PBL3-09_Recovery/I09_01/summary.md`.
2. **I09_02 đã xong:** trong cùng phiên Gateway, UAV 2 OFFLINE rồi ONLINE ở 14602; probe 2 nhận ACK và Gateway ROUTE tới cổng mới. Xem `PBL3-09_Recovery/I09_02/summary.md`.

## Bước đã hoàn thành: PBL3-10

**Mục đích:** xác nhận một người có thể build riêng Gateway và TCP client từ source đang có, lưu lệnh build, compiler, exit code và dấu vết hai executable. Xem lệnh trong [hướng dẫn log](GHI_LOG_KIEM_THU.md). Bước này không cần PX4/Gazebo/QGC chạy.

## Phần đã đo: PBL3-11

**Mục đích:** kiểm STATS đếm riêng CRC hỏng từ UDP và TCP, tách với BLOCK/no-route, queue bị hủy và client chậm. I11_01 đã chứng minh CRC UDP/TCP tách riêng; I09_01 có no-route. Nhánh queue và client chậm sẽ được kiểm ở PBL3-17/16 trước khi chốt toàn Issue.

## Bước đã hoàn thành: PBL3-12

E0_01 có 120 mẫu QoS/điểm đến 600 s; mọi mẫu có tổng MSGID khớp tổng frame/byte. Xem `PBL3-12_QoS/summary.md`.

## Bước đã hoàn thành: PBL3-13

**Mục đích:** kiểm QoS ghi offline/online và sequence gap khi ngắt nguồn có kiểm soát. I13_01 đã có đủ Gateway ingress/egress và TCP client; xem `PBL3-13_Heartbeat/I13_01/summary.md`.

## Bước đã hoàn thành: PBL3-14

**Mục đích:** đo E0 mới 600 s với đúng một UAV, một TCP client, `urb=0`, lưu config, log, CSV và tài nguyên. E0_02 đã PASS; E0_01 có CSV đủ 600 s nhưng config trống và thiếu log/tài nguyên. Xem `PBL3-14_E0/E0_02/summary.md`.

## Phần đã lưu: PBL3-15

**Mục đích:** gắn baseline E0_02 với đúng source/PX4/stream đã dùng để người khác tái lập. `E0_02/reproducibility.md` và `px4_environment.md` đã lưu các thông tin người đo cung cấp; Armed/Disarmed trong run chưa có bằng chứng.

## Bước đã hoàn thành: PBL3-16

**Mục đích:** kiểm Gateway chịu được reconnect nhiều lần, client chậm và tới giới hạn 10 client. I16_01–06 đã PASS; xem `PBL3-16_Clients/summary.md`.

## Bước đã hoàn thành: PBL3-17

**Kết quả:** [PBL3-17 PASS](ketqua/PBL3-17_Queue/summary.md), gồm BLOCK/no-route, phục hồi endpoint, hủy queue vì client ngắt/route hết hạn, giới hạn 512, lỗi `sendto` và việc request cũ không sang endpoint mới. Hai lượt I17_02 và I17_02B không đạt điều kiện đã được xóa theo yêu cầu.
