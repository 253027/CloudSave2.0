#include "rwlock.h"
#include "log.h"

mg::RWLock::RWLock()
{
    if (pthread_rwlock_init(&_lock, nullptr) != 0)
    {
        LOG_ERROR("pthread_rwlock_init failed");
    }
}

mg::RWLock::~RWLock()
{
    pthread_rwlock_destroy(&_lock);
}

void mg::RWLock::lock_shared()
{
    if (pthread_rwlock_rdlock(&_lock) != 0)
    {
        LOG_ERROR("pthread_rwlock_rdlock failed");
    }
}

void mg::RWLock::unlock_shared()
{
    if (pthread_rwlock_unlock(&_lock) != 0)
    {
        LOG_ERROR("pthread_rwlock_unlock failed");
    }
}

void mg::RWLock::lock()
{
    if (pthread_rwlock_wrlock(&_lock) != 0)
    {
        LOG_ERROR("pthread_rwlock_wrlock failed");
    }
}

void mg::RWLock::unlock()
{
    if (pthread_rwlock_unlock(&_lock) != 0)
    {
        LOG_ERROR("pthread_rwlock_unlock failed");
    }
}

mg::UniqueLock::UniqueLock(RWLock &lock)
    : _lock(lock), _ownLock(true)
{
    _lock.lock();
}

mg::UniqueLock::~UniqueLock()
{
    if (_ownLock)
        _lock.unlock();
}

mg::UniqueLock::UniqueLock(UniqueLock &&other)
    : _lock(other._lock), _ownLock(other._ownLock)
{
    other._ownLock = false;
}

mg::SharedLock::SharedLock(RWLock &lock)
    : _lock(lock), _ownLock(true)
{
    _lock.lock_shared();
}

mg::SharedLock::~SharedLock()
{
    if (_ownLock)
        _lock.unlock_shared();
}

mg::SharedLock::SharedLock(SharedLock &&other)
    : _lock(other._lock), _ownLock(other._ownLock)
{
    other._ownLock = false;
}
