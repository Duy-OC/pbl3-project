# PBL3-05 — bảo toàn frame, CRC và signature bytes

Nguồn: output người đo gửi từ Command Prompt (`cmd`) sau khi build lại helper.

```text
g++ -std=c++17 -O2 tools\mavlink_integrity_check.cpp -o mavlink_integrity_check.exe -lws2_32
.\mavlink_integrity_check.exe
MAVLINK INTEGRITY PASS: payload byte 0, round-trip, CRC, signature bytes
echo %ERRORLEVEL%
0
```

**Kết luận:** PASS cho frame MAVLink hợp lệ có byte 0 cuối payload giữ nguyên qua parse/serialize, frame bị sửa payload bị loại vì CRC, và 13 byte signature block không bị cắt khi serialize.

**Giới hạn:** không xác thực chữ ký bằng khóa; test dùng helper độc lập, còn đường Gateway thực được đối soát thêm bằng E0 ingress/egress. Lần thử đầu ở `attempt_01_invalid_test.md` là lỗi giả định của helper, không tính là lỗi Gateway.

## Lần chạy bổ sung số liệu

Log thô: `metrics_03.log` (người đo tự chạy). Payload 9 byte, frame trên dây 21 byte; 1 frame hợp lệ được nhận, 1 frame sửa payload bị loại vì CRC, 0 frame hỏng được nhận. Signature block 13 byte được giữ, frame sau khi gắn block dài 34 byte. `PASS`, `exit_code=0`.
