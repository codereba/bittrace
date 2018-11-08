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
 
 #include "stdafx.h"
#include "desktop_ui.h"

LRESULT enum_desktop_titles() 
{
	// bytes to allocate for icon text and LVITEM struct
#define ALLOC_SIZE 200

	LRESULT		ret = ERROR_SUCCESS; 
	HWND		hWnd;				// handle to desktop window
	DWORD		dwPID;				// explorer process ID (associated with desktop)
	HANDLE		hProcess;			// handle to explorer process (associated with desktop)
	LPVOID		pData;				// pointer to LVITEM struct in explorer address space
	LPVOID		pString;			// pointer to icon text in explorer address space
	WCHAR		szText[ALLOC_SIZE];	// char array of icon text in application address space
	WCHAR*		pszMessageBox;		// string to display
	SIZE_T		BytesRead;			// for ReadProcessMemory
	SIZE_T		BytesWritten;		// for WriteProcessMemory
	BOOL		fResult;			// for ReadProcessMemory/WriteProcessMemory
	LVITEM		lvi;				// LVITEM struct
	int			i;					// counter for enumeration
	int			nItemCount;			// icon item count

	// get desktop window handle
	if(((hWnd = FindWindowExW(NULL, NULL, L"Progman", NULL)) == NULL) ||
		((hWnd = FindWindowExW(hWnd, NULL, L"SHELLDLL_DefView", NULL)) == NULL) ||
		((hWnd = FindWindowExW(hWnd, NULL, L"SysListView32", NULL)) == NULL))
	{
		MessageBoxW( NULL, L"Could not get desktop window.", NULL, 0 );
		goto Exit;
	}

	// get item count on desktop
	nItemCount = ::SendMessage(hWnd, LVM_GETITEMCOUNT, (WPARAM)0, (LPARAM)0);

	// allocate memory for output string
	pszMessageBox = ( WCHAR*) malloc(ALLOC_SIZE * nItemCount);
	wsprintf(pszMessageBox, L"%d items:\n\n", nItemCount);

	// get desktop window process ID (explorer.exe)
	GetWindowThreadProcessId(hWnd, &dwPID);

	// open process to get explorer process handle
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID);

	// allocate memory in explorer's address space
	// allocate space for LVITEM struct
	pData = VirtualAllocEx(hProcess, NULL, ALLOC_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	// allocate space for string (i.e. "Network Neighborhood")
	pString = VirtualAllocEx(hProcess, NULL, ALLOC_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	// init LV_ITEM struct
	ZeroMemory(&lvi, sizeof(LVITEM));
	lvi.iItem = 0;
	lvi.iSubItem = 0;
	lvi.cchTextMax = 500;
	lvi.pszText = ( WCHAR* )pString;	// use alloc'd string space

	// write the contents of lvi into explorer's address space
	fResult = WriteProcessMemory(hProcess, pData, &lvi, sizeof(LVITEM), &BytesWritten);

	// enum all icons
	for(i = 0; i < nItemCount; i++)
	{
		// get item's name
		::SendMessage(hWnd, LVM_GETITEMTEXT, (WPARAM)i, (LPARAM)pData);

		// read back lvi struct (for debugging purposes only)
		fResult = ReadProcessMemory(hProcess, pData, szText, ALLOC_SIZE, &BytesRead);
		// read text of queried item
		fResult = ReadProcessMemory(hProcess, pString, szText, ALLOC_SIZE, &BytesRead);

		// append string to output string
		wcscat(pszMessageBox, szText);
		wcscat(pszMessageBox, L"\n");
	}	

	// output message box with all icon texts
	MessageBox( NULL, pszMessageBox, NULL, 0);

	// free alloc'd memory for message box
	free(pszMessageBox);

	// free alloc'd memory for string and LVITEM structs
	if(!VirtualFreeEx(hProcess, pString, ALLOC_SIZE, MEM_DECOMMIT))
	{
		MessageBoxW( NULL, L"VirtualFreeEx failed.", NULL, 0 );
		goto Exit;
	}
	if(!VirtualFreeEx(hProcess, pData, ALLOC_SIZE, MEM_DECOMMIT))
	{

		MessageBoxW( NULL, L"VirtualFreeEx failed.", NULL, 0 );
		goto Exit;
	}

	// close process handle
	CloseHandle(hProcess);

Exit:
	return ret;
}


