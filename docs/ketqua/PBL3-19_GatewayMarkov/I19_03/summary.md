# I19_03 — Markov ON, seed 3, một UAV và một TCP client

Nguồn: `gateway.log`, `client.log` và chín file config/CSV thô trong thư mục này. Dùng lại executable của I19_01; PX4 SYSID 1 từ `172.26.162.42:14600`. Cấu hình `urb=0.5`, `dur_s=2`, `slot_ms=100`, `lossgood=0`, `lossbad=1`, **seed 3**; Gateway và client có cửa sổ riêng 600 giây sau warm-up 10 giây.

| Điểm | Mẫu QoS | Frame tại 600 s | Byte frame | HEARTBEAT | Offline event |
|---|---:|---:|---:|---:|---:|
| Gateway ingress | 120 | 34402 | 1390111 | 564 | 0 |
| Gateway egress | 120 | 18494 | 747504 | 302 | 55 |
| TCP client RX | 119 | 18313 | 740147 | 298 | 55 |

- Gateway và client đều ghi `QOS: t=600`; Gateway tạo `_markov.csv`. Tổng MSGID theo từng mốc khớp frame/byte: 0 lệch (3718 hàng ingress, 3703 egress, 3679 client).
- Markov summary: `frames_in=34402`, `frames_kept=18494`, `frames_dropped_emu=15908`; `18494+15908=34402`, ingress QoS=34402, egress QoS=18494. Tỷ lệ drop cửa sổ `0.462415`; chênh byte ingress/egress 642607.
- CSV trạng thái có 276300/600000 ms ở B trong đúng cửa sổ (`0.460500`). Markov summary `run_slots=6100`, `run_bad_slots=2826`, `run_bad_fraction=0.463279` và mean B 1.76346 s gồm cả 10 giây warm-up.
- Ingress offline=0, egress/client offline=55. CRC/signature/BLOCK/queue dropped/cancelled/slow client=0; CSV ingress không ghi seq gap hay reordered. Chênh frame client/egress do cửa sổ client bắt đầu khác thời điểm.

**Kết luận:** PASS I19_03. Đã có đủ ba seed ON (I19_01–03) để so sánh trước khi đo OUTAGE.
