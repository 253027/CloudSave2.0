cmake_minimum_required(VERSION 3.10)

aux_source_directory(. CUR_SRC)

message(STATUS "======proxyServer Configuring======")
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(TARGET proxyServer)

message(STATUS "EXECUTABLE_OUTPUT_PATH: ${EXECUTABLE_OUTPUT_PATH}")

add_executable(${TARGET} ${CUR_SRC})
target_link_libraries(${TARGET} mgnetframe protocol hiredis)

message(STATUS "=========proxyServer done==========")
message(STATUS "")