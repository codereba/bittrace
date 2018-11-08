/*
 * Copyright 2010 JiJie Shi
 *
 * This file is part of NetMonitor.
 *
 * NetMonitor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * NetMonitor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with NetMonitor.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 *  2007.6.10	Ji Jie Shi modified it for this ui lib. 
 */

 #include "common_func.h"
#include "r3_rw_lock.h"

typedef struct _LAST_ACQUIRE_TYPE
{
	BOOLEAN bIsAcquireRead;

	struct _LAST_ACQUIRE_TYPE *pPre;

}LAST_ACQUIRE_TYPE,*PLAST_ACQUIRE_TYPE;

typedef struct _RW_LOCK
{
	INT iNowReaderCount; //读者计数

	PLAST_ACQUIRE_TYPE pLastAcquireType;

	ULONG ulTlsForWriteIndex;

	ULONG ulTlsForReadIndex;

	CRITICAL_SECTION ExclusiveLock; //写锁

	HANDLE hShareLock; //读锁

	CRITICAL_SECTION ShareReaderCountLock; //读者计数访问互斥锁

	LONG volatile lNowIsRelease;

}RW_LOCK,*PRW_LOCK;
代码:
#ifndef _RWLOCK_H_
#define _RWLOCK_H_

#ifdef __cplusplus

