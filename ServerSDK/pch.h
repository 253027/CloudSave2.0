// pch.h
#ifndef PCH_H
#define PCH_H

// 包含所有spdlog的头文件
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include "log.h"
#include "poller.h"
#include "json.hpp"
#endif // PCH_H
