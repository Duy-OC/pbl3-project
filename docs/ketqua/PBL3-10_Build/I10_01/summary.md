# PBL3-10 — build riêng Gateway và TCP client

Nguồn: `build.log` do người đo chạy lúc `2026-11-12T14:27:24.3728205+07:00`, đối chiếu lại hai executable trên đĩa.

| Thành phần | Lệnh | Exit | Kích thước | SHA-256 đối chiếu |
|---|---|---:|---:|---|
| Gateway | `g++ -std=c++17 -O2 src\gateway.cpp -o bin\gateway.exe -lws2_32` | 0 | 243261 byte | `36E92F9D4D286814A5BDB465AA2920C47C586C08A6D0ED64430F439AE93B6DD2` |
| TCP client | `g++ -std=c++17 -O2 src\tcpclient.cpp -o bin\tcpclient.exe -lws2_32` | 0 | 140548 byte | `EEB0F26968F3FE14525CE1327688FF34589949A5B78B436B53F2D3225F226FE6` |

Compiler: MinGW.org GCC 6.3.0. Git HEAD ghi trong log: `79fd39fd73371dfea43d092ee4a78994e046d607`. Hai SHA-256 tính lại từ file hiện có khớp log. Vì working tree còn thay đổi chưa commit, Git HEAD một mình không xác định toàn bộ source; dấu vết source chi tiết thuộc PBL3-15.

**Kết luận:** PASS build riêng hai thành phần từ source hiện tại; chưa đánh giá khả năng tái lập trên máy khác (PBL3-22).
