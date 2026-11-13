# PBL3-11 — tách bộ đếm STATS

- [I11_01](I11_01/summary.md): hai frame CRC hỏng được đếm riêng `UDP=1`, `TCP=1`; `Blocked messages=0`, `UDP dropped/cancelled=0`, `Slow clients=0`.
- [I09_01](../PBL3-09_Recovery/I09_01/summary.md): no-route/BLOCK tăng riêng khi UAV đích OFFLINE.
- [I16_02](../PBL3-16_Clients/I16_02/summary.md): một client chậm bị ngắt, `Slow clients=1`; các client khác tiếp tục.
- [I17_01](../PBL3-17_Queue/I17_01/summary.md), [I17_02C](../PBL3-17_Queue/I17_02C/summary.md), [I17_03](../PBL3-17_Queue/I17_03/summary.md), [I17_04](../PBL3-17_Queue/I17_04/summary.md): bộ đếm `UDP dropped/cancelled` tăng khi hủy queue hoặc `sendto` lỗi; log từng run phân biệt nguyên nhân.

**Kết luận:** PASS việc tách các nhóm lỗi trong STATS qua những lượt kích hoạt riêng. Bộ đếm `UDP dropped/cancelled` gộp nhiều nguyên nhân, nên phải dùng log nguyên nhân của từng lượt để phân tích chi tiết.
