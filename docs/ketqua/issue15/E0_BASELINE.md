# PBL3 — Mốc E0: Gateway và QoS trước Markov

Ngày lập hồ sơ gốc: 06/11/2026.

Ngày sửa tài liệu: 07/11/2026. Đã sửa bảng Markdown bị vỡ trong docs.rar; đối chiếu lại bốn CSV E0 gốc. Không thay số liệu CSV.

## 1. Nhận diện mốc

- Tên mốc đề xuất: `e0-qos-baseline-v1`.
- Bản nguồn đối chiếu: `PBL3project(1).rar` do người dùng cung cấp.
- SHA-256 RAR: `abc1f9f5e52e70d1014a53b5e64a4ce79cb14b7d6b6e4bf0206f86ae07ff6513`.
- RAR là bản chụp đã nhận; các hash dưới đây nhận diện chính xác nội dung code/CSV trong bản đó.
- Hồ sơ này không có nghĩa đã commit hoặc tạo tag trên máy Windows/GitHub. Sau khi commit theo hướng dẫn, tag sẽ nhận diện bản mã nguồn được stage.
- Tại mốc E0 gốc chưa có emulator.h; telemetry của run E0 chưa chịu mô phỏng mất frame Markov. Issue 18 phát triển sau mốc này và không làm thay đổi dữ liệu E0.

## 2. Cấu hình xác nhận từ mã nguồn và CSV

| Thuộc tính | Giá trị |
|---|---|
| Nguồn UAV | 172.26.162.42:14600 |
| Đích UDP Gateway | 0.0.0.0:14550 |
| TCP Gateway | 127.0.0.1:5760 |
| UAV được ghi trong CSV | SYSID=1, COMPID=1; một nguồn UAV |
| Điểm đo | gateway_ingress và client_rx |
| Dialect | development, thư viện MAVLink đi kèm RAR |
| Warm-up | 10 giây từ frame hợp lệ đầu tiên |
| Khoảng đo | 600 giây; Gateway/client bắt đầu độc lập |
| Khoảng ghi CSV | Khoảng 5 giây |
| Timeout ingress | 5 giây |
| Timeout QoS client | 3 giây |
| Chu kỳ chờ select tối đa cấu hình | 200 ms |
| Giới hạn client | 10 |
| Pending TCP mỗi client | 262144 byte; vượt ngưỡng thì ngắt client |
| Pending UDP | 512 frame |
| Giới hạn nguồn UDP | 32 |
| Dọn parser UDP không hoạt động | Sau 30 giây |
| Bộ đo sequence | Cửa sổ 128 vị trí; resync sau khoảng im lặng >2 giây hoặc tình huống mơ hồ |
| Markov trong run E0 | Chưa triển khai |

## 3. Thông tin người thực hiện cần bổ sung

Không suy ra các mục này chỉ từ CSV:

- Tổng số client kết nối trong run (CSV chứng minh có một client đo, không cho biết mọi client khác): **chưa ghi nhận**.
- QGroundControl có chạy/kết nối không: **chưa ghi nhận**.
- UAV đứng yên hay bay; tác vụ thực hiện trong run: **chưa ghi nhận**.
- Lệnh mavlink start và cấu hình stream chính xác của run: **chưa lưu**.
- Phiên bản PX4/Gazebo/WSL/Windows và compiler g++ thực sự dùng cho run: **chưa lưu kèm run**.
- CPU/RAM trong run: **chưa đo**.
- Log STATS CRC/BLOCK của chính run này: **không có trong bộ CSV**.
- Commit chứa code dùng cho run: **chưa gắn**. Bản RAR là nguồn đối chiếu, không chứng minh tuyệt đối EXE được build từ bản đó.

## 4. Kết quả E0

