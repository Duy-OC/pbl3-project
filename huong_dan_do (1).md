# Hướng dẫn tự đo và lấy số liệu cho dự án MAVLink Gateway

Tài liệu này giúp bạn tự đo, hiểu **vì sao** đo, và biết cách đọc kết quả. Phần 1 là nguyên tắc, phần 2 đến 5 là từng giai đoạn đo, phần 6 là bảng ghi số liệu và cách kiểm tra, **phần 7 giải thích thuật ngữ** và **phần 8 hướng dẫn lưu báo cáo HTML và chốt nghiệm thu**.

Những con số có ghi "tham chiếu từ báo cáo của bạn" lấy từ file `report.html` bạn đã gửi, chỉ đúng với **stream PX4 và máy của bạn lúc đó**, không phải giá trị mặc định của PX4.

---

## 1. Nguyên tắc chung

### 1.1 Mỗi lần đo phải trả lời một câu hỏi

Một lần đo chỉ có ý nghĩa khi có đủ ba thứ:

1. **Câu hỏi** nó trả lời.
2. **Mốc so sánh**: so với cái gì thì mới biết tốt hay xấu.
3. **Tiêu chí đạt**, xác định **trước** khi đo.

### 1.2 Đo theo kiểu "trước, thêm một thứ, đo lại"

```
A. TRƯỚC (chưa có gateway)  →  B. CÓ GATEWAY (mô phỏng tắt)  →  C. BẬT MÔ PHỎNG MARKOV
   "PX4 vốn thế nào?"           "Gateway có làm đổi gì không?"     "Đường xấu thì hệ thống ra sao?"
```

Mỗi lần chỉ đổi **đúng một thứ**, để biết thay đổi trong số liệu là do thứ đó gây ra.

### 1.3 Quy tắc áp dụng cho mọi lần đo

| Quy tắc | Cách làm | Vì sao |
|---|---|---|
| Cùng điều kiện | Cùng stream PX4 (cùng lệnh `mavlink start`), UAV cùng trạng thái (ví dụ đứng yên), QGC đóng khi chạy chương trình của bạn | So sánh mới công bằng. Báo cáo của bạn có ví dụ: E0 cũ là 60,7 frame/s còn run mới là 36,5 frame/s vì stream khác, nên hai số đó không so với nhau được |
| Bỏ 10 giây đầu (warm-up) | Không tính 10 giây đầu kể từ frame đầu tiên | Luồng dữ liệu lúc mới bắt đầu chưa ổn định |
| Đo đủ dài | 60 giây để khảo sát nhanh, **600 giây** cho số liệu chính | Đủ nhiều mẫu. 600 giây có khoảng 21.900 frame và khoảng 600 HEARTBEAT (tham chiếu từ báo cáo của bạn) |
| Lặp lại ít nhất 3 lần | Với Markov: 3 seed khác nhau | Biết mức dao động tự nhiên. Chênh lệch nhỏ hơn mức dao động đó thì không có ý nghĩa |
| Ghi lại mọi thứ | Ngày giờ, lệnh `mavlink start`, phiên bản PX4, tham số gateway, mã nguồn (commit hoặc mã băm), ghi chú | Người khác (và bạn sau này) lặp lại được |
| Phân biệt "đo được" và "suy ra" | Chỉ số nào chưa đo trực tiếp thì ghi rõ là chưa đo | Tránh khẳng định quá mức |
| Giữ cả run hỏng | Đánh dấu run lỗi hoặc sai kịch bản, không dùng để nghiệm thu nhưng không xóa | Trung thực và truy vết được |

### 1.4 Ba điểm đo trong gateway

```
PX4 ──UDP──► [INGRESS] ──► (Markov) ──► [EGRESS] ──TCP──► [CLIENT RX]
              đường thật               sau khi bị làm xấu   cái người dùng thấy
```

Đo ở cả ba điểm để **định vị chỗ thay đổi xảy ra**:
- Ingress khác egress: tác động nằm ở bộ mô phỏng (emulator).
- Egress ghi frame sau Markov, trước fan-out. Client RX ghi frame ứng dụng thực sự giải mã. Muốn đối soát, cần cùng cửa sổ, cùng client và cùng tập frame đủ điều kiện gửi cho client đó. Ghi riêng số frame xếp hàng, trạng thái kết nối và frame client nhận nếu muốn xác định nguyên nhân chênh lệch.
- Client có cửa sổ đo riêng (bắt đầu và kết thúc lệch vài giây), nên **không dùng hiệu số client và gateway để tính mất gói**.

---

## 2. Giai đoạn A: Trước khi có gateway

### A1. QGC nối trực tiếp với PX4

| Mục | Nội dung |
|---|---|
| **Câu hỏi** | Hệ thống chuẩn có chạy không, và "bình thường" trông như thế nào? |
| **Đo gì** | Trong `pxh>` gõ `mavlink status`: `tx`, `rx loss`, `partner IP`, `ping statistics`. Trong QGC: UAV có hiện không, độ cao và tư thế cập nhật mượt hay giật, có báo mất liên kết không |
| **Cách đo** | Chạy 2 phút sau khi PX4 ổn định. Gõ `mavlink status` 3 lần cách nhau vài chục giây, chụp màn hình QGC |
| **Để làm gì** | Xác nhận PX4, WSL2 và Windows thông nhau; có mốc "QGC mượt" để so sau này. Khi gateway có lỗi, đây là phép loại trừ: có bằng chứng kết nối trực tiếp hoạt động trong ca A1 |
| **Giới hạn** | Ca A1 dùng quan sát QGC và ảnh chụp. RX loss trên PX4 mô tả chiều PX4 nhận, vì vậy không dùng để kết luận mất telemetry chiều PX4 → QGC. |
| **Đạt khi** | QGC hiển thị đúng UAV và dữ liệu cập nhật liên tục trong thời gian quan sát |

