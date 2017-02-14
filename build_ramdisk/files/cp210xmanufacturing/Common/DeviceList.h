#pragma once

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#include "CriticalSectionLock.h"
#include <vector>
#include <algorithm>

/////////////////////////////////////////////////////////////////////////////
// CDeviceList Class
/////////////////////////////////////////////////////////////////////////////

template <class T>
class CDeviceList
{
    // Constructor/Destructor
public:
    CDeviceList();
    ~CDeviceList();

    // Public Methods
public:
    BOOL Validate(T* object);
    T* Construct();
    void Add(T* object);
    void Destruct(T* object);

    // Protected Members
protected:
    std::vector<T*> m_list;
    CCriticalSectionLock m_lock;
};

/////////////////////////////////////////////////////////////////////////////
// CDeviceList Class - Constructor/Destructor
/////////////////////////////////////////////////////////////////////////////

template <class T>
CDeviceList<T>::CDeviceList()
{

}

template <class T>
CDeviceList<T>::~CDeviceList()
{
    // Enter critical section
    m_lock.Lock();

    // Deallocate all device objects
    // (Destructor closes the devices)
    for (DWORD i = 0; i < m_list.size(); i++)
    {
        delete m_list[i];
    }

    // Remove all device references
    m_list.clear();

    // Leave critical section
    m_lock.Unlock();
}

/////////////////////////////////////////////////////////////////////////////
// CDeviceList Class - Public Methods
/////////////////////////////////////////////////////////////////////////////

// Make sure that a T pointer is valid

template <class T>
BOOL CDeviceList<T>::Validate(T* object)
{
    BOOL retVal = FALSE;

    // Enter critical section
    m_lock.Lock();

    if (std::find(m_list.begin(), m_list.end(), object) != m_list.end())
    {
        retVal = TRUE;
    }

    // Unlock critical section
    m_lock.Unlock();

    return retVal;
}

// Create a new T object on the heap
// and call the T constructor
// and track memory usage in m_list

template <class T>
T* CDeviceList<T>::Construct()
{
    // Create the object memory on the heap
    // Call the T constructor
    T* object = new T();

    // Enter critical section
    m_lock.Lock();

    m_list.push_back(object);

    // Leave critical section
    m_lock.Unlock();

    return object;
}

// Add a T object in m_list

template <class T>
void CDeviceList<T>::Add(T* object)
{
    // Enter critical section
    m_lock.Lock();

    m_list.push_back(object);

    // Leave critical section
    m_lock.Unlock();
}

// Remove the object pointer from the m_list
// vector and call the T destructor by
// deallocating the object

template <class T>
void CDeviceList<T>::Destruct(T* object)
{
    // Enter critical section
    m_lock.Lock();

    if (Validate(object))
    {
        // Find the object pointer in the vector and return an iterator
        typename std::vector<T*>::iterator iter = std::find(m_list.begin(), m_list.end(), object);

        // Remove the pointer from the vector if it exists
        if (iter != m_list.end())
        {
            m_list.erase(iter);
        }

        // Call the T destructor
        // Free the object memory on the heap
        delete object;
    }

    // Leave critical section
    m_lock.Unlock();
}
