"""
Pre-upload script: resets the Pico into BOOTSEL mode via 1200 baud touch.
The earlephilhower Arduino core monitors for a host connecting at 1200 baud
and reboots into BOOTSEL mode when detected — this is the standard Arduino
auto-reset protocol. After reset, picotool uploads normally.
"""

import serial
import time

Import("env")

def before_upload(source, target, env):
    port = env.get("UPLOAD_PORT")
    if not port:
        print(">>> No upload port detected — plug in the Pico and retry.")
        return
    try:
        print(f"\n>>> 1200 baud reset on {port}...")
        ser = serial.Serial(port, 1200)
        ser.dtr = False
        ser.close()
        time.sleep(2)
        print(">>> Pico entering BOOTSEL mode...\n")
    except Exception as e:
        print(f">>> Reset failed ({e}) — hold BOOTSEL and click Upload.")

env.AddPreAction("upload", before_upload)
