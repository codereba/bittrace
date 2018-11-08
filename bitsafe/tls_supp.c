/*
 *
 * Copyright 2010 JiJie Shi
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
 
 #include "StdAfx.h"
#include "tls_supp.h"

// The DLL code

#include <windows.h>

static DWORD dwTlsIndex; // address of shared memory

// DllMain() is the entry-point function for this DLL. 

BOOL WINAPI DllMain(HINSTANCE hinstDLL,  // DLL module handle
					DWORD fdwReason,                     // reason called
					LPVOID lpvReserved)                  // reserved
{ 
	LPVOID lpvData; 
	BOOL fIgnore; 

	switch (fdwReason) 
	{ 
		// The DLL is loading due to process 
		// initialization or a call to LoadLibrary. 

	case DLL_PROCESS_ATTACH: 

		// Allocate a TLS index.

		if ((dwTlsIndex = TlsAlloc()) == TLS_OUT_OF_INDEXES) 
			return FALSE; 

		// No break: Initialize the index for first thread.

		// The attached process creates a new thread. 

	case DLL_THREAD_ATTACH: 

		// Initialize the TLS index for this thread.

		lpvData = (LPVOID) LocalAlloc(LPTR, 256); 
		if (lpvData != NULL) 
			fIgnore = TlsSetValue(dwTlsIndex, lpvData); 

		break; 

		// The thread of the attached process terminates.

	case DLL_THREAD_DETACH: 

		// Release the allocated memory for this thread.

		lpvData = TlsGetValue(dwTlsIndex); 
		if (lpvData != NULL) 
			LocalFree((HLOCAL) lpvData); 

		break; 

		// DLL unload due to process termination or FreeLibrary. 

	case DLL_PROCESS_DETACH: 

		// Release the allocated memory for this thread.

		lpvData = TlsGetValue(dwTlsIndex); 
		if (lpvData != NULL) 
			LocalFree((HLOCAL) lpvData); 

		// Release the TLS index.

		TlsFree(dwTlsIndex); 
		break; 

	default: 
		break; 
	} 

	return TRUE; 
	UNREFERENCED_PARAMETER(hinstDLL); 
	UNREFERENCED_PARAMETER(lpvReserved); 
}

#ifdef __cplusplus    // If used by C++ code, 
extern "C" {          // we need to export the C interface
#endif

	__declspec(dllexport)
		BOOL WINAPI StoreData(DWORD dw)
	{
		LPVOID lpvData; 
		DWORD * pData;  // The stored memory pointer 


		// Retrieve a data pointer for the current thread
		lpvData = TlsGetValue(dwTlsIndex); 

		// If NULL, allocate memory for the TLS slot for this thread
		if (lpvData == NULL)
		{
			lpvData = (LPVOID) LocalAlloc(LPTR, 256); 
			if (lpvData == NULL) 
				return FALSE;
			if (!TlsSetValue(dwTlsIndex, lpvData))
				return FALSE;
		}

		pData = (DWORD *) lpvData; // Cast to my data type.
		// In this example, it is only a pointer to a DWORD
		// but it can be a structure pointer to contain more complicated data.

		(*pData) = dw;
		return TRUE;
	}

	__declspec(dllexport)
		BOOL WINAPI GetData(DWORD *pdw)
	{
		LPVOID lpvData; 
		DWORD * pData;  // The stored memory pointer 

		lpvData = TlsGetValue(dwTlsIndex); 
		if (lpvData == NULL)
			return FALSE;

		pData = (DWORD *) lpvData;
		(*pdw) = (*pData);
		return TRUE;
	}
#ifdef __cplusplus
}
#endif

// The process code

#include <windows.h> 
#include <stdio.h> 

#define THREADCOUNT 4 
#define DLL_NAME    TEXT("testdll")

VOID ErrorExit(LPSTR); 

extern "C" BOOL WINAPI StoreData(DWORD dw);
extern "C" BOOL WINAPI GetData(DWORD *pdw);

DWORD WINAPI ThreadFunc(VOID) 
{   
	int i;

	if(!StoreData(GetCurrentThreadId()))
		ErrorExit("StoreData error");

	for(i=0; i<5; i++)
	{
		DWORD dwOut;
		if(!GetData(&dwOut))
			ErrorExit("GetData error");
		if( dwOut != GetCurrentThreadId())
			printf("thread %d: data is incorrect (%d)\n", GetCurrentThreadId(), dwOut);
		else printf("thread %d: data is correct\n", GetCurrentThreadId());
		Sleep(0);
	}
	return 0; 
} 

int main(VOID) 
{ 
	DWORD IDThread; 
	HANDLE hThread[THREADCOUNT]; 
	int i; 

	// Load the DLL

	LoadLibrary(DLL_NAME);

	// Create multiple threads. 

	for (i = 0; i < THREADCOUNT; i++) 
	{ 
		hThread[i] = CreateThread(NULL, // default security attributes 
			0,                           // use default stack size 
			(LPTHREAD_START_ROUTINE) ThreadFunc, // thread function 
			NULL,                    // no thread function argument 
			0,                       // use default creation flags 
			&IDThread);              // returns thread identifier 

		// Check the return value for success. 
		if (hThread[i] == NULL) 
			ErrorExit("CreateThread error\n"); 
	} 

	WaitForMultipleObjects(THREADCOUNT, hThread, TRUE, INFINITE); 

	return 0; 
} 

VOID ErrorExit (LPSTR lpszMessage) 
{ 
	fprintf(stderr, "%s\n", lpszMessage); 
	ExitProcess(0); 
}

//#include <windows.h> 
//#include <stdio.h> 
//
//#define THREADCOUNT 4 
//DWORD dwTlsIndex; 
//
//VOID ErrorExit(LPSTR); 
//
//VOID CommonFunc(VOID) 
//{ 
//	LPVOID lpvData; 
//
//	// Retrieve a data pointer for the current thread. 
//
//	lpvData = TlsGetValue(dwTlsIndex); 
//	if ((lpvData == 0) && (GetLastError() != ERROR_SUCCESS)) 
//		ErrorExit("TlsGetValue error"); 
//
//	// Use the data stored for the current thread. 
//
//	printf("common: thread %d: lpvData=%lx\n", 
//		GetCurrentThreadId(), lpvData); 
//
//	Sleep(5000); 
//} 
//
//DWORD WINAPI ThreadFunc(VOID) 
//{ 
//	LPVOID lpvData; 
//
//	// Initialize the TLS index for this thread. 
//
//	lpvData = (LPVOID) LocalAlloc(LPTR, 256); 
//	if (! TlsSetValue(dwTlsIndex, lpvData)) 
//		ErrorExit("TlsSetValue error"); 
//
//	printf("thread %d: lpvData=%lx\n", GetCurrentThreadId(), lpvData); 
//
//	CommonFunc(); 
//
//	// Release the dynamic memory before the thread returns. 
//
//	lpvData = TlsGetValue(dwTlsIndex); 
//	if (lpvData != 0) 
//		LocalFree((HLOCAL) lpvData); 
//
//	return 0; 
//} 
//
//int main(VOID) 
//{ 
//	DWORD IDThread; 
//	HANDLE hThread[THREADCOUNT]; 
//	int i; 
//
//	// Allocate a TLS index. 
//
//	if ((dwTlsIndex = TlsAlloc()) == TLS_OUT_OF_INDEXES) 
//		ErrorExit("TlsAlloc failed"); 
//
//	// Create multiple threads. 
//
//	for (i = 0; i < THREADCOUNT; i++) 
//	{ 
//		hThread[i] = CreateThread(NULL, // default security attributes 
//			0,                           // use default stack size 
//			(LPTHREAD_START_ROUTINE) ThreadFunc, // thread function 
//			NULL,                    // no thread function argument 
//			0,                       // use default creation flags 
//			&IDThread);              // returns thread identifier 
//
//		// Check the return value for success. 
//		if (hThread[i] == NULL) 
//			ErrorExit("CreateThread error\n"); 
//	} 
//
//	for (i = 0; i < THREADCOUNT; i++) 
//		WaitForSingleObject(hThread[i], INFINITE); 
//
//	TlsFree(dwTlsIndex);
//
//	return 0; 
//} 
//
//VOID ErrorExit (LPSTR lpszMessage) 
//{ 
//	fprintf(stderr, "%s\n", lpszMessage); 
//	ExitProcess(0); 
//}

