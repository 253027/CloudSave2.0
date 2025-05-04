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
    run_command(
        "tar -xzf spdlog-1.15.2.tar.gz && "
        "mkdir -p lib && "
        "cd spdlog-1.15.2 && "
        "mkdir -p build && "
        "cd build && "
        "cmake -DSPDLOG_BUILD_SHARED=ON .. && "
        "make && mv libspdlog* ../../lib"
    )
    print("spdlog completed")


def make_protocol_message():
    print("protocol message installing...")
    os.chdir(current_path)
    run_command("cd message && ./generate.sh")
    print("protocol message completed")


def make_redis():
    print("redis installing...")
    os.chdir(current_path)
    run_command(
        f"mkdir -p redis && tar -xzvf redis-7.4.3.tar.gz && cd redis-7.4.3 && "
        f"make  && make PREFIX={current_path}/redis install && "
        "cp redis.conf .."
    )
    os.chdir(current_path)
    with open("redis.conf", "r+", encoding="utf-8") as file:
        content = file.read()
        content = content.replace("daemonize no", "daemonize yes")
        content = content.replace("port 6379", "port 12700")
        file.seek(0)
        file.write(content)
        file.truncate()
    print("redis completed")


def make_hiredis():
    print("hiredis installing...")
    os.chdir(current_path)
    run_command(
        "tar -xvzf hiredis-1.3.0.tar.gz && "
        "mkdir -p hiredis && "
        "cd hiredis-1.3.0 && "
        "mkdir -p build && "
        "cd build && cmake .. && "
        "make && mv libhiredis* ../../lib"
    )
    print("hiredis completed")


def make_mysql_dev():
    print("mysql dev installing...")
    os.chdir(current_path)
    run_command("sudo apt-get install libmysqlclient-dev")
    print("mysql dev completed")


if __name__ == "__main__":
    make_spdlog()
    make_protobuf()
    make_protocol_message()
    make_redis()
    make_hiredis()
    make_mysql_dev()
