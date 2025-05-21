#ifndef __MG_LOG_H__
#define __MG_LOG_H__

#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/async.h"

#include "singleton.h"

// 日志相关操作的宏封装
#define INITLOG(configuration) mg::Logger::getInstance()->init(configuration)
#define BASELOG(logger, level, ...)                                                              \
    do                                                                                           \
    {                                                                                            \
        if (logger)                                                                              \
        {                                                                                        \
            (logger)->log(spdlog::source_loc{__FILE__, __LINE__, __func__}, level, __VA_ARGS__); \
        }                                                                                        \
    } while (0)

#ifdef _DEBUG
#define LOG_TRACE(...) BASELOG(mg::Logger::getInstance()->getLogger(), spdlog::level::trace, __VA_ARGS__)
#define LOG_DEBUG(...) BASELOG(mg::Logger::getInstance()->getLogger(), spdlog::level::debug, __VA_ARGS__)
#else
#define LOG_TRACE(...) (void)0
#define LOG_DEBUG(...) (void)0
#endif

#define LOG_INFO(...) BASELOG(mg::Logger::getInstance()->getLogger(), spdlog::level::info, __VA_ARGS__)
#define LOG_WARN(...) BASELOG(mg::Logger::getInstance()->getLogger(), spdlog::level::warn, __VA_ARGS__)
#define LOG_ERROR(...) BASELOG(mg::Logger::getInstance()->getLogger(), spdlog::level::err, __VA_ARGS__)
#define LOG_CRITICAL(...) BASELOG(mg::Logger::getInstance()->getLogger(), spdlog::level::critical, __VA_ARGS__)
#define SHUTDOWNLOG() spdlog::shutdown()

namespace mg
{
    // 日志的配置项
    struct LogConfig
    {
        std::string level;
        std::string path;
        std::string logFileName;
        int64_t size = 1024 * 1024 * 20; // 20M
        int32_t fileNums = 10;
        /**
         * @brief 日志配置文件
         * @param level 日志级别 trace, debug, info, warn, err, critical
         * @param path 文件路径
         * @param filename 文件名
         */
        LogConfig(const std::string &level, const std::string &path, const std::string &filename)
        {
            this->level = level;
            this->path = path;
            this->logFileName = filename;
        }
    };

    // 日志的单例模式
    class Logger : public Singleton<Logger>
    {
    public:
        inline void init(const LogConfig &configuraion)
        {
            loggerPtr = spdlog::rotating_logger_mt<spdlog::async_factory>(configuraion.logFileName, configuraion.path + "/" + configuraion.logFileName,
                                                                          configuraion.size, configuraion.fileNums);
            // 设置格式
            // 参见文档 https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
            //[%Y-%m-%d %H:%M:%S.%e] 时间
            //[%l] 日志级别
            //[%t] 线程
            //[%s] 文件
            //[%#] 行号
            //[%!] 函数
            //[%v] 实际文本
            loggerPtr->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [thread %t] [%s:%#] %v");
            // 设置日志级别
            loggerPtr->set_level(spdlog::level::from_str(configuraion.level));
            // 设置刷新日志的日志级别，当出现level或更高级别日志时，立刻刷新日志到  disk
            loggerPtr->flush_on(spdlog::level::from_str(configuraion.level));
        }

        inline void setLogLevel(const std::string &logLevel)
        {
            auto level = spdlog::level::from_str(logLevel);
            if (level == spdlog::level::off)
                LOG_WARN("Given invalid log level {}", logLevel);
            else
            {
                loggerPtr->set_level(level);
                loggerPtr->flush_on(level);
            }
        }

        std::shared_ptr<spdlog::logger> getLogger()
        {
            return loggerPtr;
        }

    private:
        std::shared_ptr<spdlog::logger> loggerPtr;
    };
};

#endif //__MG_LOG_H__