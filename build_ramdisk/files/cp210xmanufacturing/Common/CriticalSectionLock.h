/////////////////////////////////////////////////////////////////////////////
// CriticalSectionLock.h
/////////////////////////////////////////////////////////////////////////////

#ifndef CRITICAL_SECTION_LOCK_H
#define CRITICAL_SECTION_LOCK_H

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#include <pthread.h>

/////////////////////////////////////////////////////////////////////////////
// CriticalSectionLock Class
/////////////////////////////////////////////////////////////////////////////

class CCriticalSectionLock
{
    // Constructor/Destructor
public:

    inline CCriticalSectionLock()
    {
        pthread_mutexattr_t attr;

        // Create a mutex object with recursive behavior
        //
        // This means that you can call pthread_mutex_lock()
        // more than once from the same thread before calling
        // pthread_mutex_unlock().
        //
        // Doing the same using the default mutex attributes
        // would cause deadlock.
        //
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&m_cs, &attr);
        pthread_mutexattr_destroy(&attr);
    }

    inline ~CCriticalSectionLock()
    {
        pthread_mutex_destroy(&m_cs);
    }

    // Public Methods
public:

    inline void Lock()
    {
        pthread_mutex_lock(&m_cs);
    }

    inline void Unlock()
    {
        pthread_mutex_unlock(&m_cs);
    }

    // Protected Members
protected:
    pthread_mutex_t m_cs;
};

#endif // CRITICAL_SECTION_LOCK_H