| Chỉ số | Gateway ingress | Client RX |
|---|---:|---:|
| Thời lượng | 600 s | 600 s |
| Dòng CSV tổng hợp | 120 | 120 |
| Frames | 36416 | 36415 |
| Frame bytes | 1471592 | 1471444 |
| Trung bình frame/s | 60.693 | 60.692 |
| Heartbeat | 597 | 597 |
| Mean khoảng heartbeat cuối kỳ | 1005.727 ms | 1005.545 ms |
| Jitter heartbeat cuối kỳ | 7.109 ms | 7.127 ms |
| Max silence | 93.267 ms | 94.379 ms |
| Sequence gap / duplicate / reorder / resync / uncertain | Đều 0 | Đều 0 |
| Offline events / thời gian | 0 / 0 s | 0 / 0 s |

- File theo msgid đối soát khớp tổng frames và frame_bytes tại mọi thời điểm lấy mẫu.
- Không dùng chênh lệch một frame giữa hai cửa sổ độc lập để kết luận mất gói.
- Run này không có outage, nên không thay thế bài thử ngắt/kết nối lại UAV.
- Chưa có phép đo độ trễ một chiều; sequence gap chỉ là ước lượng, không phải ground truth mất gói mạng.

## 5. Danh sách file và SHA-256

Bốn hash CSV dưới đây đã được kiểm tra lại trực tiếp từ docs.rar và khớp hồ sơ gốc. Các hash mã nguồn/thư viện được giữ từ bản PBL3project(1).rar đã kiểm tra trước; docs.rar không chứa code để kiểm tra lại phiên bản hiện tại.

Hash tính trên byte gốc trong RAR; đổi CRLF/LF cũng làm hash thay đổi.

| File | SHA-256 |
|---|---|
| `src/client_manager.h` | `93a6f0599d7b1fae893052ad9d8e6a71bc69fd849c017e0c3f5b5033e7e57521` |
| `src/config.h` | `b1d6e01a8e65e5185a77cccc3427ef429ee1f1f83703a67d2e3adaf22ba6b2b7` |
| `src/gateway.cpp` | `a5eacf3be4040ec8c55ccf4ec28f16430aeb0ce6ff24d0817d0251e0772dc8b8` |
| `src/mavlink_codec.h` | `3a0775ce9230ddde8c654cdea502c569a3990554ca19a6aee2a63ecf6c910c5a` |
| `src/network.h` | `9751523c7bd3cfa1470ad3d0bf9ec42d02c83ec7ce85227bd44b11507ac59203` |
| `src/qos.h` | `f22f69bc5a768f3bfec5828f0ffd3972f18ab57ec1268910226f4855029f7e1b` |
| `src/sequence_tracker.h` | `efcf6f446b411bb1cd43daffc79c85b024bb75903822d10e5aafc27ef5132940` |
| `src/statistics.h` | `6b6257c33960ea4142157212ffd78b23b870328b4f9fc31e7044e6dccd8e36aa` |
| `src/tcpclient.cpp` | `a3d9bc2618e07dd585688c354abd6824dc9c66f95398928bd27756a2de9c2361` |
| `src/time_utils.h` | `16c8aea7bcce027d1cd10a0501d014efe81b84c14fea07f2f36b5ae5223fa493` |
| `src/uav_manager.h` | `3fc75084290f550e1e279bb8a3eb527ab574c6cd0c9c83f2d11cbec66e28a94b` |
| `docs/ketqua/issue14/E0/E0_client_rx_1793938128483.csv` | `0bf2dbca6dc3deb79f6ba620cc9d3b3cae9aea49e37561a86d54ce12f908d669` |
| `docs/ketqua/issue14/E0/E0_client_rx_1793938128483_msgid.csv` | `b5a43e7378cb251a51911bd742dd3ce7b8190c1183be3cd30077a460b42c6a8c` |
| `docs/ketqua/issue14/E0/E0_gateway_ingress_1793938099255.csv` | `85272b2f46468cc2f19cdf6cffefb3e3f2575cc86c2865919aa4822c6daf734f` |
| `docs/ketqua/issue14/E0/E0_gateway_ingress_1793938099255_msgid.csv` | `2b36a0087e0f12dd61b96f9394985012fc8a7b40b59108407565f2adc4465363` |

