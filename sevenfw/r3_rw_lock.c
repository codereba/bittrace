/*
 * Copyright 2010-2024 JiJie.Shi.
 *
 * This file is part of bittrace.
 * Licensed under the Gangoo License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
	INT iNowReaderCount; //���߼���

	PLAST_ACQUIRE_TYPE pLastAcquireType;

	ULONG ulTlsForWriteIndex;

	ULONG ulTlsForReadIndex;

	CRITICAL_SECTION ExclusiveLock; //д��

	HANDLE hShareLock; //����

	CRITICAL_SECTION ShareReaderCountLock; //���߼������ʻ�����

	LONG volatile lNowIsRelease;

}RW_LOCK,*PRW_LOCK;
����:
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

//C++�÷�
/*
RW_LOCK_TYPE RwLock;//����������

RLock ReadLock(RwLock);//���������������ͬʱ�Ӷ���������ʱ�������,���ڶ�������Ҫ��new��delete��ͬһ�߳�

WLock WriteLock(RwLock);//����д����������ͬʱ��д��������ʱ�������,д��֧�����룬����д����Ҫ��new��delete������ͬһ�߳�

������д��ͬһ�̻߳�ϵ���ʱ�������Ȼ�ȡд�������Ż�ȡ�������������ľͲ��У���������Ҳ���ǣ�

����������˳��:

WLock WriteLock(RwLock);

RLock ReadLock(RwLock);

WLock WriteLock0(RwLock);

��ֹ������˳��:

RLock ReadLock(RwLock);

WLock WriteLock(RwLock);

WLock WriteLock0(RwLock);


Ҳ���ǣ���һ���߳��ڣ�ֻҪ�ʼ�ĵ��õ���д������ô����߳��ڣ������Ķ�д��˳��Ϳ������⣬��������

*/
#endif

