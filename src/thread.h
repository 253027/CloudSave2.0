#ifndef __MG_THREAD_H__
#define __MG_THREAD_H__

#include <string>
#include <thread>
#include <functional>
#include <memory>
#include <atomic>

namespace mg
{
    class Thread
    {
    public:
        using ThreadFunction = std::function<void()>;

        explicit Thread(ThreadFunction, const std::string &name = std::string());

        ~Thread();

        void start();

        void join();

        inline const std::string &name() const { return this->_name; };

    private:
        void setDefaultName();

        bool _start;
        bool _join;
        pid_t _tid;               // 线程pid
        std::string _name;        // 线程名字
        ThreadFunction _function; // 线程要执行的入口函数
        std::shared_ptr<std::thread> _thread;
        static std::atomic<int32_t> _threadIndex; // 线程索引
    };
};

#endif //__MG_THREAD_H__