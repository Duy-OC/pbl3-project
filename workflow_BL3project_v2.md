# Workflow v2: MAVLink Gateway (Winsock) + mô phỏng mất gói Markov + đo QoS

Bản này thay thế `workflow_BL3project.md`. Thêm so với bản cũ: **bộ mô phỏng kênh Markov (tốt/xấu)** và **đo QoS** (bước 6, 7, 9), chi tiết hơn ở mọi bước, có số liệu tham chiếu để đối chiếu.

## 1. Mục tiêu và kiến trúc

Server Winsock chạy trên Windows, nhận telemetry từ PX4 SITL (Gazebo trong WSL2) qua UDP, chuyển tiếp qua TCP cho nhiều client, **cố ý làm xấu đường truyền theo mô hình Markov** để mô phỏng UAV bay qua vật cản, và **đo chất lượng đường truyền (QoS)**.

```
PX4 SITL ──UDP 14550──► [1 Ingress: recvfrom, tách frame, CRC]
(WSL2)                        │
                              ├─► [2 QoS ingress: đếm gói, seq, tốc độ]      (đo đường THẬT)
                              ▼
                        [3 Bộ mô phỏng Markov: giữ hay bỏ gói]               (đếm "sự thật" bỏ bao nhiêu)
                              ▼
                        [4 Fan-out TCP 5760: bộ đệm riêng mỗi client] ──► QGroundControl / tcpclient.exe
                              │
                              └─► [5 QoS egress + ghi CSV]                  (đo cái client THẤY)
```

**Cơ sở khoa học:** bài báo *Increasing Reliability on UAV Fading Scenarios* (Viana và cộng sự, IEEE Access 2022) dùng chuỗi Markov hai trạng thái tốt/xấu để mô phỏng vật cản, với hệ số Urb (tỷ lệ thời gian ở trạng thái xấu) bằng 10%, 50%, 60%, và đo PER, throughput, goodput, outage. Bạn chỉ **mượn mô hình kênh và các chỉ số**, không cài lại cân bằng tín hiệu hay công suất của bài (đó là tầng vật lý). Trong báo cáo hãy nói rõ là "lấy cảm hứng từ".

## 2. Quy ước (đọc trước khi làm)

| Cửa sổ | Dấu nhắc | Dùng cho |
|---|---|---|
| Terminal PX4 (WSL2) | `pxh>` | Chỉ lệnh của PX4: `mavlink status`... |
| PowerShell (Windows) | `PS C:\...>` | `g++`, chạy `.exe`, `netstat`, `git`, `python` vẽ biểu đồ |
| QGroundControl | giao diện | Phải **đóng** khi gateway cần chiếm UDP 14550 |

- Sau mỗi bước xong: `git commit` và ghi lỗi + cách sửa vào `docs/nhatky.md`.
- Chỉ sang bước sau khi đạt **Xong khi**.
- Phần **bắt buộc** là bước 0–9 và 11. Bước 10 (vẽ biểu đồ) rất nên làm. Phần **mở rộng** (A, B, C) chỉ làm khi còn thời gian.

## 3. Cấu trúc thư mục

```
BL3project/
├── src/
│   ├── serverUDP.cpp      # Bước 1 (đã có, giữ làm mốc)
│   ├── mavlink.h/.cpp     # Bước 3: tách frame + CRC
│   ├── gateway.cpp        # Bước 4-5: select, UDP/TCP, fan-out
│   ├── qos.h/.cpp         # Bước 6: đo QoS
│   ├── emulator.h/.cpp    # Bước 7: chuỗi Markov
│   ├── markovtest.cpp     # Bước 7: chạy thử riêng bộ Markov, không cần PX4
│   └── tcpclient.cpp      # Bước 5-6: client tự viết, tự đo phía nhận
├── tools/
│   └── plot.py            # Bước 10: vẽ biểu đồ từ CSV
├── docs/
│   ├── nhatky.md
│   ├── bang_msgid.md      # Bước 2
│   └── ketqua/            # file CSV của từng thí nghiệm
├── build.bat
└── README.md
```

`build.bat` (MinGW), một dòng cho cả dự án:

```
g++ -std=c++11 -O2 src\gateway.cpp src\mavlink.cpp src\qos.cpp src\emulator.cpp -o gateway.exe -lws2_32
```

`-lws2_32` ở cuối là bắt buộc. MinGW cũ có thể thiếu `inet_ntop` (dùng `inet_ntoa`), và nên đo thời gian bằng `QueryPerformanceCounter` cho chắc (có trong mọi bản Windows).

