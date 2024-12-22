#include "rwlock.h"
#include "log.h"

RWLock::RWLock()
{
    if (pthread_rwlock_init(&_lock, nullptr) != 0)
    {
        LOG_ERROR("pthread_rwlock_init failed");
    }
}

RWLock::~RWLock()
{
    pthread_rwlock_destroy(&_lock);
}

void RWLock::lock_shared()
{
    if (pthread_rwlock_rdlock(&_lock) != 0)
    {
        LOG_ERROR("pthread_rwlock_rdlock failed");
    }
}

void RWLock::unlock_shared()
{
    if (pthread_rwlock_unlock(&_lock) != 0)
    {
        LOG_ERROR("pthread_rwlock_unlock failed");
    }
}

void RWLock::lock()
{
    if (pthread_rwlock_wrlock(&_lock) != 0)
    {
        LOG_ERROR("pthread_rwlock_wrlock failed");
    }
}

void RWLock::unlock()
{
    if (pthread_rwlock_unlock(&_lock) != 0)
    {
        LOG_ERROR("pthread_rwlock_unlock failed");
    }
}
