cmake_minimum_required(VERSION 3.10)
project(CloudSave2.0)

set(CMAKE_CXX_STANDARD 11)
set(SRC src spdlog-1.15.2/include protobuf/include)
set(LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)

include_directories(${SRC})

add_subdirectory(src)
# add_subdirectory(loginServer)