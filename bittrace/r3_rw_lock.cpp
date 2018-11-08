/*
 *
 * Copyright 2010 JiJie Shi(weixin:AIChangeLife)
 *
 * This file is part of bittrace.
 *
 * bittrace is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * bittrace is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bittrace.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common_func.h"
#include "r3_rw_lock.h"

class FastReadWriteLock
{
public:        

	FastReadWriteLock()
	{
		m_lockState = 0; 
		m_hReadSem = CreateSemaphore(0,0,0x10000,0);
		m_hWriteSem = CreateSemaphore(0,0,0x10000,0);
	}

	~FastReadWriteLock()
	{
		CloseHandle(m_hReadSem); 
		CloseHandle(m_hWriteSem);
	}

	void WaitForReadLock()
	{
		while (true)
		{
			LockState lockState = m_lockState;
			if (lockState.writeLock || lockState.writeWaiting || lockState.readLock == -1) 
			{
				if (lockState.readWaiting == -1) Sleep(0); 
				else
				{
					LockState newState = lockState; newState.readWaiting++; 
					if (lockState == InterlockedCompareExchange(&m_lockState,newState,lockState))
					{
						WaitForSingleObject(m_hReadSem,INFINITE);
						while (true)
						{
							lockState = m_lockState;
							newState = lockState; newState.readWaiting--;
							if (lockState == InterlockedCompareExchange(&m_lockState,newState,lockState)) break; 
						}
					}
				}
			}
			else
			{                
				LockState newState = lockState; newState.readLock++; 
				if (lockState == InterlockedCompareExchange(&m_lockState,newState,lockState)) break; 
			}
		} 
	}

	bool TryReadLock()
	{
		LockState lockState = m_lockState; 
		if (lockState.writeLock || lockState.writeWaiting || lockState.readLock == -1) return false; 
		LockState newState = lockState; newState.readLock++;
		return lockState == InterlockedCompareExchange(&m_lockState,newState,lockState); 
	}

	void ReleaseReadLock()
	{
		LockState lockState = m_lockState; 
		while (lockState.readLock) 
		{
			LockState newState = lockState; newState.readLock--; 
			if (lockState == InterlockedCompareExchange(&m_lockState,newState,lockState)) break;
			lockState = m_lockState; 
		} 
		if (lockState.writeWaiting)
		{
			ReleaseSemaphore(m_hWriteSem,1,NULL); 
		}
		else if (lockState.readWaiting)
		{
			ReleaseSemaphore(m_hReadSem,lockState.readWaiting,NULL); 
		}
	}

	void WaitForWriteLock()
	{
		while (true)
		{
			LockState lockState = m_lockState; 
			if (lockState.writeLock || lockState.readLock)
			{
				if (lockState.writeWaiting == -1) Sleep(0); 
				else
				{
					LockState newState = lockState; newState.writeWaiting++;
					if (lockState == InterlockedCompareExchange(&m_lockState,newState,lockState)) 
					{
						WaitForSingleObject(m_hWriteSem,INFINITE); 
						while (true) 
						{
							lockState = m_lockState; 
							newState = lockState; newState.writeWaiting--;
							if (lockState == InterlockedCompareExchange(&m_lockState,newState,lockState)) break; 
						}
					}
				}
			}
			else
			{                
				LockState newState = lockState; newState.writeLock = 1; 
				if (lockState == InterlockedCompareExchange(&m_lockState,newState,lockState)) break;
			}
		}
	}

	bool TryWriteLock()
	{
		LockState lockState = m_lockState;
		if (lockState.writeLock || lockState.readLock) return false; 
		LockState newState = lockState; newState.writeLock = 1; 
		return lockState == InterlockedCompareExchange(&m_lockState,newState,lockState);
	}

	void ReleaseWriteLock()
	{
		LockState lockState = m_lockState; 
		while (lockState.writeLock) 
		{
			LockState newState = lockState; newState.writeLock = 0; 
			if (lockState == InterlockedCompareExchange(&m_lockState,newState,lockState)) break;
			lockState = m_lockState; 
		} 
		if (lockState.writeWaiting)
		{
			ReleaseSemaphore(m_hWriteSem,1,NULL);
		}
		else if (lockState.readWaiting)
		{
			ReleaseSemaphore(m_hReadSem,lockState.readWaiting,NULL); 
		}
	}

protected:

	struct LockState
	{
		unsigned long   readWaiting     :11,
readLock        :11,                    
writeWaiting    :9,
writeLock       :1;
		inline LockState(LONG value = 0) { *(LONG*)this = value; }
		inline operator LONG() const { return *reinterpret_cast<const unsigned long*>(this); }
		inline LockState& operator =(LONG value) { *(LONG*)this = value; return *this; }
		inline LockState& operator =(const LockState& rhs) { *(LONG*)this = (LONG)rhs; return *this; }
		inline bool operator ==(LONG value) const { return (LONG)*this == value; }
	};

	volatile LONG   m_lockState;
	HANDLE          m_hReadSem;
	HANDLE          m_hWriteSem;
};

CReadWriteLock::CReadWriteLock(void)
 : m_nReaderCount(0), m_hWriterEvent(NULL), m_hNoReadersEvent(NULL)
{
    m_hWriterEvent    = CreateEvent(NULL, TRUE, TRUE, NULL);
    
    m_hNoReadersEvent = CreateEvent(NULL, TRUE, TRUE, NULL);

    InitializeCriticalSection(&m_csLockWriter);
    InitializeCriticalSection(&m_csReaderCount);
}

CReadWriteLock::CReadWriteLock(const CReadWriteLock &cReadWriteLock)
{
}

const CReadWriteLock& CReadWriteLock::operator=(const CReadWriteLock &cReadWriteLock)
{
    return *this;
}

CReadWriteLock::~CReadWriteLock(void)
{
    DeleteCriticalSection(&m_csLockWriter);
    DeleteCriticalSection(&m_csReaderCount);

    CloseHandle(m_hWriterEvent);
    CloseHandle(m_hNoReadersEvent);
}

void CReadWriteLock::LockReader()
{
    bool bLoop = true;

    while(bLoop)
    {
        WaitForSingleObject(m_hWriterEvent, INFINITE);
        IncrementReaderCount();
        if(WaitForSingleObject(m_hWriterEvent, 0) != WAIT_OBJECT_0)
        {
            DecrementReaderCount();
        }
        else
        {
            bLoop = false;
        }
    }
}

void CReadWriteLock::UnlockReader()
{
    DecrementReaderCount();
}

void CReadWriteLock::LockWriter()
{
    EnterCriticalSection(&m_csLockWriter);
    WaitForSingleObject(m_hWriterEvent, INFINITE);
    ResetEvent(m_hWriterEvent);
    WaitForSingleObject(m_hNoReadersEvent, INFINITE); 
    LeaveCriticalSection(&m_csLockWriter);
}

void CReadWriteLock::UnlockWriter()
{
    SetEvent(m_hWriterEvent);
}


void CReadWriteLock::IncrementReaderCount()
{
    EnterCriticalSection(&m_csReaderCount);        
    m_nReaderCount++;
    ResetEvent(m_hNoReadersEvent);
    LeaveCriticalSection(&m_csReaderCount);
}


void CReadWriteLock::DecrementReaderCount()
{
    EnterCriticalSection(&m_csReaderCount);        
    m_nReaderCount--;
    if(m_nReaderCount <= 0)
    {
        SetEvent(m_hNoReadersEvent);
    }
    LeaveCriticalSection(&m_csReaderCount);
}