## 4. Bảng theo dõi tiến độ

- [ ] Bước 0: Chuẩn bị
- [ ] Bước 1: Nhận UDP từ PX4
- [ ] Bước 2: Khảo sát dữ liệu PX4 (bảng msgid, tốc độ gói)
- [ ] Bước 3: Tách frame và kiểm tra CRC
- [ ] Bước 4: Chuyển tiếp sang một client TCP
- [ ] Bước 5: Nhiều client, bộ đệm gửi riêng
- [ ] Bước 6: Đo QoS (có dòng cơ sở không mất gói)
- [ ] Bước 7: Bộ mô phỏng mất gói Markov
- [ ] Bước 8: Giám sát UAV mất liên kết (outage)
- [ ] Bước 9: Chạy thí nghiệm, ghi CSV
- [ ] Bước 10: Vẽ biểu đồ và phân tích
- [ ] Bước 11: Đóng gói nộp bài
- [ ] (Mở rộng A) Lệnh ngược + heartbeat trạm mặt đất
- [ ] (Mở rộng B) Nhiều UAV
- [ ] (Mở rộng C) Gửi trùng gói kiểu CPC/APC

## 5. Kiến thức nền cần nắm (đọc một lần)

**Chuỗi Markov hai trạng thái (kênh tốt G, kênh xấu B).** Mỗi "khe" thời gian, kênh chuyển trạng thái theo xác suất:
- P_gb: từ tốt sang xấu; P_bg: từ xấu về tốt; P_gg = 1 − P_gb; P_bb = 1 − P_bg.
- **Xác suất dừng** (tỷ lệ thời gian ở mỗi trạng thái): π_B = P_gb / (P_gb + P_bg) và π_G = P_bg / (P_gb + P_bg). π_B chính là **Urb** của bài báo.
- **Thời gian trung bình một đợt xấu** = (độ dài khe) / P_bg.
- **Tỷ lệ mất gói kỳ vọng** = π_B × (xác suất mất khi xấu) + π_G × (xác suất mất khi tốt).
- Xác suất một đợt xấu kéo dài ít nhất x giây ≈ (1 − P_bg)^(x / độ dài khe).

Lưu ý: công thức (7), (8) trong bài báo (P_G = P_gg + P_bg, P_B = P_bb + P_gb) cộng theo cột nên **tổng hai giá trị luôn bằng 2**, không phải xác suất dừng. Khi tính lý thuyết hãy dùng công thức π ở trên. Mình đã kiểm tra bằng mô phỏng: π_B đo được khớp π_B tính theo công thức này (xem bảng ở bước 7).

**Chọn tham số theo ý nghĩa.** Thay vì nhập P_gb, P_bg, bạn nhập hai đại lượng dễ hiểu:
- `urb` (0 đến dưới 1): tỷ lệ thời gian ở trạng thái xấu.
- `dur` (giây): thời gian trung bình một đợt xấu.

Rồi tính: **P_bg = slot / dur**, **P_gb = P_bg × urb / (1 − urb)**. Với slot = 100 ms.

**Vì sao chuỗi chạy theo khe thời gian (100 ms), không theo từng gói.** PX4 gửi vài trăm gói mỗi giây (từ `tx` khoảng 15874 B/s bạn đã thấy, bạn sẽ đo chính xác ở bước 2). Nếu chuỗi chuyển trạng thái theo từng gói, độ dài đợt xấu sẽ phụ thuộc tốc độ gói. Chạy theo thời gian thì "vật cản kéo dài 2 giây" đúng nghĩa 2 giây.

**Hai điểm đo QoS.** Đo ở *ingress* (trước bộ mô phỏng) cho biết đường thật PX4 → gateway (kỳ vọng không mất gói). Đo ở *egress* và ở client cho biết cái client thấy sau khi bị làm xấu. So hai điểm với bộ đếm "sự thật" của bộ mô phỏng để kiểm tra phép đo của bạn đúng.

---

## Bước 0: Chuẩn bị (30 phút)

**Làm**
1. Tạo cấu trúc thư mục mục 3 (file rỗng cũng được).
2. `git init`, commit lần đầu. Tạo `docs/nhatky.md`.
3. Tạo `build.bat` như mục 3.

**Kiểm tra**
- PowerShell: `g++ --version` ra số phiên bản.
- `pxh>`: `mavlink status`, instance #0 có `UDP (18570, remote port: 14550)`.

**Xong khi:** có thư mục, git đã commit, `g++` chạy được.

---

