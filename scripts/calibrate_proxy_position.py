"""Calibrate proxy coordinates using a matched PX4 NED and Gazebo pose sample."""

import argparse
import json
import math
from pathlib import Path


def finite(value):
    number = float(value)
    if not math.isfinite(number):
        raise argparse.ArgumentTypeError("expected a finite number")
    return number


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--ned", nargs=3, required=True, type=finite, metavar=("N", "E", "D"))
    parser.add_argument("--world", nargs=3, required=True, type=finite, metavar=("X", "Y", "Z"))
    parser.add_argument("--yaw-deg", required=True, type=finite, help="verified angle from world +X to PX4 North")
    parser.add_argument("--config", type=Path, default=Path(__file__).resolve().parents[1] / "config/proxy.json")
    parser.add_argument("--write", action="store_true", help="save calibration to the selected config")
    args = parser.parse_args()

    north, east, down = args.ned
    x, y, z = args.world
    yaw = math.radians(args.yaw_deg)
    c, s = math.cos(yaw), math.sin(yaw)
    try:
        config = json.loads(args.config.read_text(encoding="utf-8-sig"))
        transform = config["position"]["transform"]
        offset = transform.get("altitude_offset_m", 0)
        origin = [x - c * north - s * east, y - s * north + c * east, z + down - offset]
        transform.update(
            type="configured_ned_to_world",
            world_origin_of_ned_m=origin,
            world_yaw_of_ned_rad=yaw,
        )
        result = json.dumps(transform, indent=2, allow_nan=False)
        if args.write:
            document = json.dumps(config, indent=2, ensure_ascii=False, allow_nan=False) + "\n"
            args.config.write_text(document, encoding="utf-8")
    except (OSError, ValueError, KeyError, TypeError, AttributeError) as exc:
        parser.error(str(exc))
    print(result)


if __name__ == "__main__":
    main()