### A2. serverUDP nối với PX4 (thay chỗ QGC)

serverUDP không chạy cùng lúc với QGC vì cả hai cùng chiếm cổng UDP 14550. Nó thay QGC làm bên nhận nhưng khác ở chỗ **đếm được**.

| Mục | Nội dung |
|---|---|
| **Câu hỏi** | PX4 gửi chính xác gì, bao nhiêu, và có đều đặn không? |
| **Đo gì** | Tổng frame/s và byte/s; số frame theo từng `msgid`; khoảng cách giữa các HEARTBEAT (trung bình và độ lệch chuẩn); tập `sysid`/`compid`; địa chỉ và cổng nguồn |
| **Cách đo** | Thêm bộ đếm vào serverUDP. Chạy 60 giây khảo sát, rồi 600 giây cho số liệu chính, lặp 3 lần |
| **Công thức** | frame/s = tổng frame ÷ thời gian; byte/s = tổng byte ÷ thời gian; byte/frame = tổng byte ÷ tổng frame |
| **Để làm gì** | (1) **Mốc để kiểm tra gateway**: nếu ingress thấp hơn A2, kiểm tra lại stream, cửa sổ đo, bộ đếm frame hợp lệ và điều kiện máy trước khi quy nguyên nhân. (2) **Căn cứ thiết kế**: tần suất gói quyết định timeout, kích thước hàng đợi, và `seq` 8 bit quay vòng sau bao lâu. (3) **Mốc nhiễu nền**: jitter HEARTBEAT lúc chưa can thiệp |
| **Đạt khi** | Số liệu lặp lại giữa 3 lần chạy (sai khác nhỏ), HEARTBEAT khoảng 1 giây một lần, tập `sysid` đúng như kỳ vọng |
| **Tham chiếu từ báo cáo của bạn** | Khoảng 36,5 frame/s, khoảng 1451 byte/s, khoảng 39,76 byte/frame, HEARTBEAT trung bình khoảng 1,008 s, 31 nhóm `msgid` |

**A1 và A2 bổ trợ nhau:** A1 là "mắt nhìn", A2 là "máy đếm", cùng nguồn PX4. Có thể so thô `tx` của PX4 với byte/s của serverUDP, nhưng chỉ khi cùng một instance MAVLink: kiểm tra bằng `partner IP` và cổng trong `mavlink status`. Mình chưa kiểm chứng hai số này khớp tuyệt đối.

---

## 3. Giai đoạn B: Có gateway, mô phỏng tắt (`--urb 0`)

### B1. Gateway có làm thay đổi dữ liệu không? (đây là E0, mốc của dự án)

| Mục | Nội dung |
|---|---|
| **Câu hỏi** | Gateway có nhận đủ, chuyển nguyên vẹn, và tốn bao nhiêu tài nguyên? |
| **Đo gì** | Tại ingress, egress, client: frame, byte, CRC lỗi, HEARTBEAT, jitter, phân bố MSGID. Thêm CPU, RAM, handle của `gateway.exe` |
| **Cách đo** | 1 UAV, 1 client, 600 giây. So ingress với kết quả A2. So ingress với egress trong cùng một run. Đo tài nguyên bằng PowerShell (xem bên dưới) |
| **Kỳ vọng** | Cùng cửa sổ, tổng frame và tổng byte ingress bằng egress, drop Markov bằng 0. Ghi CRC lỗi theo điểm đo. Nếu cần chứng minh nguyên vẹn, so byte từng frame hoặc hash của cùng chuỗi frame đã serialize. CPU, RAM và handle là số đo quan sát, đánh giá qua nhiều mẫu. |
| **Để làm gì** | Tạo mốc OFF cùng cấu hình PX4 để so với ON và OUTAGE. Đối chiếu ingress với egress trong cùng run để tách hiệu ứng Markov. |
| **Tham chiếu từ báo cáo của bạn** | Ingress và egress đều 21.900 frame và 870.852 byte; CRC = 0; CPU trung bình khoảng 0,7% một lõi; handle 92 → 92 |

**Đo CPU, RAM, handle bằng PowerShell** (chạy khi gateway đang chạy, lấy hai mẫu cách nhau vài giây):

```powershell
Get-Process gateway | Select-Object CPU, WorkingSet64, PrivateMemorySize64, HandleCount
```

- `CPU` là tổng giây CPU đã dùng. **CPU % một lõi = (CPU lần 2 − CPU lần 1) ÷ (thời gian thực giữa hai lần) × 100.**
- `WorkingSet64` là RAM đang giữ trong bộ nhớ vật lý; `PrivateMemorySize64` là bộ nhớ riêng của tiến trình (cả hai tính bằng byte).
- `HandleCount` là số "tay cầm" tài nguyên đang mở (socket, file...). Nếu tăng đều theo thời gian là dấu hiệu rò.

### B2. Nhiều client và client chậm

