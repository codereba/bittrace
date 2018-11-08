#include "stdafx.h"
#include <windows.h>
#include "rcu.h"
#include <vector>

#include <assert.h>
#define ASSERT( _x_ ) assert( _x_ ) 


//typedef INT32 ( CALLBACK *free_rcu_item )( PLIST_ENTRY item ); 
//
//typedef struct _rcu_manage
//{
//    //CRITICAL_SECTION reader_lock; 
//    //CRITICAL_SECTION inner_lock; 
//    SLIST_HEADER rcu_list; 
//    free_rcu_item f_free; 
//} rcu_manage, *prcu_manage; 
//
//INT32 WINAPI init_rcu( rcu_manage *rcu, free_rcu_item func )
//{
//    INT32 ret = 0; 
//
//    ASSERT( rcu ! =NULL ); 
//    ASSERT( func != NULL ); 
//
//    InitializeCriticalSection( &rcu->reader_lock ); 
//    InitializeCriticalSection( &rcu->inner_lock ); 
//
//    InitializeListHead( &rcu->rcu_list ); 
//    
//    rcu->f_free = func; 
//
//_return:
//    return ret; 
//}
//
//INT32 WINAPI lock_rcu_read( rcu_manage *rcu )
//{
//    INT32 ret = 0; 
//
//    ASSERT( rcu != NULL ); 
//
//    EnterCriticalSection( &rcu->reader_lock ); 
//
//_return:
//    return ret; 
//}
//
//INT32 WINAPI unlock_rcu_read( rcu_manage *rcu )
//{
//    INT32 ret = 0; 
//
//    ASSERT( rcu != NULL ); 
//
//    LeaveCriticalSection( &rcu->reader_lock ); 
//
//_return:
//    return ret; 
//}
//
//INT32 WINAPI rcu_remove( rcu_manage *rcu, PLIST_ENTRY entry )
//{
//    INT32 ret = 0; 
//
//    ASSERT( rcu != NULL ); 
//    ASSERT( entry != NULL ); 
//
//    EnterCriticalSection( &rcu->inner_lock ); 
//
//    InsertTailList( &rcu->rcu_list, entry ); 
//    
//    LeaveCriticalSection( &rcu->inner_lock ); 
//
//_return:
//    return ret; 
//}
//
//INT32 WINAPI rcu_post_read( rcu_manage *rcu )
//{
//    INT32 ret = 0; 
//    
//    ASSERT( rcu != NULL ); 
//
//    EnterCriticalSection( &rcu->reader_lock ); 
//    EnterCriticalSection( &rcu->inner_lock ); 
//
//    {
//        PLIST_ENTRY entry; 
//
//        for( ; ; )
//        {
//            if( TRUE == IsListEmpty( &rcu->rcu_list ) )
//            {
//                break; 
//            }
//
//            entry = RemoveEntryList( &rcu->rcu_list ); 
//
//            rcu->f_free( entry ); 
//            ret += 1; 
//        }
//    }
//    
//    LeaveCriticalSection( &rcu->inner_lock ); 
//    LeaveCriticalSection( &rcu->reader_lock ); 
//
//_return:
//    return ret; 
//}
//
//INT32 WINAPI uninit_rcu( rcu_manage *rcu )
//{
//    INT32 ret = 0; 
//
//    ASSERT( rcu != NULL ); 
//
//    rcu_post_read( rcu ); 
//
//    DeleteCriticalSection( &rcu->reader_lock ); 
//    DeleteCriticalSection( &rcu->inner_lock ); 
//
//_return:
//    return ret; 
//}
//
//typedef struct _data_element
//{
//    LIST_ENTRY rcu_entry; 
//    ULONG data; 
//} data_element, *pdata_element; 
//
//typedef struct _test_rcu_list
//{
//    rcu_manage rcu; 
//    std::vector datas; 
//} test_rcu_list, *ptest_rcu_list; 
//
//test_rcu_list test_datas; 

//#define DEBUG_READ_WRITE_LOCK_PERF 1
#define DEBUG_READ_WRITE_LOCK 1
#define WAIT_WRITING_END_TIME 50 
#define WAIT_READING_END_TIME 5

#define WAIT_READING_SLEEP_TIME 0
#define WAIT_WRITING_SLEEP_TIME 80


