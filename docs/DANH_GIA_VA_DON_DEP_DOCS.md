# Nhận xét docs và phương án dọn file

Ngày kiểm tra: 07/11/2026. Nguồn: docs.rar, 16 file. Chỉ sửa/tạo tài liệu tổng hợp; không sửa, cắt hoặc xóa CSV/log gốc.

## 1. Kết luận chính

- Bộ E0 gốc: đủ 600 giây, giữ nguyên bốn CSV; khớp SHA-256 đã ghi trước đây.
- Issue 16: A/B/C/D có bằng chứng đạt trong phạm vi log và các xác nhận người thực hiện trong client_tests.md. Báo cáo cũ chưa tích D dù log đã có 10 client và từ chối vượt mức.
- Số client nền thật ở B/C/D là 3, không phải 2. Đây là sai mô tả hồ sơ, không phải Gateway giữ thừa socket: client #4 đã reconnect và chưa đóng.
- Tài nguyên reconnect: handles 94→94, RAM 6.65→6.62 MB, private 1.13→1.05 MB; không thấy tăng bất thường giữa hai mẫu, chưa chứng minh tuyệt đối không leak.
- Có 15 BLOCK/no route trước khi học UAV; giữ nguyên con số và giải thích giai đoạn khởi tạo, không sửa thành 0.
- Client chậm bị ngắt một lần; giới hạn nhận 10 client, từ chối hai yêu cầu khi nền đã là 3.
- E0_BASELINE.md bị vỡ bảng và lẫn một phần dòng hash; bản sửa khôi phục cấu trúc và giữ kết quả đã kiểm chứng.

## 2. File đã sửa/tạo

| File tải về | Đặt tại dự án | Thay đổi |
|---|---|---|
| E0_BASELINE_DA_SUA.md | docs/E0_BASELINE.md | Sửa bảng, phục hồi hồ sơ hash gốc, phân biệt baseline với test client; không điền giả metadata |
| client_tests_DA_SUA.md | docs/client_tests.md | Điền A–D, sửa nền 3 client, nhận xét tài nguyên và BLOCK; ghi giới hạn bằng chứng |
| bang_msgid.md | docs/bang_msgid.md | Bảng đầy đủ 31 msgid từ baseline gốc |
| DANH_GIA_VA_DON_DEP_DOCS.md | docs/DANH_GIA_VA_DON_DEP_DOCS.md | Báo cáo này, danh mục file và cách sắp xếp |

Hai file có hậu tố DA_SUA là bản để bạn thay nội dung file tương ứng trong dự án; sau khi đối chiếu có thể đổi về tên gốc. Không cần giữ đồng thời hai bản giống nhau trong thư mục làm việc chính. Hồ sơ E0 được sửa dựa trên bản nguồn đã kiểm tra trước; docs.rar chỉ chứa tài liệu/log/CSV, không xác nhận commit/tag hoặc thư viện ZIP đã được lưu.

## 3. Kiểm tra toàn bộ CSV

Tổng cộng 12 CSV, gồm sáu cặp tổng hợp/msgid. Không có dòng sai số cột; không có thời gian hoặc counter frame/byte giảm trong mỗi stream. Sáu cặp đối soát đúng tại mọi thời điểm.

| File tổng hợp | Số dòng | t_s cuối | Frames cuối | Phân loại |
|---|---:|---:|---:|---|
| E0_client_rx_1793938128483.csv | 120 | 600.000 | 36415 | Baseline E0 gốc |
| E0_client_rx_1794011939330.csv | 120 | 600.000 | 34825 | Test client |
| E0_client_rx_1794012053592.csv | 13 | 65.266 | 3792 | Test client, phiên một phần |
| E0_client_rx_1794012180678.csv | 120 | 600.000 | 34806 | Test client |
| E0_gateway_ingress_1793938099255.csv | 120 | 600.000 | 36416 | Baseline E0 gốc |
| E0_gateway_ingress_1794011910944.csv | 120 | 600.000 | 34825 | Test client |

Các bộ test client có tốc độ trung bình khoảng 58 frame/s, thấp hơn khoảng 60.69 frame/s ở E0 gốc; mean heartbeat khoảng 1051 ms và jitter khoảng 33–35 ms cũng khác E0. Đây là hai run/điều kiện khác nhau, chưa đủ để quy nguyên nhân cho Gateway hay tải client. Trong các cửa sổ ghi nhận, sequence gap/duplicate/reorder/resync/uncertain và outage đều bằng 0.

## 4. Danh mục từng file gốc và quyết định