| Mục | Nội dung |
|---|---|
| **Câu hỏi** | Gateway chịu được bao nhiêu client, và một client chậm có kéo cả hệ thống không? |
| **Đo gì** | Số client, frame mỗi client, số lần nối lại, client chậm có bị ngắt không, số kết nối bị từ chối, CPU, RAM, handle |
| **Cách đo** | Chạy 600 giây với một client nền. Tại các mốc thời gian: nối và đóng 20 lần; thêm một socket không đọc; thử nhiều kết nối đồng thời. **Đối chiếu log của gateway**, vì kết nối TCP thành công ở phía client chưa chắc gateway đã nhận |
| **Để làm gì** | Chứng minh thiết kế **hàng đợi riêng từng client**: client chậm bị ngắt khi vượt ngưỡng hàng đợi, client nền vẫn nhận bình thường |
| **Đạt khi** | Client nền không bị rớt; số client trở về mức nền; client chậm bị ngắt; số client đang hoạt động không vượt giới hạn; handle trở về gần mức nền sau khi dọn kết nối và không tăng tích lũy qua các vòng thử |

---

## 4. Giai đoạn C: Bật mô phỏng Markov

### C1. Kiểm bộ Markov riêng (không cần PX4)

| Mục | Nội dung |
|---|---|
| **Câu hỏi** | Bộ mô phỏng có đúng mô hình không? |
| **Cách đo** | Chạy nhiều khe thời gian giả (ví dụ 3 seed × 180.000 khe). So tỷ lệ thời gian xấu với `urb`, độ dài đợt xấu trung bình với `dur` |
| **Để làm gì** | Phải chắc công cụ gây nhiễu đúng **trước khi** dùng nó. Nếu bỏ bước này, số liệu xấu sẽ không phân biệt được do kênh hay do lỗi code |
| **Đạt khi** | Mỗi seed có `seen = kept + dropped`; tỷ lệ xấu và đợt xấu trung bình gần tham số |
| **Tham chiếu từ báo cáo của bạn** | `urb` 0,5 và `dur` 2 s cho tỷ lệ xấu 49,6 / 51,4 / 50,4% và đợt xấu trung bình 1,96 / 2,05 / 1,98 s |

### C2. Đo với PX4 thật: OFF, ON, OUTAGE

| Ca | Tham số | Câu hỏi | Đo gì | Đạt khi |
|---|---|---|---|---|
| **OFF** | `urb = 0` | Emulator tắt có trong suốt không? | `frames_in`, `kept`, `dropped` | in = kept, dropped = 0, ingress = egress |
| **ON** | `urb = 0,5`, `dur = 2 s`, seed 1, 2, 3 | Mất gói có đúng mô hình không? | `frames_in`, `kept`, `dropped`, số đợt xấu, độ dài đợt | in = kept + dropped; so tỷ lệ bỏ đo được với kỳ vọng của mô hình và tỷ lệ frame đến khi kênh ở B; có chuyển trạng thái G/B |
| **OUTAGE** | `urb = 0,5`, `dur = 5 s`, seed 1, 2, 3 | Đợt xấu dài gây mất liên kết ra sao? | HEARTBEAT ở ingress và egress, số lần và tổng thời gian offline | Ingress không offline; egress offline sau timeout rồi online lại |

### 4.1 Tự tính giá trị kỳ vọng

Công thức (khe thời gian `slot` = 0,1 s):

```
P_bg = slot / dur                      (xác suất từ xấu về tốt trong 1 khe)
P_gb = P_bg × urb / (1 − urb)          (xác suất từ tốt sang xấu trong 1 khe)
Tỷ lệ thời gian xấu kỳ vọng  π_B = P_gb / (P_gb + P_bg) = urb
Tỷ lệ mất gói kỳ vọng        = urb × lossbad + (1 − urb) × lossgood
Số đợt xấu kỳ vọng           ≈ số khe × (1 − urb) × P_gb
Xác suất đợt B dài hơn x giây = (1 − P_bg) ^ floor(x / slot), với x ≥ 0
Số frame bị bỏ kỳ vọng       ≈ frames_in × tỷ lệ mất kỳ vọng
```

Công thức tỷ lệ bỏ dùng π_B khi thời điểm frame đến độc lập với trạng thái kênh. Với một run hữu hạn, ghi thêm số frame vào lúc G và B. Kỳ vọng có điều kiện theo run là `frames_in_G × lossgood + frames_in_B × lossbad`. So các số trong cùng cửa sổ 600 giây.

Đoạn Python nhỏ để tính nhanh (lưu thành `calc_markov.py`, chạy `python calc_markov.py 0.5 2 21930`, tham số lần lượt là `urb`, `dur` và (tùy chọn) số frame vào):

```python
import sys

def markov(urb, dur_s, slot_ms=100, lossbad=1.0, lossgood=0.0, run_s=600, frames_in=None):
    from math import floor, isfinite
    if not all(isfinite(x) for x in (urb, dur_s, slot_ms, lossbad, lossgood, run_s)):
        raise ValueError("Tham số phải hữu hạn")
    if not (0 <= urb < 1 and dur_s > 0 and slot_ms > 0 and run_s > 0
            and 0 <= lossbad <= 1 and 0 <= lossgood <= 1
            and (frames_in is None or frames_in >= 0)):
        raise ValueError("Tham số ngoài miền cho phép")
    if urb == 0:
        print("Emulator tắt. Tỷ lệ B và tỷ lệ bỏ kỳ vọng bằng 0.")
        return
    slot_s = slot_ms / 1000.0
    p_bg = slot_s / dur_s
    p_gb = p_bg * urb / (1 - urb)
    if not (0 < p_bg <= 1 and 0 < p_gb <= 1):
        raise ValueError("Xác suất chuyển trạng thái ngoài (0, 1]")
    slots = run_s / slot_s
    print(f"P_bg = {p_bg:.4f}   P_gb = {p_gb:.5f}")
    print(f"Tỷ lệ thời gian xấu kỳ vọng (pi_B) = {p_gb / (p_gb + p_bg):.3f}")
    print(f"Tỷ lệ mất gói kỳ vọng = {urb * lossbad + (1 - urb) * lossgood:.3f}")
    print(f"Số đợt xấu kỳ vọng trong {run_s} s ≈ {slots * (1 - urb) * p_gb:.0f}")
    print(f"Độ dài đợt xấu trung bình kỳ vọng = {dur_s} s")
    for x in (2, 3):
        p = (1 - p_bg) ** floor(x / slot_s)
        print(f"Xác suất một đợt xấu dài hơn {x} s = {p:.3f}")
    if frames_in is not None:
        print(f"Số frame bị bỏ kỳ vọng ≈ {frames_in * (urb * lossbad + (1 - urb) * lossgood):.0f} trên {frames_in} frame")

if __name__ == "__main__":
    urb = float(sys.argv[1]); dur = float(sys.argv[2])
    fin = int(sys.argv[3]) if len(sys.argv) > 3 else None
    markov(urb, dur, frames_in=fin)
```

