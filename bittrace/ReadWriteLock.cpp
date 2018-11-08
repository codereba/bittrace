#include "StdAfx.h"
#include "ReadWriteLock.h"


/*!
    Default constructor.
*/
CReadWriteLock::CReadWriteLock(void)
 : m_nReaderCount(0), m_hWriterEvent(NULL), m_hNoReadersEvent(NULL)
{
    // Create writer event with manual reset and default signaled state.
    // 
    // State:
    //          Signaled     = Writer has currently not access.
    //          Non-signaled = Writer has currently access, block readers.
    //
    m_hWriterEvent    = CreateEvent(NULL, TRUE, TRUE, NULL);
    
    // Create no readers event with manual reset and default signaled state.
    // 
    // State:
    //          Signaled     = No readers have currently access.
    //          Non-signaled = Some readers have currently access, block writer.
    //
    m_hNoReadersEvent = CreateEvent(NULL, TRUE, TRUE, NULL);

    //
    // Initialize critical sections.
    InitializeCriticalSection(&m_csLockWriter);
    InitializeCriticalSection(&m_csReaderCount);
}


/*!
    Private copy constructor.

    Prevents shallow copy of this class.
*/
CReadWriteLock::CReadWriteLock(const CReadWriteLock &cReadWriteLock)
{
}


/*!
    Private assignment operator.

    Prevents shallow copy of this class.
*/

const CReadWriteLock& CReadWriteLock::operator=(const CReadWriteLock &cReadWriteLock)
{
    return *this;
}


/*!
    Default destructor.
*/
CReadWriteLock::~CReadWriteLock(void)
{
    //
    // Delete critical sections.
    DeleteCriticalSection(&m_csLockWriter);
    DeleteCriticalSection(&m_csReaderCount);

    // Close the writer event.
    CloseHandle(m_hWriterEvent);
    // Close the no readers event.
    CloseHandle(m_hNoReadersEvent);
}


/*!
    Ensures unlimited number of reader access, and guarantees no writer access.

    \sa UnlockReader().
*/
void CReadWriteLock::LockReader()
{
    bool bLoop = true;

    // Loop.
    while(bLoop)
    {
        // Wait for Writer event to be signaled.
        WaitForSingleObject(m_hWriterEvent, INFINITE);
        // Increment number of readers.
        IncrementReaderCount();
        // If writer is become non-signaled fall back (double locking).
        if(WaitForSingleObject(m_hWriterEvent, 0) != WAIT_OBJECT_0)
        {
            // Decrement number of readers.
            DecrementReaderCount();
        }
        else
        {
            // Breakout.
            bLoop = false;
        }
    }
}


/*!
    Unlocks the reader access.

    \sa LockReader().
*/
void CReadWriteLock::UnlockReader()
{
    // Decrement number of readers.
    DecrementReaderCount();
}


/*!
    Ensure that only one writer is granted access.

    \sa UnlockWriter().
*/
void CReadWriteLock::LockWriter()
{
    // Enter critical section (prevent more than one writer).
    EnterCriticalSection(&m_csLockWriter);
    // Wait for current writer.
    WaitForSingleObject(m_hWriterEvent, INFINITE);
    // Set writer to non-signaled.
    ResetEvent(m_hWriterEvent);
    // Wait for current readers to finish.
    WaitForSingleObject(m_hNoReadersEvent, INFINITE); 
    // Leave critical section.
    LeaveCriticalSection(&m_csLockWriter);
}


/*!
    Unlocks the writer access.

    \sa LockWriter().
*/
void CReadWriteLock::UnlockWriter()
{
    // Set writer event to signaled.
    SetEvent(m_hWriterEvent);
}


/*!
    \sa DecrementReaderCount().
*/
void CReadWriteLock::IncrementReaderCount()
{
    // Enter critical section.
    EnterCriticalSection(&m_csReaderCount);        
    // Increase reader count.
    m_nReaderCount++;
    // Reset the no readers event.
    ResetEvent(m_hNoReadersEvent);
    // Leave critical section.
    LeaveCriticalSection(&m_csReaderCount);
}


/*!
    \sa IncrementReaderCount().
*/
void CReadWriteLock::DecrementReaderCount()
{
    // Enter critical section.
    EnterCriticalSection(&m_csReaderCount);        
    // Decrease reader count.
    m_nReaderCount--;
    // Are all readers out?
    if(m_nReaderCount <= 0)
    {
        // Set the no readers event.
        SetEvent(m_hNoReadersEvent);
    }
    // Leave critical section.
    LeaveCriticalSection(&m_csReaderCount);
}
