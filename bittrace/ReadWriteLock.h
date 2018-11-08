#pragma once

#include "Windows.h"


/*!
\brief Simple Read/Write Lock mechanism for multiple read, and single write thread access.

This class provides a simple mechanism for providing multiple read and single write
access to shared resources between multiple threads.

*/
class CReadWriteLock
{
public:
    //! Constructor.
    CReadWriteLock(void);
    //! Destructor.
    virtual ~CReadWriteLock(void);
    
    //! Lock for reader access.
    void LockReader();
    //! Unlock reader access.
    void UnlockReader();
    
    //! Lock for writer access.
    void LockWriter();
    //! Unlock writer access.
    void UnlockWriter();

private:
    //! Private copy constructor.
    CReadWriteLock(const CReadWriteLock &cReadWriteLock);
    //! Private assignment operator.
    const CReadWriteLock& operator=(const CReadWriteLock &cReadWriteLock);

    //! Increment number of readers.
    void IncrementReaderCount();
    //! Decrement number of readers.
    void DecrementReaderCount();

    //! Writer access event.
    HANDLE m_hWriterEvent;
    //! No readers event.
    HANDLE m_hNoReadersEvent;
    //! Number of readers.
    int m_nReaderCount;
    
    //! Critical section for protecting lock writer method.
    CRITICAL_SECTION m_csLockWriter;
    //! Critical section for protecting reader count.
    CRITICAL_SECTION m_csReaderCount;
};
