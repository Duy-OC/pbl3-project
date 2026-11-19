# I08_legacy_215s — kịch bản và kết quả

Đường dẫn: `docs/ketqua/PBL3-08_Route/I08_legacy_215s`.

## Kịch bản đo

Hai UAV có SYSID và endpoint riêng; chọn từng SYSID để gửi yêu cầu, đối chiếu ROUTE/ACK đúng đích. ACK yêu cầu thông tin không chứng minh thao tác bay thành công. Lượt thô cũ tới t=215.496 giây, hai SYSID. Thiếu log thao tác/ACK; chỉ dùng đối chiếu ingress/egress, không thay I08_01.

1. Kiểm cổng UDP/TCP, SYSID, nguồn phát và cấu hình lưu cùng lượt.
2. Thực hiện thao tác, warm-up và thời lượng đúng mô tả; nếu không ghi thời lượng thì không tự coi là phép đo 600 giây.
3. Đối chiếu log lỗi và số liệu theo từng stream/cửa sổ. Counter tích lũy không được cộng qua mọi dòng.

## Phạm vi sử dụng

Dữ liệu lịch sử không chứng nhận mã hiện tại. Gateway/client hiện không tự xuất file; lệnh đo cũ cần đúng phiên bản có bộ ghi đo. Build hiện tại: `build.bat`; công cụ kiểm thử: `build.bat tests`. Không tự bổ sung thông tin môi trường còn thiếu.

## Cấu hình của lượt

[I08_ROUTE_I08_01_config.txt](I08_ROUTE_I08_01_config.txt)

```text
scenario=I08_ROUTE
run_id=I08_01
urb=0
dur_s=2
slot_ms=100
lossgood=0
lossbad=1
seed=1
warmup_s=10
measure_s=600
route_timeout_s=5
ingress_timeout_s=5
egress_timeout_s=3
```

## Số liệu CSV

Số dòng không gồm header. Giá trị dưới đây chỉ thuộc **bản ghi cuối**, không phải tổng mọi UAV. Với summary.csv đọc metric/mean/std và đơn vị gốc; file chỉ có header không có mẫu đo.

| File | Dòng dữ liệu | Giá trị cuối tiêu biểu |
|---|---:|---|
| [I08_ROUTE_I08_01_gateway_egress.csv](I08_ROUTE_I08_01_gateway_egress.csv) | 86 | sysid=2; compid=1; t_s=215.496; frames=12491; frame_bytes=505567 |
| [I08_ROUTE_I08_01_gateway_egress_msgid.csv](I08_ROUTE_I08_01_gateway_egress_msgid.csv) | 3440 | sysid=2; compid=1; t_s=215.496; frames=203; frame_bytes=7511 |
| [I08_ROUTE_I08_01_gateway_ingress.csv](I08_ROUTE_I08_01_gateway_ingress.csv) | 86 | sysid=2; compid=1; t_s=215.496; frames=12491; frame_bytes=505567 |
| [I08_ROUTE_I08_01_gateway_ingress_msgid.csv](I08_ROUTE_I08_01_gateway_ingress_msgid.csv) | 3440 | sysid=2; compid=1; t_s=215.496; frames=203; frame_bytes=7511 |
| [I08_ROUTE_I08_01_states.csv](I08_ROUTE_I08_01_states.csv) | 2 | sysid=2 |
