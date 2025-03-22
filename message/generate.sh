#!/bin/sh
SRC_DIR=.
DST_DIR=../src/protocal

if [ $# -ge 1 ]; then
    for proto in "$@"; do
        ../protobuf/bin/protoc -I="$SRC_DIR" --cpp_out="$DST_DIR/" "$proto"
    done
else
    ../protobuf/bin/protoc -I="$SRC_DIR" --cpp_out="$DST_DIR/" "$SRC_DIR"/*.proto
fi