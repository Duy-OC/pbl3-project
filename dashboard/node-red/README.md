# Theo dõi UAV PX4 trong Gazebo

> Proxy no longer publishes UDP JSON. This flow and the UDP setup below are retained for reference.

## 1. Build proxy trên Windows

Chạy từ thư mục gốc dự án. Script tìm GCC 7 trở lên trong PATH để dùng thư viện chuẩn C++17. Trên máy hiện tại, script bỏ qua MinGW 6.3 và chọn MinGW-w64 13.2.

```bat
scripts\build_udp_proxy.bat
```

Nếu gateway chưa được build, chạy `build.bat` trong CMD.

## 2. Chạy gateway và proxy

Mở hai terminal Windows tại thư mục gốc. Script build liên kết tĩnh thư viện runtime MinGW của proxy.

```bat
bin\gateway.exe --udp-port 14550
```

```bat
bin\udp_proxy.exe --config config\proxy.json 14600 127.0.0.1 14550
```

Lệnh cũ `bin\udp_proxy.exe 14600 127.0.0.1 14550` vẫn chạy bằng mặc định tích hợp. Khi dùng `--config`, cú pháp đầy đủ là `udp_proxy.exe [--config FILE] [listen_port [gateway_ipv4 [gateway_port [bind_ipv4]]]]`. Các giá trị positional được áp dụng sau cấu hình. Ví dụ trên ghi đè cổng Gateway A `14560` trong JSON thành `14550` để khớp gateway vừa chạy. `--config` yêu cầu đường dẫn file nếu được dùng. Mọi trường cấu hình đều có mặc định hoặc có thể bỏ qua.

| Phần | Các trường đọc | Mặc định tích hợp |
| --- | --- | --- |
| `px4` | `listen_address`, `listen_port`, `expected_sysid` | `0.0.0.0`, `14600`, `1` |
| Phần tử `gateways` có `id: "A"` | `address`, `port` | `127.0.0.1`, `14550` |
| `position` | `stale_timeout_ms`, `transform` | `1000` ms, chưa hiệu chỉnh |
| `metrics` | `enabled`, `destination_address`, `destination_port`, `interval_ms` | `true`, `127.0.0.1`, `9871`, `100` ms |

Dùng địa chỉ IPv4 dạng số. Chu kỳ metrics nhận giá trị nguyên từ 100 đến 60.000 ms. Timeout vị trí nhận giá trị nguyên từ 1 đến 600.000 ms. Bảng trên liệt kê toàn bộ trường được áp dụng ở bước này. Các trường đường dẫn trong JSON hiện thuộc phần tính năng chưa dùng. Đường dẫn truyền cho `--config` tính từ thư mục chạy lệnh. Các đường dẫn tương đối bên trong JSON, khi được dùng, phải tính từ thư mục chứa file cấu hình.

Proxy chốt một cặp IP:cổng PX4 từ datagram đầu tiên cho đến khi khởi động lại. SYSID chỉ lọc phép theo dõi vị trí từ endpoint đó. Proxy vẫn chuyển tiếp nguyên datagram MAVLink hai chiều.

## 3. Mở PX4 SITL và Gazebo

Tiếp tục dùng phiên SITL đang chạy nếu đúng world và model. Khi khởi chạy phiên mới, đưa thư mục tuyệt đối `simulation/worlds` của repo vào đường dẫn tài nguyên Gazebo. Ví dụ từ thư mục PX4-Autopilot trong Bash:

```sh
export GZ_SIM_RESOURCE_PATH="/duong-dan/PBL3project/simulation/worlds${GZ_SIM_RESOURCE_PATH:+:$GZ_SIM_RESOURCE_PATH}"
PX4_GZ_WORLD=gateway_2zones_40m PX4_GZ_MODEL_POSE=0,-10,0.25 make px4_sitl gz_x500
```

