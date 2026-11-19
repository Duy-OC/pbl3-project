#!/usr/bin/env python3
"""
Simulate PX4 sending full telemetry stream (HEARTBEAT, SYS_STATUS, GLOBAL_POS, ATTITUDE, VFR_HUD)
Usage: python scripts/simulate_px4.py [target_ip] [target_port] [frequency_hz]
"""
import sys
import time
import math
from pymavlink import mavutil

target_ip = sys.argv[1] if len(sys.argv) > 1 else "127.0.0.1"
target_port = int(sys.argv[2]) if len(sys.argv) > 2 else 14550
freq_hz = float(sys.argv[3]) if len(sys.argv) > 3 else 10.0

mav = mavutil.mavlink_connection(f'udpout:{target_ip}:{target_port}', source_system=1, source_component=1)

print(f"[Simulate PX4] Gui telemetry MAVLink v2 den {target_ip}:{target_port} o tan so {freq_hz} Hz...")
print("Bao gom: HEARTBEAT, SYS_STATUS, GLOBAL_POSITION_INT, ATTITUDE, VFR_HUD, GPS_RAW_INT")
print("Nhan Ctrl+C de dung.\n")

interval = 1.0 / freq_hz
step = 0

try:
    while True:
        t_ms = int(time.time() * 1000) & 0xFFFFFFFF
        t_sec = step * interval

        # 1. HEARTBEAT (1 Hz) - PX4 Armed in Position Control
        if step % int(freq_hz) == 0:
            custom_mode = (3 << 16) # POSCTL
            base_mode = mavutil.mavlink.MAV_MODE_FLAG_CUSTOM_MODE_ENABLED | mavutil.mavlink.MAV_MODE_FLAG_SAFETY_ARMED
            mav.mav.heartbeat_send(
                mavutil.mavlink.MAV_TYPE_QUADROTOR,
                mavutil.mavlink.MAV_AUTOPILOT_PX4,
                base_mode,
                custom_mode,
                mavutil.mavlink.MAV_STATE_ACTIVE
            )

        # 2. SYS_STATUS (1 Hz) - Pin 15.2V, 2.4A, 85%
        if step % int(freq_hz) == 0:
            mav.mav.sys_status_send(
                0, 0, 0, 500,
                15200, # 15.20 V
                240,   # 2.40 A
                85,    # 85%
                0, 0, 0, 0, 0, 0
            )

        # 3. GLOBAL_POSITION_INT (5 Hz)
        if step % int(max(1, freq_hz // 5)) == 0:
            alt_m = 15.0 + math.sin(t_sec * 0.5) * 2.0
            mav.mav.global_position_int_send(
                t_ms,
                int(16.073456 * 1e7),
                int(108.152341 * 1e7),
                int((alt_m + 12.0) * 1000),
                int(alt_m * 1000),
                int(350), # vx cm/s
                int(120), # vy cm/s
                int(10),  # vz cm/s
                int(185 * 100) # hdg cdeg
            )

        # 4. ATTITUDE (10 Hz)
        roll_rad = math.sin(t_sec) * 0.05
        pitch_rad = math.cos(t_sec) * 0.03
        yaw_rad = 185.0 * math.pi / 180.0
        mav.mav.attitude_send(t_ms, roll_rad, pitch_rad, yaw_rad, 0.01, 0.01, 0.0)

        # 5. VFR_HUD (5 Hz)
        if step % int(max(1, freq_hz // 5)) == 0:
            mav.mav.vfr_hud_send(
                4.8, # airspeed
                4.5, # groundspeed
                185, # heading
                62,  # throttle %
                15.0, # alt
                0.2   # climb m/s
            )

        # 6. GPS_RAW_INT (1 Hz)
        if step % int(freq_hz) == 0:
            mav.mav.gps_raw_int_send(
                int(time.time() * 1e6),
                3, # 3D Fix
                int(16.073456 * 1e7),
                int(108.152341 * 1e7),
                int(27000),
                100, 100, 450, 18500,
                16 # 16 satellites
            )

        step += 1
        time.sleep(interval)

except KeyboardInterrupt:
    print("\n[Simulate PX4] Dung gia lap.")
