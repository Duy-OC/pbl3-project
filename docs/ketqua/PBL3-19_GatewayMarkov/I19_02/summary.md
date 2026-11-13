# I19_02 — Markov ON, seed 2, một UAV và một TCP client

Nguồn: `gateway.log`, `client.log` và chín file config/CSV thô trong thư mục này. Dùng lại hai executable của I19_01; PX4 SYSID 1 từ `172.26.162.42:14600`. Cấu hình `urb=0.5`, `dur_s=2`, `slot_ms=100`, `lossgood=0`, `lossbad=1`, **seed 2**; Gateway và client có cửa sổ đo riêng 600 giây sau warm-up 10 giây.

| Điểm | Mẫu QoS | Frame tại 600 s | Byte frame | HEARTBEAT | Offline event |
|---|---:|---:|---:|---:|---:|
| Gateway ingress | 120 | 33781 | 1365117 | 553 | 0 |
| Gateway egress | 120 | 14790 | 597917 | 238 | 63 |
| TCP client RX | 119 | 15234 | 615849 | 244 | 60 |

- Gateway và client đều ghi `QOS: t=600`; Gateway tạo `_markov.csv`. Tổng MSGID khớp frame/byte tại mọi mốc (0 lệch; 3718 hàng ingress, 3714 egress, 3667 client).
- Markov summary: `frames_in=33781`, `frames_kept=14790`, `frames_dropped_emu=18991`; `14790+18991=33781`, ingress QoS=33781, egress QoS=14790. Tỷ lệ drop cửa sổ đo `0.562180`; egress thấp hơn ingress 767200 byte.
- Từ CSV trạng thái, trạng thái B chiếm 334300/600000 ms (`0.557167`) trong đúng cửa sổ đo. `run_slots=6100` và `run_bad_slots=3369` **gồm warm-up 10 giây**; `run_bad_fraction=0.552295` và mean B 2.31528 s của summary không chỉ riêng 600 giây.
- Ingress offline=0, egress offline=63, client offline=60. Ingress có `seq_reordered=13`, `seq_gap_estimate=0`; hiện tượng sequence của luồng nguồn được giữ như quan sát, không quy cho Markov. Client bắt đầu cửa sổ riêng nên không lấy chênh frame client/egress làm mất gói TCP. Gateway CRC/signature/BLOCK/queue dropped/cancelled/slow client=0.

**Kết luận:** PASS I19_02 về đối soát Markov ON seed 2 và phân biệt ingress luôn online với egress có outage. Tỷ lệ drop khác I19_01 (seed 1: 0.506509), không có ngưỡng bắt buộc 50% cho từng run 600 giây.
