# Proxy UDP PX4 - gateway A (Windows Winsock)

```text
PX4 <--> proxy:14600 <--> gateway A:14550
```

Proxy chuyen nguyen tung UDP datagram, khong sua MAVLink/CRC, khong retry.
Moi tien trinh phuc vu mot PX4. Endpoint IP:port cua PX4 duoc hoc tu goi
dau tien va giu co dinh den khi khoi dong lai; nguon khac bi bo qua.
Neu PX4 khoi dong lai voi port nguon moi, khoi dong lai proxy.

## Build va chay

Tu thu muc goc du an, dung MinGW g++ co trong PATH:

```bat
scripts\build_udp_proxy.bat
bin\udp_proxy.exe 14600 127.0.0.1 14550
```

Cu phap: `udp_proxy.exe [--config FILE] [listen_port [gateway_ipv4 [gateway_port [bind_ipv4]]]]`.
Mac dinh: `14600 127.0.0.1 14550 0.0.0.0`. Chi nhan IPv4 dang so.
Nhan Ctrl+C de dung. Loi socket/bind se thoat voi ma 1.

Cau hinh PX4 gui UDP toi IP may Windows chay proxy, port 14600.
Neu PX4 chay trong WSL/may khac, su dung IP Windows truy cap duoc tu PX4.
Gateway A lang nghe UDP 14550 va gui phan hoi ve endpoint nguon da nhan;
day la socket rieng cua proxy voi port nguon tam thoi, on dinh trong phien.
Phan hoi gui ve PX4 tu dung socket lang nghe 14600.

Gateway hien tai nhan UDP 14550 theo mac dinh. Neu dung port khac,
truyen port tuong ung khi chay proxy. Khong cau hinh gateway gui nguoc ve
port telemetry cua chinh no hoac de PX4 gui thang toi gateway.
Socket phia gateway chi nhan du lieu tu dung IP:port gateway da cau hinh.

## Kiem thu loopback