| File gốc trong docs.rar | Quyết định | Lý do |
|---|---|---|
| E0_BASELINE.md | Thay bằng bản sửa | Markdown vỡ bảng, thiếu đoạn hồ sơ; giữ lịch sử cũ trong Git hoặc bản sao nếu cần. |
| client_tests.md | Thay bằng bản sửa | Sai số client nền và còn trống kết luận/D. |
| client_tests_gateway.log | Giữ nguyên | Bằng chứng log không trùng nhau; phục vụ các bài kiểm thử khác nhau. |
| ketqua/E0_client_rx_1793938128483.csv | Giữ | Một trong bốn CSV baseline E0 gốc, không đổi tên/số liệu. |
| ketqua/E0_client_rx_1793938128483_msgid.csv | Giữ | Một trong bốn CSV baseline E0 gốc, không đổi tên/số liệu. |
| ketqua/E0_client_rx_1794011939330.csv | Giữ; nên gom vào nhóm test client | Dữ liệu issue 16, không phải bản trùng của E0 gốc dù tên có E0. |
| ketqua/E0_client_rx_1794011939330_msgid.csv | Giữ; nên gom vào nhóm test client | Dữ liệu issue 16, không phải bản trùng của E0 gốc dù tên có E0. |
| ketqua/E0_client_rx_1794012053592.csv | Giữ hoặc chuyển lưu trữ | Phiên 65.266 s dùng cho kiểm thử đóng client; không hỏng, không phải baseline đủ 600 s. |
| ketqua/E0_client_rx_1794012053592_msgid.csv | Giữ hoặc chuyển lưu trữ | Phiên 65.266 s dùng cho kiểm thử đóng client; không hỏng, không phải baseline đủ 600 s. |
| ketqua/E0_client_rx_1794012180678.csv | Giữ; nên gom vào nhóm test client | Dữ liệu issue 16, không phải bản trùng của E0 gốc dù tên có E0. |
| ketqua/E0_client_rx_1794012180678_msgid.csv | Giữ; nên gom vào nhóm test client | Dữ liệu issue 16, không phải bản trùng của E0 gốc dù tên có E0. |
| ketqua/E0_gateway_ingress_1793938099255.csv | Giữ | Một trong bốn CSV baseline E0 gốc, không đổi tên/số liệu. |
| ketqua/E0_gateway_ingress_1793938099255_msgid.csv | Giữ | Một trong bốn CSV baseline E0 gốc, không đổi tên/số liệu. |
| ketqua/E0_gateway_ingress_1794011910944.csv | Giữ; nên gom vào nhóm test client | Dữ liệu issue 16, không phải bản trùng của E0 gốc dù tên có E0. |
| ketqua/E0_gateway_ingress_1794011910944_msgid.csv | Giữ; nên gom vào nhóm test client | Dữ liệu issue 16, không phải bản trùng của E0 gốc dù tên có E0. |
| routing_recovery.log | Giữ nguyên | Bằng chứng log không trùng nhau; phục vụ các bài kiểm thử khác nhau. |

## 5. File nào có thể xóa?

**Không có file raw nào trong 16 file được xác định là trùng hoàn toàn và nên xóa ngay.** Các CSV đều có dữ liệu, các hash file khác nhau. Đặc biệt hai file 1794012053592 không phải file rác vì phiên thử bị dừng là một phần bài disconnect.

Có thể xóa có điều kiện:

- Bản sao tài liệu cũ sau khi đã thay bằng bản sửa và lưu lịch sử Git/bản dự phòng; không xóa file tên gốc đang dùng trong dự án.
- Các bản CSV/log trùng byte nếu sau này copy ra nhiều chỗ và đã xác nhận SHA-256 bằng nhau. Không phát hiện loại này trong bộ hiện tại.
- Hai CSV của phiên 1794012053592 chỉ có thể bỏ khỏi bộ báo cáo thống kê đầy đủ nếu đã giữ trong Git/bản lưu trữ và báo cáo không cần phân tích phiên đóng client. Khuyến nghị hiện tại là giữ.

Không xóa routing_recovery.log, client_tests_gateway.log hoặc bốn CSV E0 gốc. Không xóa dữ liệu chỉ vì counters bằng 0. Không có file rỗng trong bộ hiện tại.

## 6. Cách sắp xếp đề xuất

Để đơn giản và không làm hỏng đường dẫn trong hồ sơ cũ, hiện giữ raw CSV nguyên ở docs/ketqua và dùng bảng này phân loại. Nếu muốn dọn về sau:

- docs/ketqua/baseline_E0/: bốn CSV 179393...
- docs/ketqua/client_tests/: tám CSV 179401...
- docs/logs/: hai file log.
- docs/: E0_BASELINE.md, client_tests.md, bang_msgid.md và báo cáo này.

