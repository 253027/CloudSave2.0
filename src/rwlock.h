#ifndef __MG_RWLOCK_H__
#define __MG_RWLOCK_H__

#include "noncopyable.h"

#include <pthread.h>

class RWLock : public mg::noncopyable
{
public:
    RWLock();

    ~RWLock();

    // 获取读锁
    void lock_shared();

    // 释放读锁
    void unlock_shared();

    // 获取写锁
    void lock();

    // 释放写锁
    void unlock();

private:
    pthread_rwlock_t _lock;
};

#endif // __MG_RWLOCK_H__