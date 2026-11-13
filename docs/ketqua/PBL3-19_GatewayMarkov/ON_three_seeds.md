# Markov ON — tổng hợp ba seed trong Gateway

Cả ba run I19_01–03 dùng cùng executable, một PX4 SYSID 1 từ cổng 14600, một TCP client, `urb=0.5`, `dur_s=2`, `slot_ms=100`, `lossgood=0`, `lossbad=1`, mỗi run 600 giây sau warm-up 10 giây. Số liệu và giới hạn chi tiết: [seed 1](I19_01/summary.md), [seed 2](I19_02/summary.md), [seed 3](I19_03/summary.md).

| Seed | Ingress | Egress | Drop | Tỷ lệ drop | Tỷ lệ thời gian B trong 600 s | Offline egress |
|---:|---:|---:|---:|---:|---:|---:|
| 1 | 33877 | 16718 | 17159 | 0.506509 | 0.508833 | 69 |
| 2 | 33781 | 14790 | 18991 | 0.562180 | 0.557167 | 63 |
| 3 | 34402 | 18494 | 15908 | 0.462415 | 0.460500 | 55 |
| Tổng | 102060 | 50002 | 52058 | 0.510073 | 0.508833 | 187 |

Mỗi run có `ingress=egress+drop`, tổng MSGID khớp từng mốc, Gateway ingress offline=0 còn egress có outage. Tỷ lệ drop khác nhau theo seed; không coi ba run là ba mẫu độc lập của luồng PX4 tuyệt đối giống hệt, vì nguồn SITL được chạy ở các thời điểm khác nhau. `run_slots` trong `_markov.csv` gồm warm-up; tỷ lệ thời gian B ở bảng được tính lại từ `_states.csv` trên đoạn 10–610 giây.
