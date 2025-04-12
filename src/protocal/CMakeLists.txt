cmake_minimum_required(VERSION 3.10)
project(protocal)
set(LIBRARY_OUTPUT_PATH ${LIB_PATH})
aux_source_directory(. CUR_SRC)

add_library(protocal STATIC ${CUR_SRC})