cmake_minimum_required(VERSION 3.10)
message(STATUS "============src Configuring============")

# set if make shared library
set(MGNET_BUILD_SHARED ON)
# dependency include path
set(INCLUDE_PATH ${PROJECT_SOURCE_DIR}/spdlog-1.15.2/include)
include_directories(${INCLUDE_PATH})

message(STATUS "INCLUDE_PATH: ${INCLUDE_PATH}")

# get all source file
aux_source_directory(./base BASE_SRC)
set(FRAME_SRC ${BASE_SRC})

# set ouput lib path
set(LIBRARY_OUTPUT_PATH ${LIB_PATH})
message(STATUS "LIBRARY_OUTPUT_PATH: ${LIBRARY_OUTPUT_PATH}")

if(MGNET_BUILD_SHARED)
    message(STATUS "Building mgnet as a shared library")
    add_library(mgnetframe SHARED ${FRAME_SRC})
else()
    message(STATUS "Building mgnet as a static library")
    add_library(mgnetframe STATIC ${FRAME_SRC})
endif()

target_link_libraries(mgnetframe PUBLIC spdlog mysqlclient protobuf hiredis)

add_subdirectory(protocol)
message(STATUS "================src done===============")
message(STATUS "")