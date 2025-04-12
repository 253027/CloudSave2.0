cmake_minimum_required(VERSION 3.10)
project(CloudSave2.0)

set(CMAKE_CXX_STANDARD 11)
set(SRC src spdlog-1.15.2/include protobuf/include)
set(LIB_PATH ${PROJECT_SOURCE_DIR}/lib)
set(LIB_INCLUDE_PATH ${LIB_PATH} ${PROJECT_SOURCE_DIR}/protobuf/lib)

include_directories(${SRC})
link_directories(PUBLIC ${LIB_INCLUDE_PATH})

add_subdirectory(src)
add_subdirectory(loginServer)
add_subdirectory(messageServer)
add_subdirectory(proxyServer)