## Bước 1: Nhận UDP từ PX4 (đang làm)

**Làm**
1. Đóng QGroundControl. PowerShell: `netstat -ano | findstr 14550` không in dòng nào (nếu còn, đóng chương trình giữ cổng).
2. Biên dịch và chạy `serverUDP.cpp`: `g++ serverUDP.cpp -o serverUDP.exe -lws2_32` rồi `.\serverUDP.exe` (chạy từ PowerShell, không bấm đúp).
3. Windows hỏi tường lửa: tích cả mạng private và public.

**Kiểm tra:** màn hình in liên tục dạng `172.26.x.x:18570 ... sysid=1 compid=1 msgid=0` (HEARTBEAT) xen `msgid=30` (ATTITUDE). Chụp màn hình.

**Xong khi:** thấy đều đặn `msgid=0` từ `sysid=1`.

| Sự cố | Cách xử lý |
|---|---|
| `bind loi 10048` | Cổng 14550 đang bị giữ, đóng QGC, kiểm tra lại `netstat` |
| Im lặng sau `Dang nghe...` | Thêm luật tường lửa (PowerShell quyền Administrator): `New-NetFirewallRule -DisplayName "UDP 14550" -Direction Inbound -Protocol UDP -LocalPort 14550 -Action Allow`. Kiểm tra `partner IP` trong `mavlink status`, IP Windows trong WSL2 có thể đổi sau khi khởi động lại; nếu lệch thì tắt và chạy lại PX4 |
| `inet_ntop was not declared` | MinGW cũ, dùng `inet_ntoa` |

---

## Bước 2: Khảo sát dữ liệu PX4 (1 buổi)

**Làm:** sửa `serverUDP.cpp` (hoặc bản sao) để đếm theo `msgid` trong 30 giây, in bảng cuối cùng gồm: msgid, số gói, gói/giây, byte/giây. Đồng thời đếm **tổng gói/giây** và **tổng byte/giây**.

**Kiểm tra**
- Tổng byte/giây cùng bậc với `tx` trong `mavlink status` (bạn từng thấy khoảng 15874 B/s).
- HEARTBEAT khoảng 1 gói/giây; ATTITUDE nhiều lần/giây.

**Ghi vào `docs/bang_msgid.md`:** bảng `msgid | tên | gói/giây | byte/giây`. Tra tên trong MAVLink common message set (mavlink.io): 0 HEARTBEAT, 1 SYS_STATUS, 24 GPS_RAW_INT, 30 ATTITUDE, 33 GLOBAL_POSITION_INT...

**Xong khi:** có bảng, và bạn biết **tổng gói/giây** (số này quyết định cách đo ở bước 6).

---

## Bước 3: Tách frame và kiểm tra CRC

**Làm:** viết `mavlink.h/.cpp` với:
- Hàm tách mọi frame MAVLink 2 từ một bộ đệm byte, trả về `sysid`, `compid`, `msgid`, `seq`, vị trí và độ dài frame.
- Hàm kiểm tra CRC.
- Bảng `CRC_EXTRA[msgid]`: copy từ file `crc_extra.py` trong `mavgw.zip` (210 bản tin). Với msgid không có trong bảng thì chấp nhận khung nhưng đếm riêng "không kiểm tra được".

**Cấu trúc frame:** `0xFD | len | incompat | compat | seq | sysid | compid | msgid(3 byte, little-endian) | payload(len) | CRC(2 byte, little-endian) | [chữ ký 13 byte nếu incompat & 1]`. Độ dài cả frame = 12 + len (+13 nếu có chữ ký).

**Thuật toán CRC (CRC-16/X.25):**

```
crc = 0xFFFF
với mỗi byte b trong [len, incompat, compat, seq, sysid, compid, msgid(3), payload...]:
    tmp = b ^ (crc & 0xFF)
    tmp = (tmp ^ (tmp << 4)) & 0xFF
    crc = ((crc >> 8) ^ (tmp << 8) ^ (tmp << 3) ^ (tmp >> 4)) & 0xFFFF
rồi áp dụng thêm một vòng nữa với b = CRC_EXTRA[msgid]
```

**Test vector (mình đã kiểm tra khớp với thư viện pymavlink):** một HEARTBEAT của trạm mặt đất (sysid 255, compid 190, seq 0):

```
fd 09 00 00 00 ff be 00 00 00 00 00 00 00 06 08 00 04 03 3d 48
```