#if ( _WIN32_WINNT >= 0x0600 ) && ( DEBUG_READ_WRITE_LOCK == 0 )
class read_write_lock
{
public: 
    read_write_lock() 
    {
		InitializeSRWLock( &lock ); 
    }

    ~read_write_lock()
    {
		 
    }

public: 
    LRESULT lock_read()
    {
        LRESULT ret = ERROR_SUCCESS; 

		AcquireSRWLockShared( &lock ); 

//_return:
        return ret; 
    }

    //ReleaseSRWLockExclusive
    LRESULT unlock_read()
    {
        LRESULT ret = ERROR_SUCCESS; 

		ReleaseSRWLockShared( &lock ); 

//_return:
        return ret; 
    }

    LRESULT lock_write()
    {
        LRESULT ret = ERROR_SUCCESS; 

		AcquireSRWLockExclusive( &lock ); 
//_return:
        return ret; 
    }

    LRESULT unlock_write()
    {
		LRESULT ret = ERROR_SUCCESS; 

		ReleaseSRWLockExclusive( &lock ); 
//_return:
        return ret; 
    }

protected: 
	SRWLOCK lock; 
}; 
#else
class read_write_lock
{
public: 
    read_write_lock() : 
      reader_count( 0 ), 
      initialized( FALSE )
    {
        InitializeCriticalSection( &inner_lock ); 
        no_reader = CreateEventW( NULL, TRUE, FALSE, NULL );  
        no_writer = CreateEventW( NULL, TRUE, TRUE, NULL ); 

        if( no_reader != NULL 
            && no_writer != NULL )
        {
            initialized = TRUE; 
        }
    }

    ~read_write_lock()
    {
        ULONG wait_ret; 
        if( no_reader != NULL )
        {
            wait_ret = WaitForSingleObject( no_reader, INFINITE ); 

            if( wait_ret != WAIT_OBJECT_0 )
            {
                OutputDebugStringW( L"wait all reading end error\n" ); 
                ASSERT( FALSE ); 
            }
            else
            {
                ASSERT( reader_count == 0 ); 
            }

            CloseHandle( no_reader ); 
        }

        if( no_writer != NULL )
        {
            wait_ret = WaitForSingleObject( no_writer, INFINITE ); 

            if( wait_ret != WAIT_OBJECT_0 )
            {
                OutputDebugStringW( L"wait writing end error\n" ); 
                ASSERT( FALSE ); 
            }

            CloseHandle( no_writer ); 
        }

		if( TRUE == initialized )
		{
			DeleteCriticalSection( &inner_lock ); 
		}
    }

public: 
    LRESULT lock_read()
    {
        LRESULT ret = ERROR_SUCCESS; 
        ULONG wait_ret; 
		INT32 _ret; 
		LONG old_reader_count; 
#if DEBUG_READ_WRITE_LOCK_PERF
		LONG try_count = 0; 
#endif //DEBUG_READ_WRITE_LOCK_PERF
		for( ; ; )
		{
			do
			{
				Sleep( WAIT_WRITING_SLEEP_TIME ); 
				; 
#if DEBUG_READ_WRITE_LOCK_PERF
				wprintf( L"try lock reading, try count:%u\n", try_count ); 
#endif //DEBUG_READ_WRITE_LOCK_PERF
				EnterCriticalSection( &inner_lock ); 

				wait_ret = WaitForSingleObject( no_writer, WAIT_WRITING_END_TIME ); 

				if( wait_ret != WAIT_OBJECT_0 )
				{
					if( wait_ret != WAIT_TIMEOUT )
					{
						OutputDebugStringW( L"wait writing end error\n" ); 
						ASSERT( FALSE ); 
						ret = GetLastError(); 
						LeaveCriticalSection( &inner_lock ); 
						goto _return; 
					}
					else
					{
						LeaveCriticalSection( &inner_lock ); 
						break; 
					}
				}

				old_reader_count = InterlockedIncrement( &reader_count ); 

				if( old_reader_count == 1 )
				{
					_ret = ResetEvent( no_reader ); 

					if( FALSE == _ret )
					{
						ret = GetLastError(); 
						OutputDebugStringW( L"reset no reading tip event error\n" ); 
						LeaveCriticalSection( &inner_lock ); 
						goto _return; 
					}
					else
					{
						LeaveCriticalSection( &inner_lock ); 
						goto _return; 
					}
				}
				else
				{
#ifdef _DEBUG
					wait_ret = WaitForSingleObject( no_reader, 0 ); 
					ASSERT( wait_ret == WAIT_TIMEOUT ); 
#endif //_DEBUG
					LeaveCriticalSection( &inner_lock ); 
					goto _return; 
				}
			}while( FALSE ); 

#if DEBUG_READ_WRITE_LOCK_PERF
			try_count += 1; 
#endif //DEBUG_READ_WRITE_LOCK_PERF

		}
_return:
        return ret; 
    }

