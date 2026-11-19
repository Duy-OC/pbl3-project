# Đo mất sóng UAV → Gateway bằng Markov

## Mục đích

Giả lập đường truyền UAV bị mất sóng theo từng đợt và xem Gateway nhận được bao nhiêu frame MAVLink. Chỉ đo **chiều UAV → Gateway**; không dùng dữ liệu TCP client để tính mất gói.

## Cách hoạt động

```text
PX4 UAV → proxy UDP 14551 → Gateway UDP 14550
              ↑ Markov tạo các đợt mất sóng
```

PX4 gửi tới proxy thay vì gửi thẳng Gateway. Proxy có hai trạng thái của mô hình Gilbert–Elliott:

- **G (có sóng):** giữ mọi frame (`lossgood=0`).
- **B (mất sóng):** bỏ mọi frame (`lossbad=1`).

`urb` điều chỉnh tỷ lệ thời gian trung bình ở B; `dur_s` điều chỉnh thời gian trung bình một đợt B. Đây là **mô phỏng mất sóng**, không phải phép đo chất lượng sóng vô tuyến thật. Gateway chạy `--urb 0` để tránh áp dụng Markov lần thứ hai. Bài kiểm độc lập [I18_01](ketqua/PBL3-18_Markov/I18_01/summary.md) đã đạt cho đúng cấu hình G giữ hết/B bỏ hết này.

## Trình tự đơn giản

| Lượt | Việc làm | Điều cần thấy |
|---|---|---|
| U19_00 | Proxy không bỏ frame, chạy khoảng 30 giây | **PASS**: [10139 frame proxy gửi = Gateway nhận](ketqua/PBL3-19_GatewayMarkov/upstream/U19_00/summary.md). Đây chỉ là bước kiểm kết nối. |
| U19_01 | `urb=0`, đo 600 giây | **PASS**: [33890 frame giữ/gửi, 0 drop](ketqua/PBL3-19_GatewayMarkov/upstream/U19_01/summary.md); trace 38612/38612. |
| U19_02 | `urb=0.5`, `dur_s=2`, `lossgood=0`, `lossbad=1`, đo 600 giây | **PASS**: [16912/33458 frame bị bỏ khi mất sóng](ketqua/PBL3-19_GatewayMarkov/upstream/U19_02/summary.md); trace proxy → Gateway khớp. |
| U19_03 | Giữ `urb=0.5`, đổi `dur_s=5`, đo 600 giây | Các đợt mất sóng dài hơn U19_02. |

Mỗi lượt đo chính có 10 giây khởi động rồi 600 giây ghi số liệu. Giữ một UAV, cùng luồng MAVLink và cùng máy đo. Trước mắt chạy **một seed cố định là 1** cho mỗi cấu hình; chỉ lặp thêm seed khi cần đánh giá độ dao động.

## Đọc kết quả

1. `proxy_pre`: số frame proxy nhận từ UAV trước khi mô phỏng mất sóng.
2. `dropped_markov`: số frame proxy bỏ vì đang mất sóng.
3. `proxy_post`: số frame proxy gửi thành công tới Gateway.
4. `gateway_ingress`: số frame Gateway thực nhận. Dùng file trace để kiểm `missing=0`, `extra=0` giữa proxy gửi và Gateway nhận.

Trong cùng cửa sổ đo của proxy cần `pre = kept + dropped_markov` và, khi `send_errors=0`, `kept = post_sent`. File QoS của Gateway bắt đầu cửa sổ riêng, nên đối chiếu **trace từng frame** để xác nhận proxy gửi tới Gateway; không lấy hiệu hai tổng QoS lệch thời điểm để tính mất gói.

Proxy đặt trên Windows, sau đoạn PX4/WSL → Windows. Vì vậy kết quả cho biết **mất gói được mô phỏng trước Gateway** và frame Gateway thực nhận; nó không đo được gói đã mất trước khi tới proxy.

**Bước đang làm:** [U19_03 và lệnh chạy](GHI_LOG_KIEM_THU.md#u19_03-giả-lập-mất-sóng-dài-uav-tới-gateway). Chỉ người đo chạy và lưu kết quả.
