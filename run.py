#! /usr/bin/env python3
import subprocess
from typing import List

process = ["./SessionServer/server", "./GatewayServer/server"]

def run(proclist: List[str]) -> bool:
    for name in proclist:
        result = subprocess.Popen([name, "-daemon"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        try:
            stdout, stderr = result.communicate(timeout=0.5)
            if stderr:
                print("Error:", stderr.decode())
                return False
            if stdout:
                print("Output:", stdout.decode())
        except subprocess.TimeoutExpired:
            pass
    return True

if __name__ == "__main__":
    run(process)