- `len = 9`, `msgid = 0`, payload = `00 00 00 00 06 08 00 04 03`.
- CRC tính trên 18 byte từ `09` đến `03` cùng `CRC_EXTRA[0] = 50` phải bằng **0x483D** (trong frame là hai byte `3d 48`).
- Làm hỏng một byte bất kỳ thì CRC phải sai.
- `CRC_EXTRA[30] = 39` (ATTITUDE), `CRC_EXTRA[33] = 104` (GLOBAL_POSITION_INT).

**Lưu ý khi viết**
- MAVLink 2 **cắt các byte 0 ở cuối payload**, nên khi đọc một trường phải bù 0 cho đủ kích thước bản tin.
- Dùng `memcpy` để đọc số nhiều byte, đừng ép kiểu con trỏ.
- Khi CRC sai: bỏ 1 byte rồi tìm lại `0xFD` tiếp theo (đồng bộ lại).
- Một datagram UDP có thể chứa nhiều frame nối tiếp.

**Kiểm tra**
1. Cho test vector qua parser: nhận ra 1 frame, CRC đúng, `sysid=255 compid=190 msgid=0`.
2. Sửa một byte của test vector: parser báo CRC sai.
3. Chạy với PX4 thật 1 phút: in số CRC đúng và CRC sai mỗi 5 giây.

**Xong khi:** test vector đúng, và với PX4 thật tỷ lệ CRC đúng gần 100% (nếu sai nhiều, nghi `CRC_EXTRA` sai hoặc quên cộng byte này).

---

## Bước 4: Chuyển tiếp sang một client TCP

**Làm:** trong `gateway.cpp`, thêm socket TCP lắng nghe cổng 5760 cùng UDP 14550, dùng `select()`. Mỗi frame hợp lệ (đúng CRC) được ghi cho client. Nhớ gọi `WSAIoctl(SIO_UDP_CONNRESET)` cho socket UDP.

**Kiểm tra**
1. Server chạy: `Test-NetConnection 127.0.0.1 -Port 5760` cho `TcpTestSucceeded : True`.
2. QGroundControl: thêm kết nối **TCP** tới `127.0.0.1:5760`; tắt tùy chọn tự kết nối UDP của QGC nếu nó tranh cổng 14550.

**Xong khi:** QGC hiện UAV, bản đồ, độ cao và tư thế cập nhật liên tục, hoàn toàn qua gateway.

---

## Bước 5: Nhiều client, bộ đệm gửi riêng

**Làm**
1. Gateway giữ danh sách client. Mỗi client có **hàng đợi gửi riêng** (giới hạn khoảng vài trăm gói). `send()` non-blocking có thể gửi một phần hoặc trả `WSAEWOULDBLOCK`: giữ phần còn lại và dùng tập `writefds` của `select`. Hàng đợi đầy thì **bỏ gói cũ nhất** và tăng bộ đếm `client_dropped`.
2. Viết `tcpclient.cpp`: kết nối vào 5760, tự tách frame (dùng lại `mavlink.cpp`), in thống kê mỗi 5 giây (gói/giây, số gói nhận).

**Kiểm tra**
1. QGC và `tcpclient.exe` cùng kết nối, cả hai nhận dữ liệu.
2. Ngắt đột ngột `tcpclient.exe` (Ctrl+C): QGC vẫn chạy, gateway không văng.
3. Mở rồi đóng client 20 lần liên tiếp.

**Xong khi:** thêm bớt client tùy ý mà gateway vẫn ổn, không rò socket (số client in ra đúng).

---

## Bước 6: Đo QoS (phần mới)

**Làm:** viết `qos.h/.cpp` và gọi từ gateway. Mỗi **5 giây** in ra màn hình và ghi một dòng vào CSV `docs/ketqua/<ten_thi_nghiem>.csv`.

**Các chỉ số ở gateway (ingress, trước bộ mô phỏng):**
- `frames_in`, `bytes_in`, `crc_bad`, gói/giây theo `msgid`.
- `seq_lost_ingress`: mất gói theo trường `seq`. **Theo dõi riêng cho từng cặp (sysid, compid)**, vì `seq` là của từng nguồn gửi. Công thức, có quay vòng 0–255: `lost += (seq − last_seq − 1) & 0xFF`. Ví dụ `last = 254`, `seq = 1` cho 2 gói mất.
- `hb_count` và **jitter của HEARTBEAT**: độ lệch chuẩn của khoảng cách giữa hai HEARTBEAT liên tiếp (lý tưởng 1 s), đo bằng `QueryPerformanceCounter`.
- `max_silence_ms`: khoảng lặng lớn nhất giữa hai gói nhận được.

