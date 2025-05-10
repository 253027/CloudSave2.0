cmake_minimum_required(VERSION 3.10)

aux_source_directory(. CUR_SRC)

message(STATUS "======messageServer Configuring======")
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(TARGET messageServer)

message(STATUS "EXECUTABLE_OUTPUT_PATH: ${EXECUTABLE_OUTPUT_PATH}")

add_executable(${TARGET} ${CUR_SRC})
target_link_libraries(${TARGET} mgnetframe protocol)

message(STATUS "=========messageServer done==========")
message(STATUS "")