Kết quả mẫu (đã chạy thử): với `urb = 0,5`, `dur = 2 s` thì P_bg = P_gb = 0,05, khoảng 150 lượt vào B trong 600 s ở trạng thái dừng, xác suất một đợt dài hơn 2 s là 0,358 và dài hơn 3 s là 0,215.

### 4.2 Cách đọc outage (chỗ dễ nhầm)

Trong cấu hình ghi trong tài liệu, route và QoS ingress dùng timeout 5 giây, QoS egress và client dùng 3 giây. Ghi timeout thực tế theo cấu hình của mỗi run. Mỗi điểm đo bắt đầu theo dõi mất heartbeat sau khi đã thấy HEARTBEAT hợp lệ đầu tiên. Trước đó ghi “chưa xác định”.

Đối chiếu timestamp của HEARTBEAT ingress/egress, thời điểm đổi G/B và sự kiện offline/online trên cùng trục thời gian. Ví dụ, nhận heartbeat tại giây 0 và giây 4, với timeout 3 giây và bộ đo vẫn được tick, thì khoảng offline dự kiến là từ giây 3 đến giây 4. Ghi số lần offline, tổng thời gian offline và các đợt còn mở khi kết thúc đo. Nếu nhận lại đúng mốc timeout, đọc thứ tự xử lý trong log để giải thích kết quả.

Một đợt B có thể bỏ số heartbeat khác nhau tùy pha heartbeat và xác suất bỏ. Số đợt B và số lần offline là hai chỉ số riêng. Để nghiệm thu OUTAGE, chọn run có heartbeat egress ban đầu, egress offline và hồi phục, đồng thời ingress vẫn online trong cùng đoạn thời gian.

### 4.3 Vì sao phải đo 3 seed

Mỗi seed là một "kịch bản vật cản" khác nhau. Một seed chỉ cho một mẫu, nhất là khi đợt xấu dài và ít đợt. Ví dụ ca OUTAGE chỉ có khoảng 59 đợt, tỷ lệ mất đo được 45,2% so với 50% lý thuyết. Mức lệch đó nằm trong dao động hợp lý, ba seed cho một ước lượng ban đầu về độ dao động.

---

## 5. Giai đoạn D: Định tuyến, mất liên kết và phục hồi hai UAV

Khởi động hai PX4 SITL có SYSID khác nhau. Ghi SYSID, endpoint UDP, trạng thái route và thời điểm từng thao tác. Dùng Markov OFF trong các ca định tuyến để dễ đối chiếu. Mở QGC qua Gateway TCP hoặc dùng client phát request thử.

| Ca | Thao tác | Bằng chứng đạt |
|---|---|---|
| D1 — Định tuyến đúng | Gửi riêng một lệnh thử có thể kiểm tra ACK tới UAV 1, sau đó UAV 2 | Log có target, endpoint đúng và ACK từ UAV tương ứng. Nếu yêu cầu nghiệm thu Takeoff/Land, quan sát thêm phản ứng trong Gazebo và xác nhận UAV còn lại không bị tác động. |
| D2 — Route offline | Ngắt UAV 2, chờ log OFFLINE, gửi request tới UAV 2 rồi gửi request tới UAV 1 | Có BLOCK cho UAV 2, counter no-route tăng. UAV 1 tiếp tục nhận request và phản hồi. |
| D3 — Phục hồi endpoint | Khởi động lại UAV 2, có thể đổi cổng nguồn sau khi route cũ đã hết hạn | Có ONLINE, route mới và ACK/phản hồi của request mới trong cùng phiên Gateway. |
| D4 — Hủy queue | Giữ một request trong queue bằng chế độ kiểm thử. Thử riêng việc route hết hiệu lực và việc client ngắt kết nối | Log xác nhận request đã xếp hàng, nguyên nhân hủy và counter tăng. Request cũ không xuất hiện ở endpoint mới. |
| D5 — Queue đầy và lỗi gửi | Hạ giới hạn queue hoặc dùng cơ chế gây lỗi gửi có kiểm soát trong bản test | Quan sát đúng chính sách khi queue đầy/lỗi gửi, log có lý do và Gateway tiếp tục xử lý sau khi bỏ lỗi. |

D4–D5 cần khả năng tạo tình huống có kiểm soát. Ghi “chưa thử” nếu chưa kích hoạt được nhánh. Báo cáo cần phân biệt lỗi gửi thực tế với lỗi được giả lập trong bản kiểm thử.

## 6. Ghi số liệu, kiểm tra và cách đọc kết quả

### 6.1 Bảng ghi mỗi lần chạy (một dòng một run)

