# PBL3-12 — kiểm đủ cửa sổ QoS và tổng MSGID từ E0_01

Nguồn dữ liệu đo do người đo chạy trước đó tại `docs/ketqua/PBL3-14_E0/E0_01/`. Kiểm toán đọc-only bằng `Import-Csv`, ghép mỗi mẫu theo `t_s`, `sysid`, `compid`, cộng `frames` và `frame_bytes` của mọi MSGID rồi so với hàng tổng cùng thời điểm. Không tạo lượt đo mới.

| Điểm | Số mẫu QoS | Hàng MSGID | Mẫu sai tổng frame/byte | Mốc cuối | Frame cuối | Byte frame cuối |
|---|---:|---:|---:|---:|---:|---:|
| Gateway ingress | 120 | 3720 | 0 | 600.000 s | 34900 | 1410290 |
| Gateway egress | 120 | 3720 | 0 | 600.000 s | 34900 | 1410290 |
| TCP client RX | 120 | 3718 | 0 | 600.000 s | 34818 | 1406906 |

Client chạy một cửa sổ độc lập nên tổng client không cần bằng tổng Gateway tại cùng mốc t. Các phép đối chiếu MSGID trong từng điểm đều đạt. Config `E0_01` còn trống, vì vậy baseline đầy đủ vẫn phải bổ sung ở PBL3-14; thiếu sót đó không làm mất bằng chứng CSV 600 s của PBL3-12.

**Kết luận:** PASS đủ 120 mẫu đến 600 s và tổng MSGID khớp frame/byte tại mọi mẫu của cả ba điểm.