Sau khi build, chay trong PowerShell:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\tests\test_udp_proxy.ps1
```

Test dung PX4/gateway UDP gia lap, kiem tra byte nhi phan hai chieu,
datagram 60.000 byte, datagram rong, port nguon phan hoi va loai nguon PX4 la.
Day la kiem thu mang cuc bo; can chay PX4 SITL thuc de xac nhan cau hinh
IP/port va luong MAVLink cua moi truong mo phong.

## Theo doi LOCAL_POSITION_NED

`src/proxy/position_tracker.h` duoc tich hop vao chieu PX4 -> gateway.
Parser MAVLink v1/v2 kiem CRC, ho tro frame chia/gop va khong sua datagram
duoc forward. Chi du lieu tu endpoint PX4 da hoc duoc dua vao tracker.
`positions()` tra bang theo SYSID, gom x/y/z (m), vx/vy/vz (m/s),
`timeBootMs` tu PX4, `receivedAt` (gio he thong) va `receivedSteady`
(dong ho monotonic de tinh tuoi du lieu). Proxy chi theo doi SYSID duoc
cau hinh va giu mau huu han moi nhat de bao `STALE` khi qua timeout.

Log in khi doi trang thai va khoang moi giay, ke ca khi mat vi tri.
Khi co vi tri NED nhung chua hieu chinh transform, vi du:

```text
[POSITION] SYSID=1 status=TRANSFORM_NOT_CONFIGURED transform=TRANSFORM_NOT_CONFIGURED N=0.000 E=0.000 D=-1.500 age_ms=10 boot_ms=1234 rx_unix_ms=1790000000000 state=CAT_CANH_DANG_LEN X/Y/Z=- forwarded_to_gateway=12 forwarded_to_px4=3
```

NED: North/East/Down, z am la phia tren goc toa do local; day khong phai
do cao tren mat dat hay toa do world Gazebo. `rx_unix_ms` la thoi diem
nhan theo Unix milliseconds, khac voi `boot_ms` cua PX4.

- `DUNG_YEN`: toc do 3D thap, bao gom dung tren dat va hover.
- `CAT_CANH_DANG_LEN`: vz < -0.30 m/s; giu trang thai den khi vz >= -0.15.
  Day la suy doan di len, khong xac nhan cat canh thuc su hay armed/landed.
- `DI_CHUYEN`: toc do 3D > 0.25 m/s; giu den khi <= 0.15 m/s.

Neu chua co log, yeu cau PX4 gui ban tin tren dung kenh dang toi proxy.
Vi du trong console PX4, voi cong local 14611:

```sh
mavlink stream -u 14611 -s LOCAL_POSITION_NED -r 5
```

Chi co du lieu khi PX4 co uoc luong vi tri local hop le.
Xem cach hieu chinh NED sang world trong [tai lieu dashboard PX4](../dashboard/node-red/README.md).
Proxy hien chi in vi tri ra terminal.
Neu proxy cu dang chay, dung Ctrl+C truoc khi build/chay lai. Co the build
ten rieng bang `scripts\build_udp_proxy.bat bin/udp_proxy_position.exe`.

Test tracker:

```bat
g++ -std=c++17 -O2 -w tests/test_position_tracker.cpp -o bin/test_position_tracker.exe
bin\test_position_tracker.exe
```

## Hieu chinh NED sang Gazebo

Chay proxy voi --config config/proxy.json. Khi UAV dung yen va mau NED
con moi, ghi N E D tu dong [POSITION] va X Y Z tu Gazebo cua cung mau.
Trong WSL, dung gz model -m x500_0 --pose neu ten model la x500_0.
Xac nhan huong +North cua PX4 trong world truoc khi nhap yaw. Voi ENU,
yaw la 90 do neu +North trung +Y world va +East trung +X world.
Khong lay vi tri spawn lam goc NED.

Tu thu muc goc tren Windows, chay:
```powershell
python .\scripts\calibrate_proxy_position.py --ned N E D --world X Y Z --yaw-deg YAW --write
```

Thay cac chu bang so do. Bo --write de xem truoc ket qua. Khoi dong lai
proxy voi --config config/proxy.json va doi chieu dong status=VALID, X/Y/Z
voi pose Gazebo o mot diem khac. Hieu chinh lai neu goc PX4 thay doi.

## Kiem tra vung phu A/B

World gateway_2zones_40m co A tai (0,0) va B tai (60,0). Proxy doc
tam va ban kinh ngang tu zones.json. Voi --config config/proxy.json,
coverage duoc bat va chi cho qua datagram hai chieu khi UAV cach gateway
dang chon khong qua 40 m. Thieu vi tri world hop le hoac vi tri qua han
se chan duong gateway. PX4 van gui vi tri cho proxy khi bi chan.

Build hai executable bang `build.bat gateways`. Chay A va B trong hai terminal Windows:
```powershell
.\bin\gatewayA.exe
.\bin\gatewayB.exe
```

Gateway A mac dinh UDP 14560 / TCP 5760. Gateway B mac dinh UDP 14561 / TCP 5761.

Chay proxy voi A, sau do dung proxy va chay lai voi B de thu rieng:
```powershell
.\bin\udp_proxy.exe --config .\config\proxy.json --gateway A
.\bin\udp_proxy.exe --config .\config\proxy.json --gateway B
```

Neu gateway A cua phien hien tai van dung 14550, them
14600 127.0.0.1 14550 sau --gateway A. Truong hop dat ten file build rieng,
thay udp_proxy.exe bang ten file da build.

Doc gateway, coverage va distance_xy_m tren dong [POSITION].
IN_RANGE cho qua; OUT_OF_RANGE chan; BLOCKED_NO_POSITION nghia la chua co
toa do world moi hop le. Thu vuot bien va quay lai. Neu UAV o trong vung
con lai nhung ngoai vung gateway dang chon, duong hien tai van bi chan.
QGC co the van giu ket noi TCP toi gateway trong luc mat du lieu UAV.
