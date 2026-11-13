# I18_01 — mô hình Markov độc lập

Nguồn: `build.log` và `model.log`. Helper đọc trực tiếp `src/emulator.h`; thời gian được mô phỏng, không dùng PX4 hay Gateway. Cấu hình ON: `urb=0.5`, `dur_s=2`, `slot_ms=100`, `lossgood=0`, `lossbad=1`; mỗi seed 180000 slot và một frame mỗi slot.

| Seed / SYSID | Slot B / tổng | Tỷ lệ B | Đợt B hoàn chỉnh | Trung bình đợt B | Frame drop / seen |
|---|---:|---:|---:|---:|---:|
| 1 / 1 | 89275 / 180000 | 0.495972 | 4557 | 1.958789 s | 89275 / 180000 |
| 2 / 1 | 92485 / 180000 | 0.513806 | 4517 | 2.046646 s | 92485 / 180000 |
| 3 / 1 | 90656 / 180000 | 0.503644 | 4569 | 1.982491 s | 90656 / 180000 |
| 1 / 2 | 92382 / 180000 | 0.513233 | 4529 | 2.039678 s | 92382 / 180000 |

`build_exit=0`, `model_exit=0`, `MARKOV MODEL RESULT: pass=1`. Mỗi seed có `seen=kept+dropped`; xác suất chuyển trạng thái ghi `pBG=pGB=0.05`. Dòng CHECK xác nhận cùng seed tái lập, SYSID khác và seed khác cho chuỗi riêng, tick từng slot và tick nhảy cho cùng lịch trạng thái. Với `urb=0`, 180000/180000 frame được giữ, 0 slot B và 0 drop. Năm cấu hình sai đều bị từ chối.

**Kết luận:** PASS PBL3-18 cho các bất biến và phân bố kiểm bằng 180000 slot giả/seed. Đây là kiểm thử mô hình rời rạc, chưa phải kết quả khi ghép vào Gateway; việc đó thuộc PBL3-19.