    //ReleaseSRWLockExclusive
    LRESULT unlock_read()
    {
        LRESULT ret = ERROR_SUCCESS; 
        INT32 _ret; 
        LONG old_reader_count; 

        EnterCriticalSection( &inner_lock ); 

        old_reader_count = InterlockedDecrement( &reader_count ); 

        if( old_reader_count == 0 )
        {
            _ret = SetEvent( no_reader ); 

            if( FALSE == _ret )
            {
                ret = GetLastError(); 
                OutputDebugStringW( L"set no reading tip event error\n" ); 
                goto _return; 
            }
        }

_return:
        LeaveCriticalSection( &inner_lock ); 
        return ret; 
    }

    LRESULT lock_write()
    {
        LRESULT ret = ERROR_SUCCESS; 
        ULONG wait_ret; 
        INT32 _ret; 
		INT32 old_priority; 
		BOOLEAN priority_boosted = FALSE; 

#if DEBUG_READ_WRITE_LOCK_PERF
		LONG try_count = 0; 
#endif //DEBUG_READ_WRITE_LOCK_PERF

		old_priority = GetThreadPriority( GetCurrentThread() ); 
		_ret = SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_HIGHEST ); 
		if( TRUE == _ret )
		{
			priority_boosted = TRUE; 
		}

		for( ; ; )
		{
			do
			{
				Sleep( WAIT_READING_SLEEP_TIME ); 

#if DEBUG_READ_WRITE_LOCK_PERF
				wprintf( L"try lock writing, try count:%u\n", try_count ); 
#endif //DEBUG_READ_WRITE_LOCK_PERF

				EnterCriticalSection( &inner_lock ); 

				wait_ret = WaitForSingleObject( no_writer, WAIT_READING_END_TIME ); 

				if( wait_ret != WAIT_OBJECT_0 )
				{
					if( wait_ret != WAIT_TIMEOUT )
					{
						OutputDebugStringW( L"wait writing end error\n" ); 
						ASSERT( FALSE ); 
						ret = GetLastError(); 

						LeaveCriticalSection( &inner_lock ); 
						goto _return; 
					}
					else
					{
						LeaveCriticalSection( &inner_lock ); 
						break; 
					}
				}
				else
				{

					wait_ret = WaitForSingleObject( no_reader, WAIT_READING_END_TIME ); 

					if( wait_ret != WAIT_OBJECT_0 )
					{
						if( wait_ret != WAIT_TIMEOUT )
						{
							OutputDebugStringW( L"wait reading end error\n" ); 
							ASSERT( FALSE ); 
							ret = GetLastError(); 

							LeaveCriticalSection( &inner_lock ); 
							goto _return; 
						}
						else
						{
							LeaveCriticalSection( &inner_lock ); 
							break; 
						}
					}
					else
					{
						ASSERT( 0 == reader_count ); 

						_ret = ResetEvent( no_writer ); 

						if( FALSE == _ret )
						{
							ret = GetLastError(); 
							OutputDebugStringW( L"reset no writing tip event error\n" ); 

							LeaveCriticalSection( &inner_lock ); 
							goto _return; 
						}
						else
						{
							LeaveCriticalSection( &inner_lock ); 
							goto _return; 
						}
					}
				}
			}while( FALSE ); 

#if DEBUG_READ_WRITE_LOCK_PERF
			try_count += 1; 
#endif //DEBUG_READ_WRITE_LOCK_PERF

		}

_return:

