#! /usr/bin/env python3
import subprocess
from typing import List

procress = ["./SessionServer/server"]

def run(list: List[str]) -> bool:
    for name in procress:
        result = subprocess.Popen([name], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

if __name__ == "__main__":
    run(procress)
