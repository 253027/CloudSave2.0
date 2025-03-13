#include "threadpool.h"

#include <string.h>

#include "log.h"

mg::ThreadPool::ThreadPool(std::string name, int queueSize)
    : _name(name), _running(false), _maxQueueSize(queueSize)
{
    ;
}

mg::ThreadPool::~ThreadPool()
{
    if (this->_running)
        stop();
}

void mg::ThreadPool::start(int threadNums)
{
    this->_running = true;
    this->_threads.reserve(threadNums);
    for (int i = 0; i < threadNums; i++)
    {
        char buf[128] = {0};
        ::strncpy(buf, this->_name.data(), sizeof(buf) - 1);
        ::snprintf(buf + this->_name.size(),
                   sizeof(buf) - this->_name.size() - 1, "-ID[%d]", i);
        this->_threads.emplace_back(
            new Thread(std::bind(&ThreadPool::threadTask, this), buf));
        this->_threads[i]->start();
    }
    if (!threadNums && this->_initialTask)
        this->_initialTask();
}

void mg::ThreadPool::append(Task task)
{
    if (this->_threads.empty())
        task();
    else
    {
        std::unique_lock<std::mutex> lock(this->_mutex);
        this->_productor.wait(lock, [&]()
                              { return this->_taskQueue.size() < this->_maxQueueSize; });
        this->_taskQueue.push(std::move(task));
        this->_consumer.notify_one();
    }
}

void mg::ThreadPool::stop()
{
    this->_running = false;
    {
        std::lock_guard<std::mutex> lock(this->_mutex);
        this->_consumer.notify_all();
    }
    for (auto &x : this->_threads)
        x->join();
}

void mg::ThreadPool::threadTask()
{
    try
    {
        if (this->_initialTask)
            this->_initialTask();
        while (this->_running)
        {
            Task task;
            {
                std::unique_lock<std::mutex> lock(this->_mutex);
                this->_consumer.wait(lock, [&]()
                                     { return !this->_running || !this->_taskQueue.empty(); });
                if (!this->_taskQueue.empty())
                {
                    task = this->_taskQueue.front();
                    this->_taskQueue.pop();
                    this->_productor.notify_all();
                }
            }
            if (task)
                task();
        }
    }
    catch (...)
    {
        LOG_ERROR("ThreadPool[{}] catch exception", this->_name);
    }
}
