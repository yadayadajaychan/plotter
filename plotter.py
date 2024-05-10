#!/usr/bin/env python3

# Copyright (C) 2024 Ethan Cheng <ethan@nijika.org>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3 of the License.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

import serial
import time
import sys

def open_serial(port):
    try:
        ser = serial.Serial(port, 9600, timeout=60)
        return ser
    except serial.SerialException:
        print(f"Error: Could not open serial port {port}.")
        return

def wait_until_ready(ser):
    while ser.readline() != b'READY\r\n':
        time.sleep(0.1)

def write_line(ser, line):
    ser.write(line.encode())
    return ser.readline()

def write_to_serial(port):
    # open serial port
    ser = open_serial(port)
    if not ser:
        return

    wait_until_ready(ser)

    while True:
        try:
            time.sleep(0.050)
            line = sys.stdin.readline()
            if not line:
                break

            response = write_line(ser, line)

            retries = 0
            while not response and retries < 3:
                retries += 1
                print("NO RESPONSE FROM SERIAL PORT")
                print(f"RETRYING ({retries}/3)...")
                ser.close()
                ser = open_serial(port)
                if not ser:
                    return
                wait_until_ready(ser)
                response = write_char(ser, ch)

            sys.stdout.write(response.decode())

        except KeyboardInterrupt:
            break

    ser.close()

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: ./script.py SERIAL_DEVICE")
        sys.exit(1)

    write_to_serial(sys.argv[1])
