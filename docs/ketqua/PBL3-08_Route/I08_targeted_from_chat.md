# PBL3-08 — thao tác riêng từng UAV qua QGC

Nguồn: đoạn terminal và xác nhận trực tiếp của người đo trong hội thoại. Người đo báo đã Arm và Disarm riêng từng UAV, thao tác hoạt động bình thường. Đây là bản trích/bảng tổng hợp từ nội dung người đo gửi, không phải bản sao `gateway.log` đầy đủ hay video Gazebo.

| Thứ tự trong đoạn log | ROUTE | ACK |
|---:|---|---|
| 1 | Client #1 → UAV 1 → 172.26.162.42:14600 | UAV 1, command=400, result=0 |
| 2 | Client #1 → UAV 1 → 172.26.162.42:14600 | UAV 1, command=400, result=0 |
| 3 | Client #1 → UAV 2 → 172.26.162.42:14601 | UAV 2, command=400, result=0 |
| 4 | Client #1 → UAV 2 → 172.26.162.42:14601 | UAV 2, command=400, result=0 |

`400` là MAV_CMD_COMPONENT_ARM_DISARM; Gateway hiện chỉ ghi mã lệnh, không ghi tham số phân biệt Arm/Disarm. Thứ tự thao tác và kết quả trạng thái được ghi theo xác nhận của người đo. Trong đoạn STATS gửi kèm, Gateway duy trì `UAV online=2`, `Clients=1`, `CRC errors=0`, `Signature errors=0`, `Blocked messages=0`, `UDP dropped/cancelled=0`. Mẫu cuối gửi có `UDP RX datagrams=11759`, `UDP RX bytes=475082`, `MAVLink UAV RX=11759`, `Client RX=208`, `TCP TX bytes=426121`, `UDP TX frames=306`, UAV 1 RX=5881 và UAV 2 RX=5873. QOS đạt `t=90.2063 s`, `frames=10578` ở cả hai điểm.
