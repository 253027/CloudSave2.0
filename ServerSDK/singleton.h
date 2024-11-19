/**
 * @brief 单例模板类
 *
 * @author mogaitesheng
 *
 * @date 2024-07-09
 */
#ifndef __MG_SINGLETION_H__
#define __MG_SINGLETION_H__

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
        if (!instance)
            instance = new T();
        return instance;
    }

    static T &getMe()
    {
        return *getInstance();
    }
};

template <typename T>
T *Singleton<T>::instance = nullptr;

#endif //__MG_SINGLETION_H__