extern "C"
{
#endif

	PVOID __stdcall CreateRWLock();

	VOID __stdcall DeleteRWLock(PVOID pRWLock);

	VOID __stdcall AcquireShareLock(PVOID pRWLock);

	BOOL __stdcall TryAcquireShareLock(PVOID pRWLock);

	VOID __stdcall AcquireExclusiveLock(PVOID pRWLock);

	BOOL __stdcall TryAcquireExclusiveLock(PVOID pRWLock);

	VOID __stdcall ReleaseLock(PVOID pLock);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

typedef class _RW_LOCK_TYPE
{
public:
	_RW_LOCK_TYPE()
	{
		m_RwLockHandle=CreateRWLock();
	}

	~_RW_LOCK_TYPE()
	{
		if(IsLockOk())
		{
			DeleteRWLock(m_RwLockHandle);

			m_RwLockHandle=NULL;
		}
	}

	BOOLEAN IsLockOk(){return (m_RwLockHandle!=NULL);}

	PVOID m_RwLockHandle;

}RW_LOCK_TYPE,*PRW_LOCK_TYPE;

typedef class _WLock
{
public:
	_WLock(RW_LOCK_TYPE &RwLock)
	{
		if (RwLock.IsLockOk())
		{
			AcquireExclusiveLock(RwLock.m_RwLockHandle);

			m_RwLockHandle=RwLock.m_RwLockHandle;
		}
	}

	~_WLock()
	{
		if (IsLockOk())
		{
			ReleaseLock(m_RwLockHandle);
		}
	}

	BOOLEAN IsLockOk(){return (m_RwLockHandle!=NULL);}

private:
	PVOID m_RwLockHandle;

}WLock,*PWLock;

typedef class _RLock
{
public:
	_RLock(RW_LOCK_TYPE &RwLock)
	{
		if (RwLock.IsLockOk())
		{
			AcquireShareLock(RwLock.m_RwLockHandle);

			m_RwLockHandle=RwLock.m_RwLockHandle;
		}
	}

	~_RLock()
	{
		if (IsLockOk())
		{
			ReleaseLock(m_RwLockHandle);
		}
	}

	BOOLEAN IsLockOk(){return (m_RwLockHandle!=NULL);}

private:
	PVOID m_RwLockHandle;

}RLock,*PRLock;

//C++用法
/*
RW_LOCK_TYPE RwLock;//定义锁变量

RLock ReadLock(RwLock);//定义读锁变量而且同时加读锁（析构时会解锁）,对于读锁，不要求new和delete在同一线程

WLock WriteLock(RwLock);//定义写锁变量而且同时加写锁（析构时会解锁）,写锁支持重入，对于写锁，要求new和delete必须在同一线程

读锁和写锁同一线程混合调用时，允许先获取写锁，接着获取读锁，反过来的就不行，会死锁，也就是，

允许这样的顺序:

WLock WriteLock(RwLock);

RLock ReadLock(RwLock);

WLock WriteLock0(RwLock);

禁止这样的顺序:

RLock ReadLock(RwLock);

WLock WriteLock(RwLock);

WLock WriteLock0(RwLock);


也就是，在一个线程内，只要最开始的调用的是写锁，那么这个线程内，后续的读写锁顺序就可以随意，不受限制

*/
#endif

#endif
代码:
PVOID __stdcall CreateRWLock()
{
	PRW_LOCK pLock=NULL;

	BOOLEAN bIsOk=FALSE;

	do 
	{
		pLock=(PRW_LOCK)malloc(sizeof(RW_LOCK));

		if (!pLock)
		{
			break;
		}

		RtlZeroMemory(pLock,sizeof(RW_LOCK));

		pLock->ulTlsForWriteIndex=TlsAlloc();

		if (pLock->ulTlsForWriteIndex==(ULONG)-1)
		{
			break;
		}

		pLock->ulTlsForReadIndex=TlsAlloc();

		if (pLock->ulTlsForReadIndex==(ULONG)-1)
		{
			break;
		}

		pLock->iNowReaderCount=0;

		pLock->hShareLock=CreateEvent(NULL,FALSE,TRUE,NULL);

		if (!pLock->hShareLock)
		{
			break;
		}

		InitializeCriticalSection(&pLock->ExclusiveLock);

		InitializeCriticalSection(&pLock->ShareReaderCountLock);

		pLock->pLastAcquireType=NULL;

		bIsOk=TRUE;

	} while (FALSE);

	if (!bIsOk &&
		pLock)
	{
		if (pLock->hShareLock)
		{
			CloseHandle(pLock->hShareLock);

			pLock->hShareLock=NULL;
		}

		if (pLock->ulTlsForWriteIndex!=(ULONG)-1)
		{
			TlsFree(pLock->ulTlsForWriteIndex);

			pLock->ulTlsForWriteIndex=(ULONG)-1;
		}

		if (pLock->ulTlsForReadIndex!=(ULONG)-1)
		{
			TlsFree(pLock->ulTlsForReadIndex);

			pLock->ulTlsForReadIndex=(ULONG)-1;
		}
	}

	return (PVOID)pLock;
}

VOID __stdcall DeleteRWLock(PVOID pLock)
{
	PRW_LOCK pRwLock=(PRW_LOCK)pLock;

	PLAST_ACQUIRE_TYPE pTmpLastType=NULL;

	if (pRwLock)
	{

		if (pRwLock->ulTlsForWriteIndex!=(ULONG)-1)
		{
			TlsFree(pRwLock->ulTlsForWriteIndex);

			pRwLock->ulTlsForWriteIndex=(ULONG)-1;
		}

		if (pRwLock->ulTlsForReadIndex!=(ULONG)-1)
		{
			TlsFree(pRwLock->ulTlsForReadIndex);

			pRwLock->ulTlsForReadIndex=(ULONG)-1;
		}

		DeleteCriticalSection(&pRwLock->ExclusiveLock);

		DeleteCriticalSection(&pRwLock->ShareReaderCountLock);

		if (pRwLock->hShareLock)
		{
			CloseHandle(pRwLock->hShareLock);

			pRwLock->hShareLock=NULL;
		}

		pTmpLastType=pRwLock->pLastAcquireType;

		if (pTmpLastType)
		{
			pRwLock->pLastAcquireType=pTmpLastType->pPre;
		}else
		{
			pRwLock->pLastAcquireType=NULL;
		}

		while(pTmpLastType)
		{
			free(pTmpLastType);

			pTmpLastType=pRwLock->pLastAcquireType;

			if (pTmpLastType)
			{
				pRwLock->pLastAcquireType=pTmpLastType->pPre;
			}else
			{
				pRwLock->pLastAcquireType=NULL;
			}
		}

		free(pLock);

		pLock=NULL;

	}
}

VOID __stdcall AcquireShareLock(PVOID pLock)
{
	PRW_LOCK pRwLock=(PRW_LOCK)pLock;

	ULONG_PTR ulExclusiveFlag=0;

	ULONG_PTR ulSharedFlag=0;

	PLAST_ACQUIRE_TYPE pLastType=malloc(sizeof(LAST_ACQUIRE_TYPE));

	do 
	{
		if (!pLastType)
		{
			__debugbreak();
		}

		ulSharedFlag=(ULONG_PTR)TlsGetValue(pRwLock->ulTlsForReadIndex);

		ulExclusiveFlag=(ULONG_PTR)TlsGetValue(pRwLock->ulTlsForWriteIndex);


		//判断当前线程的读写锁状态

		if (ulSharedFlag!=1 &&
			ulSharedFlag!=0)//当前线程在读锁状态,增加计数就够了
		{
			EnterCriticalSection(&pRwLock->ShareReaderCountLock);

			pRwLock->iNowReaderCount++;

			pLastType->bIsAcquireRead=TRUE;

			pLastType->pPre=pRwLock->pLastAcquireType;

			pRwLock->pLastAcquireType=pLastType;

			LeaveCriticalSection(&pRwLock->ShareReaderCountLock);

			if (!TlsSetValue(pRwLock->ulTlsForReadIndex,(PVOID)(ulSharedFlag+1)))
			{
				__debugbreak();
			}

			break;
		}

		if (ulExclusiveFlag!=1 &&
			ulExclusiveFlag!=0)//当前线程占有写锁，所以只要记录下当前操作就够了
		{
			pLastType->bIsAcquireRead=TRUE;

			pLastType->pPre=pRwLock->pLastAcquireType;

			pRwLock->pLastAcquireType=pLastType;

			break;
		}


		//判断其它线程的读写锁状态

		EnterCriticalSection(&pRwLock->ExclusiveLock);//当前如果有其它线程企图获取写锁，先把这个读锁请求阻塞，使得写优先;同时，这个逻辑还有个作用就是判断其它线程是否在写状态,如果是，这里会阻塞

		EnterCriticalSection(&pRwLock->ShareReaderCountLock);

		if (pRwLock->iNowReaderCount>0)//有其它线程在读锁状态
		{
			pRwLock->iNowReaderCount++;

			pLastType->bIsAcquireRead=TRUE;

			pLastType->pPre=pRwLock->pLastAcquireType;

			pRwLock->pLastAcquireType=pLastType;

			LeaveCriticalSection(&pRwLock->ShareReaderCountLock);

			LeaveCriticalSection(&pRwLock->ExclusiveLock);

			ulSharedFlag=1;

			if (!TlsSetValue(pRwLock->ulTlsForReadIndex,(PVOID)(ulSharedFlag+1)))
			{
				__debugbreak();
			}

			break;
		}

		//没有任何线程占有任何读写锁

		WaitForSingleObject(pRwLock->hShareLock,INFINITE);//获取读锁，使得企图获取写锁的线程，阻塞在这里

		pLastType->bIsAcquireRead=TRUE;

		pLastType->pPre=pRwLock->pLastAcquireType;

		pRwLock->pLastAcquireType=pLastType;

		pRwLock->iNowReaderCount++;

		LeaveCriticalSection(&pRwLock->ShareReaderCountLock);

		LeaveCriticalSection(&pRwLock->ExclusiveLock);

		ulSharedFlag=1;

		if (!TlsSetValue(pRwLock->ulTlsForReadIndex,(PVOID)(ulSharedFlag+1)))
		{
			__debugbreak();
		}

	} while (FALSE);
}

BOOL __stdcall TryAcquireShareLock(PVOID pLock)
{
	PRW_LOCK pRwLock=(PRW_LOCK)pLock;

	BOOL bIsAcquired=FALSE;

	ULONG_PTR ulExclusiveFlag=0;

	PLAST_ACQUIRE_TYPE pLastType=NULL;

	ULONG_PTR ulSharedFlag=0;

	do 
	{
		pLastType=malloc(sizeof(LAST_ACQUIRE_TYPE));

		if (!pLastType)
		{
			break;
		}

		//首先判断当前线程的读写锁状态

		ulSharedFlag=(ULONG_PTR)TlsGetValue(pRwLock->ulTlsForReadIndex);

		ulExclusiveFlag=(ULONG_PTR)TlsGetValue(pRwLock->ulTlsForWriteIndex);

		if (ulSharedFlag!=1 &&
			ulSharedFlag!=0)//当前线程在读锁状态。只要计数增加，不需要新获取读锁状态
		{
			if (!TlsSetValue(pRwLock->ulTlsForReadIndex,(PVOID)(ulSharedFlag+1)))
			{
				bIsAcquired=FALSE;

				break;
			}

			bIsAcquired=TryEnterCriticalSection(&pRwLock->ShareReaderCountLock);

			if (!bIsAcquired)
			{
				break;
			}

			pLastType->bIsAcquireRead=TRUE;

			pLastType->pPre=pRwLock->pLastAcquireType;

			pRwLock->pLastAcquireType=pLastType;

			pLastType=NULL;

			pRwLock->iNowReaderCount ++;

			LeaveCriticalSection(&pRwLock->ShareReaderCountLock);

			break;
		}


		//当前线程不占有读锁但是占有写锁，只要记录下当前操作就够了
		if (ulExclusiveFlag!=1 &&
			ulExclusiveFlag!=0)
		{
			pLastType->bIsAcquireRead=TRUE;

			pLastType->pPre=pRwLock->pLastAcquireType;

			pRwLock->pLastAcquireType=pLastType;

			break;
		}


		//到这里说明当前线程不占有任和锁，继续判断其它线程的读写锁状态

		bIsAcquired=TryEnterCriticalSection(&pRwLock->ExclusiveLock);

		if (!bIsAcquired)//说明其它线程占有写锁，失败返回,否则会阻塞
		{
			break;
		}

		bIsAcquired=TryEnterCriticalSection(&pRwLock->ShareReaderCountLock);

		if (!bIsAcquired)
		{
			LeaveCriticalSection(&pRwLock->ExclusiveLock);

			break;
		}

		if (pRwLock->iNowReaderCount>0)
		{
			ulSharedFlag=1;

			if (!TlsSetValue(pRwLock->ulTlsForReadIndex,(PVOID)(ulSharedFlag+1)))
			{
				LeaveCriticalSection(&pRwLock->ShareReaderCountLock);

				LeaveCriticalSection(&pRwLock->ExclusiveLock);

				break;
			}

			pRwLock->iNowReaderCount++;

			pLastType->bIsAcquireRead=TRUE;

			pLastType->pPre=pRwLock->pLastAcquireType;

			pRwLock->pLastAcquireType=pLastType;

			pLastType=NULL;

			LeaveCriticalSection(&pRwLock->ShareReaderCountLock);

			LeaveCriticalSection(&pRwLock->ExclusiveLock);

			break;
		}


		//到这里说明没有线程占有任何锁

		WaitForSingleObject(pRwLock->hShareLock,INFINITE);//获取读锁，使得企图获取写锁的线程，阻塞在这里

		ulSharedFlag=1;

		if (!TlsSetValue(pRwLock->ulTlsForReadIndex,(PVOID)(ulSharedFlag+1)))
		{
			bIsAcquired=FALSE;

			SetEvent(pRwLock->hShareLock);

			LeaveCriticalSection(&pRwLock->ShareReaderCountLock);

			LeaveCriticalSection(&pRwLock->ExclusiveLock);

			break;
		}

		pLastType->bIsAcquireRead=TRUE;

		pLastType->pPre=pRwLock->pLastAcquireType;

		pRwLock->pLastAcquireType=pLastType;

		pLastType=NULL;

		pRwLock->iNowReaderCount++;

		LeaveCriticalSection(&pRwLock->ShareReaderCountLock);

		LeaveCriticalSection(&pRwLock->ExclusiveLock);

	} while (FALSE);

	if (pLastType)
	{
		free(pLastType);

		pLastType=NULL;
	}

	return bIsAcquired;
}

VOID __stdcall ReleaseShareLock(PVOID pLock)
{
	PRW_LOCK pRwLock=(PRW_LOCK)pLock;

	ULONG_PTR ulExclusiveFlag=0;

	ULONG_PTR ulSharedFlag=0;

	do 
	{
		ulSharedFlag=(ULONG_PTR)TlsGetValue(pRwLock->ulTlsForReadIndex);

		if (ulSharedFlag!=0 &&
			ulSharedFlag!=1)//当前线程在读锁状态
		{
			EnterCriticalSection(&pRwLock->ShareReaderCountLock);

			if (pRwLock->iNowReaderCount==0)
			{
				__debugbreak();
			}

			pRwLock->iNowReaderCount--;

			if (!TlsSetValue(pRwLock->ulTlsForReadIndex,(PVOID)(ulSharedFlag-1)))
			{
				__debugbreak();
			}

			if( pRwLock->iNowReaderCount == 0 )
			{
				SetEvent(pRwLock->hShareLock);
			}

			LeaveCriticalSection(&pRwLock->ShareReaderCountLock);

			break;
		}

	} while (FALSE);
}

VOID __stdcall AcquireExclusiveLock(PVOID pLock)
{
	PRW_LOCK pRwLock=(PRW_LOCK)pLock;

	ULONG_PTR ulExclusiveFlag=0;

	ULONG_PTR ulShareFlag=0;

	PLAST_ACQUIRE_TYPE pLastType=malloc(sizeof(LAST_ACQUIRE_TYPE));

	do 
	{
		if (!pLastType)
		{
			__debugbreak();
		}

		ulShareFlag=(ULONG_PTR)TlsGetValue(pRwLock->ulTlsForReadIndex);

		ulExclusiveFlag=(ULONG_PTR)TlsGetValue(pRwLock->ulTlsForWriteIndex);

		//先判断当前线程的读写锁状态

		////当前线程处于读锁状态，若获取写锁，会死锁在WaitForSingleObject里，所以直接让程序崩溃，便于找出bug位置
		if (ulShareFlag!=1 &&
			ulShareFlag!=0)
		{
			__debugbreak();
		}

		//当前线程占有写锁,写锁计数+1
		if(ulExclusiveFlag!=1 &&
			ulExclusiveFlag!=0)
		{
			EnterCriticalSection(&pRwLock->ExclusiveLock);

			if (!TlsSetValue(pRwLock->ulTlsForWriteIndex,(PVOID)(ulExclusiveFlag+1)))
			{
				__debugbreak();
			}

			pLastType->bIsAcquireRead=FALSE;

			pLastType->pPre=pRwLock->pLastAcquireType;

			pRwLock->pLastAcquireType=pLastType;

			break;

		}

		//当前线程不占有读写锁

		ulExclusiveFlag=1;//初始化当前线程占有写锁的标记为首次占用

		//以下一行代码主要这样几个目的:

		//1、若其它线程占有写锁，会阻塞在这个锁里
		//2、若其它线程不占有写锁，那这行代码使得后续企图获取写锁的线程阻塞在这个锁里
		//3、使得那些不占有读锁的线程，企图获取读锁时，阻塞在这个锁里，达到写优先的目的
		//4、占有写锁
		EnterCriticalSection(&pRwLock->ExclusiveLock);


		//本线程获取到写锁的基本条件，但还需要等待其它占有读锁的线程释放读锁
		//任何时候，只会有一个线程到达这里

		WaitForSingleObject(pRwLock->hShareLock,INFINITE);//等待所有已经获取读锁的线程完成操作(一定只有一个想获取写锁的线程在这里等待，因为其它想获取写锁的线程会卡在上面一行）

		SetEvent(pRwLock->hShareLock);

		if (!TlsSetValue(pRwLock->ulTlsForWriteIndex,(PVOID)(ulExclusiveFlag+1)))
		{
			__debugbreak();
		}

		pLastType->bIsAcquireRead=FALSE;

		pLastType->pPre=pRwLock->pLastAcquireType;

		pRwLock->pLastAcquireType=pLastType;

	} while (FALSE);

	return;
}

BOOL __stdcall TryAcquireExclusiveLock(PVOID pLock)
{
	PRW_LOCK pRwLock=(PRW_LOCK)pLock;

	BOOL bIsAcquired=FALSE;

	PLAST_ACQUIRE_TYPE pLastType=NULL;

	ULONG_PTR ulExclusiveFlag=0;

	ULONG_PTR ulSharedFlag=0;

	do 
	{
		pLastType=(PLAST_ACQUIRE_TYPE)malloc(sizeof(LAST_ACQUIRE_TYPE));

		if (!pLastType)
		{
			break;
		}

		ulSharedFlag=(ULONG_PTR)TlsGetValue(pRwLock->ulTlsForReadIndex);

		ulExclusiveFlag=(ULONG_PTR)TlsGetValue(pRwLock->ulTlsForWriteIndex);

		//先判断当前线程的读写锁状态

		if (ulSharedFlag!=0 &&
			ulSharedFlag!=1)//本线程之前占用了读锁，失败返回
		{
			bIsAcquired=FALSE;

			break;
		}

		if (ulExclusiveFlag!=0 &&
			ulExclusiveFlag!=1)//本线程之前占用写锁
		{
			if(!TlsSetValue(pRwLock->ulTlsForWriteIndex,(PVOID)(ulExclusiveFlag+1)))
			{
				bIsAcquired=FALSE;

				break;
			}

			EnterCriticalSection(&pRwLock->ExclusiveLock);//增加写锁引用

			pLastType->bIsAcquireRead=FALSE;

			pLastType->pPre=pRwLock->pLastAcquireType;

			pRwLock->pLastAcquireType=pLastType;

			pLastType=NULL;

			break;
		}



		//接下来判断其它线程占有读写锁的状态


		//以下一行代码主要这样几个目的:

		//1、若其它线程不占有写锁，那这行代码使得后续企图获取写锁的线程阻塞在这个锁里
		//2、使得那些不占有读锁的线程，企图获取读锁时，阻塞在这个锁里，达到写优先的目的
		//3、占有写锁
		//4、因为Try类型，所以如果失败就返回，不阻塞
		bIsAcquired=TryEnterCriticalSection(&pRwLock->ExclusiveLock);

		if (!bIsAcquired)//其它线程占用写锁,失败返回
		{
			break;
		}

		bIsAcquired=TryEnterCriticalSection(&pRwLock->ShareReaderCountLock);

		if (!bIsAcquired)
		{
			LeaveCriticalSection(&pRwLock->ExclusiveLock);

			break;
		}

		if (pRwLock->iNowReaderCount>0)//其它线程占有读锁，失败返回
		{
			LeaveCriticalSection(&pRwLock->ShareReaderCountLock);

			LeaveCriticalSection(&pRwLock->ExclusiveLock);

			bIsAcquired=FALSE;

			break;
		}else
		{
			LeaveCriticalSection(&pRwLock->ShareReaderCountLock);//释放掉辅助锁，保留写锁
		}


		//没有任何线程占用读锁或者写锁

		ulExclusiveFlag=1;//初始化当前线程占有写锁的标记为首次占用

		if(!TlsSetValue(pRwLock->ulTlsForWriteIndex,(PVOID)(ulExclusiveFlag+1)))
		{
			LeaveCriticalSection(&pRwLock->ExclusiveLock);//失败，释放掉写锁

			bIsAcquired=FALSE;

			break;
		}

		pLastType->bIsAcquireRead=FALSE;

		pLastType->pPre=pRwLock->pLastAcquireType;

		pRwLock->pLastAcquireType=pLastType;

		pLastType=NULL;

	} while (FALSE);


	if (pLastType)
	{
		free(pLastType);

		pLastType=NULL;
	}

	return bIsAcquired;
}


VOID __stdcall ReleaseExclusiveLock(PVOID pLock)
{
	PRW_LOCK pRwLock=(PRW_LOCK)pLock;

	ULONG_PTR ulExclusiveFlag=0;

	ULONG_PTR ulSharedFlag=0;

	ulExclusiveFlag=(ULONG_PTR)TlsGetValue(pRwLock->ulTlsForWriteIndex);

	if (ulExclusiveFlag<=1)
	{
		__debugbreak();
	}

	if(!TlsSetValue(pRwLock->ulTlsForWriteIndex,(PVOID)(ulExclusiveFlag-1)))
	{
		__debugbreak();
	}

	LeaveCriticalSection(&pRwLock->ExclusiveLock);
}

VOID __stdcall ReleaseLock(PVOID pLock)
{
	PRW_LOCK pRwLock=(PRW_LOCK)pLock;

	PLAST_ACQUIRE_TYPE pTmpLastType=NULL;

	while(InterlockedCompareExchange(&pRwLock->lNowIsRelease,1,0)!=0)
	{
		Sleep(0);
	}

	pTmpLastType=pRwLock->pLastAcquireType;

	pRwLock->pLastAcquireType=pTmpLastType->pPre;

	InterlockedExchange(&pRwLock->lNowIsRelease,0);

	if (!pTmpLastType)
	{
		__debugbreak();
	}

	if (pTmpLastType->bIsAcquireRead)
	{
		ReleaseShareLock(pRwLock);


	}else
	{
		ReleaseExclusiveLock(pRwLock);

		pRwLock->pLastAcquireType=pTmpLastType->pPre;
	}

	free(pTmpLastType);

	pTmpLastType=NULL;
}