| Cột | Ví dụ |
|---|---|
| `run_id`, ngày giờ | `A2_20261112_1` |
| Ca và tham số | `A2`, hoặc `ON urb=0.5 dur=2 seed=1` |
| Stream PX4 (lệnh `mavlink start`) | ghi nguyên văn |
| Trạng thái UAV | đứng yên |
| Thời lượng và warm-up | 600 s, bỏ 10 s |
| Frame, byte, frame/s, byte/frame | |
| HEARTBEAT: số lượng, trung bình, jitter | |
| CRC lỗi | |
| Offline: số lần, tổng giây | |
| `dropped`, `kept` (nếu có Markov) | |
| CPU, RAM, handle (nếu đo) | |
| Mã nguồn (commit hoặc mã băm) | |
| Ghi chú, hiện tượng lạ | |

### 6.2 Các phép tự kiểm sau mỗi run

1. **Đối soát:** `frames_in = frames_kept + frames_dropped_emu`. Sai là có lỗi đếm.
2. **Tỷ lệ mất đo được** `= frames_dropped_emu ÷ frames_in` so với kỳ vọng `urb × lossbad + (1 − urb) × lossgood` khi emulator bật. OFF phải có tỷ lệ bỏ bằng 0. Nếu frames_in bằng 0, ghi chưa có mẫu để tính tỷ lệ.
3. **Ingress với egress** khi mô phỏng tắt: bằng nhau về tổng frame và tổng byte. Muốn kiểm chứng nội dung, so byte/hash của cùng chuỗi frame.
4. **Tổng theo MSGID** bằng tổng frame.
5. **Giữa các lần lặp**: tính trung bình và khoảng dao động; ghi số lần lặp và khoảng dao động. Nếu chênh lệch nhỏ so với dao động, ghi chưa đủ bằng chứng để kết luận..

### 6.3 Khi số liệu không khớp thì kiểm tra gì

| Hiện tượng | Nghi ngờ trước |
|---|---|
| Ingress thấp hơn serverUDP (A2) nhiều | Lỗi tách frame hoặc CRC; cổng bị tranh; tường lửa |
| CRC lỗi lớn hơn 0 khi chưa mô phỏng | Sai bảng `CRC_EXTRA` hoặc parser; thử lại với test vector |
| Ingress khác egress khi `urb = 0` | Emulator chưa tắt hẳn hoặc có lỗi chuyển tiếp |
| Tỷ lệ Markov bỏ lệch xa kỳ vọng | Run quá ngắn, chỉ một seed, hoặc lỗi bộ Markov (chạy lại C1) |
| Frame/s khác hẳn lần trước | Đổi stream PX4 hoặc trạng thái UAV; so lại lệnh `mavlink start` |
| Jitter thay đổi vài chục ms | Có thể là nhiễu nền của WSL2 và Windows; so với mức dao động của mốc |
| Handle tăng đều theo thời gian | Rò kết nối hoặc socket chưa đóng |

### 6.4 Những gì dự án này chưa đo được

- **Độ trễ một chiều** từ PX4 đến client: cần định danh từng frame và đồng hồ chung giữa PX4 (trong WSL2) và Windows. Hãy ghi rõ là chưa đo.
- **Tỷ lệ mất gói thật theo `seq`:** chỉ số `seq gap` hiện chưa dùng được (xem mục 7.4). Dùng `frames_dropped_emu` để xác nhận số frame emulator chủ động bỏ.
- Kết quả chỉ có giá trị **trong mô phỏng** (PX4 SITL trong máy), chưa có đường truyền vô tuyến thật.

---

## 7. Bảng giải thích thuật ngữ và thông số

### 7.1 Hệ thống và giao thức

| Thuật ngữ | Ý nghĩa |
|---|---|
| **UAV** | Phương tiện bay không người lái (drone). |
| **PX4** | Phần mềm điều khiển bay mã nguồn mở. Nó tự bay, đồng thời gửi dữ liệu trạng thái ra ngoài. |
| **SITL** | Software-In-The-Loop: chạy PX4 như một chương trình trên máy tính thay vì trên phần cứng drone. |
| **Gazebo** | Phần mềm mô phỏng thế giới 3D, cho UAV giả lập "bay" trong đó. |
| **WSL2** | Môi trường Linux chạy trong Windows. PX4 chạy ở đây, còn gateway chạy trên Windows. |
| **QGC (QGroundControl)** | Chương trình trạm mặt đất để xem và điều khiển UAV. |
| **MAVLink** | Giao thức nhắn tin giữa UAV và trạm mặt đất. Đây là "ngôn ngữ" của gói tin trong dự án. Dự án dùng phiên bản 2. |
| **Frame / gói** | Một gói tin MAVLink hoàn chỉnh, gồm header, payload và CRC. |
| **Payload** | Phần nội dung chính của frame (ví dụ góc nghiêng, tọa độ). |
| **`sysid`** | Số định danh của hệ thống gửi (UAV số 1, số 2...). Trạm mặt đất thường dùng 255. |
| **`compid`** | Số định danh thành phần trong hệ thống (autopilot, camera...). |
| **`msgid`** | Số định danh loại bản tin. Ví dụ 0 là HEARTBEAT, 30 là ATTITUDE, 33 là GLOBAL_POSITION_INT. |
| **HEARTBEAT** | Bản tin "tôi còn sống", gửi khoảng 1 giây một lần. Dùng để phát hiện mất liên kết. |
| **ATTITUDE** | Bản tin báo góc nghiêng của UAV (roll, pitch, yaw). |
| **`seq`** | Số thứ tự của frame từ một nguồn gửi, từ 0 đến 255 rồi quay vòng về 0. Dùng để phát hiện gói bị mất. |
| **CRC** | Mã kiểm tra tính toàn vẹn gắn vào cuối frame. Bên nhận tính lại, không khớp thì frame bị hỏng và bị loại. |
| **`CRC_EXTRA`** | Một byte cố định riêng cho từng loại bản tin, cộng vào phép tính CRC để hai bên chắc chắn dùng cùng định nghĩa bản tin. |
| **UDP** | Giao thức gửi gói không kết nối, không đảm bảo giao đủ hay đúng thứ tự. PX4 gửi telemetry qua UDP. |
| **TCP** | Giao thức có kết nối, đảm bảo giao đủ và đúng thứ tự, nhưng là luồng byte nên phải tự ghép lại thành frame. Gateway phục vụ client qua TCP. |
| **Socket** | "Cổng" lập trình để gửi nhận dữ liệu qua mạng. |
| **Cổng 14550 / 5760** | Cổng UDP gateway nhận từ PX4 (14550); cổng TCP gateway phục vụ client (5760). |
| **Gateway** | Chương trình trung gian: nhận từ UAV, kiểm tra, phân phối cho nhiều client. |
| **Fan-out** | Một nguồn phân phối cho nhiều nơi nhận cùng lúc. |
| **Client** | Chương trình nhận dữ liệu từ gateway (QGC, `tcpclient`). |
| **Route / định tuyến** | Việc gateway quyết định gói đi đâu, ví dụ lệnh cho UAV 2 phải gửi về đúng địa chỉ UAV 2. |
| **ACK** | Bản tin xác nhận đã nhận lệnh. |
| **`BLOCK`** | Gateway chặn không gửi lệnh vì không có đường tới UAV (ví dụ UAV offline). |

