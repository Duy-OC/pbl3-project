# Kiểm kê bằng chứng hiện có để kiểm thử lại toàn hệ thống

Đọc trực tiếp các file hiện có trong `docs/ketqua/` ngày 2026-11-12. Không tạo số đo mới và không điền phần chưa quan sát bằng số suy đoán. Tình trạng issue GitHub là lịch sử; bảng này chỉ đánh giá bằng chứng còn có ở workspace hiện tại.

| Ca | Bằng chứng hiện có | Kết luận hiện tại / còn thiếu |
|---|---|---|
| A1 QGC trực tiếp | `PBL3-01_A1/A1_QGC_truc_tiep.md`: ba mẫu `mavlink status`, GCS heartbeat hợp lệ, RX loss 0% ở chiều PX4 nhận. | **Tạm hoãn theo yêu cầu người đo**: chưa ghi QGC có hiện UAV/cập nhật liên tục 2 phút, ảnh hoặc trạng thái Gazebo; chưa có lệnh stream, thời gian và phiên bản PX4. Không đánh dấu PASS. |
| A2 UDP trực tiếp | Một khảo sát 60 s, một run 600 s trong `PBL3-03_A2/A2_*.md`; run 600 s có 34522 frame, 1394433 byte, 31 MSGID, 566 HEARTBEAT, CRC/signature 0; nguồn `172.26.162.42:14600` được đối chiếu với PX4 instance #4. | **Đạt mục đích mốc UDP khảo sát**, nhưng thiếu metadata nguồn/stream/phiên bản. Người đo đã bỏ run lặp thêm; không xem đây là bằng chứng độ phân tán qua nhiều run. |
| E0 Gateway OFF | `PBL3-14_E0/E0_01/E0_E0_01_*` và `PBL3-14_E0/E0_01/E0_1794462221839_client_rx*.csv`. Ingress và egress: 120 dòng đến `t=600.000`, mỗi điểm 34900 frame, 1410290 byte, 572 HEARTBEAT, jitter 22.983 ms, offline 0. Từng mốc frame/byte/heartbeat khớp; 31 MSGID cuối khớp tổng. Summary: in=kept=34900, drop=0, QoS ingress=egress=34900. Client: 120 dòng đến 600 s, 34818 frame, 1406906 byte, 571 HEARTBEAT, jitter 25.500 ms, offline 0; 31 MSGID khớp tổng của chính client. | **Đạt đối soát chính của E0**, nhưng **chưa đủ bằng chứng nghiệm thu toàn ca**: `E0_E0_01_config.txt` dài 0 byte, không có log STATS/CRC tổng hoặc CPU/RAM/handle, lệnh run/stream, hash source, số client thực tế. Chênh 82 frame giữa client và Gateway không được coi là mất gói vì cửa sổ đo khác nhau. Cần E0 mới sau khi sửa file config. |
| T0 thành phần cơ bản | Source `src/` có parser, serializer và socket; test cũ đã được xóa theo yêu cầu dọn dự án. | **Chưa thử lại** UDP echo, parser chia/gộp, CRC hỏng, payload byte 0 và signature. |
| QGC qua Gateway TCP | Không có run/ảnh/log mới. | **Chưa thử lại** hiển thị và cập nhật qua TCP 5760. |
| B2 nhiều client | Không có run/log mới. | **Chưa thử lại** fan-out 1/5/10 client, 20 reconnect, client chậm, giới hạn 10, CPU/RAM/handle. |
| D1–D5 route hai UAV | Không có run/log mới. | **Chưa thử lại** command/ACK đúng đích, BLOCK khi offline, phục hồi endpoint, hủy queue, queue đầy/sendto lỗi. |
| R phân tích/bàn giao | Có hướng dẫn và kế hoạch đo; README hiện rất ngắn. | **Chưa có** script phân tích mới, biểu đồ, báo cáo HTML mới, hồ sơ môi trường đầy đủ, README tái lập, slide/video mới. |

## Việc đã sửa sau khi đọc E0

`src/gateway.cpp` nay đóng và kiểm tra file `_config.txt` ngay trước vòng lặp nhận dữ liệu. Trước sửa, file E0 cũ trống vì tiến trình có thể bị dừng khi `ofstream` còn mở. Không thay hoặc tự bổ sung nội dung vào file thô E0 cũ. Mã mới đã qua `g++ -std=c++17 -fsyntax-only src/gateway.cpp`; chưa chạy một run mới.

## Thứ tự tiếp tục

1. A1 tạm hoãn theo yêu cầu người đo; giữ trạng thái `CHƯA ĐỦ BẰNG CHỨNG` cho đến khi có quan sát QGC.
2. Theo đúng thứ tự trong `docs/TRINH_TU_KIEM_THU_THEO_ISSUE.md`, bước đang làm là PBL3-02 (UDP WinSock echo). A1 tạm hoãn; không tự chuyển sang E0.
3. Tới PBL3-14, chạy E0 mới bằng bản Gateway đã sửa; kiểm file config ngay khi khởi động, giữ STATS/tài nguyên. Run ID mới `E0_02`.