Đây chỉ là đề xuất di chuyển, chưa áp dụng lên file gốc. Nếu chuyển, sửa các đường dẫn trong tài liệu theo vị trí mới; không cần đổi tên file CSV để giữ truy vết.

## 7. Giới hạn và việc tiếp theo

- client_tests.md đã đánh dấu các client bình thường không bị ảnh hưởng. Log Gateway cho thấy không đóng các client nền và TCP TX tiếp tục tăng; chưa có log nhận của từng client trong toàn bộ giai đoạn sau khi QoS 600 giây đã kết thúc.
- Đoạn cuối hai log đều dừng giữa STATS. Giữ raw; dùng khối đầy đủ cuối trước đó để tổng kết. Không gọi CSV một phần là hỏng chỉ vì chưa tới 600 s.
- Không có commit/tag, docs/dependencies/mavlink_E0.zip hoặc môi trường E0 đầy đủ trong docs.rar. Issue 15 chưa thể đóng toàn bộ chỉ từ bộ docs này.
- Không có timestamp sự kiện client #25 trực tiếp, chưa chốt chính xác thời gian chờ ngắt; không cần chạy lại chỉ để điền con số này nếu không phải mục tiêu đo.
- Issue 16 có thể đóng theo phạm vi A–D đã mô tả, kèm báo cáo sửa, log và xác nhận quan sát. Chưa thay cho ma trận E4/E5/E6 hoặc thử riêng partial send.
- Không suy ra issue 17 đã đạt: 15 BLOCK trước khi học UAV không phải bài cố ý phát request tới một UAV đã từng online rồi offline; hủy queue chưa được kích hoạt.
- Sau khi cập nhật hồ sơ, tiếp tục phần Markov độc lập.

## 8. Danh sách raw và hash đối chiếu

Hash tính trên byte gốc trong docs.rar. Các raw không bị sửa trong quá trình lập báo cáo.

| File | Byte | SHA-256 |
|---|---:|---|
| client_tests_gateway.log | 227940 | `d2271a2dd3372d8862b515e597c7e81fb71c7ba5ef5fbe793ec6ad5ee31ee13d` |
| ketqua/E0_client_rx_1793938128483.csv | 14240 | `0bf2dbca6dc3deb79f6ba620cc9d3b3cae9aea49e37561a86d54ce12f908d669` |
| ketqua/E0_client_rx_1793938128483_msgid.csv | 237089 | `b5a43e7378cb251a51911bd742dd3ce7b8190c1183be3cd30077a460b42c6a8c` |
| ketqua/E0_client_rx_1794011939330.csv | 14353 | `e8e9698484a7d70cc6a77f6f99acde736991ba13c73de698b6e54e07960fa683` |
| ketqua/E0_client_rx_1794011939330_msgid.csv | 236802 | `729328c435939e08e59ea625b921dddb25367787baa68be55b03570beba32e18` |
| ketqua/E0_client_rx_1794012053592.csv | 1732 | `6a8686a0ad1c0f719e4a0edf493a68f884bc2ccf62a05eeb5abafb35340ec8d9` |
| ketqua/E0_client_rx_1794012053592_msgid.csv | 24533 | `06561c1a4ec4916522c245e0b764a2df438fc67bf185dc5259a02b04d8ede39b` |
| ketqua/E0_client_rx_1794012180678.csv | 14352 | `678c8b1f94cad71698dbe9b4563add9bd0c834ddd023640fb1b1824cc72829ee` |
| ketqua/E0_client_rx_1794012180678_msgid.csv | 236785 | `586ab071e9da65c286e1696479549eaa3be935b510cd66c92c9216e18e73c545` |
| ketqua/E0_gateway_ingress_1793938099255.csv | 15560 | `85272b2f46468cc2f19cdf6cffefb3e3f2575cc86c2865919aa4822c6daf734f` |
| ketqua/E0_gateway_ingress_1793938099255_msgid.csv | 278014 | `2b36a0087e0f12dd61b96f9394985012fc8a7b40b59108407565f2adc4465363` |
| ketqua/E0_gateway_ingress_1794011910944.csv | 15673 | `dbab9146644775b64607bf6b2242cd10fc2d042f76371e565272bedc02f66497` |
| ketqua/E0_gateway_ingress_1794011910944_msgid.csv | 277697 | `1a2d21ad93dcb385b44bcf5bddf0f87843e54f17b22de56489b3ef9f0cbc2a56` |
| routing_recovery.log | 126518 | `6b6a9996bc95b7cd12d1f987e322a748480b10fa3fb5f389ec21be366c8de745` |
