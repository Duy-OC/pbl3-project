# I19_01 — Markov ON, seed 1, một UAV và một TCP client

Nguồn: bốn log build/terminal và chín file config/CSV thô trong thư mục này. PX4 UAV SYSID 1 từ `172.26.162.42:14600`; Markov `urb=0.5`, `dur_s=2`, `slot_ms=100`, `lossgood=0`, `lossbad=1`, seed 1. Gateway và client đều có warm-up 10 giây, cửa sổ đo riêng 600 giây.

| Điểm | Mẫu QoS | Frame tại 600 s | Byte frame | HEARTBEAT | Offline event |
|---|---:|---:|---:|---:|---:|
| Gateway ingress | 120 | 33877 | 1368348 | 555 | 0 |
| Gateway egress | 120 | 16718 | 675675 | 275 | 69 |
| TCP client RX | 119 | 16533 | 668147 | 272 | 68 |

- Cả hai build exit 0. Gateway ghi đủ `QOS: t=600` cho ingress và egress, rồi tạo `_markov.csv`; client cũng ghi `QOS: t=600`. Tổng MSGID theo từng mốc khớp frame/byte ở cả ba điểm: 0 mốc lệch (3718 hàng ingress, 3718 egress, 3689 client).
- Markov summary: `frames_in=33877`, `frames_kept=16718`, `frames_dropped_emu=17159`; `16718+17159=33877`, ingress QoS=33877 và egress QoS=16718. Tỷ lệ drop trong cửa sổ đo `0.506509`. Byte egress thấp hơn ingress 692673 byte.
- CSV trạng thái cho 305300 ms ở B trong đúng cửa sổ 600000 ms (`0.508833`). `run_slots=6100`, `run_bad_slots=3098` trong Markov summary **gồm 10 giây warm-up** nên `run_bad_fraction=0.507869` không phải riêng cửa sổ 600 giây. Mean B 2.15845 s cũng tính cả đoạn này.
- Ingress có 0 offline event; egress có 69, client có 68 trong cửa sổ riêng. Chênh client/egress 185 frame do cửa sổ client bắt đầu khác thời điểm; không tính trực tiếp là mất gói TCP. Gateway STATS giữ một UAV online, một client trong thời gian đo; CRC/signature, BLOCK, queue dropped/cancelled và slow client đều 0.

**Kết luận:** PASS I19_01 về ghép Markov trước egress/fan-out và đối soát frame trong một run ON seed 1. PBL3-19 còn các seed ON khác, OUTAGE và kiểm nhiều client dùng cùng quyết định drop.