### 7.2 Điểm đo và cách chạy một lần đo

| Thuật ngữ | Ý nghĩa |
|---|---|
| **Ingress** | Điểm đo ngay khi gói vào gateway từ PX4 (trước bộ mô phỏng). Phản ánh đường truyền thật. |
| **Egress** | Điểm đo ngay trước khi gói rời gateway sang client (sau bộ mô phỏng). |
| **Client RX** | Số liệu do chính client tự đo, với cửa sổ thời gian riêng. |
| **Baseline / mốc / E0** | Lần đo cơ sở khi chưa có tác động nào, làm chuẩn để so sánh. E0 là mốc của dự án. |
| **Run / `run_id`** | Một lần chạy đo, có mã định danh riêng. |
| **Warm-up** | Khoảng đầu bỏ qua (10 giây) để luồng dữ liệu ổn định. |
| **Cửa sổ đo** | Khoảng thời gian thực sự được tính vào kết quả (600 giây sau warm-up). |
| **Seed** | Hạt giống của bộ sinh số ngẫu nhiên. Cùng seed, cấu hình, cách khởi tạo và thứ tự đầu vào có thể tái lập chuỗi. Hai run SITL có thời điểm frame khác nhau có thể cho số drop khác nhau. |
| **Nghiệm thu** | Danh sách điều kiện run phải thỏa. Lưu ý: đạt nghiệm thu chỉ nghĩa là chạy xong và nhất quán, không nghĩa là kết quả chính xác với lý thuyết. |

### 7.3 Chỉ số QoS

| Thuật ngữ | Ý nghĩa và cách tính |
|---|---|
| **QoS** | Chất lượng dịch vụ: tập số đo cho biết đường truyền tốt hay xấu. Không có danh sách cố định, bạn chọn số đo phù hợp. |
| **frame/s** | Số frame nhận được mỗi giây. |
| **byte/s** | Lượng dữ liệu mỗi giây. |
| **byte/frame** | Kích thước trung bình một frame = tổng byte ÷ tổng frame. |
| **Thông lượng** | Lượng dữ liệu đi qua mỗi giây (frame/s hoặc byte/s). |
| **Mất gói** | Tỷ lệ gói gửi đi nhưng không tới nơi. Trong báo cáo, ghi riêng số frame bị Markov bỏ và các chỉ số ước lượng mất frame theo sequence. |
| **Jitter** | Độ lệch chuẩn của khoảng cách giữa các HEARTBEAT liên tiếp (lý tưởng 1 giây). Càng nhỏ càng đều. |
| **`hb_mean_ms`** | Khoảng cách trung bình giữa hai HEARTBEAT, tính bằng mili giây. |
| **`max_silence` / khoảng lặng** | Khoảng thời gian dài nhất không nhận được gói nào. |
| **Outage / offline** | UAV bị coi là mất liên kết khi quá thời gian timeout không có HEARTBEAT hợp lệ. Đo bằng số lần và tổng thời gian. |
| **Timeout** | Thời gian chờ tối đa. Trong báo cáo của bạn: route và ingress 5 giây, egress và client 3 giây. |
| **`seq gap`** | Ước lượng số khoảng trống trong chuỗi `seq` (xem mục 7.4). |
| **Độ trễ (latency)** | Thời gian gói đi từ nguồn đến đích. Dự án này chưa đo được độ trễ một chiều. |
| **Goodput** | Lượng dữ liệu hữu ích thực sự nhận được. Nếu đo goodput ứng dụng, đếm byte payload hữu ích mà client nhận trong cửa sổ đo. Byte frame egress gồm cả overhead MAVLink. |

### 7.4 Lưu ý về `seq` và `seq gap`

Công thức ước lượng số gói mất theo `seq`, tính riêng cho từng nguồn gửi (`sysid`, `compid`): `mất += (seq − seq_trước − 1) mod 256`. Ví dụ `seq_trước = 254` và `seq = 1` cho 2 gói mất.

