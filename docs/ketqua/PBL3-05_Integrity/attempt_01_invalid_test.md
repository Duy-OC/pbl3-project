# PBL3-05 — lần thử 1: helper kiểm thử sai giả định

Output người đo cung cấp:

```text
g++ -std=c++17 -O2 tools\mavlink_integrity_check.cpp -o mavlink_integrity_check.exe -lws2_32
.\mavlink_integrity_check.exe
MAVLINK INTEGRITY FAIL: byte 0 cuoi payload bi cat
echo %ERRORLEVEL%
1
```

**Phân loại:** phép thử chưa hợp lệ, chưa kết luận Gateway/serializer lỗi. Bản helper đầu tiên đặt byte 0 ở cuối payload rồi gọi MAVLink 2 `finalize`, nhưng hàm này chủ động trim byte 0 cuối. Điều kiện `len=9` của helper vì vậy sai. Helper đã được sửa để tạo frame dài 9 byte có CRC hợp lệ trước khi kiểm tra round-trip.

Giữ output này để truy vết; lần thử lại cần build helper mới từ source và ghi thành `attempt_02`.