**Các chỉ số ở egress và client:**
- `frames_out`, `bytes_out`, `client_dropped` (bỏ do client chậm).
- Ở `tcpclient.exe`: số gói nhận, `max_silence_ms`, số lần "mất liên kết" (xem bước 8).

**Lưu ý quan trọng về `seq`:** `seq` chỉ có 8 bit, quay vòng sau 256 gói. PX4 gửi hàng trăm gói/giây, nên 256 gói chỉ tương ứng khoảng nửa giây. **Nếu một đợt mất kéo dài từ 256 gói trở lên, công thức trên sẽ đếm thiếu** (nó không phân biệt được mất 10 gói hay 266 gói). Vì vậy:
- Ở ingress (đường thật, kỳ vọng không mất) dùng `seq` bình thường.
- Sau bộ mô phỏng, **chỉ số chuẩn là bộ đếm "sự thật" của bộ mô phỏng** (bước 7), còn `seq` ở client dùng để so sánh và để nhận xét giới hạn này trong báo cáo.

**CSV, các cột gợi ý:** `t_s, scenario, urb, dur_s, lossbad, seed, n_clients, frames_in, frames_dropped_emu, frames_out, bytes_out, crc_bad, seq_lost_ingress, hb_jitter_ms, max_silence_ms, offline_events, offline_time_s, client_dropped`.

**Kiểm tra (cơ sở, chưa có mô phỏng)**
1. Chạy 10 phút với bộ mô phỏng tắt: `seq_lost_ingress` bằng 0 hoặc rất nhỏ, `frames_dropped_emu = 0`.
2. Jitter HEARTBEAT nhỏ so với 1000 ms (ghi lại giá trị để so sánh sau).
3. Tổng byte/giây khớp bước 2.

**Xong khi:** có file CSV cơ sở (`co_so.csv`) và các số ở trên hợp lý. Nếu `seq_lost_ingress` lớn dù chưa mô phỏng, nghi giả định `seq` theo từng (sysid, compid) sai; ghi vào nhật ký và kiểm tra lại.

---

## Bước 7: Bộ mô phỏng mất gói Markov (phần mới)

**Làm:** viết `emulator.h/.cpp`.

**Tham số dòng lệnh của gateway:**

| Tham số | Ý nghĩa | Mặc định |
|---|---|---|
| `--urb X` | Tỷ lệ thời gian ở trạng thái xấu (0 đến dưới 1). 0 = tắt bộ mô phỏng | 0 |
| `--dur S` | Thời gian trung bình một đợt xấu (giây) | 2.0 |
| `--slot MS` | Độ dài khe thời gian (ms) | 100 |
| `--lossbad P` | Xác suất mất gói khi ở trạng thái xấu | 1.0 |
| `--lossgood P` | Xác suất mất gói khi ở trạng thái tốt | 0.0 |
| `--seed N` | Hạt giống bộ sinh số ngẫu nhiên | 1 |

**Thuật toán**
1. Tính `P_bg = slot / (dur × 1000)` và `P_gb = P_bg × urb / (1 − urb)`. Kiểm tra cả hai nằm trong [0, 1], nếu không thì báo lỗi tham số.
2. Khởi tạo trạng thái theo xác suất dừng: xấu với xác suất `urb`.
3. Cập nhật chuỗi theo thời gian, **kiểu bù**: giữ `next_slot_time`; mỗi lần có gói đến, `while (now ≥ next_slot_time) { bước chuỗi; next_slot_time += slot }`. Một bước chuỗi: ở trạng thái tốt thì chuyển sang xấu với xác suất `P_gb`; ở trạng thái xấu thì chuyển về tốt với xác suất `P_bg`.
4. Với mỗi frame hợp lệ từ UAV: `p = (đang xấu) ? lossbad : lossgood`. Sinh số ngẫu nhiên trong [0, 1); nhỏ hơn `p` thì **bỏ frame** (không chuyển cho client), tăng `frames_dropped_emu`.
5. Dùng bộ sinh số cố định theo `--seed` (ví dụ `std::mt19937` với `-std=c++11`; nếu MinGW của bạn thiếu `<random>` thì dùng `srand(seed)` và `rand()`) để chạy lại được đúng kết quả.
6. Bộ đếm "sự thật": `slots_total`, `slots_bad`, `bursts` (số đợt xấu) và độ dài từng đợt, `frames_seen`, `frames_dropped_emu`.

**Giá trị tham số mẫu (slot = 100 ms):**

