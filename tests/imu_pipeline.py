#!/usr/bin/env python3
"""Run with: python3 tests/imu_pipeline.py build/main (no hardware or GUI needed)."""

import argparse
import os
from pathlib import Path
import pty
import re
import shlex
import signal
import struct
import subprocess
import sys
import tempfile
import time


def imu_frame():
    payload = struct.pack("<6H", 10, 20, 30, 100, 200, 300)
    length = struct.pack("<H", len(payload))
    crc = 0
    for byte in length:
        crc ^= byte
        for _ in range(8):
            crc = ((crc << 1) ^ (0x31 if crc & 0x80 else 0)) & 0xFF
    return b"\xaa\x55" + length + bytes([crc, 0x02]) + struct.pack("<I", 0) + payload


def check_pipeline(executable):
    with tempfile.TemporaryDirectory(prefix="imu-pipeline-") as directory:
        root = Path(directory)
        capture = root / "capture.py"
        capture.write_text(
            "import os, pathlib, sys\n"
            "path = pathlib.Path(os.environ['IMU_TEST_CAPTURE']) / f'{os.getpid()}.gp'\n"
            "with path.open('w') as output:\n"
            "    for line in sys.stdin:\n"
            "        output.write(line)\n"
            "        output.flush()\n"
        )
        gnuplot = root / "gnuplot"
        gnuplot.write_text(
            f"#!/bin/sh\nexec {shlex.quote(sys.executable)} {shlex.quote(str(capture))}\n"
        )
        gnuplot.chmod(0o755)
        env = os.environ.copy()
        env["PATH"] = str(root) + os.pathsep + env.get("PATH", "")
        env["IMU_TEST_CAPTURE"] = str(root)

        master, slave = pty.openpty()
        process = None
        try:
            process = subprocess.Popen(
                [str(executable), os.ttyname(slave)],
                stdin=subprocess.DEVNULL,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                env=env,
                start_new_session=True,
                text=True,
            )
            deadline = time.monotonic() + 5
            while time.monotonic() < deadline:
                if process.poll() is not None:
                    output, _ = process.communicate()
                    raise AssertionError(
                        f"Application exited before plotting (status {process.returncode}):\n"
                        + output
                    )
                os.write(master, imu_frame())
                for path in root.glob("*.gp"):
                    commands = path.read_text()
                    if "unset multiplot\n" not in commands:
                        continue
                    assert "set title 'Translation'" in commands
                    assert "set title 'Rotation'" in commands
                    assert re.search(
                        r"^\d+\.\d+ 100 200 300 10 20 30$", commands, re.MULTILINE
                    ), "IMU values did not reach the translation/rotation plots"
                    return
                time.sleep(0.02)
            raise AssertionError("No IMU plot received within 5 seconds")
        finally:
            # The application blocks on serial input; stop its entire test process group.
            if process is not None:
                try:
                    os.killpg(process.pid, signal.SIGTERM)
                except ProcessLookupError:
                    pass
                try:
                    process.communicate(timeout=3)
                except subprocess.TimeoutExpired:
                    os.killpg(process.pid, signal.SIGKILL)
                    process.communicate()
            os.close(master)
            os.close(slave)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("executable", type=Path)
    args = parser.parse_args()
    check_pipeline(args.executable.resolve())
    print("PASS: startup, IMU serial input, parsing, and translation/rotation plot output")