#endif
����:
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


		//�жϵ�ǰ�̵߳Ķ�д��״̬

		if (ulSharedFlag!=1 &&
			ulSharedFlag!=0)//��ǰ�߳��ڶ���״̬,���Ӽ����͹���
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
			ulExclusiveFlag!=0)//��ǰ�߳�ռ��д��������ֻҪ��¼�µ�ǰ�����͹���
		{
			pLastType->bIsAcquireRead=TRUE;

			pLastType->pPre=pRwLock->pLastAcquireType;

			pRwLock->pLastAcquireType=pLastType;

			break;
		}


		//�ж������̵߳Ķ�д��״̬

		EnterCriticalSection(&pRwLock->ExclusiveLock);//��ǰ����������߳���ͼ��ȡд�����Ȱ������������������ʹ��д����;ͬʱ������߼����и����þ����ж������߳��Ƿ���д״̬,����ǣ����������

		EnterCriticalSection(&pRwLock->ShareReaderCountLock);

		if (pRwLock->iNowReaderCount>0)//�������߳��ڶ���״̬
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

		//û���κ��߳�ռ���κζ�д��

		WaitForSingleObject(pRwLock->hShareLock,INFINITE);//��ȡ������ʹ����ͼ��ȡд�����̣߳�����������

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

		//�����жϵ�ǰ�̵߳Ķ�д��״̬

		ulSharedFlag=(ULONG_PTR)TlsGetValue(pRwLock->ulTlsForReadIndex);

		ulExclusiveFlag=(ULONG_PTR)TlsGetValue(pRwLock->ulTlsForWriteIndex);

		if (ulSharedFlag!=1 &&
			ulSharedFlag!=0)//��ǰ�߳��ڶ���״̬��ֻҪ�������ӣ�����Ҫ�»�ȡ����״̬
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


		//��ǰ�̲߳�ռ�ж�������ռ��д����ֻҪ��¼�µ�ǰ�����͹���
		if (ulExclusiveFlag!=1 &&
			ulExclusiveFlag!=0)
		{
			pLastType->bIsAcquireRead=TRUE;

			pLastType->pPre=pRwLock->pLastAcquireType;

			pRwLock->pLastAcquireType=pLastType;

			break;
		}


		//������˵����ǰ�̲߳�ռ���κ����������ж������̵߳Ķ�д��״̬

		bIsAcquired=TryEnterCriticalSection(&pRwLock->ExclusiveLock);

		if (!bIsAcquired)//˵�������߳�ռ��д����ʧ�ܷ���,���������
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


		//������˵��û���߳�ռ���κ���

		WaitForSingleObject(pRwLock->hShareLock,INFINITE);//��ȡ������ʹ����ͼ��ȡд�����̣߳�����������

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
			ulSharedFlag!=1)//��ǰ�߳��ڶ���״̬
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

		//���жϵ�ǰ�̵߳Ķ�д��״̬

		////��ǰ�̴߳��ڶ���״̬������ȡд������������WaitForSingleObject�����ֱ���ó�������������ҳ�bugλ��
		if (ulShareFlag!=1 &&
			ulShareFlag!=0)
		{
			__debugbreak();
		}

		//��ǰ�߳�ռ��д��,д������+1
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

		//��ǰ�̲߳�ռ�ж�д��

		ulExclusiveFlag=1;//��ʼ����ǰ�߳�ռ��д���ı��Ϊ�״�ռ��

		//����һ�д�����Ҫ��������Ŀ��:

		//1���������߳�ռ��д�������������������
		//2���������̲߳�ռ��д���������д���ʹ�ú�����ͼ��ȡд�����߳��������������
		//3��ʹ����Щ��ռ�ж������̣߳���ͼ��ȡ����ʱ���������������ﵽд���ȵ�Ŀ��
		//4��ռ��д��
		EnterCriticalSection(&pRwLock->ExclusiveLock);


		//���̻߳�ȡ��д���Ļ���������������Ҫ�ȴ�����ռ�ж������߳��ͷŶ���
		//�κ�ʱ��ֻ����һ���̵߳�������

		WaitForSingleObject(pRwLock->hShareLock,INFINITE);//�ȴ������Ѿ���ȡ�������߳���ɲ���(һ��ֻ��һ�����ȡд�����߳�������ȴ�����Ϊ�������ȡд�����̻߳Ῠ������һ�У�

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

		//���жϵ�ǰ�̵߳Ķ�д��״̬

		if (ulSharedFlag!=0 &&
			ulSharedFlag!=1)//���߳�֮ǰռ���˶�����ʧ�ܷ���
		{
			bIsAcquired=FALSE;

			break;
		}

		if (ulExclusiveFlag!=0 &&
			ulExclusiveFlag!=1)//���߳�֮ǰռ��д��
		{
			if(!TlsSetValue(pRwLock->ulTlsForWriteIndex,(PVOID)(ulExclusiveFlag+1)))
			{
				bIsAcquired=FALSE;

				break;
			}

			EnterCriticalSection(&pRwLock->ExclusiveLock);//����д������

			pLastType->bIsAcquireRead=FALSE;

			pLastType->pPre=pRwLock->pLastAcquireType;

			pRwLock->pLastAcquireType=pLastType;

			pLastType=NULL;

			break;
		}



		//�������ж������߳�ռ�ж�д����״̬


		//����һ�д�����Ҫ��������Ŀ��:

		//1���������̲߳�ռ��д���������д���ʹ�ú�����ͼ��ȡд�����߳��������������
		//2��ʹ����Щ��ռ�ж������̣߳���ͼ��ȡ����ʱ���������������ﵽд���ȵ�Ŀ��
		//3��ռ��д��
		//4����ΪTry���ͣ��������ʧ�ܾͷ��أ�������
		bIsAcquired=TryEnterCriticalSection(&pRwLock->ExclusiveLock);

		if (!bIsAcquired)//�����߳�ռ��д��,ʧ�ܷ���
		{
			break;
		}

		bIsAcquired=TryEnterCriticalSection(&pRwLock->ShareReaderCountLock);

		if (!bIsAcquired)
		{
			LeaveCriticalSection(&pRwLock->ExclusiveLock);

			break;
		}

		if (pRwLock->iNowReaderCount>0)//�����߳�ռ�ж�����ʧ�ܷ���
		{
			LeaveCriticalSection(&pRwLock->ShareReaderCountLock);

			LeaveCriticalSection(&pRwLock->ExclusiveLock);

			bIsAcquired=FALSE;

			break;
		}else
		{
			LeaveCriticalSection(&pRwLock->ShareReaderCountLock);//�ͷŵ�������������д��
		}


		//û���κ��߳�ռ�ö�������д��

		ulExclusiveFlag=1;//��ʼ����ǰ�߳�ռ��д���ı��Ϊ�״�ռ��

		if(!TlsSetValue(pRwLock->ulTlsForWriteIndex,(PVOID)(ulExclusiveFlag+1)))
		{
			LeaveCriticalSection(&pRwLock->ExclusiveLock);//ʧ�ܣ��ͷŵ�д��

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