		if( TRUE == priority_boosted )
		{
			_ret = SetThreadPriority( GetCurrentThread(), old_priority ); 
			ASSERT( TRUE == _ret ); 
		}

		return ret; 
    }

    LRESULT unlock_write()
    {
        LRESULT ret = ERROR_SUCCESS; 
        INT32 _ret; 

        //EnterCriticalSection( &inner_lock ); 

        _ret = SetEvent( no_writer ); 

        if( FALSE == _ret )
        {
            ret = GetLastError(); 
            OutputDebugStringW( L"reset no reading tip event error\n" ); 
            goto _return; 
        }

_return:
        //LeaveCriticalSection( &inner_lock ); 
        return ret; 
    }

public:
		BOOLEAN initialized; 
protected: 
    CRITICAL_SECTION inner_lock; 
    HANDLE no_reader; 
    HANDLE no_writer; 

    LONG reader_count; 
}; 
#endif //( _WIN32_WINNT >= 0x0600 ) && ( DEBUG_READ_WRITE_LOCK == 0 )

CRITICAL_SECTION printf_lock; 
CRITICAL_SECTION sprintf_lock; 

#define sync_printf( _fmt_, ... ) EnterCriticalSection( &printf_lock ); wprintf_s( _fmt_, __VA_ARGS__ ); LeaveCriticalSection( &printf_lock ); 
#define sync_swprintf( _buf_, _len_, _fmt_, ... ) _ret = swprintf_s( _buf_, _len_, _fmt_, __VA_ARGS__ ); 


#define PRINTING_TEXT_LEN 256
WCHAR printing_text[ PRINTING_TEXT_LEN ] = { 0 }; 

BOOLEAN stop_work = FALSE; 
ULONG CALLBACK read_worker( PVOID param )
{
    ULONG ret = 0; 
    read_write_lock *lock; 

    ASSERT( param != NULL ); 

    lock = ( read_write_lock* )param; 

    for( ; ; )
    {
        if( TRUE == stop_work )
        {
            break; 
        }

        lock->lock_read(); 

        sync_printf( printing_text ); 

        lock->unlock_read(); 
        
        Sleep( 0 ); 
    }

//_return:
    return ret; 
}

#include <stdlib.h>
#include <time.h>

ULONG CALLBACK write_worker( PVOID param )
{
    ULONG ret = 0; 
    INT32 _ret; 
    read_write_lock *lock; 
    INT32 write_count; 
    SYSTEMTIME now_time; 
	INT32 string_length; 

    ASSERT( param != NULL ); 

    srand( ( INT32 )time( NULL ) );    
    write_count = rand(); 

    lock = ( read_write_lock* )param; 

    for( ; ; )
    {
        if( TRUE == stop_work )
        {
            break; 
        }

        lock->lock_write(); 

        GetSystemTime( &now_time ); 

		EnterCriticalSection( &sprintf_lock ); 
        sync_swprintf( printing_text, ARRAYSIZE( printing_text ), L"%04u/%02u/%02u %02u:%02u:%02u.%03u ", 
            now_time.wYear, 
            now_time.wMonth, 
            now_time.wDay, 
            now_time.wHour, 
            now_time.wMinute, 
            now_time.wSecond, 
            now_time.wMilliseconds ); 

		if( _ret < 0 )
		{
			string_length = 0; 
		}
		else
		{
			string_length = _ret; 
		}

		sync_swprintf( printing_text + string_length, ARRAYSIZE( printing_text ) - string_length, L"No.:%d|%d|%d|%d|%d|%d\n", 
            write_count, 
            write_count, 
            write_count, 
            write_count, 
            write_count, 
            write_count ); 

		LeaveCriticalSection( &sprintf_lock ); 

		//if( _ret < 0 )
		//{
		//	string_length = 0; 
		//}
		//else
		//{
		//	string_length += _ret; 
		//}

        lock->unlock_write(); 

        Sleep( 0 ); 

        write_count += 1; 
    }

//_return:
    return ret; 
}

#if 0
#include <stdio.h>
#include <assert.h>
#include <windows.h>

#define LOCK_READ 0
#define LOCK_WRITE 1

class rwlock{
    public:
        rwlock();
        ~rwlock();

    public:
        void lock(int direct);
        void unlock(int direct);

