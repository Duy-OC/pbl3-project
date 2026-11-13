# PBL3-04 — Parser MAVLink frame chia/gộp

Nguồn: output người đo gửi từ Command Prompt (`cmd`) tại thư mục gốc dự án.

```text
g++ -std=c++17 -O2 tools\mavlink_parser_check.cpp -o mavlink_parser_check.exe -lws2_32
.\mavlink_parser_check.exe
MAVLINK PARSER PASS: frame chia/gop, sysid, compid, msgid
echo %ERRORLEVEL%
0
```

**Kết luận:** PASS cho hai frame HEARTBEAT được đưa vào parser theo cách chia/gộp, với SYSID 1/2, COMPID 1 và MSGID 0 đúng kỳ vọng. Dữ liệu A2/E0 đã có 31 MSGID và tổng MSGID khớp frame, bổ sung kiểm tra với telemetry thực.

**Giới hạn:** ca này chưa kiểm frame sai CRC, payload byte 0 cuối hoặc signature; các mục đó thuộc PBL3-05.

## Lần chạy bổ sung số liệu

Log thô: `metrics_02.log` (người đo tự chạy). Hai frame dài 21 byte mỗi frame; chia frame đầu tại byte thứ 10 thì parser nhận 0 frame, nhận đủ 21 byte thì có 1 frame. Ghép hai frame dài 42 byte, parser nhận 2 frame với SYSID 1/2, COMPID 1, MSGID 0. `PASS`, `exit_code=0`.