**Giới hạn:**
- `seq` chỉ có 8 bit, nên nếu mất từ 256 gói liên tiếp trở lên thì công thức đếm thiếu. Ở tốc độ khoảng 36,5 frame/s, 256 frame tương đương khoảng 7 giây, nên các đợt xấu dài hơn 7 giây (có thể xảy ra khi `dur` = 5 s) có thể bị đếm thiếu.
- Trong báo cáo của bạn, `seq gap` ở egress ca ON là 992 trong khi emulator thật sự bỏ 11.065 frame. Con số 992 không thể là số frame mất. Định nghĩa của cột này chưa rõ (có thể là số lần phát hiện khoảng trống, đây mới là giả thuyết). **Hãy dùng `frames_dropped_emu` làm số chuẩn** và chỉ dùng `seq gap` để tham khảo sau khi làm rõ định nghĩa trong code.

### 7.5 Mô hình Markov và các tham số bộ mô phỏng

| Thuật ngữ | Ý nghĩa |
|---|---|
| **Chuỗi Markov hai trạng thái** | Mô hình kênh có hai trạng thái: **G** (good, tốt) và **B** (bad, xấu). Mỗi khe thời gian, kênh có một xác suất đổi trạng thái. |
| **Trạng thái G / B** | G dùng xác suất bỏ `lossgood`. B dùng xác suất bỏ `lossbad`. Mô phỏng UAV bị vật cản che rồi thoát ra. |
| **Khe thời gian (`slot`)** | Đơn vị thời gian của chuỗi, mặc định 100 ms. Chuỗi cập nhật trạng thái mỗi khe, **theo thời gian chứ không theo từng gói**. |
| **`urb`** | Urban factor: tỷ lệ thời gian ở trạng thái xấu (0 đến dưới 1). 0 là tắt mô phỏng, 0,5 là một nửa thời gian bị che. Bài báo tham khảo dùng 10%, 50%, 60%. |
| **`dur`** | Thời gian trung bình một đợt xấu, tính bằng giây. |
| **`lossbad`** | Xác suất mất gói khi đang ở trạng thái xấu (1,0 nghĩa là mất hết). |
| **`lossgood`** | Xác suất mất gói khi đang ở trạng thái tốt (thường 0). |
| **`P_gb`** | Xác suất chuyển từ tốt sang xấu trong một khe. |
| **`P_bg`** | Xác suất chuyển từ xấu về tốt trong một khe. `P_bg = slot ÷ dur`. |
| **`π_B` (xác suất dừng)** | Tỷ lệ thời gian lâu dài ở trạng thái xấu = `P_gb ÷ (P_gb + P_bg)`, và bằng `urb`. |
| **Đợt xấu (burst)** | Một chuỗi liên tục các khe ở trạng thái xấu. |
| **Emulator** | Bộ mô phỏng: cố ý bỏ gói theo chuỗi Markov. |
| **`frames_in`** | Số frame vào bộ mô phỏng. |
| **`frames_kept`** | Số frame được Markov giữ trước fan-out. |
| **`frames_dropped_emu`** | Số frame bị bộ mô phỏng bỏ. Đây là bộ đếm **"sự thật"** (ground truth): emulator tự biết nó bỏ bao nhiêu. Luôn có `frames_in = frames_kept + frames_dropped_emu`. |
| **`measurement_drop_fraction`** | `frames_dropped_emu ÷ frames_in` trong đúng cửa sổ đo 600 giây. |
| **`run_bad_fraction`** | Tỷ lệ khe xấu trong cả run, **tính cả warm-up**. Vì cửa sổ khác nhau nên hai số này lệch nhau chút ít, không phải lỗi. |
| **Ground truth ("sự thật")** | Số liệu mà hệ thống biết chắc vì chính nó gây ra, dùng làm chuẩn để kiểm các phép đo gián tiếp. |
| **Thời gian giả** | Khi kiểm định Markov riêng, chạy hàng trăm nghìn khe nhanh bằng mô phỏng, không đợi đủ giờ thật. |

### 7.6 Tài nguyên máy tính và hàng đợi

| Thuật ngữ | Ý nghĩa |
|---|---|
| **CPU % một lõi** | Mức dùng một lõi CPU của tiến trình: (hiệu CPU time) ÷ (hiệu thời gian thực) × 100. |
| **Working set** | Bộ nhớ vật lý tiến trình đang giữ. |
| **Private bytes** | Bộ nhớ riêng của tiến trình. |
| **Handle** | Tay cầm tới tài nguyên hệ điều hành đang mở (socket, file...). Tăng đều theo thời gian là dấu hiệu rò. |
| **Hàng đợi (queue)** | Nơi giữ tạm gói chờ gửi. Mỗi client có hàng đợi riêng. |
| **Client chậm (slow client)** | Client đọc dữ liệu chậm hơn tốc độ gateway gửi, làm hàng đợi của nó đầy dần. |
| **Pending** | Lượng dữ liệu đang chờ gửi cho client. Quá ngưỡng thì gateway ngắt client chậm (báo cáo của bạn: 256 KiB). |
| **Giới hạn số client** | Số kết nối đồng thời tối đa gateway cho phép (báo cáo của bạn: 10). |

### 7.7 Các trường trong `mavlink status` của PX4