| Urb | dur (s) | P_bg | P_gb |
|---|---|---|---|
| 0.10 | 2.0 | 0.05 | 0.00556 |
| 0.50 | 2.0 | 0.05 | 0.05 |
| 0.60 | 2.0 | 0.05 | 0.075 |
| 0.50 | 0.5 | 0.20 | 0.20 |
| 0.50 | 5.0 | 0.02 | 0.02 |

Urb = 10%, 50%, 60% là các giá trị bài báo dùng (Table 2 và mục IV).

**Kiểm tra A: chạy riêng bộ Markov, không cần PX4** (`markovtest.cpp`, chạy 180000 slot tương đương 5 giờ mô phỏng). Giá trị tham chiếu mình đã tính bằng mô phỏng Python (bạn nên đạt gần các số này, sai lệch vài phần trăm là bình thường):

| Urb | dur | π_B đo (kỳ vọng) | Đợt xấu trung bình (kỳ vọng) | P(đợt ≥ 3 s) đo (lý thuyết) | Số đợt xấu mỗi giờ |
|---|---|---|---|---|---|
| 0.10 | 0.5 s | 0.096 (0.10) | 0.48 s (0.5) | gần 0 | khoảng 719 |
| 0.10 | 2.0 s | 0.097 (0.10) | 1.93 s (2.0) | 0.207 (0.215) | khoảng 181 |
| 0.50 | 0.5 s | 0.506 (0.50) | 0.51 s (0.5) | gần 0 | khoảng 3601 |
| 0.50 | 2.0 s | 0.500 (0.50) | 2.02 s (2.0) | 0.231 (0.215) | khoảng 892 |
| 0.60 | 0.5 s | 0.600 (0.60) | 0.50 s (0.5) | gần 0 | khoảng 4325 |
| 0.60 | 2.0 s | 0.601 (0.60) | 2.03 s (2.0) | 0.232 (0.215) | khoảng 1065 |

**Kiểm tra B: với PX4 thật**
1. `--urb 0`: không bỏ gói nào (`frames_dropped_emu = 0`), kết quả giống cơ sở ở bước 6.
2. `--urb 0.5 --dur 2 --seed 1`, chạy ít nhất **10 phút**: tỷ lệ `frames_dropped_emu / frames_seen` gần 0.5 (với `lossbad = 1`, `lossgood = 0`). Chạy ngắn thì dao động lớn vì mỗi đợt kéo dài vài giây.
3. Quan sát QGC: dữ liệu đứng hình từng đợt rồi tiếp tục.
4. Chạy hai lần cùng `--seed`: kết quả tuần tự giống hệt (kiểm tra tính lặp lại).

**Xong khi:** Kiểm tra A đạt (π_B sai lệch dưới khoảng 5% tương đối so với lý thuyết), Kiểm tra B-1 và B-2 đạt.

---

## Bước 8: Giám sát UAV mất liên kết (outage)

**Làm**
- Gateway (và `tcpclient.exe`) theo dõi thời điểm HEARTBEAT cuối cùng của từng UAV (phía sau bộ mô phỏng, tức cái client thấy). Quá **3 giây** không có HEARTBEAT thì đánh dấu **mất liên kết**; có lại thì đánh dấu **online**.
- Ghi: `offline_events` (số lần mất), `offline_time_s` (tổng thời gian mất), và in dòng log mỗi lần chuyển trạng thái, có mốc thời gian.

**Liên hệ bài báo:** đây là phiên bản mức ứng dụng của "outage" (không nhận được khối nào sau R lần thử).

**Kiểm tra**
1. Tắt PX4: sau tối đa khoảng 3 đến 4 giây log báo mất liên kết; bật lại thì báo online.
2. `--urb 0.5 --dur 2`: log báo mất liên kết **một phần** các đợt xấu, không phải mọi đợt (các đợt ngắn hơn khoảng 3 giây thì HEARTBEAT kịp đến). Tỷ lệ đợt xấu từ 3 giây trở lên theo lý thuyết khoảng 22% (xem bảng bước 7). Đây là con số xấp xỉ, vì HEARTBEAT chỉ gửi 1 lần/giây.

**Xong khi:** số lần mất liên kết đo được cùng bậc với số đợt xấu nhân với tỷ lệ đợt đủ dài, và log phản ánh đúng.

---

## Bước 9: Chạy thí nghiệm và ghi CSV