        void lock_exclusive(void);
        void unlock_exclusive(void);

        void wrlock() { lock(LOCK_WRITE); }
        void wrunlock() { unlock(LOCK_WRITE); }

        void rdlock() { lock(LOCK_READ); }
        void rdunlock() { unlock(LOCK_READ); }

    private:
        volatile LONG count;
        volatile LONG direct;
        HANDLE finish_event;
        CRITICAL_SECTION start_lock;
};

rwlock::rwlock()
{
    count = 0;
    direct = 0;
    finish_event = CreateEvent(NULL, FALSE, FALSE, NULL);
    InitializeCriticalSection(&start_lock);
}

rwlock::~rwlock()
{
    assert(count == 0);
    CloseHandle(finish_event);
    DeleteCriticalSection(&start_lock);
}

void rwlock::lock(int _direct)
{
    EnterCriticalSection(&start_lock);
    while (count > 0 &&
            direct != _direct) {
        WaitForSingleObject(finish_event, INFINITE);
    }
    direct = _direct;
    InterlockedIncrement(&count);
    LeaveCriticalSection(&start_lock);
}

void rwlock::unlock(int _direct)
{
    assert(count > 0);
    assert(direct == _direct);
    InterlockedDecrement(&count);
    SetEvent(finish_event);
}

void rwlock::lock_exclusive(void)
{
    EnterCriticalSection(&start_lock);
    while (count > 0 &&
            direct != _direct) {
        WaitForSingleObject(finish_event, INFINITE);
    }
    InterlockedIncrement(&count);
}

void unlock_exclusive(void)
{
    InterlockedDecrement(&count);
    LeaveCriticalSection(&start_lock);
}

static DWORD CALLBACK ReadFunc(LPVOID lpVoid)
{
    rwlock * plock = (rwlock *)lpVoid;
    while ( 1 ) {
        plock->rdlock();
        printf("Start Read: threaid %x\n", GetCurrentThreadId());
        printf("Call Read: threaid %x\n", GetCurrentThreadId());
        printf("End Read: threaid %x\n", GetCurrentThreadId());
        plock->rdunlock();
    }
    return 0;
}

int main(int argc, char * argv[])
{
    DWORD id;
    rwlock lock;

    for (int i = 0; i < 10; i ++) {
        HANDLE handle = CreateThread(NULL, 0, ReadFunc, &lock, 0, &id);
        CloseHandle(handle);
    }

    while ( 1 ) {
        lock.wrlock();
        printf("Start Write: threaid %x\n", GetCurrentThreadId());
        for (int j = 0; j < 10; j++) {
            printf("Call Write: threaid %x\n", GetCurrentThreadId());
        }
        printf("End Write: threaid %x\n", GetCurrentThreadId());
        lock.wrunlock();
    }

    return 0;
}
#endif //0

#include <conio.h>

read_write_lock test_lock; 
INT32 _tmain(int argc, _TCHAR* argv[])
{
    INT32 ret = 0; 
    INT32 i; 

	InitializeCriticalSection( &printf_lock ); 
	InitializeCriticalSection( &sprintf_lock ); 

#if ( _WIN32_WINNT < 0x0600 ) || ( DEBUG_READ_WRITE_LOCK == 1 )
    if( test_lock.initialized == FALSE )
    {
        ret = -1; 
        goto _return; 
    }
#endif //( _WIN32_WINNT < 0x0600 ) || ( DEBUG_READ_WRITE_LOCK == 0 )

#define TEST_READ_WORKER_COUNT 8 
#define TEST_WRITE_WORKER_COUNT 2 

    for( i = 0; i < TEST_READ_WORKER_COUNT; i ++ )
    { 
        CreateThread( NULL, 0, read_worker, &test_lock, 0, NULL ); 
    }

    for( i = 0; i < TEST_WRITE_WORKER_COUNT; i ++ )
    { 
        CreateThread( NULL, 0, write_worker, &test_lock, 0, NULL ); 
    }

	for( ; ; )
	{
		//getch(); 
		//stop_work = TRUE; 
		Sleep( 1000 ); 
	}

_return:
	
	DeleteCriticalSection( &sprintf_lock ); 
	DeleteCriticalSection( &printf_lock ); 

    return ret; 
}