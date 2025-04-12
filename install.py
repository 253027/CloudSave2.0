#! /usr/bin/env python3
import os
import tarfile
import zipfile
import subprocess
import sys
from typing import List

current_path = os.path.dirname(os.path.realpath(__file__))

def run_command(cmd: str):
    print(f"\033[32m{cmd}\033[0m")
    result = subprocess.run(cmd, shell=True)
    if result.returncode != 0:
        sys.exit(1)


def make_protobuf():
    print("protobuf installing...")
    run_command("sudo apt-get install automake libtool autoconf curl make g++ unzip -y")
    
    PROTOBUF = "protobuf-3.21.11"
    install_path = os.path.join(current_path)
    os.chdir(install_path)

    file = os.path.join(install_path, f"{PROTOBUF}.zip")
    if not os.path.exists(file):
        print(f"{os.path.basename(file)} not found")
        sys.exit(1)
        
    with zipfile.ZipFile(file, "r") as zip_ref:
        zip_ref.extractall(install_path)
    
    build_path = os.path.join(install_path, PROTOBUF)
    os.chdir(build_path)

    configure = os.path.join(build_path, "configure")

    run_command(f"chmod +x {configure}")
    run_command(f"{configure} --prefix={install_path}/protobuf")
    run_command("make -j10 && make install")
    print("protobuf completed")

def make_spdlog():
    print("spdlog installing...")
    run_command("tar -xzf spdlog-1.15.2.tar.gz && "
                "mkdir -p lib && "
                "cd spdlog-1.15.2 && "
                "mkdir -p build && "
                "cd build && "
                "cmake -DSPDLOG_BUILD_SHARED=ON .. && "
                "make && mv libspdlog* ../../lib")
    print("spdlog completed")

def make_protocal_message():
    print("protocal message installing...")
    os.chdir(current_path)
    run_command("cd message && ./generate.sh")
    print("protocal message completed")

if __name__ == "__main__":
    make_spdlog()
    make_protobuf()
    make_protocal_message()