| Trường | Ý nghĩa |
|---|---|
| **`tx` (B/s)** | Lượng dữ liệu PX4 gửi ra trên kênh đó mỗi giây. |
| **`txerr`** | Lỗi khi gửi. |
| **`rx` (B/s)** | Lượng dữ liệu PX4 nhận vào (ví dụ từ trạm mặt đất). |
| **`rx loss`** | Tỷ lệ mất gói theo PX4 báo cáo ở chiều nhận. |
| **`partner IP`** | Địa chỉ IP bên đối tác mà kênh đang nối tới. |
| **`transport protocol: UDP (a, remote port: b)`** | Giao thức, cổng cục bộ của PX4 và cổng đích. |
| **`GCS heartbeat valid`** | PX4 đang nhận được HEARTBEAT hợp lệ từ một trạm mặt đất. |
| **`ping statistics`** | Thống kê độ trễ khứ hồi PX4 tự đo với trạm mặt đất. Chỉ cập nhật khi trạm mặt đất gửi gói đồng bộ về, nên với gateway một chiều số này sẽ khác khi nối QGC trực tiếp. |
| **Giới hạn data rate** | Mức tối đa PX4 cho phép gửi trên một kênh. Báo cáo của bạn ghi stream mới bị giới hạn 2880 B/s, nên tốc độ gói khác E0 cũ. |

---

## 8. Lưu báo cáo HTML và đối chiếu nghiệm thu

Mỗi run có một báo cáo HTML tự chứa dữ liệu, biểu đồ và bảng sự kiện. Nếu Gateway dùng một tên file cố định, lưu bản báo cáo nghiệm thu bằng run ID trước khi chạy ca kế tiếp. Báo cáo kết thúc sớm cần ghi thời lượng thực và trạng thái “chưa hoàn tất”.

| Phần báo cáo | Nội dung cần lưu |
|---|---|
| Nhận diện run | run ID, ca, commit, lệnh PX4/Gateway, SYSID, số client, timeout, cấu hình Markov và seed |
| Cửa sổ đo | Mốc đầu và cuối, warm-up, thời lượng thực. Ingress và egress dùng chung cửa sổ. Ghi rõ cửa sổ client nếu client bắt đầu độc lập. |
| Đồ thị thông lượng | Frame/s và byte/s từ chênh lệch counter chia cho thời gian thực giữa hai mẫu. Ghi theo từng UAV, kể cả khoảng có 0 frame. |
| Đồ thị Markov và heartbeat | Timeline G/B, điểm nhận heartbeat, offline/online ở ingress và egress. Ghi thời điểm chuyển G/B theo mốc khe. |
| Bảng số liệu | Ingress, kept, dropped, egress, CRC lỗi theo điểm đo, phân bố msgid, jitter heartbeat, số lần offline và thời gian offline. |
| Bảng sự kiện | Client connect/disconnect, hàng đợi, route, ACK, BLOCK, hủy và đổi endpoint. Ghi timestamp và định danh UAV/client. |
| Tải và tài nguyên | Đo các mức 1/5/10 client nếu Gateway giới hạn 10. Ghi frame/s của client đang theo dõi, peak pending, CPU, RAM và handle ở các mốc cùng giai đoạn. |
| Tiêu chí và kết luận | Ghi PASS, FAIL, CHƯA THỬ hoặc CHƯA ĐỦ BẰNG CHỨNG cho từng ca, kèm liên kết đến run hoặc sự kiện liên quan. |

Dùng 60 giây để kiểm tra nhanh cấu hình và đường dẫn báo cáo. Với lần đo chính, dùng warm-up 10 giây và cửa sổ đo 600 giây. Giữ stream PX4 và trạng thái UAV như nhau giữa OFF, ON và OUTAGE. So CRC lỗi, số frame Markov bỏ và sequence gap ở các cột riêng. Nếu cần định lượng ảnh hưởng của client chậm, giữ một client nền, ghi tốc độ nhận của nó trước, trong và sau khi client chậm xuất hiện.

### 8.1 Các mốc cần giữ

- Giữ E0 lịch sử cùng cấu hình cũ. Khi stream hoặc phiên bản Gateway thay đổi, chạy OFF mới cùng cấu hình với các ca Markov để có mốc so sánh phù hợp.
- Issue 16 lưu ca fan-out, reconnect, client chậm và giới hạn kết nối. Issue 17 lưu route, BLOCK, phục hồi và hủy queue. Issue 18 lưu test Markov độc lập. Issue 19 lưu OFF, ON, OUTAGE và ca thêm client để kiểm tra mỗi frame chỉ được quyết định một lần trước fan-out.
- Phép đo này đánh giá PX4 SITL và Gateway trong môi trường thử. Độ trễ một chiều, chất lượng vô tuyến thực tế và tỷ lệ mất mạng chính xác cần thiết kế phép đo riêng nếu đề cương yêu cầu.

## 9. Tóm tắt một dòng cho từng lần đo

| Lần đo | Đo để làm gì |
|---|---|
| A1: QGC nối trực tiếp PX4 | Xác nhận hệ thống chuẩn chạy, có mốc quan sát |
| A2: serverUDP nối PX4 | Biết PX4 gửi gì, bao nhiêu; có mốc đếm và mốc nhiễu nền |
| B1: gateway, mô phỏng tắt (E0) | Chứng minh gateway không làm đổi dữ liệu, biết chi phí tài nguyên |
| B2: nhiều client, client chậm | Chứng minh hàng đợi riêng và độ bền khi quản lý kết nối |
| C1: bộ Markov riêng | Chắc bộ mô phỏng đúng trước khi dùng |
| C2: OFF / ON / OUTAGE | Chứng minh mất gói đúng mô hình và xem đợt xấu dài gây mất liên kết thế nào |
| D: hai UAV | Chứng minh định tuyến đúng đích, BLOCK, phục hồi và hủy queue |
