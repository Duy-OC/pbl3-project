# Đo lại Gateway theo kịch bản K0–K7

Chạy bình thường: `gateway.exe`, `tcpclient.exe` và proxy không tự ghi CSV/log hoặc khởi chạy bài kiểm thử. Các script dưới đây là công cụ đo riêng; chúng truyền `--measure-seconds` để bật ghi kết quả khi được gọi rõ ràng.

Thời lượng thực hiện theo chỉ đạo mới nhất: **K1–K2 đo 120 giây × 3 lượt; từ K3 đo 60 giây × 2 lượt**. Mỗi lượt có thêm 5 giây ổn định. K0 dùng 100.000 gói tổng hợp mỗi lượt. K6 giữ 50 lệnh mỗi mức loss, mỗi lượt. Dữ liệu cũ A1/B5/B10 (60 giây) không trộn vào trung bình mới.

## Cấu hình và định nghĩa

| Bài | Cổng PX4 gửi tới | Gateway UDP | Mô hình |
|---|---:|---:|---|
| K1 | 14550 | 14550 | Không có proxy |
| K2 | 14550 | 14551 | 5% loss, cụm trung bình 4 **frame**, cả hai chiều |
| K3 | 14550 | 14551 | 5% loss, cụm trung bình 1 **frame**, cả hai chiều |
| K4 | 14550 | 14551 | 10% loss, cụm trung bình 4 **frame**, cả hai chiều |
| K5 | 14550 | 14551 | Không thả gói; trạng thái xấu có delay ngẫu nhiên 20–80 ms ở chiều PX4 → gateway |
| K7 | 14550 | 14551 | Không thả gói; xác suất 2% lật 1 bit payload chiều PX4 → gateway |
| K6 | 14650 (UAV giả) | 14651 | Mức 0/5/10% ở hai chiều, cụm trung bình 4 frame; 50 lệnh ARM/mức |

K2–K4 và K6 chuyển trạng thái Markov **một lần cho mỗi frame/datagram**, nên `r = 1/cụm_TB` có đơn vị *mỗi gói*. Với trạng thái tốt giữ toàn bộ, trạng thái xấu thả toàn bộ, `p = r·L/(1−L)`:

| Bài/mức | `L` | `r = P(B→G)` | `p = P(G→B)` |
|---|---:|---:|---:|
| K2/K6 5% | 0,05 | 0,25 | 0,013158 |
| K3 | 0,05 | 1 | 0,052632 |
| K4/K6 10% | 0,10 | 0,25 | 0,027778 |

K5 dùng Markov theo **thời gian**: slot 100 ms, `urb=10%`, `dur_s=0,4`, `lossgood=lossbad=0`; chỉ gói đến ở `B` được xếp hàng chờ 20–80 ms. Bảng này không gọi delay 20–80 ms là “jitter đo được”: jitter ATTITUDE là biến thiên khoảng cách nhận, gồm cả jitter nền và tác dụng xếp hàng.

## Môi trường và PX4

Ghi một lần trong [moi_truong_do_K.md](moi_truong_do_K.md). PX4 SITL gửi tới IP Windows `172.26.160.1:14550`:

```text
mavlink start -u 14600 -o 14550 -t 172.26.160.1 -r 100000
mavlink stream -u 14600 -s HEARTBEAT -r 1
mavlink stream -u 14600 -s ATTITUDE -r 10
```

Nếu instance cổng 14600 đã chạy, không chạy `mavlink start` lần nữa. Kiểm tra `mavlink status`; Windows có thể kiểm tra nhanh bằng `python -B scripts/probe_px4.py 14550` khi gateway/proxy **chưa mở**. PX4 giữ chạy liên tục qua các lượt K1–K5/K7.

## Build và chạy

Từ PowerShell tại thư mục gốc dự án:

```powershell
New-Item -ItemType Directory -Force bin | Out-Null
g++ -std=c++17 -O2 -Isrc/common -Isrc/gateway -Ithird_party -o bin/gateway.exe src/gateway/gateway.cpp -lws2_32
g++ -std=c++17 -O2 -Isrc/common -Isrc/proxy -Ithird_party -o bin/upstream_markov_proxy.exe src/proxy/upstream_markov_proxy.cpp -lws2_32
g++ -std=c++17 -O2 -Isrc/common -o bin/markov_k0.exe tests/markov_k0.cpp
```

K0 đã có kết quả trong [thư mục K0](K0/20261114_172116/result.csv), gồm 3 seed cho mỗi mức. Để tạo thêm một lượt K0:

```powershell
$run = Get-Date -Format 'yyyyMMdd_HHmmss'
$dir = "docs/ketqua/K0/$run"
New-Item -ItemType Directory -Force $dir | Out-Null
& .\bin\markov_k0.exe "$dir/result.csv" 2>&1 | Tee-Object -FilePath "$dir/run.log"
python -B scripts/summarize_k0.py $dir
```

Khi PX4 đang gửi, chạy **một bài một lần**. Script tự mở gateway/proxy, đợi 5+120 giây, ghi `result.csv`, dừng chương trình và chuyển sang lượt kế tiếp:

