# Ghi số liệu và log khi kiểm thử

Các file `PASS/FAIL` cũ là bằng chứng cho nhánh kiểm thử đã chạy, nhưng chưa chứa đủ số liệu. Helper PBL3-02/04/05 nay in thêm dòng `METRICS`; chỉ lần chạy **sau khi build lại** mới có dòng này. Không chép số liệu từ lần tự kiểm của người sửa code thành số liệu của người đo. Giữ log cũ, ghi lần mới vào file khác.

Bạn đang dùng **Command Prompt (`cmd`)**: gõ `powershell` để vào PowerShell và thấy dấu nhắc bắt đầu bằng `PS`. Sau đó chạy các lệnh bên dưới tại thư mục gốc dự án. `Tee-Object` vừa hiện output trên terminal vừa ghi file log. `$LASTEXITCODE` dùng **trong PowerShell** để đọc mã thoát của chương trình vừa chạy. Nếu quay lại `cmd` bằng `exit`, dùng `echo %ERRORLEVEL%` thay thế. Không chạy lại với cùng tên log, vì `Tee-Object -FilePath` sẽ thay nội dung cũ.

## Bổ sung số liệu cho PBL3-02, 04, 05

### PBL3-02: UDP echo

```powershell
g++ -std=c++17 -O2 tools\udp_echo_check.cpp -o udp_echo_check.exe -lws2_32
.\udp_echo_check.exe 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-02_UDP\metrics_02.log'
"exit_code=$LASTEXITCODE" | Tee-Object -FilePath 'docs\ketqua\PBL3-02_UDP\metrics_02.log' -Append
```

Dòng METRICS gồm cổng loopback, độ dài payload và số byte gửi/nhận/trả lại. Đây là **một** trao đổi thử, không đo thông lượng hay độ trễ.

### PBL3-04: parser chia/gộp

```powershell
g++ -std=c++17 -O2 tools\mavlink_parser_check.cpp -o mavlink_parser_check.exe -lws2_32
.\mavlink_parser_check.exe 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-04_Parser\metrics_02.log'
"exit_code=$LASTEXITCODE" | Tee-Object -FilePath 'docs\ketqua\PBL3-04_Parser\metrics_02.log' -Append
```

Dòng METRICS gồm byte của từng frame, điểm chia, số frame sau mỗi phần và SYSID/COMPID/MSGID.

### PBL3-05: integrity/CRC

```powershell
g++ -std=c++17 -O2 tools\mavlink_integrity_check.cpp -o mavlink_integrity_check.exe -lws2_32
.\mavlink_integrity_check.exe 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-05_Integrity\metrics_03.log'
"exit_code=$LASTEXITCODE" | Tee-Object -FilePath 'docs\ketqua\PBL3-05_Integrity\metrics_03.log' -Append
```

Dòng METRICS gồm byte payload/wire, frame hợp lệ, frame sai CRC bị loại và độ dài signature. Phép thử này không xác thực chữ ký bằng khóa.

## Log cho PBL3-07 fan-out

Chỉ làm sau khi bổ sung ba log ngắn ở trên. Dùng **run ID mới** cho mỗi lần; không ghi đè log hoặc CSV. Dừng Gateway và client của ca trước trước khi bắt đầu. Tạo thư mục trước:

```powershell
mkdir docs\ketqua\PBL3-07_Fanout\I07_01
g++ -std=c++17 -O2 src\gateway.cpp -o gateway.exe -lws2_32
g++ -std=c++17 -O2 src\tcpclient.cpp -o tcpclient.exe -lws2_32
```

Cửa sổ PowerShell Gateway:

```powershell
.\gateway.exe --scenario I07_FANOUT --run-id I07_01 --urb 0 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-07_Fanout\I07_01\gateway.log'
```

Nối QGC TCP `127.0.0.1:5760`, sau đó mở hai cửa sổ PowerShell cho client A và B:

```powershell
.\tcpclient.exe 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-07_Fanout\I07_01\client_A.log'
```

```powershell
.\tcpclient.exe 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-07_Fanout\I07_01\client_B.log'
```

Các dòng `CONNECTED`, `DISCONNECTED`, STATS `Clients`, `TCP TX bytes`, `Slow clients` và CLIENT STATS/HEARTBEAT sẽ có trong log. Đóng client B trước; giữ QGC và client A chạy để quan sát còn nhận frame. Nhấn Ctrl+C để kết thúc các ca ngắn khi đã có đủ bằng chứng. File CSV do chương trình tạo ở đường mặc định sẽ được chuyển vào thư mục run sau khi tiến trình dừng. Bản log và CSV giữ nguyên, không điền lại số đo thủ công.

## PBL3-08: chuẩn bị hai UAV để đo định tuyến

Mục tiêu của lượt đầu là xác nhận hai nguồn `SYSID=1` và `SYSID=2` cùng online. Chỉ chuyển sang phép gửi lệnh khi Gateway đã hiện hai UAV và QGC nhận cả hai. Không dùng lại tên run đã có.

1. Giữ UAV 1 đang chạy với MAVLink local `14600` gửi tới Windows UDP `14550`. Trong cửa sổ WSL khác, khởi động UAV 2 bằng cách chạy multi-vehicle phù hợp bản PX4/Gazebo của bạn. Nếu đang dùng PX4 Gazebo `gz_x500` và UAV đầu tiên là instance mặc định, ví dụ từ thư mục `PX4-Autopilot` là:

   ```bash
   PX4_SYS_AUTOSTART=4001 PX4_GZ_MODEL_POSE="0,2" PX4_SIM_MODEL=gz_x500 ./build/px4_sitl_default/bin/px4 -i 1
   ```

   Trong `pxh>` UAV 2, chạy `param show MAV_SYS_ID`; yêu cầu kết quả là `2`. Chạy `mavlink status` để kiểm xem đã có stream local `14601` tới Windows UDP `14550` chưa. Chỉ khi chưa có stream đó, chạy lệnh sau, thay `<WINDOWS_IP>` bằng địa chỉ Windows hiện tại từ WSL:

   ```text
   mavlink start -u 14601 -o 14550 -t <WINDOWS_IP> -m normal
   mavlink status
   ```

   Kiểm thêm `param show MAV_SYS_ID` và `mavlink status` ở UAV 1. Lưu nguyên văn lệnh khởi động, IP và hai status; các cổng mặc định của bản PX4 có thể khác nên phải đọc status thực tế.

2. Trong PowerShell ở gốc dự án, khi các tiến trình Gateway/client cũ đã dừng, tạo thư mục và chạy Gateway trong một cửa sổ:

   ```powershell
   New-Item -ItemType Directory -Force 'docs\ketqua\PBL3-08_Route\I08_01'
   .\gateway.exe --scenario I08_ROUTE --run-id I08_01 --urb 0 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-08_Route\I08_01\gateway.log'
   ```

3. Kết nối QGC tới Gateway bằng TCP `127.0.0.1:5760`. Kiểm hai dòng `UAV 1 ONLINE tai ...:14600` và `UAV 2 ONLINE tai ...:14601` trong `gateway.log`; hai PX4 còn có các MAVLink instance mặc định khác hướng tới UDP 14550 nên không được chỉ dựa vào `UAV online=2`. Quan sát STATS có hai UAV online ở ít nhất hai mẫu liên tiếp và QGC thấy cả hai. Nếu Gateway học cổng 18570/18571 hoặc ghi `CONFLICT`, dừng trước khi phát lệnh và gửi log để xử lý luồng trùng. Gửi đoạn log ONLINE/STATS và quan sát QGC để xác nhận điều kiện đầu vào trước khi đo lệnh riêng từng UAV. Giữ Gateway và PX4 chạy nếu muốn tiếp tục ngay.

