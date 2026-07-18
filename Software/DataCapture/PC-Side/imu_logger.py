#!/usr/bin/env python3
"""
imu_logger.py

Reads raw IMU data ("ax,ay,az,gx,gy,gz\r\n") streamed from the STM32's
USB CDC virtual COM port, prints each sample to the console, and logs
every sample (with a host-side timestamp) to a CSV file.

Usage:
    python imu_logger.py --port COM5
    python imu_logger.py --port /dev/ttyACM0 --out session1.csv
    python imu_logger.py --list                # list available serial ports

Install dependency once:
    pip install pyserial
"""

import argparse
import csv
import sys
import time
from datetime import datetime

import serial
import serial.tools.list_ports


def list_ports():
    ports = list(serial.tools.list_ports.comports())
    if not ports:
        print("No serial ports found.")
        return
    print("Available serial ports:")
    for p in ports:
        print(f"  {p.device} - {p.description}")


def guess_port():
    """Try to find a likely STM32 virtual COM port automatically."""
    candidates = list(serial.tools.list_ports.comports())
    for p in candidates:
        desc = (p.description or "").lower()
        if "stm" in desc or "virtual com" in desc or "cdc" in desc:
            return p.device
    return candidates[0].device if candidates else None


def parse_line(raw_line: str):
    """Parse 'ax,ay,az,gx,gy,gz' -> tuple of 6 ints, or None if malformed."""
    parts = raw_line.strip().split(",")
    if len(parts) != 6:
        return None
    try:
        return tuple(int(p) for p in parts)
    except ValueError:
        return None


def main():
    parser = argparse.ArgumentParser(description="Log IMU data from STM32 USB CDC to CSV.")
    parser.add_argument("--port", help="Serial port (e.g. COM5 or /dev/ttyACM0). "
                                        "If omitted, the script will try to auto-detect.")
    parser.add_argument("--baud", type=int, default=115200,
                         help="Baud rate (ignored by USB CDC but required by pyserial). Default 115200.")
    parser.add_argument("--out", default=None,
                         help="Output CSV filename. Default: imu_log_<timestamp>.csv")
    parser.add_argument("--list", action="store_true", help="List available serial ports and exit.")
    args = parser.parse_args()

    if args.list:
        list_ports()
        return

    port = args.port or guess_port()
    if not port:
        print("Could not find a serial port automatically. Run with --list to see options, "
              "then pass --port <name>.")
        sys.exit(1)

    out_path = args.out or f"imu_log_{datetime.now().strftime('%Y%m%d_%H%M%S')}.csv"

    print(f"Opening {port} @ {args.baud} baud...")
    try:
        ser = serial.Serial(port, args.baud, timeout=1)
    except serial.SerialException as e:
        print(f"Failed to open serial port {port}: {e}")
        sys.exit(1)

    # Let the port settle (helps on some platforms after open/reset)
    time.sleep(0.5)
    ser.reset_input_buffer()

    print(f"Logging to {out_path}")
    print("Columns: timestamp, ax_raw, ay_raw, az_raw, gx_raw, gy_raw, gz_raw")
    print("Press Ctrl+C to stop.\n")

    sample_count = 0
    bad_count = 0

    try:
        with open(out_path, "w", newline="") as f:
            writer = csv.writer(f)
            writer.writerow(["timestamp", "ax_raw", "ay_raw", "az_raw",
                              "gx_raw", "gy_raw", "gz_raw"])
            f.flush()

            while True:
                try:
                    raw = ser.readline().decode("utf-8", errors="replace")
                except UnicodeDecodeError:
                    continue

                if not raw:
                    continue  # timeout with no data, just loop again

                values = parse_line(raw)
                if values is None:
                    bad_count += 1
                    # Uncomment to debug malformed lines:
                    # print(f"[skipped malformed line]: {raw!r}")
                    continue

                timestamp = datetime.now().isoformat(timespec="milliseconds")
                ax, ay, az, gx, gy, gz = values

                print(f"{timestamp}  AX={ax:6d} AY={ay:6d} AZ={az:6d} "
                      f"GX={gx:6d} GY={gy:6d} GZ={gz:6d}")

                writer.writerow([timestamp, ax, ay, az, gx, gy, gz])
                f.flush()  # write-through so data isn't lost if interrupted

                sample_count += 1

    except KeyboardInterrupt:
        print("\nStopped by user.")
    finally:
        ser.close()
        print(f"\nSamples logged: {sample_count}")
        if bad_count:
            print(f"Malformed lines skipped: {bad_count}")
        print(f"CSV saved to: {out_path}")


if __name__ == "__main__":
    main()
