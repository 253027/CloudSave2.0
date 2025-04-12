cmake_minimum_required(VERSION 3.10)
project(messageServer)

aux_source_directory(. CUR_SRC)

message(STATUS "======messageServer Configuring======")
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_LIST_DIR})
set(TARGET messageServer)

message(STATUS "EXECUTABLE_OUTPUT_PATH: ${EXECUTABLE_OUTPUT_PATH}")

add_executable(${TARGET} ${CUR_SRC})
target_link_libraries(${TARGET} spdlog protobuf mgnetframe protocal)

message(STATUS "=========messageServer done==========")
message(STATUS "")