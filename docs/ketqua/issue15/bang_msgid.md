# Bảng msgid — Baseline E0 gốc

Nguồn: `../issue14/E0/E0_gateway_ingress_1793938099255_msgid.csv`. SYSID=1, COMPID=1; thời lượng đo 600 giây. Tần suất trung bình = tổng frame cuối / 600, byte rate trung bình = tổng byte frame cuối / 600. Đây không phải cột tốc độ cửa sổ cuối trong CSV.

| MSGID | Tên | Frames | Byte frame | Trung bình Hz | Trung bình B/s |
|---:|---|---:|---:|---:|---:|
| 0 | HEARTBEAT | 597 | 12537 | 0.995 | 20.895 |
| 1 | SYS_STATUS | 597 | 31044 | 0.995 | 51.740 |
| 4 | PING | 60 | 1277 | 0.100 | 2.128 |
| 8 | LINK_NODE_STATUS | 597 | 18507 | 0.995 | 30.845 |
| 24 | GPS_RAW_INT | 2983 | 167048 | 4.972 | 278.413 |
| 29 | SCALED_PRESSURE | 597 | 15522 | 0.995 | 25.870 |
| 30 | ATTITUDE | 8949 | 357960 | 14.915 | 596.600 |
| 31 | ATTITUDE_QUATERNION | 5966 | 262504 | 9.943 | 437.507 |
| 32 | LOCAL_POSITION_NED | 597 | 23880 | 0.995 | 39.800 |
| 33 | GLOBAL_POSITION_INT | 2983 | 119320 | 4.972 | 198.867 |
| 36 | SERVO_OUTPUT_RAW | 597 | 9552 | 0.995 | 15.920 |
| 42 | MISSION_CURRENT | 592 | 10656 | 0.987 | 17.760 |
| 74 | VFR_HUD | 2387 | 69223 | 3.978 | 115.372 |
| 83 | ATTITUDE_TARGET | 1194 | 57312 | 1.990 | 95.520 |
| 85 | POSITION_TARGET_LOCAL_NED | 895 | 56385 | 1.492 | 93.975 |
| 141 | ALTITUDE | 597 | 26268 | 0.995 | 43.780 |
| 147 | BATTERY_STATUS | 299 | 15847 | 0.498 | 26.412 |
| 230 | ESTIMATOR_STATUS | 299 | 16146 | 0.498 | 26.910 |
| 241 | VIBRATION | 60 | 1920 | 0.100 | 3.200 |
| 242 | HOME_POSITION | 299 | 20033 | 0.498 | 33.388 |
| 245 | EXTENDED_SYS_STATE | 597 | 8358 | 0.995 | 13.930 |
| 290 | ESC_INFO | 597 | 34626 | 0.995 | 57.710 |
| 291 | ESC_STATUS | 597 | 9552 | 0.995 | 15.920 |
| 380 | TIME_ESTIMATE_TO_TARGET | 296 | 9472 | 0.493 | 15.787 |
| 411 | CURRENT_EVENT_SEQUENCE | 198 | 2574 | 0.330 | 4.290 |
| 436 | CURRENT_MODE | 299 | 5980 | 0.498 | 9.967 |
| 441 | GNSS_INTEGRITY | 597 | 17313 | 0.995 | 28.855 |
| 514 | ESTIMATOR_SENSOR_FUSION_STATUS | 299 | 19136 | 0.498 | 31.893 |
| 515 | MANUAL_INPUT_STATUS | 597 | 7761 | 0.995 | 12.935 |
| 12901 | OPEN_DRONE_ID_LOCATION | 597 | 41790 | 0.995 | 69.650 |
| 12904 | OPEN_DRONE_ID_SYSTEM | 597 | 22089 | 0.995 | 36.815 |
| **Tổng** | **31 msgid** | **36416** | **1471592** | **60.693** | **2452.653** |

Tên được đối chiếu header MAVLink đi kèm bản nguồn đã kiểm tra. Tổng tại mọi thời điểm khớp CSV tổng hợp. Không cộng các dòng tích lũy ở nhiều thời điểm để tính tổng run.
