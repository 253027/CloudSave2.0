#! /usr/bin/env python3
import subprocess
import os
import signal
from typing import List

procress = ["SessionServer/server"]

def stop(procress: List[str]) -> bool:
    for name in procress:
        try:
            result = subprocess.run(["ps", "x"], capture_output=True, text=True)
            for line in result.stdout.splitlines():
                if name in line:
                    print("Find procress: ", line)
                    pid = int(line.split()[0])
                    os.kill(pid, signal.SIGINT)
                    return True
        except Exception as e:
            print("Error: ", e)
            return False

if __name__ == "__main__":
    stop(procress)


