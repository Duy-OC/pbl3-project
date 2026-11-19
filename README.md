# PBL3 — MAVLink Gateway cho UAV PX4

Project C++17 trên Windows/Winsock, nhận MAVLink từ UAV qua UDP và phục vụ nhiều TCP client. Gateway kiểm tra CRC, quản lý đường về UAV, chuyển tiếp telemetry, đo QoS và hỗ trợ điều khiển có khóa quyền.

## Kiến trúc và cấu hình

```text
PX4 SITL/UAV <-- UDP 14550 --> Gateway <-- TCP 5760 --> TCP client / QGroundControl
                                  |
                            CSV, trace, thống kê
```

- MAVLink dialect `development`, hỗ trợ frame v1/v2; parser riêng cho từng nguồn UDP và từng TCP client.
- Tối đa 10 client, 32 nguồn UDP; hàng đợi TCP 256 KiB/client, hàng đợi UDP 512 frame.
- Timeout định tuyến của project hiện tại: 5 giây. Dữ liệu hợp lệ tiếp tục duy trì route; dọn hàng đợi khi route đổi, UAV offline hoặc client đóng kết nối.
- Markov tích hợp trong gateway được giữ lại: mỗi UAV có trạng thái tốt/xấu theo slot, seed và xác suất mất riêng theo trạng thái. Mặc định `urb=0`, không bật nhiễu.
- Bộ đo ingress/egress khởi động sau 10 giây warm-up kể từ frame đầu tiên và đo 600 giây. Không kết thúc chương trình tự động; chờ ghi đủ kết quả rồi dừng bằng Ctrl+C.

## Tính năng hiện có

- Nhận UDP, xác minh CRC, phục hồi parser sau dữ liệu rác/frame lỗi; xử lý nhiều frame trong datagram và frame TCP bị chia nhỏ.
- Phát telemetry nhị phân cho nhiều client; gửi không chặn, kiểm soát hàng đợi và client chậm.
- Định tuyến lệnh theo UAV đích và thế hệ route, tránh gửi nhầm khi UAV đổi địa chỉ/kết nối lại.
- Thống kê lưu lượng, lỗi CRC, sequence gap, trùng/đảo thứ tự, mất cụm, jitter ATTITUDE, thời gian offline. Có CSV theo stream/msgid, burst, mất theo giây và trace.
- Giải mã HEARTBEAT, SYS_STATUS, GPS_RAW_INT, ATTITUDE, GLOBAL_POSITION_INT, VFR_HUD: mode/armed, pin, GPS, vị trí, độ cao, vận tốc, góc và throttle.
- Giao thức TCP văn bản lấy chi tiết JSON, đăng ký cập nhật 1–5 Hz, khóa quyền điều khiển duy nhất.
- ARM, DISARM, TAKEOFF, LAND qua giao thức văn bản; ghép COMMAND_ACK, gửi tối đa 3 lần cách nhau 1 giây, báo kết quả/timeout/offline. Hủy lệnh chờ và nhả khóa khi chủ sở hữu ngắt kết nối.
- QGroundControl/MAVLink TCP: nhận telemetry, chuyển tiếp các yêu cầu đọc được cho phép, đổi mode, COMMAND_LONG/INT, MANUAL_CONTROL và Reposition (`MAV_CMD_DO_REPOSITION`, 192). Client MAVLink lấy khóa khi gửi điều khiển đầu tiên; client khác bị chặn. Gateway phát heartbeat GCS 1 Hz.

Joystick và Reposition dùng chung kết nối QGC và khóa quyền, định tuyến theo SYSID; Reposition qua COMMAND_INT giữ nguyên tọa độ, độ cao và frame, ACK từ UAV được chuyển về QGC. Kiểm thử `test_qgc_control.py` xác nhận cả hai lệnh trên hai UAV giả, không gửi chéo và chặn Reposition có target=0 hoặc client khác quyền. Chưa xác nhận thực thi Reposition trên PX4 trong phiên kiểm thử này.

Sau khi build, khởi động lại gateway để dùng thay đổi. Trong QGC có thể dùng joystick rồi chọn thao tác Go to/Reposition trên bản đồ. QGC chỉ gửi joystick tới UAV được chọn; thay đổi gateway không giữ tín hiệu joystick cho UAV vừa bỏ chọn và không thay cấu hình failsafe PX4.

Gateway hỗ trợ `--tcp-port PORT` để chạy kiểm thử ở cổng riêng; mặc định vẫn là 5760. Các bài QGC tự chọn cổng TCP riêng để tránh gửi lệnh thử qua gateway đang vận hành.
- Công cụ phát UAV giả, probe PX4, client điều khiển và cầu joystick UDP→TCP.

## Build và chạy

Yêu cầu Windows, MinGW `g++` trong PATH; Python 3 và `pymavlink` cho các công cụ/kiểm thử Python.

```powershell
.\build.bat
python -m pip install pymavlink
.\bin\gateway.exe --scenario DEMO --run-id demo_01
```

Build tạo gateway, tcpclient, ba bộ test C++ và công cụ kiểm tra Markov/parser/integrity trong `bin/`. Build không lấy proxy từ file nén. Chọn run ID mới mỗi lần để tránh ghi đè bằng chứng.

Một terminal khác có thể phát dữ liệu giả trực tiếp:

```powershell
python scripts/simulate_px4.py 127.0.0.1 14550 50
.\bin\tcpclient.exe 127.0.0.1 5760
# Client ?i?u khi?n v?n b?n (t?y ch?n):
python scripts/control_client.py 127.0.0.1 5760
```