Xác nhận Gazebo đã mở world `gateway_2zones_40m`. Khi Gazebo đã có phiên chạy, PX4 có thể dùng world đang chạy. [Hướng dẫn PX4 với Gazebo](https://docs.px4.io/main/en/sim_gazebo_gz/)

## 4. Định tuyến MAVLink tới proxy

Trong console PX4, chạy `mavlink status` trước để xem cổng UDP local, IP đích và cổng đích. Dùng instance đang gửi tới IP Windows có thể truy cập từ SITL và cổng đích `14600`. Có thể xem IPv4 Windows bằng `ipconfig`. Nếu instance dành cho đường proxy đang gửi sai đích, sửa cấu hình khởi động của chính instance đó hoặc dừng rồi chạy lại instance đó với cổng local cũ. Khi chưa có instance dùng được, chọn một cổng local đang rảnh, ví dụ `14611` nếu đã kiểm tra.

```sh
mavlink start -u 14611 -o 14600 -t WINDOWS_IP -m onboard
mavlink status
```

Thay `WINDOWS_IP` bằng IPv4 thực. Chỉ chạy lệnh `start` khi cần. Nếu PX4 đổi cổng nguồn, khởi động lại proxy để học lại endpoint. [Tham số MAVLink của PX4](https://docs.px4.io/main/en/modules/modules_communication)

## 5. Xác nhận LOCAL_POSITION_NED trên đúng instance

Trong console PX4, dùng cổng **local của instance PX4**, ví dụ `14611`. Lệnh `mavlink status streams` liệt kê stream theo instance trên các bản PX4 có hỗ trợ. Nếu cần bật hoặc chỉnh tần số, yêu cầu 10 Hz trên chính instance này.

```sh
mavlink status streams
mavlink stream -u 14611 -s LOCAL_POSITION_NED -r 10
mavlink status streams
```

Kiểm tra dòng `LOCAL_POSITION_NED` và tốc độ thực nếu có. Trong proxy, `PX4 endpoint:` phải khớp endpoint mong muốn. `forwarded_to_gateway` tăng cho biết có datagram được gửi tới gateway. Khi `N/E/D` có số và `age_ms` nhỏ, proxy đã giải mã được vị trí của đúng SYSID. Nếu chỉ có `NO_POSITION`, kiểm tra SYSID (tham số `MAV_SYS_ID`) và ước lượng local bằng `listener vehicle_local_position`. [Mã nguồn lệnh MAVLink của PX4](https://github.com/PX4/PX4-Autopilot/blob/main/src/modules/mavlink/mavlink_main.cpp)

## 6. Đo và xác nhận phép biến đổi

SDF khai báo `world_frame_orientation=ENU`, `heading_deg=0` và trọng lực theo −Z. Trong cấu hình SDF này, X world ứng với East và Y world ứng với North. Điều đó cho giả thuyết góc `θ = π/2` nếu estimator PX4 dùng cùng hướng. Vị trí spawn `(0, -10, 0.25)` chỉ dùng để đặt mô hình.

Khi UAV đứng yên và ước lượng đã ổn định, ghi một bộ `N, E, D` từ terminal proxy và một bộ `X, Y, Z` tương ứng từ terminal Gazebo:

```sh
gz model -m x500_0 --pose
```

Sau đó so sánh độ dịch chuyển NED và world ở hai hướng ngang độc lập trong cùng phiên. Với hướng ENU dự kiến, tăng E phải tăng X, tăng N phải tăng Y và độ dịch chuyển tính bằng mét phải khớp. Khi đo được một dịch chuyển chủ yếu theo +N, có thể ước lượng `θ = atan2(ΔY, ΔX)` và dùng dịch chuyển thứ hai để kiểm tra chiều +E. Nếu lần đo có cả ΔN và ΔE, góc từ một cặp dịch chuyển có thể tính bằng `atan2(ΔY, ΔX) - atan2(-ΔE, ΔN)`. Kiểm tra dự đoán trên cặp thứ hai trước khi lưu.

Với `θ` đã biết, đặt `c = cos(θ)`, `s = sin(θ)` và lấy cặp mẫu đứng yên để tính:

```text
origin_x = X - c*N - s*E
origin_y = Y - s*N + c*E
origin_z = Z + D - altitude_offset_m
```

Điền `world_origin_of_ned_m` theo thứ tự `[origin_x, origin_y, origin_z]`, `world_yaw_of_ned_rad` bằng `θ` (góc từ +X world tới +North, ngược chiều kim đồng hồ khi nhìn từ +Z). `altitude_offset_m` là hiệu chỉnh Z bổ sung, mặc định 0. Nếu đo trực tiếp cả ba thành phần từ cùng một cặp mẫu, giữ offset bằng 0 vì phần dịch Z đã nằm trong `origin_z`. Công thức proxy dùng là `X = origin_x + c*N + s*E`, `Y = origin_y + s*N - c*E`, `Z = origin_z + altitude_offset_m - D`.

## 7. Bật và xác minh transform

Lưu file và chạy lại proxy với `--config`. So lại X/Y/Z in trong dòng `[POSITION]` với pose Gazebo ở điểm đứng yên và ở một điểm khác. Trước khi điền đủ, log có `transform=TRANSFORM_NOT_CONFIGURED` và metrics có X/Y/Z bằng `null`. Sau khi hiệu chỉnh và có mẫu mới, mong đợi `status=VALID transform=CONFIGURED`.

Proxy hủy transform của phiên chạy nếu phát hiện `time_boot_ms` lùi đáng kể. Khi PX4 reboot hoặc estimator reset làm đổi gốc local, kiểm tra lại cặp NED/world và hiệu chỉnh trước khi dùng lại transform. `LOCAL_POSITION_NED` riêng lẻ có thể tiếp tục tăng timestamp sau reset estimator, nên cần phát hiện hoặc xác nhận reset từ PX4. Nếu biết gốc đã đổi, chạy proxy với gốc và yaw để `null` trong lúc đo lại.

## 8. Import flow và mở dashboard

1. Khởi chạy Node-RED. Trong **Manage palette → Install**, cài `@flowfuse/node-red-dashboard` nếu chưa có. [Tài liệu ui-template của FlowFuse](https://dashboard.flowfuse.com/nodes/widgets/ui-template.html)
2. Trong editor chọn **Import → select a file to import**, chọn `flows.json` trong thư mục này rồi **Deploy**. Flow tự tạo tab **PX4 Proxy - Gazebo** và dashboard riêng. Mã trong node **Gazebo UAV position** khớp với `uav_view.vue`.
3. Mở [dashboard Gazebo cục bộ](http://localhost:1880/px4-dashboard/gazebo). Nguồn DEMO cũ có thể ở trang riêng. Khi muốn tắt DEMO, chỉ vô hiệu hóa node phát DEMO trong flow cũ. Nếu một flow khác đang chiếm UDP 9871, đổi cấu hình `metrics.destination_port` và cổng của node **PX4 proxy UDP 9871** về cùng một cổng rảnh.

## 9–12. Đối chiếu khi chạy thật

9. **Đứng yên.** So X/Y/Z trên log với pose Gazebo. Icon giữ vị trí trong giới hạn nhiễu ước lượng. Trước mẫu world hợp lệ đầu tiên, mặt phẳng chỉ có gateway và vòng bán kính 40 m.
10. **Di chuyển.** So hai dịch chuyển ngang độc lập. Icon phải đi đúng hướng, đúng khoảng cách theo vạch mét, và quỹ đạo giữ tối đa 240 điểm.
11. **Lên cao.** Khi Gazebo Z tăng thì D giảm. `Cao độ world Z` trên dashboard phải tăng cùng với Gazebo.
12. **Ngừng dữ liệu.** Tạm dừng stream bằng `mavlink stream -u 14611 -s LOCAL_POSITION_NED -r 0` với cổng local thực. Sau mặc định 1.000 ms, log và dashboard báo `STALE`, tuổi mẫu tiếp tục tăng, icon mờ ở vị trí cuối đã biết và quỹ đạo dừng. Khôi phục stream bằng `-r 10`. Khi dừng proxy, trình duyệt báo **MẤT CẬP NHẬT UDP** sau khoảng 1,5 giây với chu kỳ mặc định. Với chu kỳ dài hơn, timeout là giá trị lớn hơn giữa 1,5 giây và ba chu kỳ metrics.

`metrics_udp_error` ghi mã lỗi Winsock nếu gửi metrics thất bại. Proxy vẫn chuyển tiếp MAVLink. UDP gửi thành công chỉ xác nhận hệ điều hành đã nhận datagram để gửi.