```powershell
python -B scripts/run_k_live.py K1 --repeats 3 --seconds 120
python -B scripts/run_k_live.py K2 --repeats 3 --seconds 120
python -B scripts/run_k_live.py K3 --repeats 2 --seconds 60
python -B scripts/run_k_live.py K4 --repeats 2 --seconds 60
python -B scripts/run_k_live.py K5 --repeats 2 --seconds 60
python -B scripts/run_k_live.py K7 --repeats 2 --seconds 60
```

Không chạy các lệnh trên song song vì chúng dùng chung cổng 14550, TCP 5760. Mỗi lượt lưu ở `docs/ketqua/<K>/<ngày_giờ>_r<n>/`, có `gateway.log`, `proxy.log` nếu dùng proxy, CSV QoS, trace, `run.log` và `result.csv`. `docs/ketqua/<K>/summary.csv` có **trung bình, độ lệch chuẩn mẫu và số lượt** cho các đại lượng số; từ K3 độ lệch chuẩn dựa trên 2 lượt.

K6 dùng UAV giả chỉ gửi HEARTBEAT và trả `COMMAND_ACK` cho ARM. Điều này tránh đổi trạng thái bay của PX4 50 lần. Cũng chạy từng mức một sau khi K1–K5/K7 xong:

```powershell
python -B scripts/run_k6.py --loss 0 --repeats 2 --commands 50 --seconds 60 --spacing 0.7
python -B scripts/run_k6.py --loss 5 --repeats 2 --commands 50 --seconds 60 --spacing 0.7
python -B scripts/run_k6.py --loss 10 --repeats 2 --commands 50 --seconds 60 --spacing 0.7
```

K6 ghi `commands.csv` từng lệnh, `result.csv` từng lượt và `summary.csv` mỗi mức. `retry_mean_per_command = (số COMMAND_LONG gateway gửi − số lệnh được nhận chờ ACK)/50`; `success_pct = số lệnh có ACK thành công/50 × 100%`. Kết quả K6 là độ tin cậy của logic gateway dưới mô phỏng mất gói; không phải tỷ lệ thành công cất cánh/hạ cánh của PX4 thật.

## Đọc kết quả và giới hạn

- `loss_gateway_pct` dùng sequence của frame **hợp lệ** mà gateway nhận: `seq_gap_estimate/(frames+seq_gap_estimate)`. `proxy_loss_pct` dùng quyết định thả gói trong cửa sổ proxy: `dropped/pre`. Hai cửa sổ có thể lệch. Với K3/K4, chạy `python -B scripts/compare_markov_common.py <thư_mục_lượt>` để tạo `common.csv` trên khoảng thời gian giao nhau của hai trace. K2 dùng bản ghi trước khi bổ sung trace gateway, nên chỉ có so sánh tổng 120 giây gần đúng. Không dùng phép cộng các gap *tạm thời* trong `common.csv` để kết luận loss ở K5 vì delay có thể làm gói đến đảo thứ tự rồi được sequence tracker sửa lại.
- `gateway_bursts` là số lần sequence nhảy cóc, `gateway_burst_mean_packets` và `gateway_burst_max_packets` là độ dài các bước nhảy. Nếu gói đến muộn, sequence tracker có thể sửa `seq_gap_estimate`; file `_bursts.csv` vẫn là sự kiện nhảy quan sát ban đầu.
- `attitude_jitter_std_ms` là độ lệch chuẩn mẫu của các khoảng ATTITUDE liên tiếp, đo bằng QueryPerformanceCounter; bỏ khoảng có gap MAVLink đã phát hiện giữa hai ATTITUDE. `attitude_jitter_max_ms` là độ lệch tuyệt đối lớn nhất so với khoảng trung bình của lượt, không phải độ trễ một chiều.
- K7 đối chiếu `flipped_sent` của proxy với `crc_errors` gateway. Sai khác có thể đến từ cửa sổ không trùng hoặc datagram không tới; giữ `gateway.log`, `_faults.csv` và `_post_trace.csv` khi giải thích.
- Chiều gateway → PX4 chủ yếu HEARTBEAT 1 Hz. Trong 120 giây có khoảng 120 datagram, nên phần trăm loss của chiều này vẫn dao động mạnh; luôn báo cả tử số/mẫu số.
- Với mức đúng bằng 0, công thức sai số tương đối `|đo−đúng|/đúng` không xác định; ghi sai số tuyệt đối theo **điểm phần trăm**. Với mức khác 0, dùng mẫu số là giá trị *proxy thực thả* khi kiểm tra gateway, và mức cấu hình khi kiểm định mô hình trên nhiều seed.

K0/K1–K7 và các bài chức năng là hai loại bằng chứng khác nhau. Bảng B5/B7/B12/B13 cần kết quả test parser/sequence/TCP/khóa; kết quả đo QoS không thay thế chúng. Bảng/biểu đồ H1–H5 chỉ lập sau khi các lượt liên quan đủ dữ liệu; không điền số từ các lượt 60 giây cũ.
