/**
 * @brief 单例模板类
 *
 * @author mogaitesheng
 *
 * @date 2024-07-09
 */
#ifndef __MG_SINGLETION_H__
#define __MG_SINGLETION_H__

#include <pthread.h>

template <typename T>
class Singleton
{
private:
    /**
     * @brief 禁用拷贝构造函数
     */
    Singleton(const Singleton &) = delete;

    /**
     * @brief 禁用赋值运算函数
     */
    const Singleton &operator=(const Singleton &) = delete;

protected:
    static T *instance;
    static pthread_once_t ponce;

    Singleton() {}

    ~Singleton() {}

public:
    static void destroyInstance()
    {
        if (instance == nullptr)
            return;
        delete instance;
        instance = nullptr;
    }

    static T *getInstance()
    {
        pthread_once(&ponce, &Singleton::init);
        return instance;
    }

    static T &get()
    {
        return *getInstance();
    }

private:
    static void init()
    {
        instance = new T();
    }
};

template <typename T>
T *Singleton<T>::instance = nullptr;

template <typename T>
pthread_once_t Singleton<T>::ponce = PTHREAD_ONCE_INIT;

#endif //__MG_SINGLETION_H__