Thư viện MAVLink: 499 file thường, bỏ metadata Git. Hash cây theo thứ tự đường dẫn: `980643c2f23af41f55163d2342a5d21b4f1ba29647b7d2fb769c5545d66cba57`.
Cách tính: với từng file theo đường dẫn tương đối được sắp xếp, nối UTF-8(path) + byte NUL + SHA-256 nhị phân của nội dung; SHA-256 toàn chuỗi.

## 6. Phân biệt baseline với các bài thử client

- Chỉ bộ ingress `1793938099255` và client `1793938128483` trong mục 5 là baseline E0 đang được hồ sơ này mô tả.
- Bộ ingress `1794011910944` và các client `1794011939330`, `1794012053592`, `1794012180678` thuộc lần thử client. Tiền tố E0 trong tên do code đặt cố định, không làm chúng trở thành baseline mới.
- Không đưa CPU/RAM của bài reconnect 20 lần vào kết quả tài nguyên E0: đó là hai lần đo khác nhau.
- routing_recovery.log là bài nhiều UAV/ngắt kết nối; client_tests_gateway.log là bài fan-out/client chậm/giới hạn kết nối. Giữ riêng hai log.

## 7. Lưu vào dự án Windows

Đặt tài liệu này tại `docs/ketqua/issue15/E0_BASELINE.md`. Giữ bốn CSV gốc trong `docs/ketqua/issue14/E0/`.

Tại thư mục gốc dự án, stage đúng phần cần lưu:

```cmd
git add src docs/ketqua/issue15/E0_BASELINE.md
git add docs/ketqua/issue14/E0/E0_gateway_ingress_1793938099255.csv docs/ketqua/issue14/E0/E0_gateway_ingress_1793938099255_msgid.csv
git add docs/ketqua/issue14/E0/E0_client_rx_1793938128483.csv docs/ketqua/issue14/E0/E0_client_rx_1793938128483_msgid.csv
git diff --cached --stat
git diff --cached --name-only
```

Chỉ commit khi danh sách staged là phần muốn lưu (Git có thể đã stage file khác từ trước):

```cmd
git commit -m "Save E0 QoS baseline before Markov"
git tag -a e0-qos-baseline-v1 -m "E0: 600 s baseline with sequence and outage metrics"
git log -1 --oneline
git show --stat e0-qos-baseline-v1
```

Không dùng -f để thay tag đã tồn tại. Không cần push để có mốc local. Không commit EXE.

Nếu `third_party/mavlink` là repo lồng/submodule và có thay đổi, commit cha không tự lưu nội dung sửa bên trong. Giữ RAR gốc làm bản đối chiếu; kiểm tra và lưu thay đổi thư viện riêng trước khi coi tag là bản tái lập hoàn chỉnh.

## 8. Điều kiện hoàn tất mốc

- [x] Có bản RAR nguồn để đối chiếu.
- [x] Có bốn CSV E0 đủ 600 giây.
- [x] Có cấu hình suy ra được từ code và hash nhận diện.
- [ ] Người thực hiện bổ sung thông tin run còn thiếu ở mục 3.
- [ ] Lưu tài liệu vào docs/ketqua/issue15/E0_BASELINE.md trên Windows.
- [ ] Commit và tạo tag local trên đúng repo đang làm việc.
- [ ] Xác nhận trạng thái thư viện MAVLink để mốc Git có thể tái lập.

## 9. Phân loại hồ sơ sau khi sắp xếp

Bốn CSV ở `../issue14/E0/` là kết quả đo của issue 14. Tài liệu này và `bang_msgid.md` phục vụ chốt hồ sơ issue 15, dùng chung bộ CSV issue 14, không sao chép thành một run mới. Hash code/RAR và các mục chưa xác nhận trong hồ sơ gốc được giữ nguyên; gói docs hiện tại không chứa source/Git để xác minh lại. Log routing ở `../issue17/routing_recovery.log`; log client ở `../issue16/client_tests_gateway.log`. Việc sắp xếp tài liệu không xác nhận đã tạo tag Git.
