/**
 * @brief 时间戳表示类
 *
 * @author mogaitesheng
 *
 * @date 2024-09-16
 */
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
         *        needUTC = true 转换为UTC时间，否则根据本地时区转换为本地时间
         */
        std::string toFormatString(bool showMileSecond = false, bool needUTC = false) const;

        /**
         * @brief 得到TimeStamp对象具体的Unix时间戳（UTC时间）
         */
        inline int64_t getMircoSecond() const { return this->_microsecond; };
        inline int64_t getMileSeconds() const { return this->_microsecond / 1000; };
        inline int64_t getSeconds() const { return this->_microsecond / _mircoSecondsPerSecond; };

        /**
         * @brief 获取当前时间戳
         */
        static TimeStamp now();

        /**
         * @brief 比较两个时间的大小
         */
        inline bool operator<(const TimeStamp &rhs) const
        {
            return this->_microsecond < rhs._microsecond;
        }

        /**
         * @brief 判断两个时间戳是否相等
         */
        inline bool operator==(const TimeStamp &rhs) const
        {
            return this->_microsecond == rhs._microsecond;
        }

        inline bool operator>(const TimeStamp &rhs) const
        {
            return this->_microsecond > rhs._microsecond;
        }

        inline TimeStamp operator-(const TimeStamp &rhs) const
        {
            return TimeStamp(this->_microsecond - rhs._microsecond);
        }

        // 1秒等于的微秒数
        static const int _mircoSecondsPerSecond = 1000 * 1000;

        friend TimeStamp addTime(TimeStamp timestamp, double seconds);

    private:
        // 自1970年1月1日至今的微秒数
        int64_t _microsecond;
    };

    TimeStamp addTime(TimeStamp timestamp, double seconds);
}

#endif //__MG_TIME_STAMP_H__
