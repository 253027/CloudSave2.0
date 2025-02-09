#!/bin/sh
SRC_DIR=./
DST_DIR=../src/protocal

../protobuf/bin/protoc -I=$SRC_DIR --cpp_out=$DST_DIR/ $SRC_DIR/*.proto