Hoặc cấu hình PX4 gửi UDP tới IP máy Windows, cổng 14550; trong QGroundControl tạo kết nối TCP tới IP gateway, cổng 5760. Các kiểm thử tự động sử dụng UAV giả; chưa xác nhận bay thật/joystick trên PX4 trong phiên tích hợp này.

Tham số gateway (đều theo cặp tên–giá trị):

| Tham số | Ý nghĩa |
|---|---|
| `--scenario`, `--run-id` | Nhãn dữ liệu, chỉ dùng chữ/số/gạch dưới/gạch ngang |
| `--udp-port` | Cổng UDP, 1–65535; mặc định 14550 |
| `--urb`, `--dur-s`, `--slot-ms` | Tỉ lệ trạng thái xấu, thời gian xấu trung bình, độ dài slot |
| `--lossgood`, `--lossbad`, `--seed` | Xác suất mất trong trạng thái tốt/xấu và seed |
| `--test-hold-udp-ms`, `--test-sendto-invalid-addrlen-once` | Chèn tình huống hàng đợi/lỗi sendto để kiểm thử |

Chi tiết Markov và cách đo hiện tại: [docs/MARKOV_UAV_GATEWAY.md](docs/MARKOV_UAV_GATEWAY.md). Gateway hiện tại giữ cơ chế đo Issue19; các script đo K của bản nén chưa được chuyển thành workflow chạy cho phiên bản này.

## Giao thức văn bản TCP

Mỗi yêu cầu kết thúc bằng ký tự xuống dòng. Dùng `scripts/control_client.py` để nhập:

| Yêu cầu | Hành vi |
|---|---|
| `GET_DETAIL [sysid]` | Trả `DETAIL` kèm JSON hoặc `DETAIL NONE` |
| `SUBSCRIBE 1..5`, `UNSUBSCRIBE` | Bật/tắt cập nhật định kỳ |
| `ACQUIRE`, `RELEASE` | Lấy/nhả quyền điều khiển |
| `COMMAND 1 ARM`, `COMMAND 1 DISARM` | Arm/disarm UAV 1 |
| `COMMAND 1 TAKEOFF 10`, `COMMAND 1 LAND` | Cất cánh ở độ cao yêu cầu/hạ cánh; độ cao 1–100 m |

Lệnh hợp lệ trả `OK COMMAND_PENDING`, sau đó `RESULT`; các lỗi bao gồm `ERR LOCKED`, `ERR NOT_OWNER`, `ERR UAV_OFFLINE`, `ERR COMMAND_BUSY`. Retry áp dụng cho lệnh văn bản; lệnh MAVLink thô được chuyển tiếp theo chính sách và khóa quyền.

## Kiểm thử và kết quả

```powershell
.\build.bat tests
.\bin\test_stage1_framing.exe
.\bin\test_stage2_telemetry.exe
.\bin\test_sequence_cases.exe docs/ketqua/tich_hop_projectltm/sequence.csv
python -m unittest discover -s tests -p test_client_dashboard.py -v
python -m unittest discover -s tests -p test_control_protocol.py -v
python -m unittest discover -s tests -p test_qgc_control.py -v
```

Dừng gateway đang chạy trước khi kiểm thử tích hợp để tránh chiếm cổng. Các test Python tự mở và đóng gateway.

- [Kết quả kiểm thử bản tích hợp](docs/ketqua/tich_hop_projectltm/README.md): log build, parser/CRC, telemetry, sequence, điều khiển và QGC.
- [Bằng chứng nhập từ projectltm.rar](docs/ketqua/projectltm_import/README.md): giữ riêng nguồn và điều kiện đo, không gán các số liệu PX4/proxy cũ cho bản tích hợp.
- `docs/ketqua/PBL3-*`: bằng chứng kiểm thử sẵn có theo issue; giữ nguyên các chỉnh sửa đang có trước lần tích hợp này.
- `docs/ketqua/issue19`: file đo do gateway sinh khi chạy, gồm cấu hình, trạng thái Markov, QoS và trace tùy chọn.

## Cấu trúc mã và tài liệu

| Đường dẫn | Nội dung |
|---|---|
| `src/gateway/gateway.cpp` | Vòng lặp Winsock, định tuyến, khóa và ACK/retry |
| `src/gateway/client_manager.h`, `src/client/tcpclient.cpp` | TCP client, framing, buffer và chương trình client nhị phân |
| `src/gateway/uav_manager.h` | Route và telemetry chi tiết |
| `src/common/mavlink_codec.h`, `src/common/network.h` | MAVLink và tiện ích socket |
| `src/common/qos.h`, `src/common/sequence_tracker.h`, `src/gateway/statistics.h` | Đo QoS, sequence và thống kê |
| `src/common/emulator.h` | Mô hình Markov tích hợp được giữ từ project hiện tại |
| `tools/` | Probe UDP, route, queue, tải nhiều client, parser/CRC và Markov |
| `tests/`, `scripts/` | Kiểm thử và tiện ích mới từ bản nén đã thích nghi |
| `third_party/mavlink/` | Bộ header MAVLink |
| `docs/` | Hướng dẫn đo, kế hoạch nghiệm thu, ghi log và bằng chứng |

`src/proxy/upstream_markov_proxy.cpp` và công cụ đối chiếu upstream đã tồn tại trước lần tích hợp, được giữ nguyên. Không nhập hay thay thế phần proxy bằng mã từ bản nén.
