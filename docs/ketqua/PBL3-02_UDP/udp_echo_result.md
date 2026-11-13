# PBL3-02 — UDP WinSock echo

Nguồn: output người đo gửi từ Command Prompt (`cmd`) tại thư mục gốc dự án.

```text
C:\Users\User\OneDrive - The University of Technology\Desktop\PBL3project>.\udp_echo_check.exe
UDP ECHO PASS: sendto, recvfrom, reply dung du lieu
```

**Kết luận:** `PASS` cho vòng gửi → nhận → trả lại cùng dữ liệu trên UDP loopback. Trong source, dòng `UDP ECHO PASS` chỉ in sau khi payload nhận và payload echo đều khớp; nhánh này trả mã thoát 0. Người đo chưa chụp riêng mã thoát ngay sau lệnh.

Lệnh `$LASTEXITCODE` người đo thử sau đó không hợp lệ trong `cmd`; cú pháp kiểm mã thoát của `cmd` là `echo %ERRORLEVEL%` ngay sau khi chạy `.\udp_echo_check.exe`. Lỗi cú pháp này không phải lỗi của phép thử UDP.

**Giới hạn:** phép thử dùng loopback Windows, chưa kiểm PX4/WSL; phần đó thuộc PBL3-03.

## Lần chạy bổ sung số liệu

Log thô: `metrics_02.log` (người đo tự chạy). UDP loopback `127.0.0.1:63847`, payload 14 byte; client gửi 14 byte, server nhận 14 byte, server trả 14 byte, client nhận lại 14 byte. `PASS`, `exit_code=0`. Cổng động 63847 chỉ thuộc lần chạy này, không phải cổng MAVLink 14550.
