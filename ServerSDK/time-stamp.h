#ifndef __MG_TIME_STAMP_H__
#define __MG_TIME_STAMP_H__

#include <sys/time.h>
#include <iostream>
#include <string>

namespace mg
{
    class TimeStamp
    {
    public:
        TimeStamp();

        explicit TimeStamp(int64_t mircosecond);

        /**
         * @brief 将unix时间戳转换为字符串形式"秒数.微秒数"
         */
        std::string toString() const;

        /**
         * @brief 将unix时间戳转化为标准时间格式"20240916-04:29:39"
         *        showMileSecond = true 显示为"20240916-04:29:39.728531"
         */
        std::string toFormatString(bool showMileSecond = false) const;

        /**
         * @brief 获取当前时间戳
         */
        static TimeStamp now();

        /**
         * @brief 比较两个时间的大小
         */
        inline bool operator<(TimeStamp &rhs)
        {
            return this->_microsecond < rhs._microsecond;
        }

        /**
         * @brief 判断两个时间戳是否相等
         */
        inline bool operator==(TimeStamp &rhs)
        {
            return this->_microsecond == rhs._microsecond;
        }

    private:
        // 自1970年1月1日至今的微秒数
        int64_t _microsecond;
        // 1秒等于的毫秒数
        static const int _mircoSecondsPerSecond = 10'000 * 10'000;
    };
}

#endif //__MG_TIME_STAMP_H__