Ví dụ multi-vehicle: [PX4 Gazebo multi-vehicle](https://docs.px4.io/v1.16/en/sim_gazebo_gz/multi_vehicle_simulation). Ý nghĩa `-u`, `-o`, `-t`: [PX4 MAVLink module](https://docs.px4.io/main/en/modules/modules_communication).

### I08_02: kiểm thao tác chọn riêng từng UAV

I08_01 đã xác nhận hai cổng, ROUTE/ACK và QGC thấy hai UAV; còn thiếu quan sát một UAV đổi trạng thái trong khi UAV kia giữ nguyên. Chạy lượt mới với cùng hai PX4, `--urb 0`, tên `I08_02`; lưu `gateway.log` vào thư mục riêng theo mẫu I08_01. Trong QGC Fly View, chọn rõ **UAV 1** bằng bộ chọn nhiều UAV, ghi trạng thái Armed/Disarmed của cả hai, rồi Arm UAV 1 nếu QGC báo sẵn sàng. Kiểm UAV 1 đổi sang Armed, UAV 2 vẫn Disarmed; sau đó Disarm UAV 1 khi đang trên mặt đất. Lặp lại với **UAV 2**, quan sát UAV 1 giữ Disarmed. Không dùng Force Arm hoặc lệnh Takeoff trong lượt này. Nếu QGC từ chối Arm do preflight, giữ nguyên log ACK và lý do trên QGC, không tính là lỗi định tuyến khi chưa thấy `ROUTE` sai. Gửi thời điểm tương đối của từng thao tác, trạng thái hai UAV trước/sau, dòng `ROUTE`/`ACK command=401` và ảnh QGC/Gazebo nếu có. [QGC mô tả bộ chọn UAV và điều kiện Arm](https://docs.qgroundcontrol.com/master/en/qgc-user-guide/fly_view/fly_view_toolbar.html).

```powershell
New-Item -ItemType Directory -Force 'docs\ketqua\PBL3-08_Route\I08_02'
.\gateway.exe --scenario I08_ROUTE --run-id I08_02 --urb 0 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-08_Route\I08_02\gateway.log'
```

## PBL3-09: ngắt riêng UAV 2 và kiểm BLOCK

Đóng Gateway của lượt PBL3-08 sau khi đã lưu terminal. Không dùng lại `I08_01`: lượt thao tác vừa rồi có file thô mới mang tên này. Giữ cả hai PX4/Gazebo, tạo run mới `I09_01` và bật Gateway khi hai UAV còn online. Lệnh trong PowerShell gốc dự án:

```powershell
New-Item -ItemType Directory -Force 'docs\ketqua\PBL3-09_Recovery\I09_01'
g++ -std=c++17 -O2 tools\route_probe.cpp -o route_probe.exe -lws2_32
.\gateway.exe --scenario I09_ROUTE --run-id I09_01 --urb 0 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-09_Recovery\I09_01\gateway.log'
```

Chờ `UAV online=2` và log ONLINE đúng cổng `14600`, `14601`. Trong `pxh>` của **UAV 2**, chạy `mavlink stop -u 14601`; giữ UAV 1 hoạt động. [PX4 hỗ trợ dừng một MAVLink instance theo cổng local](https://docs.px4.io/main/en/modules/modules_communication). Chờ ít nhất 7 giây và kiểm Gateway ghi `UAV 2 OFFLINE`, STATS `UAV online=1`, `UAV 1: ONLINE`. Nếu UAV 2 lại ONLINE ở cổng 18571, dừng lượt đo và gửi log vì luồng mặc định đang thay chỗ luồng 14601.

Trong một cửa sổ PowerShell khác, chạy từng lệnh dưới đây khi UAV 2 đã OFFLINE:

```powershell
.\route_probe.exe 2 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-09_Recovery\I09_01\probe_uav2_offline.log'
"exit_code=$LASTEXITCODE" | Tee-Object -FilePath 'docs\ketqua\PBL3-09_Recovery\I09_01\probe_uav2_offline.log' -Append
.\route_probe.exe 1 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-09_Recovery\I09_01\probe_uav1_online.log'
"exit_code=$LASTEXITCODE" | Tee-Object -FilePath 'docs\ketqua\PBL3-09_Recovery\I09_01\probe_uav1_online.log' -Append
```

Probe gửi `MAV_CMD_REQUEST_MESSAGE` (`512`) yêu cầu một bản AUTOPILOT_VERSION; không gửi lệnh bay. Khi UAV 2 offline, probe 2 dự kiến `TIMEOUT` và exit 1 **cùng với** Gateway `BLOCK: target_system=2` và counter `no route` tăng. Chỉ TIMEOUT mà thiếu BLOCK không đủ kết luận. Probe 1 cần ACK `sysid=1, command=512, result=0`, exit 0 và Gateway ROUTE tới `14600`. Gửi log các mốc này trước khi khởi động lại UAV 2 cho bước phục hồi.

### I09_02: phục hồi UAV 2 trên endpoint mới

Phải làm đủ OFFLINE→ONLINE trong **cùng một tiến trình Gateway** để kiểm route cũ hết hiệu lực và route mới được học. Sau I09_01, luồng `14601` của UAV 2 đã dừng. Trong `pxh>` UAV 2, khôi phục luồng đó trước khi bắt đầu I09_02 (thay IP nếu Windows hiện đã đổi):

```text
mavlink start -u 14601 -o 14550 -t 172.26.160.1 -m normal -r 100000
mavlink status
```

Khởi động Gateway với run ID mới bằng PowerShell tại gốc dự án:

```powershell
New-Item -ItemType Directory -Force 'docs\ketqua\PBL3-09_Recovery\I09_02'
.\gateway.exe --scenario I09_RECOVERY --run-id I09_02 --urb 0 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-09_Recovery\I09_02\gateway.log'
```

Chờ log học `UAV 2 ONLINE tai ...:14601` và STATS `UAV online=2`. Trong `pxh>` UAV 2 chạy `mavlink stop -u 14601`; chờ Gateway báo `UAV 2 OFFLINE`, `UAV online=1`. **Giữ Gateway đang chạy**, rồi trong cùng `pxh>` chạy:

```text
mavlink start -u 14602 -o 14550 -t 172.26.160.1 -m normal -r 100000
mavlink status
```

Chờ Gateway ghi `UAV 2 ONLINE tai ...:14602`, STATS trở lại `UAV online=2`. Trong PowerShell khác chạy probe tới UAV 2:

```powershell
.\route_probe.exe 2 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-09_Recovery\I09_02\probe_uav2_recovered.log'
"exit_code=$LASTEXITCODE" | Tee-Object -FilePath 'docs\ketqua\PBL3-09_Recovery\I09_02\probe_uav2_recovered.log' -Append
```

Cần thấy ACK `sysid=2`, `command=512`, `result=0`, exit 0 và Gateway `ROUTE ... UAV 2 -> ...:14602`; không có ROUTE mới tới 14601 sau khi phục hồi. Gửi `gateway.log`, probe log và `mavlink status` của UAV 2. Khi run đã kết thúc, các CSV do Gateway tạo sẽ được chuyển vào thư mục I09_02.

## PBL3-10: hồ sơ build từ source

Làm trong **PowerShell** ở gốc dự án. PX4, Gazebo, QGC và Gateway không cần chạy. Dùng thư mục output mới để không nhầm với `gateway.exe`/`tcpclient.exe` cũ ở gốc. Lệnh dưới đây chỉ tạo log và executable khi **bạn** chạy:

```powershell
$run = 'docs\ketqua\PBL3-10_Build\I10_01'
New-Item -ItemType Directory -Force "$run\bin" | Out-Null
"time=$(Get-Date -Format o)" | Tee-Object -FilePath "$run\build.log"
"compiler=$(g++ --version | Select-Object -First 1)" | Tee-Object -FilePath "$run\build.log" -Append
"git_head=$(git rev-parse HEAD)" | Tee-Object -FilePath "$run\build.log" -Append
'gateway_command=g++ -std=c++17 -O2 src\gateway.cpp -o bin\gateway.exe -lws2_32' | Tee-Object -FilePath "$run\build.log" -Append
g++ -std=c++17 -O2 src\gateway.cpp -o "$run\bin\gateway.exe" -lws2_32 2>&1 | Tee-Object -FilePath "$run\build.log" -Append
$gatewayExit = $LASTEXITCODE
"gateway_exit_code=$gatewayExit" | Tee-Object -FilePath "$run\build.log" -Append
'client_command=g++ -std=c++17 -O2 src\tcpclient.cpp -o bin\tcpclient.exe -lws2_32' | Tee-Object -FilePath "$run\build.log" -Append
g++ -std=c++17 -O2 src\tcpclient.cpp -o "$run\bin\tcpclient.exe" -lws2_32 2>&1 | Tee-Object -FilePath "$run\build.log" -Append
$clientExit = $LASTEXITCODE
"client_exit_code=$clientExit" | Tee-Object -FilePath "$run\build.log" -Append
if ($gatewayExit -eq 0 -and $clientExit -eq 0) {
  Get-ChildItem "$run\bin" -File | Select-Object Name,Length | Format-Table -AutoSize | Out-String | Tee-Object -FilePath "$run\build.log" -Append
  Get-FileHash "$run\bin\gateway.exe","$run\bin\tcpclient.exe" -Algorithm SHA256 | Select-Object Path,Hash | Format-List | Out-String | Tee-Object -FilePath "$run\build.log" -Append
} else {
  'build_failed=true' | Tee-Object -FilePath "$run\build.log" -Append
}
```

Nếu một exit code khác 0, giữ log lỗi và báo lại; đừng lấy hash của executable cũ. Kết quả đạt khi cả hai exit code bằng 0, có hai executable mới và hash SHA-256 tương ứng. Commit/hash source chi tiết và điều kiện môi trường sẽ gom thêm ở PBL3-15.

## PBL3-11: tách CRC sai qua UDP và TCP trong STATS

PBL3-09 đã có bằng chứng `Blocked messages=1 (no route=1)` trong `I09_01/gateway.log`. Lượt I11_01 kiểm riêng hai nguồn CRC. Dừng PX4, QGC và các Gateway cũ để chỉ Gateway mới nhận frame thử. `stats_crc_probe` tạo một HEARTBEAT MAVLink với checksum bị sửa, gửi **một bản qua UDP 14550** và **một bản qua TCP 5760**; chương trình chỉ xác nhận số byte đã gửi, còn kết quả parse phải đọc từ Gateway STATS.

PowerShell cửa sổ 1, tại gốc dự án:

```powershell
New-Item -ItemType Directory -Force 'docs\ketqua\PBL3-11_Stats\I11_01'
.\docs\ketqua\PBL3-10_Build\I10_01\bin\gateway.exe --scenario I11_STATS --run-id I11_01 --urb 0 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-11_Stats\I11_01\gateway.log'
```

PowerShell cửa sổ 2, sau khi Gateway đã in header UDP/TCP:

```powershell
g++ -std=c++17 -O2 tools\stats_crc_probe.cpp -o 'docs\ketqua\PBL3-11_Stats\I11_01\stats_crc_probe.exe' -lws2_32
.\docs\ketqua\PBL3-11_Stats\I11_01\stats_crc_probe.exe 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-11_Stats\I11_01\probe.log'
"exit_code=$LASTEXITCODE" | Tee-Object -FilePath 'docs\ketqua\PBL3-11_Stats\I11_01\probe.log' -Append
```

Chờ ít nhất một mẫu `=== STATS ===` sau probe (Gateway in mỗi 5 giây), rồi Ctrl+C ở cửa sổ Gateway. Kỳ vọng `CRC errors=2 (UDP=1, TCP=1)`; `MAVLink UAV RX=0`, `Client RX=0`, `Blocked messages=0`, `UDP dropped/cancelled=0`, `Slow clients=0` nếu không có nguồn khác. `probe.log` exit 0 chỉ chứng minh gửi thành công; chính STATS chứng minh Gateway đếm/loại riêng hai frame hỏng. Gửi hai log để đối chiếu. Các nhánh queue và slow client sẽ kiểm riêng ở PBL3-17/PBL3-16; chưa coi counter bằng 0 là đã kích hoạt nhánh đó.

## PBL3-13: ngắt và khôi phục heartbeat, đọc QoS ở ba điểm

I09_01/02 đã đo ingress và egress khi UAV 2 mất heartbeat. Lượt này thêm TCP client để kiểm cột `hb_online`, `offline_events`, `offline_time_s` và các trường sequence tại client; không cần QGC. Giữ PX4 UAV 1 gửi từ `14600` tới Windows UDP `14550`. UAV 2 có thể còn chạy ở `14602`, nhưng phép thử chỉ xem SYSID 1. Đảm bảo Gateway và client cũ đã dừng.

PowerShell cửa sổ 1:

```powershell
New-Item -ItemType Directory -Force 'docs\ketqua\PBL3-13_Heartbeat\I13_01'
.\docs\ketqua\PBL3-10_Build\I10_01\bin\gateway.exe --scenario I13_HB --run-id I13_01 --urb 0 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-13_Heartbeat\I13_01\gateway.log'
```

PowerShell cửa sổ 2:

```powershell
.\docs\ketqua\PBL3-10_Build\I10_01\bin\tcpclient.exe 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-13_Heartbeat\I13_01\client.log'
```

Chờ cả Gateway và client thấy HEARTBEAT UAV 1 trong ít nhất 15 giây. Trong `pxh>` của **UAV 1**, chạy `mavlink stop -u 14600`; chờ khoảng 12–15 giây để các timeout 3 s và 5 s đều phát sinh. Giữ Gateway/client đang chạy và mở lại luồng bằng `mavlink start -u 14600 -o 14550 -t 172.26.160.1 -m normal -r 100000`, thay IP nếu Windows hiện đã đổi. Chờ thêm ít nhất 15 giây, xem Gateway và client ghi ONLINE trở lại; sau đó Ctrl+C dừng client và Gateway. Gửi hai log và `mavlink status` sau khôi phục.

Kỳ vọng QoS egress và client báo OFFLINE sau khoảng 3 s mất HEARTBEAT, ingress và route Gateway sau khoảng 5 s; cả ba chuyển ONLINE khi luồng trở lại. So các cột `seq_gap_estimate`, `seq_resyncs`, `seq_uncertain_gaps` trong CSV trước/sau: khởi động lại MAVLink instance có thể reset sequence, nên không diễn giải mọi bước nhảy là mất gói. CSV Gateway được tạo trong `docs/ketqua/issue19/`, CSV client được tạo trong `docs/ketqua/`; chúng sẽ được chuyển vào thư mục I13_01 sau khi tiến trình dừng.

## PBL3-14: đo lại baseline E0_02 trong 600 giây

Mục tiêu là một UAV (SYSID 1, cổng 14600) → Gateway Markov OFF (`urb=0`) → đúng một TCP client, 10 giây warm-up rồi đo 600 giây. Sau I13_01, kiểm `mavlink status` UAV 1 có instance `14600 → 14550`; dừng luồng UAV 2 ở 14602 hoặc tắt PX4 thứ hai. Đóng QGC, probe và các Gateway/client cũ. Không dùng lại `E0_01` vì config của run đó trống.

PowerShell ở gốc dự án, chuẩn bị run và dấu vết source:

```powershell
New-Item -ItemType Directory -Force 'docs\ketqua\PBL3-14_E0\E0_02'
$run = 'docs\ketqua\PBL3-14_E0\E0_02'
"time=$(Get-Date -Format o)" | Tee-Object -FilePath "$run\metadata.log"
"git_head=$(git rev-parse HEAD)" | Tee-Object -FilePath "$run\metadata.log" -Append
Get-FileHash src\gateway.cpp,src\tcpclient.cpp,src\qos.h,src\emulator.h -Algorithm SHA256 | Format-List | Out-String | Tee-Object -FilePath "$run\metadata.log" -Append
```

Cửa sổ PowerShell Gateway:

```powershell
.\docs\ketqua\PBL3-10_Build\I10_01\bin\gateway.exe --scenario E0 --run-id E0_02 --urb 0 --seed 1 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-14_E0\E0_02\gateway.log'
```

Ngay sau khi Gateway ghi `UAV 1 ONLINE`, mở cửa sổ client:

```powershell
.\docs\ketqua\PBL3-10_Build\I10_01\bin\tcpclient.exe 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-14_E0\E0_02\client.log'
```

Trong cửa sổ PowerShell thứ ba, lấy 11 mẫu CPU/RAM/handle cách nhau 60 giây (chạy song song với phép đo):

```powershell
$resourceLog = 'docs\ketqua\PBL3-14_E0\E0_02\resources.csv'
'timestamp,process,id,cpu_seconds,working_set_bytes,private_bytes,handles' | Tee-Object -FilePath $resourceLog
for ($i = 0; $i -le 10; $i++) {
  $stamp = Get-Date -Format o
  Get-Process gateway,tcpclient -ErrorAction SilentlyContinue | ForEach-Object {
    "$stamp,$($_.ProcessName),$($_.Id),$($_.CPU),$($_.WorkingSet64),$($_.PrivateMemorySize64),$($_.HandleCount)"
  } | Tee-Object -FilePath $resourceLog -Append
  if ($i -lt 10) { Start-Sleep -Seconds 60 }
}
```

Giữ nguyên nguồn và trạng thái UAV trong run. Chờ **cả** Gateway ingress/egress và client CSV hiện mốc `t=600.000` (hai cửa sổ đo bắt đầu lệch nhau); Gateway cũng ghi file `E0_E0_02_markov.csv`. Hai chương trình vẫn tiếp tục chạy sau khi hoàn tất cửa sổ đo; lúc đó nhấn Ctrl+C để đóng client rồi Gateway. Không dừng ở phút 600 tính từ khi nhấn Enter nếu client chưa đủ mốc. Gửi `gateway.log`, `client.log`, `resources.csv`, `metadata.log` và `mavlink status` của stream 14600. Các CSV thô của Gateway/client sẽ được chuyển vào E0_02 sau khi tiến trình dừng.

## PBL3-15: bổ sung metadata để tái lập E0_02

Không cần chạy lại E0. `metadata.log` của E0_02 đã có Git HEAD và hash bốn file; `build.log` I10_01 có compiler và hash executable. Để chốt **toàn bộ source Gateway/client**, chạy trong PowerShell ở gốc dự án:

```powershell
$out = 'docs\ketqua\PBL3-14_E0\E0_02\source_manifest.log'
"capture_time=$(Get-Date -Format o)" | Tee-Object -FilePath $out
"git_head=$(git rev-parse HEAD)" | Tee-Object -FilePath $out -Append
"mavlink_submodule=$(git -C third_party/mavlink rev-parse HEAD)" | Tee-Object -FilePath $out -Append
'git_status_begin' | Tee-Object -FilePath $out -Append
git status --short | Tee-Object -FilePath $out -Append
'git_status_end' | Tee-Object -FilePath $out -Append
Get-ChildItem src -File | Sort-Object Name | Get-FileHash -Algorithm SHA256 | ForEach-Object {
  "$($_.Hash)  $($_.Path)"
} | Tee-Object -FilePath $out -Append
```

Trong `pxh>` của **UAV 1**, sao chép kết quả ba lệnh sau vào hội thoại hoặc lưu thủ công thành `docs/ketqua/PBL3-14_E0/E0_02/px4_environment.txt`:

```text
ver all
param show MAV_SYS_ID
mavlink status
```

Ghi thêm lệnh bạn đã dùng để khởi động PX4/Gazebo và lệnh `mavlink start` tạo stream 14600 trong E0_02; nếu không còn nhớ, ghi rõ `không lưu được lệnh gốc` thay vì đoán. Xác nhận UAV khi đo E0_02 ở trạng thái nào (ví dụ đứng yên trên mặt đất, Armed/Disarmed) nếu có ghi chép; không suy trạng thái của run cũ từ trạng thái PX4 hiện tại. `ver all` là lệnh PX4 in phiên bản/build: [PX4 command reference](https://docs.px4.io/main/en/modules/modules_command).

## PBL3-16: lượt 1, 20 lần reconnect có ACK

Giữ một PX4 UAV 1 gửi từ 14600 tới Gateway UDP 14550. Đóng QGC và các TCP client khác; Gateway cần ghi `UAV online=1` trước khi bắt đầu. `route_probe` mỗi lần mở một kết nối TCP mới, gửi yêu cầu message 512 tới SYSID 1, đợi ACK rồi đóng kết nối. Vì vậy 20 vòng kiểm đồng thời connect, route, reply và disconnect mà không điều khiển bay.

PowerShell cửa sổ Gateway tại gốc dự án:

```powershell
New-Item -ItemType Directory -Force 'docs\ketqua\PBL3-16_Clients\I16_01'
.\docs\ketqua\PBL3-10_Build\I10_01\bin\gateway.exe --scenario I16_RECONNECT --run-id I16_01 --urb 0 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-16_Clients\I16_01\gateway.log'
```

Cửa sổ PowerShell thứ hai, sau khi Gateway học UAV 1:

```powershell
$run = 'docs\ketqua\PBL3-16_Clients\I16_01'
g++ -std=c++17 -O2 tools\route_probe.cpp -o "$run\route_probe.exe" -lws2_32
"time=$(Get-Date -Format o)" | Tee-Object -FilePath "$run\reconnect.log"
'phase,time,cpu_seconds,working_set_bytes,handles' | Tee-Object -FilePath "$run\resources.csv"
for ($i = 1; $i -le 20; $i++) {
  "cycle=$i" | Tee-Object -FilePath "$run\reconnect.log" -Append
  & ".\$run\route_probe.exe" 1 2>&1 | Tee-Object -FilePath "$run\reconnect.log" -Append
  "exit_code=$LASTEXITCODE" | Tee-Object -FilePath "$run\reconnect.log" -Append
  if ($i -eq 1 -or $i % 5 -eq 0) {
    $stamp = Get-Date -Format o
    Get-Process gateway -ErrorAction SilentlyContinue | ForEach-Object {
      "cycle_$i,$stamp,$($_.CPU),$($_.WorkingSet64),$($_.HandleCount)"
    } | Tee-Object -FilePath "$run\resources.csv" -Append
  }
  Start-Sleep -Milliseconds 250
}
```

Chờ Gateway in thêm ít nhất một STATS sau vòng 20 rồi Ctrl+C. Kỳ vọng `reconnect.log` có 20 `ACK: sysid=1 command=512 result=0`, 20 exit code 0; Gateway có 20 `CONNECTED`, 20 `DISCONNECTED`, cuối cùng `Clients=0`, không CRC/BLOCK/slow-client. Gửi `reconnect.log`, `gateway.log` và `resources.csv`. Nếu một vòng lỗi, giữ log và báo số vòng lỗi; không xóa hoặc chạy ghi đè run cũ.

### I16_02: một client không đọc và một client thường

Giữ UAV 1 gửi từ 14600, đóng QGC và Gateway/client cũ. Lượt này dùng một TCP client bình thường để chứng minh vẫn nhận HEARTBEAT và một TCP connection có receive buffer nhỏ **không đọc dữ liệu**. Chạy khoảng 600 giây; nếu Gateway chưa ghi `Client qua cham` sau thời gian đó, giữ nguyên log và đánh dấu lượt này chưa kích hoạt được nhánh slow client.

PowerShell cửa sổ 1 (Gateway):

```powershell
New-Item -ItemType Directory -Force 'docs\ketqua\PBL3-16_Clients\I16_02'
.\docs\ketqua\PBL3-10_Build\I10_01\bin\gateway.exe --scenario I16_SLOW --run-id I16_02 --urb 0 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-16_Clients\I16_02\gateway.log'
```

PowerShell cửa sổ 2 (client bình thường):

```powershell
.\docs\ketqua\PBL3-10_Build\I10_01\bin\tcpclient.exe 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-16_Clients\I16_02\normal_client.log'
```

PowerShell cửa sổ 3 (client không đọc):

```powershell
$slowLog = 'docs\ketqua\PBL3-16_Clients\I16_02\slow_client.log'
$slow = New-Object System.Net.Sockets.TcpClient
$slow.ReceiveBufferSize = 1024
$slow.Connect('127.0.0.1', 5760)
"connected=$(Get-Date -Format o) receive_buffer_bytes=$($slow.ReceiveBufferSize)" | Tee-Object -FilePath $slowLog
for ($i = 1; $i -le 20; $i++) {
  Start-Sleep -Seconds 30
  "elapsed_s=$($i * 30) no_recv=true" | Tee-Object -FilePath $slowLog -Append
}
$slow.Close()
'socket_closed_by_test=true' | Tee-Object -FilePath $slowLog -Append
```

PowerShell cửa sổ 4 (tài nguyên, chạy song song):

```powershell
$resourceLog = 'docs\ketqua\PBL3-16_Clients\I16_02\resources.csv'
'timestamp,process,id,cpu_seconds,working_set_bytes,private_bytes,handles' | Tee-Object -FilePath $resourceLog
for ($i = 0; $i -le 10; $i++) {
  $stamp = Get-Date -Format o
  Get-Process gateway,tcpclient -ErrorAction SilentlyContinue | ForEach-Object {
    "$stamp,$($_.ProcessName),$($_.Id),$($_.CPU),$($_.WorkingSet64),$($_.PrivateMemorySize64),$($_.HandleCount)"
  } | Tee-Object -FilePath $resourceLog -Append
  if ($i -lt 10) { Start-Sleep -Seconds 60 }
}
```

Chờ Gateway và client thường có CSV mốc `t=600`, sau đó dừng client thường và Gateway bằng Ctrl+C. Kỳ vọng Gateway có lúc `Clients=2`, rồi `Client qua cham`, `Slow clients=1`, `Clients=1`, trong khi `normal_client.log` tiếp tục tăng frame/HEARTBEAT và CRC=0. Xác nhận client bình thường vẫn nhận **sau** thời điểm slow client bị ngắt. Gửi bốn log; CSV do chương trình tạo sẽ được chuyển vào I16_02 sau khi các tiến trình dừng.

### I16_03: giới hạn 10 client và thử kết nối thứ 11

Đây là ca ngắn để kiểm giới hạn kết nối. Giữ một UAV ở 14600, đóng Gateway/QGC/client cũ. Mười client sẽ **đọc dữ liệu** trong lúc thử client thứ 11, vì vậy không trộn kết quả này với ca client chậm I16_02.

PowerShell cửa sổ Gateway:

```powershell
New-Item -ItemType Directory -Force 'docs\ketqua\PBL3-16_Clients\I16_03'
.\docs\ketqua\PBL3-10_Build\I10_01\bin\gateway.exe --scenario I16_CAP --run-id I16_03 --urb 0 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-16_Clients\I16_03\gateway.log'
```

Khi Gateway đã ghi `UAV 1 ONLINE`, chạy khối dưới trong PowerShell thứ hai. Nó mở 10 TCP socket, đọc byte telemetry trước và sau khi thử socket thứ 11, rồi đóng toàn bộ. `11th_read=0` hoặc lỗi đọc cùng dòng Gateway `Da du so client` là bằng chứng từ chối; chỉ việc `Connect()` thành công chưa có nghĩa Gateway đã nhận client.

```powershell
& {
  $clients = @()
  for ($i = 1; $i -le 10; $i++) {
    $c = New-Object System.Net.Sockets.TcpClient
    $c.Connect('127.0.0.1', 5760)
    $clients += $c
    "client_$i connected=true"
  }
  Start-Sleep -Seconds 5
  Get-Process gateway | ForEach-Object { "gateway_at_10 cpu_seconds=$($_.CPU) working_set_bytes=$($_.WorkingSet64) handles=$($_.HandleCount)" }
  for ($i = 0; $i -lt 10; $i++) {
    $n = $clients[$i].Available
    if ($n -gt 0) {
      $buffer = New-Object byte[] $n
      $read = $clients[$i].GetStream().Read($buffer, 0, $n)
    } else { $read = 0 }
    "client_$($i+1) read_before_11th=$read"
  }
  $extra = New-Object System.Net.Sockets.TcpClient
  $extra.ReceiveTimeout = 2000
  $extra.Connect('127.0.0.1', 5760)
  Start-Sleep -Seconds 2
  $one = New-Object byte[] 1
  try { $read11 = $extra.GetStream().Read($one, 0, 1); "11th_read=$read11" }
  catch { "11th_read_error=$($_.Exception.Message)" }
  $extra.Close()
  Start-Sleep -Seconds 5
  for ($i = 0; $i -lt 10; $i++) {
    $n = $clients[$i].Available
    if ($n -gt 0) {
      $buffer = New-Object byte[] $n
      $read = $clients[$i].GetStream().Read($buffer, 0, $n)
    } else { $read = 0 }
    "client_$($i+1) read_after_11th=$read"
    $clients[$i].Close()
  }
  Start-Sleep -Seconds 2
  Get-Process gateway | ForEach-Object { "gateway_after_close cpu_seconds=$($_.CPU) working_set_bytes=$($_.WorkingSet64) handles=$($_.HandleCount)" }
} 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-16_Clients\I16_03\cap11.log'
```

Chờ Gateway in STATS `Clients=0` sau khi mười socket đóng, rồi Ctrl+C. Kỳ vọng có 10 CONNECTED, STATS `Clients=10`, dòng `Da du so client` khi thử socket thứ 11, sau đó 10 DISCONNECTED; cả mười client có byte nhận trước/sau, không có Slow clients/BLOCK/CRC. Gửi `cap11.log` và `gateway.log`. Các mức tải 1/5/10 trong 600 s/mức sẽ làm ở lượt sau.

### I16_04: tải 1 client trong 600 giây

Ca đầu của bộ 1/5/10 dùng cùng chương trình `multi_client_load` để đếm frame, byte và HEARTBEAT trên từng socket. Giữ một PX4 UAV 1 ở 14600, không mở QGC/client khác, dùng Markov OFF. Gateway/client đều có warm-up 10 s; mỗi run tải đo 600 s. Dùng executable Gateway của I10_01 và build helper vào thư mục run mới.

PowerShell cửa sổ 1 (Gateway):

```powershell
New-Item -ItemType Directory -Force 'docs\ketqua\PBL3-16_Clients\I16_04'
.\docs\ketqua\PBL3-10_Build\I10_01\bin\gateway.exe --scenario I16_LOAD1 --run-id I16_04 --urb 0 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-16_Clients\I16_04\gateway.log'
```

PowerShell cửa sổ 2, sau khi Gateway học UAV 1:

```powershell
$run = 'docs\ketqua\PBL3-16_Clients\I16_04'
g++ -std=c++17 -O2 tools\multi_client_load.cpp -o "$run\multi_client_load.exe" -lws2_32
& ".\$run\multi_client_load.exe" 1 600 2>&1 | Tee-Object -FilePath "$run\client_load.log"
"exit_code=$LASTEXITCODE" | Tee-Object -FilePath "$run\client_load.log" -Append
```

PowerShell cửa sổ 3, chạy lấy 11 mẫu tài nguyên trong lúc helper đo:

```powershell
$resourceLog = 'docs\ketqua\PBL3-16_Clients\I16_04\resources.csv'
'timestamp,process,id,cpu_seconds,working_set_bytes,private_bytes,handles' | Tee-Object -FilePath $resourceLog
for ($i = 0; $i -le 10; $i++) {
  $stamp = Get-Date -Format o
  Get-Process gateway,multi_client_load -ErrorAction SilentlyContinue | ForEach-Object {
    "$stamp,$($_.ProcessName),$($_.Id),$($_.CPU),$($_.WorkingSet64),$($_.PrivateMemorySize64),$($_.HandleCount)"
  } | Tee-Object -FilePath $resourceLog -Append
  if ($i -lt 10) { Start-Sleep -Seconds 60 }
}
```

Khi `client_load.log` in `CLIENT LOAD RESULT ... pass=1`, exit 0, đợi Gateway có `QOS t=600` và Markov summary rồi Ctrl+C Gateway. Kiểm trong Gateway log `Clients=1` khi đo, CRC/BLOCK/slow=0. Gửi ba log. Sau khi đối chiếu, dùng cùng helper và PX4 stream cho mức 5 và 10 client, mỗi mức một run ID riêng.

### I16_05: tải 5 client trong 600 giây

Giữ đúng PX4 UAV 1 và stream 14600 như I16_04. Đóng Gateway/client cũ, không mở QGC. Dùng lại executable `multi_client_load.exe` đã build ở I16_04 để giữ cùng cách đếm; không sửa source giữa các mức tải.

PowerShell cửa sổ 1:

```powershell
New-Item -ItemType Directory -Force 'docs\ketqua\PBL3-16_Clients\I16_05'
.\docs\ketqua\PBL3-10_Build\I10_01\bin\gateway.exe --scenario I16_LOAD5 --run-id I16_05 --urb 0 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-16_Clients\I16_05\gateway.log'
```

PowerShell cửa sổ 2, sau khi Gateway ghi UAV 1 ONLINE:

```powershell
.\docs\ketqua\PBL3-16_Clients\I16_04\multi_client_load.exe 5 600 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-16_Clients\I16_05\client_load.log'
"exit_code=$LASTEXITCODE" | Tee-Object -FilePath 'docs\ketqua\PBL3-16_Clients\I16_05\client_load.log' -Append
```

PowerShell cửa sổ 3 (11 mẫu tài nguyên, chạy song song):

```powershell
$resourceLog = 'docs\ketqua\PBL3-16_Clients\I16_05\resources.csv'
'timestamp,process,id,cpu_seconds,working_set_bytes,private_bytes,handles' | Tee-Object -FilePath $resourceLog
for ($i = 0; $i -le 10; $i++) {
  $stamp = Get-Date -Format o
  Get-Process gateway,multi_client_load -ErrorAction SilentlyContinue | ForEach-Object {
    "$stamp,$($_.ProcessName),$($_.Id),$($_.CPU),$($_.WorkingSet64),$($_.PrivateMemorySize64),$($_.HandleCount)"
  } | Tee-Object -FilePath $resourceLog -Append
  if ($i -lt 10) { Start-Sleep -Seconds 60 }
}
```

Khi helper kết thúc sau warm-up 10 s + 600 s, cần thấy `clients=5`, năm dòng client có HEARTBEAT, `disconnected=0`, `errors=0`, `pass=1`, exit 0. Gateway phải giữ `Clients=5` trong cửa sổ đo, CRC/BLOCK/Slow clients=0, ingress=egress và Markov drop=0. Chờ Gateway ghi mốc 600 s rồi Ctrl+C. Gửi ba log để đối chiếu trước khi chạy mức 10 client.

### I16_06: tải 10 client trong 600 giây

Giữ cùng PX4 UAV 1/stream 14600 và executable Gateway/helper của I16_04–05. Đóng Gateway và mọi client của run trước; không mở QGC. Lượt này đo ổn định ở đúng giới hạn 10 client, không thử thêm client thứ 11 (I16_03 đã kiểm việc từ chối).

PowerShell cửa sổ 1:

```powershell
New-Item -ItemType Directory -Force 'docs\ketqua\PBL3-16_Clients\I16_06'
.\docs\ketqua\PBL3-10_Build\I10_01\bin\gateway.exe --scenario I16_LOAD10 --run-id I16_06 --urb 0 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-16_Clients\I16_06\gateway.log'
```

PowerShell cửa sổ 2, sau khi Gateway ghi UAV 1 ONLINE:

```powershell
.\docs\ketqua\PBL3-16_Clients\I16_04\multi_client_load.exe 10 600 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-16_Clients\I16_06\client_load.log'
"exit_code=$LASTEXITCODE" | Tee-Object -FilePath 'docs\ketqua\PBL3-16_Clients\I16_06\client_load.log' -Append
```

PowerShell cửa sổ 3 (11 mẫu tài nguyên trong lúc đo):

```powershell
$resourceLog = 'docs\ketqua\PBL3-16_Clients\I16_06\resources.csv'
'timestamp,process,id,cpu_seconds,working_set_bytes,private_bytes,handles' | Tee-Object -FilePath $resourceLog
for ($i = 0; $i -le 10; $i++) {
  $stamp = Get-Date -Format o
  Get-Process gateway,multi_client_load -ErrorAction SilentlyContinue | ForEach-Object {
    "$stamp,$($_.ProcessName),$($_.Id),$($_.CPU),$($_.WorkingSet64),$($_.PrivateMemorySize64),$($_.HandleCount)"
  } | Tee-Object -FilePath $resourceLog -Append
  if ($i -lt 10) { Start-Sleep -Seconds 60 }
}
```

Khi helper in 10 dòng client và `CLIENT LOAD RESULT ... clients=10 measure_s=600 ... pass=1`, exit 0, đợi Gateway có QOS mốc 600 s và Markov summary rồi Ctrl+C. Gateway cần duy trì `Clients=10` suốt cửa sổ đo, CRC/BLOCK/Slow clients=0, ingress=egress, drop=0. Gửi ba log; file thô sẽ được chuyển vào I16_06 sau khi các tiến trình dừng.
## PBL3-17: hàng đợi UDP và route

### I17_01: hủy gói đang chờ khi TCP client ngắt

**Mục đích:** chứng minh một lệnh đã vào hàng đợi UDP nhưng bị hủy khi client gửi lệnh đóng kết nối. Đây là phép thử có chủ ý dùng `--test-hold-udp-ms 10000`: Gateway giữ gói tối đa 10 giây để nhánh hủy có thể quan sát. Không dùng tùy chọn này cho phép đo hiệu năng thông thường. Lệnh probe chỉ yêu cầu `AUTOPILOT_VERSION` (MAV_CMD_REQUEST_MESSAGE=512), không Arm hay điều khiển bay. Kỳ vọng probe hết hạn sau 5 giây và tự ngắt TCP.

Chuẩn bị một PX4 UAV SYSID 1 đang chạy trên mặt đất, stream từ cổng 14600 đến Gateway cổng 14550 như E0_02. Đóng Gateway cũ, QGC và client cũ. Chạy trong PowerShell tại thư mục gốc dự án. Không chạy lại cùng `--run-id` nếu đã tạo file thô; báo tôi để đặt ID mới.

PowerShell cửa sổ 1, biên dịch mã Gateway hiện tại và khởi động đo:

```powershell
New-Item -ItemType Directory -Force 'docs\ketqua\PBL3-17_Queue\I17_01'
g++ -std=c++17 -O2 src\gateway.cpp -o docs\ketqua\PBL3-17_Queue\I17_01\gateway_queue.exe -lws2_32 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-17_Queue\I17_01\build.log'
$buildExit = $LASTEXITCODE
"gateway_build_exit=$buildExit" | Tee-Object -FilePath 'docs\ketqua\PBL3-17_Queue\I17_01\build.log' -Append
if ($buildExit -ne 0) { throw 'Gateway build failed' }
.\docs\ketqua\PBL3-17_Queue\I17_01\gateway_queue.exe --scenario I17_CANCEL_CLIENT --run-id I17_01 --urb 0 --test-hold-udp-ms 10000 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-17_Queue\I17_01\gateway.log'
```

Sau khi cửa sổ 1 in `UAV 1: ONLINE`, PowerShell cửa sổ 2:

```powershell
.\docs\ketqua\PBL3-16_Clients\I16_01\route_probe.exe 1 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-17_Queue\I17_01\probe.log'
"probe_exit=$LASTEXITCODE" | Tee-Object -FilePath 'docs\ketqua\PBL3-17_Queue\I17_01\probe.log' -Append
```

Đợi Gateway in một mẫu `=== STATS ===` sau dòng `QUEUE CANCEL`, rồi Ctrl+C ở cửa sổ 1. Gửi `build.log`, `gateway.log`, `probe.log`; tôi sẽ kiểm và chuyển các CSV thô vào I17_01 sau khi Gateway dừng. Dấu hiệu đúng: probe có `TIMEOUT` và `probe_exit=1` **do cố tình giữ gói**; Gateway có `QUEUE: target_system=1 ... queued=1`, `Client #... DISCONNECTED`, `QUEUE CANCEL: reason=client_disconnected count=1`; STATS sau đó `UDP queued=0 | UDP dropped/cancelled=1`. Trong lượt này `UDP TX frames=0` nếu không có client khác. Nếu xuất hiện ACK/`probe_exit=0` hoặc không có `QUEUE CANCEL`, giữ nguyên log và báo tôi, không tự ghi PASS.

### I17_02C: hủy gói khi route UAV hết hiệu lực

Lượt này giữ gói tối đa 300 giây và giữ probe mở 180 giây để có đủ thời gian dừng luồng UAV trước khi client ngắt. Hai giới hạn chỉ dùng cho phép thử queue; build Gateway/probe từ mã hiện tại vào thư mục I17_02C. Giữ QGC đóng. Nếu UAV 1 đã dừng, khởi động lại PX4 SYSID 1 và stream 14600 như E0_02 trước khi chạy Gateway.

PowerShell cửa sổ 1:

```powershell
New-Item -ItemType Directory -Force 'docs\ketqua\PBL3-17_Queue\I17_02C'
g++ -std=c++17 -O2 src\gateway.cpp -o docs\ketqua\PBL3-17_Queue\I17_02C\gateway_queue.exe -lws2_32 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-17_Queue\I17_02C\gateway_build.log'
$buildExit = $LASTEXITCODE
"gateway_build_exit=$buildExit" | Tee-Object -FilePath 'docs\ketqua\PBL3-17_Queue\I17_02C\gateway_build.log' -Append
if ($buildExit -ne 0) { throw 'Gateway build failed' }
.\docs\ketqua\PBL3-17_Queue\I17_02C\gateway_queue.exe --scenario I17_CANCEL_ROUTE --run-id I17_02C --urb 0 --test-hold-udp-ms 300000 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-17_Queue\I17_02C\gateway.log'
```

Sau khi Gateway in `UAV 1: ONLINE`, PowerShell cửa sổ 2:

```powershell
g++ -std=c++17 -O2 tools\route_probe.cpp -o docs\ketqua\PBL3-17_Queue\I17_02C\route_probe_wait.exe -lws2_32 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-17_Queue\I17_02C\probe_build.log'
$buildExit = $LASTEXITCODE
"probe_build_exit=$buildExit" | Tee-Object -FilePath 'docs\ketqua\PBL3-17_Queue\I17_02C\probe_build.log' -Append
if ($buildExit -ne 0) { throw 'Probe build failed' }
.\docs\ketqua\PBL3-17_Queue\I17_02C\route_probe_wait.exe 1 180 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-17_Queue\I17_02C\probe.log'
"probe_exit=$LASTEXITCODE" | Tee-Object -FilePath 'docs\ketqua\PBL3-17_Queue\I17_02C\probe.log' -Append
```

Ngay sau dòng `QUEUE: target_system=1 hold_ms=300000 queued=1`, **kết thúc tiến trình PX4 UAV 1 trong cửa sổ WSL của chính UAV 1**. Kiểm `UDP RX datagrams` ngừng tăng; chờ `UAV 1 OFFLINE` và `QUEUE CANCEL: reason=route_offline sysid=1 count=1` xuất hiện **trước** `Client #1 DISCONNECTED`. Sau khi STATS ghi `UDP queued=0 | UDP dropped/cancelled=1`, để probe kết thúc rồi Ctrl+C Gateway. Gửi bốn log (`gateway_build.log`, `probe_build.log`, `gateway.log`, `probe.log`). Nếu không thể dừng luồng PX4 trước 180 giây, giữ log và báo tình trạng cửa sổ PX4 để chọn cách ngắt stream khác.

### I17_03: hàng đợi UDP đạt giới hạn 512 gói

**Mục đích:** kiểm Gateway từ chối lệnh thứ 513 khi hàng đợi có 512 gói, ngắt client gây tràn và hủy 512 gói đã xếp hàng. Probe chỉ gửi MAV_CMD_REQUEST_MESSAGE=512 để yêu cầu thông tin; nó không Arm hoặc điều khiển bay. Gateway dùng `--test-hold-udp-ms 300000` để queue không tự rút trước khi đạt giới hạn. Chế độ giữ này chỉ dùng cho phép thử nhánh, không dùng khi đo hiệu năng.

Khởi động PX4 UAV 1 SYSID 1 và stream cổng 14600 tới Gateway cổng 14550 như E0_02. Đóng QGC, Gateway/client cũ. Chạy PowerShell tại thư mục gốc dự án. Không chạy lại cùng run ID nếu đã có file thô; báo tôi để đặt ID mới.

PowerShell cửa sổ 1:

```powershell
New-Item -ItemType Directory -Force 'docs\ketqua\PBL3-17_Queue\I17_03'
g++ -std=c++17 -O2 src\gateway.cpp -o docs\ketqua\PBL3-17_Queue\I17_03\gateway_queue.exe -lws2_32 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-17_Queue\I17_03\gateway_build.log'
$buildExit = $LASTEXITCODE
"gateway_build_exit=$buildExit" | Tee-Object -FilePath 'docs\ketqua\PBL3-17_Queue\I17_03\gateway_build.log' -Append
if ($buildExit -ne 0) { throw 'Gateway build failed' }
.\docs\ketqua\PBL3-17_Queue\I17_03\gateway_queue.exe --scenario I17_QUEUE_FULL --run-id I17_03 --urb 0 --test-hold-udp-ms 300000 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-17_Queue\I17_03\gateway.log'
```

Khi Gateway ghi `UAV 1: ONLINE`, PowerShell cửa sổ 2:

```powershell
g++ -std=c++17 -O2 tools\queue_fill_probe.cpp -o docs\ketqua\PBL3-17_Queue\I17_03\queue_fill_probe.exe -lws2_32 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-17_Queue\I17_03\probe_build.log'
$buildExit = $LASTEXITCODE
"probe_build_exit=$buildExit" | Tee-Object -FilePath 'docs\ketqua\PBL3-17_Queue\I17_03\probe_build.log' -Append
if ($buildExit -ne 0) { throw 'Probe build failed' }
.\docs\ketqua\PBL3-17_Queue\I17_03\queue_fill_probe.exe 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-17_Queue\I17_03\probe.log'
"probe_exit=$LASTEXITCODE" | Tee-Object -FilePath 'docs\ketqua\PBL3-17_Queue\I17_03\probe.log' -Append
```

Đợi Gateway in một mẫu STATS sau `QUEUE FULL`/`QUEUE CANCEL`, rồi Ctrl+C Gateway. Gửi bốn log (`gateway_build.log`, `probe_build.log`, `gateway.log`, `probe.log`); tôi sẽ chuyển CSV thô vào I17_03 và đối chiếu. Kỳ vọng probe có `frames_sent=513`, `peer_closed=1`, `probe_exit=0`. Gateway phải ghi `QUEUE FULL: ... queued=512 incoming=1 limit=512`, sau đó `Client #1 DISCONNECTED` và `QUEUE CANCEL: reason=client_disconnected count=512`; STATS sau đó `Client RX=513`, `UDP queued=0 | UDP dropped/cancelled=512`, `UDP TX frames=0`, CRC/signature errors=0. Bộ đếm hủy là 512 vì lệnh thứ 513 bị từ chối trước khi vào queue. Nếu khác các mốc này, giữ log và báo tôi để phân tích, không tự ghi PASS.

### I17_04: lỗi hệ thống khi gọi `sendto` và phục hồi

**Mục đích:** kiểm nhánh xử lý `sendto` thất bại và lần gửi kế tiếp thành công. Chế độ thử `--test-sendto-invalid-addrlen-once 1` gọi `sendto` lần đầu với `addrlen=0`, để Winsock trả lỗi thật từ lời gọi; Gateway không tự cộng bộ đếm giả. Lần gọi thứ hai dùng địa chỉ hợp lệ. Đây là phép tiêm lỗi có kiểm soát, không phải đo xác suất lỗi mạng tự nhiên. Hai probe chỉ gửi MAV_CMD_REQUEST_MESSAGE=512, không Arm hay điều khiển bay.

Khởi động PX4 UAV 1 SYSID 1, stream từ 14600 tới Gateway 14550 như E0_02. Đóng QGC, Gateway/client cũ. Chạy PowerShell tại thư mục gốc dự án.

PowerShell cửa sổ 1:

```powershell
New-Item -ItemType Directory -Force 'docs\ketqua\PBL3-17_Queue\I17_04'
g++ -std=c++17 -O2 src\gateway.cpp -o docs\ketqua\PBL3-17_Queue\I17_04\gateway_sendto.exe -lws2_32 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-17_Queue\I17_04\gateway_build.log'
$buildExit = $LASTEXITCODE
"gateway_build_exit=$buildExit" | Tee-Object -FilePath 'docs\ketqua\PBL3-17_Queue\I17_04\gateway_build.log' -Append
if ($buildExit -ne 0) { throw 'Gateway build failed' }
.\docs\ketqua\PBL3-17_Queue\I17_04\gateway_sendto.exe --scenario I17_SENDTO --run-id I17_04 --urb 0 --test-sendto-invalid-addrlen-once 1 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-17_Queue\I17_04\gateway.log'
```

Khi Gateway ghi `UAV 1: ONLINE`, PowerShell cửa sổ 2 chạy probe lần 1:

```powershell
.\docs\ketqua\PBL3-16_Clients\I16_01\route_probe.exe 1 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-17_Queue\I17_04\probe_first.log'
"probe_first_exit=$LASTEXITCODE" | Tee-Object -FilePath 'docs\ketqua\PBL3-17_Queue\I17_04\probe_first.log' -Append
```

Đợi Gateway có một mẫu STATS sau dòng `sendto loi`, rồi cùng cửa sổ 2 chạy probe lần 2:

```powershell
.\docs\ketqua\PBL3-16_Clients\I16_01\route_probe.exe 1 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-17_Queue\I17_04\probe_second.log'
"probe_second_exit=$LASTEXITCODE" | Tee-Object -FilePath 'docs\ketqua\PBL3-17_Queue\I17_04\probe_second.log' -Append
```

Đợi thêm một mẫu STATS, rồi Ctrl+C Gateway. Gửi `gateway_build.log`, `gateway.log`, `probe_first.log`, `probe_second.log`. Kỳ vọng probe 1 `TIMEOUT`/exit 1; Gateway ghi `TEST SENDTO: ... addrlen=0`, `sendto loi: <mã Winsock>`, `UDP dropped/cancelled=1`, `UDP TX frames=0`. Probe 2 phải có `ACK: sysid=1 command=512 result=0`/exit 0; STATS sau đó `Client RX=2`, `UDP TX frames=1`, `UDP dropped/cancelled=1`, queue=0, CRC/signature errors=0. Không giả định mã lỗi Winsock cố định; lưu đúng số thực tế từ log. Nếu probe 1 có ACK hoặc probe 2 không có ACK, giữ log và báo tôi.

### I17_05: lệnh cũ không sang endpoint mới sau khi route phục hồi

**Mục đích:** bổ sung điều kiện D4: request đã xếp hàng ở endpoint cũ `14600` bị hủy khi route hết hạn, rồi UAV cùng SYSID trở lại ở `14602` trong cùng phiên Gateway; request cũ không được gửi sau khi route mới xuất hiện. Gateway giữ UDP 300 giây, probe giữ TCP 180 giây. Không mở QGC hoặc client khác. Đây là phép thử hàng đợi có kiểm soát.

Khởi động PX4 UAV 1 SYSID 1 và stream `14600 → 14550` như E0_02. Đóng Gateway cũ. PowerShell cửa sổ 1 tại thư mục gốc dự án:

```powershell
New-Item -ItemType Directory -Force 'docs\ketqua\PBL3-17_Queue\I17_05'
.\docs\ketqua\PBL3-17_Queue\I17_02C\gateway_queue.exe --scenario I17_ENDPOINT_CHANGE --run-id I17_05 --urb 0 --test-hold-udp-ms 300000 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-17_Queue\I17_05\gateway.log'
```

Khi Gateway ghi UAV 1 ONLINE tại `:14600`, PowerShell cửa sổ 2:

```powershell
.\docs\ketqua\PBL3-17_Queue\I17_02C\route_probe_wait.exe 1 180 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-17_Queue\I17_05\old_probe.log'
"old_probe_exit=$LASTEXITCODE" | Tee-Object -FilePath 'docs\ketqua\PBL3-17_Queue\I17_05\old_probe.log' -Append
```

Khi Gateway ghi `QUEUE: target_system=1 hold_ms=300000 queued=1`, dừng **tiến trình PX4 UAV 1** trong cửa sổ WSL của nó. Chờ Gateway ghi `UAV 1 OFFLINE` và `QUEUE CANCEL: reason=route_offline sysid=1 count=1`; STATS phải có `Clients=1`, `UDP queued=0 | UDP dropped/cancelled=1`, `UDP TX frames=0`.

Sau đó, **trước khi old probe hết 180 giây**, khởi động lại PX4 UAV 1 trong WSL (cùng SYSID 1). Tại thư mục `~/PX4-Autopilot` dùng lệnh PX4 đã lưu từ E0_02:

```bash
PX4_SYS_AUTOSTART=4001 PX4_GZ_MODEL_POSE="0,0,0,0,0,0" ./build/px4_sitl_default/bin/px4 -i 0
```

Trong dấu nhắc `pxh>` của PX4 vừa khởi động, tạo **chỉ** stream thử mới từ cổng `14602`:

```text
mavlink start -u 14602 -o 14550 -t 172.26.160.1 -r 100000
```

Chờ Gateway ghi `UAV 1 ONLINE tai ...:14602` trong khi `Clients=1`. Đợi một mẫu STATS sau ONLINE: `UDP queued=0`, `UDP TX frames=0`, `UDP dropped/cancelled=1`. Để old probe tự TIMEOUT/exit 1; nó không được nhận ACK của request cũ. Sau đó Ctrl+C Gateway. Gửi `gateway.log`, `old_probe.log` và kết quả `mavlink status` của instance cổng 14602. Nếu probe ngắt trước khi UAV 1 ONLINE ở 14602, hoặc `UDP TX frames` tăng, giữ log và báo tôi; chưa tính đạt điều kiện endpoint mới.

## PBL3-18: mô hình Markov độc lập, không cần PX4

### I18_01: ba seed, 180000 slot mỗi seed

**Mục đích:** kiểm mô hình Markov trực tiếp bằng thời gian mô phỏng, không dùng luồng PX4 hoặc Gateway. Công cụ in tham số, tỷ lệ slot B, độ dài trung bình đợt B, frame kept/dropped và dấu vân tay chuỗi theo seed. Nó còn kiểm cùng seed tái lập, SYSID khác có chuỗi riêng, tick từng slot bằng tick nhảy thời gian, `urb=0` không drop, và năm cấu hình sai bị từ chối. Không tạo sẵn file đo.

PowerShell tại thư mục gốc dự án (có thể đóng PX4, QGC và Gateway):

```powershell
New-Item -ItemType Directory -Force 'docs\ketqua\PBL3-18_Markov\I18_01'
g++ -std=c++17 -O2 tools\markov_model_check.cpp -o docs\ketqua\PBL3-18_Markov\I18_01\markov_model_check.exe 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-18_Markov\I18_01\build.log'
$buildExit = $LASTEXITCODE
"build_exit=$buildExit" | Tee-Object -FilePath 'docs\ketqua\PBL3-18_Markov\I18_01\build.log' -Append
if ($buildExit -ne 0) { throw 'Markov build failed' }
.\docs\ketqua\PBL3-18_Markov\I18_01\markov_model_check.exe 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-18_Markov\I18_01\model.log'
"model_exit=$LASTEXITCODE" | Tee-Object -FilePath 'docs\ketqua\PBL3-18_Markov\I18_01\model.log' -Append
```

Kỳ vọng `build_exit=0`, `model_exit=0`, `MARKOV MODEL RESULT: pass=1 seeds=3 slots_per_seed=180000`. Mỗi seed 1/2/3 phải có `slots=180000`, `frames_seen=frames_kept+frames_dropped`; `bad_fraction` và `drop_fraction` gần 0.5, `mean_bad_s` gần 2.0. Dòng CHECK phải có `repeat_same_seed=1`, `independent_sysid=1`, `distinct_seeds=1`, `tick_step_equals_jump=1`, `urb_zero_keeps_all=1`, `invalid_configs_rejected=5/5`. Gửi `build.log` và `model.log`; tôi sẽ đối chiếu số liệu thực trước khi sang PBL3-19.

## PBL3-19: Markov ghép vào Gateway

### I19_01: Markov ON, một UAV, một client, seed 1

**Mục đích:** sau [E0_02 Markov OFF](ketqua/PBL3-14_E0/E0_02/summary.md), kiểm Markov ON trong Gateway với cùng một nguồn PX4 và một TCP client. Gateway ghi chung cửa sổ ingress/egress 600 giây; kiểm `frames_in=frames_kept+frames_dropped_emu`, ingress=frames_in và egress=frames_kept. Chạy seed 1 trước, đối chiếu log rồi mới làm seed tiếp theo. `run_slots` trong Markov summary hiện tính cả 10 giây warm-up; số liệu frame/CSV 600 giây mới là mẫu dùng để đánh giá run. Ghi rõ giới hạn này, không so trực tiếp `run_slots` với 6000 slot.

Khởi động PX4 UAV 1 SYSID 1, stream từ `14600 → 14550` như E0_02. Đóng QGC, Gateway và client cũ; không bật UAV 2. Chạy PowerShell tại thư mục gốc dự án.

PowerShell cửa sổ 1:

```powershell
New-Item -ItemType Directory -Force 'docs\ketqua\PBL3-19_GatewayMarkov\I19_01'
g++ -std=c++17 -O2 src\gateway.cpp -o docs\ketqua\PBL3-19_GatewayMarkov\I19_01\gateway.exe -lws2_32 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-19_GatewayMarkov\I19_01\gateway_build.log'
$buildExit = $LASTEXITCODE
"gateway_build_exit=$buildExit" | Tee-Object -FilePath 'docs\ketqua\PBL3-19_GatewayMarkov\I19_01\gateway_build.log' -Append
if ($buildExit -ne 0) { throw 'Gateway build failed' }
.\docs\ketqua\PBL3-19_GatewayMarkov\I19_01\gateway.exe --scenario I19_ON --run-id I19_01 --urb 0.5 --dur-s 2 --slot-ms 100 --lossgood 0 --lossbad 1 --seed 1 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-19_GatewayMarkov\I19_01\gateway.log'
```

Sau khi Gateway ghi `UAV 1: ONLINE`, PowerShell cửa sổ 2:

```powershell
g++ -std=c++17 -O2 src\tcpclient.cpp -o docs\ketqua\PBL3-19_GatewayMarkov\I19_01\tcpclient.exe -lws2_32 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-19_GatewayMarkov\I19_01\client_build.log'
$buildExit = $LASTEXITCODE
"client_build_exit=$buildExit" | Tee-Object -FilePath 'docs\ketqua\PBL3-19_GatewayMarkov\I19_01\client_build.log' -Append
if ($buildExit -ne 0) { throw 'Client build failed' }
.\docs\ketqua\PBL3-19_GatewayMarkov\I19_01\tcpclient.exe 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-19_GatewayMarkov\I19_01\client.log'
```

Để Gateway và client chạy qua đủ warm-up 10 giây và cửa sổ 600 giây. **Không dừng Gateway khi vừa thấy client QOS `t=600`;** chờ Gateway cũng in `QOS: t=600` và `Issue19: da ghi ..._markov.csv`. Sau khi cả hai điểm đã tới 600 giây, Ctrl+C client trước rồi Ctrl+C Gateway. Gửi bốn log build/Gateway/client; tôi sẽ chuyển CSV thô từ `docs/ketqua/issue19` và CSV client từ `docs/ketqua` vào thư mục I19_01 sau khi tiến trình dừng.

Kỳ vọng Gateway có 120 mốc ingress và egress đến 600 giây, cùng SYSID 1; ingress nhiều frame hơn egress, Markov drop >0, `frames_in=frames_kept+frames_dropped_emu`, `ingress_qos_frames=frames_in`, `egress_qos_frames=frames_kept`. CRC/signature errors=0, BLOCK/queue/sendto/slow client=0. Client vẫn kết nối, nhận được HEARTBEAT; có thể có đợt thiếu heartbeat do kênh B. Không yêu cầu tỷ lệ drop chính xác 50% ở một run 600 giây.

### I19_02: Markov ON, cùng cấu hình, seed 2

**Mục đích:** lặp cấu hình ON của I19_01 với seed 2 và **cùng hai executable đã build ở I19_01**. So sánh hai seed bằng CSV trạng thái và Markov summary, không đổi tham số, nguồn PX4, số client hoặc mã giữa hai lượt. Đây là một run 600 giây mới, không gộp số liệu vào I19_01.

Giữ PX4 UAV 1 SYSID 1 stream từ 14600 tới Gateway 14550. Đóng Gateway và client cũ, QGC và UAV 2. PowerShell cửa sổ 1:

```powershell
New-Item -ItemType Directory -Force 'docs\ketqua\PBL3-19_GatewayMarkov\I19_02'
.\docs\ketqua\PBL3-19_GatewayMarkov\I19_01\gateway.exe --scenario I19_ON --run-id I19_02 --urb 0.5 --dur-s 2 --slot-ms 100 --lossgood 0 --lossbad 1 --seed 2 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-19_GatewayMarkov\I19_02\gateway.log'
```

Sau khi Gateway ghi `UAV 1: ONLINE`, PowerShell cửa sổ 2:

```powershell
.\docs\ketqua\PBL3-19_GatewayMarkov\I19_01\tcpclient.exe 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-19_GatewayMarkov\I19_02\client.log'
```

Đợi Gateway **và client** đều ghi `QOS: t=600`, Gateway ghi `Issue19: da ghi ..._markov.csv`. Ctrl+C client rồi Gateway, gửi hai log. Tôi sẽ đối chiếu và chuyển CSV thô sau khi chúng đã dừng. Kỳ vọng mỗi Gateway ingress/egress có 120 mốc; `frames_in=frames_kept+frames_dropped_emu`, ingress=frames_in, egress=frames_kept, CRC/signature/BLOCK/queue/slow=0. Chỉ số drop/đợt B có thể khác I19_01; đó là lý do đổi seed. `run_slots` vẫn gồm 10 giây warm-up; phân tích cửa sổ 600 giây từ CSV và trạng thái.

### I19_03: Markov ON, cùng cấu hình, seed 3

**Mục đích:** lượt ON thứ ba để có đủ ba seed độc lập trong Gateway trước khi chuyển sang OUTAGE. Dùng **cùng executable Gateway/client của I19_01** và một PX4 UAV 1 stream 14600. Chỉ đổi run ID và seed; đóng QGC, UAV 2, Gateway/client cũ.

PowerShell cửa sổ 1 tại thư mục gốc dự án:

```powershell
New-Item -ItemType Directory -Force 'docs\ketqua\PBL3-19_GatewayMarkov\I19_03'
.\docs\ketqua\PBL3-19_GatewayMarkov\I19_01\gateway.exe --scenario I19_ON --run-id I19_03 --urb 0.5 --dur-s 2 --slot-ms 100 --lossgood 0 --lossbad 1 --seed 3 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-19_GatewayMarkov\I19_03\gateway.log'
```

Khi Gateway ghi `UAV 1: ONLINE`, PowerShell cửa sổ 2:

```powershell
.\docs\ketqua\PBL3-19_GatewayMarkov\I19_01\tcpclient.exe 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-19_GatewayMarkov\I19_03\client.log'
```

Đợi cả hai tới `QOS: t=600` và Gateway ghi file `_markov.csv`; Ctrl+C client rồi Gateway. Gửi `gateway.log`, `client.log`. Tôi sẽ kiểm tổng MSGID, `frames_in=kept+dropped_emu`, ingress/egress và trạng thái B trong đúng cửa sổ 600 giây trước khi hướng dẫn OUTAGE. Không yêu cầu tỷ lệ drop của seed 3 bằng seed 1 hoặc 2.

### I19_04: OUTAGE, đợt B dài 5 giây, seed 1

**Mục đích:** giữ `urb=0.5` nhưng tăng `dur_s` từ 2 lên 5 giây để kiểm các đợt mất telemetry dài hơn khi Markov được ghép vào Gateway. Ingress từ PX4 phải tiếp tục online, còn egress/client có thể offline trong những đợt B. Đây là lượt OUTAGE đầu; kiểm log trước khi chạy seed tiếp theo.

Giữ một PX4 UAV 1 SYSID 1 stream `14600 → 14550`; đóng Gateway/client cũ, QGC và UAV 2. Dùng lại **cùng executable Gateway/client đã build tại I19_01**. PowerShell cửa sổ 1 tại thư mục gốc dự án:

```powershell
New-Item -ItemType Directory -Force 'docs\ketqua\PBL3-19_GatewayMarkov\I19_04'
.\docs\ketqua\PBL3-19_GatewayMarkov\I19_01\gateway.exe --scenario I19_OUTAGE --run-id I19_04 --urb 0.5 --dur-s 5 --slot-ms 100 --lossgood 0 --lossbad 1 --seed 1 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-19_GatewayMarkov\I19_04\gateway.log'
```

Khi Gateway ghi `UAV 1: ONLINE`, PowerShell cửa sổ 2:

```powershell
.\docs\ketqua\PBL3-19_GatewayMarkov\I19_01\tcpclient.exe 2>&1 | Tee-Object -FilePath 'docs\ketqua\PBL3-19_GatewayMarkov\I19_04\client.log'
```

Đợi **cả Gateway và client** ghi `QOS: t=600`, Gateway ghi file `_markov.csv`; Ctrl+C client trước rồi Gateway. Gửi `gateway.log` và `client.log`. Tôi sẽ đối chiếu config, ingress/egress/Markov summary, trạng thái B và offline event từ CSV. Kỳ vọng `frames_in=kept+dropped_emu`, ingress QoS=frames_in, egress QoS=kept; ingress offline=0, egress có offline event; CRC/signature/BLOCK/queue/slow=0. Không đặt ngưỡng offline event hay tỷ lệ drop cố định cho một seed. `run_slots` trong summary vẫn gồm warm-up 10 giây; phân tích thời gian B của đúng cửa sổ 600 giây từ `_states.csv`.
