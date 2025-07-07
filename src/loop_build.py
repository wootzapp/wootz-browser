#!/usr/bin/env python3
"""
Repeatedly run:
    autoninja -C out/Default chrome_public_apk

Stop when the *total* work-unit count printed by Ninja (the right-hand number
in "[done/total]") drops to <= 15_000, EVEN IF previous invocations failed.

Copyright 2025  (you may use / modify freely)
"""

import subprocess
import re
import sys
from typing import Optional

THRESHOLD = 9_000
BUILD_CMD  = ["autoninja", "-C", "out/Default", "chrome_public_apk"]

# Regex that grabs both numbers from a progress line like "[123/18823]"
PROG_RE = re.compile(r"\[(\d+)/(\d+)]")

def run_build() -> Optional[int]:
    """
    Launches one build, streams Ninja output to the console,
    and returns the **largest total** value (N in "[*/N]") seen.
    If no progress line is seen, returns None.
    Does *not* raise if Ninja exits with an error.
    """
    print("\n▶  Running:", " ".join(BUILD_CMD), "\n", flush=True)

    proc = subprocess.Popen(
        BUILD_CMD,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        bufsize=1,
    )

    max_total: Optional[int] = None

    for line in proc.stdout:
        sys.stdout.write(line)
        m = PROG_RE.search(line)
        if m:
            total = int(m.group(2))
            max_total = total if max_total is None else max(max_total, total)

    proc.wait()

    if proc.returncode != 0:
        print(f"\n⚠️  build exited with status {proc.returncode}; "
              "will retry if threshold not hit.\n")

    return max_total

def main() -> None:
    while True:
        max_total = run_build()

        if max_total is None:
            # Never saw a progress line – be conservative and keep looping.
            print("No [*/N] line found; retrying…\n")
            continue

        if max_total <= THRESHOLD:
            print(f"\n✓ total work units {max_total} ≤ {THRESHOLD}; stopping.\n")
            break

        print(f"\n↻ total work units {max_total} still > {THRESHOLD}; "
              "rebuilding…\n")

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\nInterrupted by user.")
        sys.exit(130)
