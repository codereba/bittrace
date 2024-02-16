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
 
 #ifdef _DEBUG_MEM_LEAKS
#undef _DEBUG_MEM_LEAKS
#endif //_DEBUG_MEM_LEAKS

#include "common_func.h"
#include "proc_shared_data.h"
#include "vista.h"

CProcessSharedData::CProcessSharedData(void)
{
	m_hFileMap = NULL;
	m_lpFileData = NULL;
	m_dwFileSize = 0;
	m_hMutex = NULL;
}

CProcessSharedData::~CProcessSharedData(void)
{
	Close();
}

BOOL CProcessSharedData::Create(LPCTSTR lpName, DWORD dwSize)
{
	ASSERT (m_hFileMap == NULL);
	ASSERT (m_lpFileData == NULL);

	BOOL bResult = FALSE;
	
	CDacl dacl;
	dacl.AddAllowedAce( Sids::World(), FILE_ALL_ACCESS );
	dacl.AddAllowedAce( Sids::Admins(), FILE_ALL_ACCESS  );

	CSecurityDesc sd;
	sd.SetDacl( dacl );
	SetSecurityDescriptorToLowIntegrity(sd);

	CSecurityAttributes sa;
	sa.Set( sd );

	m_hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE,
		&sa,
		PAGE_READWRITE,
		0,
		dwSize,
		lpName
		);
	if ( m_hFileMap != NULL )
	{
		m_lpFileData = MapViewOfFile(m_hFileMap,
			FILE_MAP_READ | FILE_MAP_WRITE,
			0,
			0,
			0
			);
		if ( m_lpFileData != NULL )
		{
			TCHAR szObjName[MAX_PATH];

			CDacl dacl;
			dacl.AddAllowedAce( Sids::World(), MUTEX_ALL_ACCESS );
			dacl.AddAllowedAce( Sids::Admins(), MUTEX_ALL_ACCESS  );

			CSecurityDesc sd;
			sd.SetDacl( dacl );
			SetSecurityDescriptorToLowIntegrity(sd);

			CSecurityAttributes sa;
			sa.Set( sd );

			_sntprintf(szObjName, MAX_PATH, _T("%s_psd_mutex"), lpName);
			szObjName[MAX_PATH - 1] = 0;
			m_hMutex = CreateMutex(&sa, FALSE, szObjName);
			if ( m_hMutex != NULL )
			{
				bResult = TRUE;
			}

			m_dwFileSize = dwSize;
		}
		else
		{
//			DebugPrintf(_T("CProcessSharedData::Create MapViewOfFileʧ��0x%08X"), GetLastError());
		}
	}
	else
	{
//		DebugPrintf(_T("CProcessSharedData::Create CreateFileMappingʧ��0x%08X"), GetLastError());
	}

	if ( !bResult )
	{
		Close();
		return FALSE;
	}

	return TRUE;
}

BOOL CProcessSharedData::Open(LPCTSTR lpName, DWORD dwSize)
{
	ASSERT(m_hFileMap == NULL);
	ASSERT(m_lpFileData == NULL);
	BOOL bResult = FALSE;

	m_hFileMap = OpenFileMapping(FILE_MAP_WRITE | FILE_MAP_READ,
		FALSE,
		lpName
		);
	if ( m_hFileMap != NULL )
	{
		m_lpFileData = MapViewOfFile(m_hFileMap,
			FILE_MAP_READ | FILE_MAP_WRITE,
			0,
			0,
			0
			);
		if ( m_lpFileData != NULL )
		{
			TCHAR szObjName[MAX_PATH];

			_sntprintf(szObjName, MAX_PATH, _T("%s_psd_mutex"), lpName);
			szObjName[MAX_PATH - 1] = 0;

			m_hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, szObjName);
			if ( m_hMutex != NULL )
			{
				bResult = TRUE;
			}
			else
			{
		//		DebugPrintf(_T("CProcessSharedData::Open: OpenMutexʧ��0x%08X"), GetLastError());
			}

			m_dwFileSize = dwSize;
		}
		else
		{
//			DebugPrintf(_T("CProcessSharedData::Open: MapViewOfFileʧ��0x%08X"), GetLastError());
		}
	}
	else
	{
//		DebugPrintf(_T("CProcessSharedData::Open: OpenFileMappingʧ��0x%08X"), GetLastError());
	}

	if ( !bResult )
	{
		Close();
		return FALSE;
	}

	return bResult;
}

void CProcessSharedData::Close()
{
	if ( m_lpFileData != NULL )
	{
		UnmapViewOfFile(m_lpFileData);
		m_lpFileData = NULL;
	}

	if ( m_hFileMap != NULL )
	{
		CloseHandle(m_hFileMap);
		m_hFileMap = NULL;
	}

	if ( m_hMutex != NULL )
	{
		CloseHandle(m_hMutex);
		m_hMutex = NULL;
	}
}