Mỗi thí nghiệm chạy **10 phút**, lặp **3 hạt giống** (`--seed 1, 2, 3`), ghi file CSV riêng, tên gợi ý `E1_urb50_dur2_seed1.csv`. Trước mỗi lần chạy ghi vào nhật ký: ngày giờ, tham số, ghi chú.

| Mã | Thí nghiệm | Tham số | Câu hỏi cần trả lời | Ghi lại |
|---|---|---|---|---|
| E0 | Cơ sở | `--urb 0` | Đường thật có mất gói không? Jitter HEARTBEAT nền là bao nhiêu? | seq_lost_ingress, jitter, tốc độ gói |
| E1 | Quét Urb (như bài báo) | `--urb` = 0.1, 0.5, 0.6; `--dur 2` | Tỷ lệ mất đo được có khớp π_B × lossbad không? | frames_dropped_emu / frames_seen, offline_events, offline_time_s |
| E2 | Độ dài đợt xấu | `--urb 0.5`; `--dur` = 0.5, 2, 5 | **Cùng tỷ lệ mất, đợt dài hay ngắn ảnh hưởng outage thế nào?** | offline_events, offline_time_s, max_silence_ms |
| E3 | Mất một phần | `--urb 0.5 --dur 2`; `--lossbad` = 1.0, 0.5 | Mất một phần (không đứt hẳn) ảnh hưởng ra sao? | tỷ lệ mất, jitter |
| E4 | Số client | `--urb 0`; 1, 5, 10, 20 client | Gateway chịu được bao nhiêu client? | frames_out, client_dropped, CPU (Task Manager) |
| E5 | Client chậm | `--urb 0`; 1 client cố ý đọc chậm | Client chậm có kéo cả hệ thống không? | client_dropped của client chậm so với client thường |
| E6 | Ổn định | `--urb 0.1 --dur 2`; chạy 30 phút | Chạy lâu có crash hay rò socket không? | Gateway còn sống, số client đúng |

Quan sát thêm (ghi lời nhận xét, không cần số): hiển thị của QGC khi đợt xấu xảy ra, và việc PX4 có báo mất liên kết dữ liệu hay không (chỉ có ý nghĩa nếu UAV đang bay).

**Lưu ý trung thực khi viết kết quả**
- Mất gói theo `seq` ở client bị đếm thiếu khi một đợt mất từ 256 gói trở lên (xem bước 6). Hãy dùng bộ đếm "sự thật" làm chuẩn và nói rõ điều này.
- E4 và E5 đo trên cùng một máy nên CPU tranh nhau giữa gateway, PX4, Gazebo. Hãy ghi rõ cấu hình máy.

**Xong khi:** có đủ CSV cho E0 đến E6, mỗi thí nghiệm có ít nhất 3 hạt giống (E4, E5, E6 một lần là đủ).

---

## Bước 10: Vẽ biểu đồ và phân tích

**Làm:** viết `tools/plot.py` (Python, matplotlib, đọc các CSV) và vẽ:

1. **Đường thời gian của một lần chạy** (E1, urb 0.5): gói/giây theo thời gian, vùng bị đứt hiện rõ các đợt xấu.
2. **Tỷ lệ mất đo được so với lý thuyết** theo Urb (E1): cột đo, đường lý thuyết `π_B × lossbad`.
3. **Số lần mất liên kết theo độ dài đợt xấu** (E2): cùng tỷ lệ mất nhưng outage khác nhau.
4. **Mất một phần** (E3).
5. **Số client và gói bị bỏ** (E4), và so sánh client chậm với client thường (E5).

**Phân tích cần có trong báo cáo**
- Mất gói đo được khớp lý thuyết đến mức nào, sai lệch vì sao (đợt dài, chạy ngắn).
- Vì sao cùng một tỷ lệ mất mà đợt xấu dài lại gây nhiều outage hơn (nối với ý tưởng "mất theo đợt" của bài báo).
- Giới hạn: bộ mô phỏng làm ở tầng ứng dụng, không phải tầng vật lý như bài báo; mô hình chỉ có hai trạng thái và một chuỗi chung cho mọi gói.

**Xong khi:** có ít nhất 4 biểu đồ có chú thích trục và đơn vị, kèm nhận xét.

---

## Bước 11: Đóng gói nộp bài

**Làm**
1. **Sơ đồ kiến trúc** (sơ đồ mục 1) và **sơ đồ luồng một gói tin**: UAV, UDP, tách frame, kiểm tra CRC, đếm QoS ingress, bộ mô phỏng, hàng đợi từng client, TCP, client.
2. **README**: cách cài PX4, cách biên dịch, thứ tự bật các thành phần, ví dụ lệnh có tham số mô phỏng.
3. Repo GitHub (không đưa file build vào, thêm `.gitignore` cho `*.exe`).
4. Slide và **video demo** (khoảng 3 đến 5 phút).

