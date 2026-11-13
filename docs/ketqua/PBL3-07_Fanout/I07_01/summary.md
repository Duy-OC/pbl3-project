# PBL3-07 — fan-out ba TCP client, run I07_01

Nguồn: `gateway.log`, `client_A.log`, `client_B.log`, sáu file Gateway và bốn CSV client trong cùng thư mục. Đây là ca kết nối ngắn, không phải bài tải 600 giây PBL3-16/E4.

## Cấu hình và số liệu

Gateway: `scenario=I07_FANOUT`, `run_id=I07_01`, `urb=0`, `seed=1`, warm-up 10 s. CSV ingress và egress có 29 mốc đến `t=145.612 s`; mỗi điểm có 8409 frame, 339560 byte, 136 HEARTBEAT. Từng mốc frame/byte ingress và egress khớp, CRC UDP/TCP trong log bằng 0.

| Điểm nhận | Cửa sổ CSV đã ghi | Frame | Byte frame | HEARTBEAT | Nhóm MSGID | Tổng MSGID khớp? |
|---|---:|---:|---:|---:|---:|---|
| Client A | 120.494 s | 6978 | 281558 | 113 | 40 | Có |
| Client B | 60.322 s | 3549 | 142662 | 57 | 40 | Có |

Counter vận hành cuối trong log client A: `Frames=7542`, `TCP RX bytes=304418`; client B: `Frames=4122`, `TCP RX bytes=165949`. Hai counter này tính từ khi client bắt đầu chạy, còn CSV bỏ 10 giây warm-up, vì vậy không thay lẫn cho nhau.

## Trình tự kết nối/ngắt

- Gateway ghi `Client #1`, `#2`, `#3 CONNECTED`, rồi nhiều STATS `Clients=3`. Client #3 có gửi request tới UAV 1 và nhận ACK command 512, phù hợp vai trò QGC; đây là suy luận từ log, không có định danh QGC trong dòng CONNECTED.
- Gateway ghi `Client #2 DISCONNECTED | TCP TX=168673 bytes`, sau đó `Clients=2` qua nhiều mẫu cho đến cuối run. Counter `TCP TX bytes` tiếp tục tăng từ 445067 trước khi ngắt lên 656891 ở cuối. Client A vẫn ghi HEARTBEAT/CLIENT STATS sau thời điểm client B dừng.
- Gateway không ghi CRC, signature, BLOCK, UDP dropped/cancelled hoặc slow-client trong các STATS được đọc.
- Cuối `client_A.log` có `recv loi: 10054` gần lúc Gateway dừng; trước đó client A vẫn nhận dữ liệu. Ghi nhận lỗi đóng kết nối này riêng, không dùng nó để kết luận client B làm A ngắt. Không có log QGC màn hình sau khi B dừng.

## Kết luận

**PASS cho fan-out và cô lập ngắt một client:** ba kết nối TCP cùng hoạt động; sau một disconnect còn hai kết nối và dữ liệu vẫn được gửi/nhận. **Chưa thử** áp lực 1/5/10 client, client đọc chậm và reconnect 20 vòng; các ca đó thuộc PBL3-16.
