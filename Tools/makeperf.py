#! /usr/bin/env python3
import sys
import subprocess
from typing import List

#FlameGraph Path: https://github.com/brendangregg/FlameGraph.git
# run "perf record -e cpu-clock -g ./your_program" to generate the perf.data

path = "../../FlameGraph/"
filepath = sys.argv[1]

executive = [f"sudo perf script -i {filepath} > perf.unfold 2>&1",
            f"sudo {path}stackcollapse-perf.pl ./perf.unfold > perf.folded",
            f"sudo {path}flamegraph.pl ./perf.folded > perf.svg"]

def make():
    for command in executive:
        print(f"runing {command}")
        result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True, check=True)
        try:
            if result.stderr:
                print(result.stderr.decode("utf-8"), end="")
                return False
        except subprocess.TimeoutExpired:
            pass
    print("generate done!")
        

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("need filepath")
    else:
        make()