**Kịch bản demo:** (1) PX4 chạy, gateway tắt mô phỏng: QGC và `tcpclient` cùng xem dữ liệu; (2) bật `--urb 0.5 --dur 2`: dữ liệu đứng từng đợt, log báo mất liên kết và online lại; (3) ngắt một client: gateway vẫn chạy; (4) cho xem biểu đồ đo được so với lý thuyết.

**Bố cục báo cáo gợi ý:**
1. Giới thiệu và mục tiêu
2. Cơ sở lý thuyết: MAVLink, UDP/TCP, mô hình Markov hai trạng thái, các chỉ số QoS, bài báo tham khảo
3. Thiết kế hệ thống
4. Cài đặt (Winsock, `select`, bộ đệm client, bộ mô phỏng)
5. Thực nghiệm và kết quả (E0 đến E6)
6. Kết luận, hạn chế, hướng phát triển

**Xong khi:** người khác làm theo README là chạy lại được.

---

## Mở rộng (chỉ làm khi bước 0–11 xong)

**A. Lệnh ngược và heartbeat trạm mặt đất.** Gateway nhận gói từ client, đọc `target_system` (COMMAND_LONG: byte thứ 30 của payload; nếu payload bị cắt ngắn hơn thì coi như 0, tức gửi cho tất cả), rồi `sendto` về địa chỉ UAV đã học từ gói đến. Gửi HEARTBEAT trạm mặt đất mỗi 1 giây (test vector ở bước 3 chính là loại gói này). Điểm cần cẩn thận: PX4 có chấp nhận lệnh arm/takeoff hay không phụ thuộc GPS lock và chế độ bay; bạn đọc `COMMAND_ACK` và log PX4 để chỉnh. Phần này mình chưa kiểm chứng với PX4 thật.

**B. Nhiều UAV.** Bảng `sysid → địa chỉ UAV` (`std::map`). Chạy thêm instance PX4 theo mục Multi-Vehicle Simulation trong tài liệu PX4 đúng phiên bản bạn cài. Khi đó chuỗi Markov nên có trạng thái riêng cho từng UAV.

**C. Gửi trùng gói kiểu CPC/APC.** Với gói quan trọng (HEARTBEAT, lệnh) gửi N lần và bên nhận loại gói trùng theo `seq`: N cố định (kiểu CPC) hoặc tăng khi đo thấy mất gói tăng (kiểu APC). Đo đánh đổi giữa tỷ lệ chuyển thành công và băng thông tiêu tốn. Đây chỉ là phép tương tự mức ứng dụng; bài báo đạt lợi ích nhờ cộng năng lượng tín hiệu ở tầng vật lý, gửi trùng gói chỉ cho đa dạng theo thời gian.

---

## 6. Những điều cần giải thích được khi vấn đáp

- Gateway không điều khiển bay; PX4 tự bay, gateway chỉ trung chuyển và đo.
- UDP khác TCP; vì sao phía UAV dùng UDP, phía client dùng TCP; TCP là luồng byte nên phải tự ghép khung.
- Vì sao mỗi client cần hàng đợi gửi riêng (client chậm không kéo cả hệ thống).
- Cấu trúc frame MAVLink 2, `sysid`, `msgid`, `seq`, CRC và `CRC_EXTRA`.
- Mô hình Markov hai trạng thái: π_B, thời gian đợt xấu, vì sao chạy theo khe thời gian.
- Vì sao tính tỷ lệ mất theo `seq` bị đếm thiếu khi đợt mất dài, và cách bạn xử lý.
- Phân biệt mô phỏng ở tầng ứng dụng (của bạn) với tầng vật lý (của bài báo).
- Hạn chế: gateway là điểm lỗi duy nhất, chưa xác thực, chưa mã hóa.

## 7. Tài nguyên

- Bản Python tham chiếu (`mavgw.zip`): đối chiếu hoặc test khi cần, không bắt buộc. Bảng `CRC_EXTRA` lấy từ `crc_extra.py` trong đó.
- MAVLink: mavlink.io (common message set, packet format MAVLink 2).
- PX4: docs.px4.io (Simulation, Multi-Vehicle Simulation, MAVLink).
- Bài báo: Viana et al., *Increasing Reliability on UAV Fading Scenarios*, IEEE Access, vol. 10, 2022.
