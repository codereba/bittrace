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

#include "stdafx.h"
#include "safe_install.h"

#define CHECK_VERSION_DELIM( ch ) ( ( _T( '.' ) == ( ch ) ) || ( _T( ',' ) == ( ch ) ) )

CStringFormat::CStringFormat(void)
{
	m_pMsg = NULL;
}

CStringFormat::~CStringFormat(void)
{
	if(m_pMsg)
	{
		delete m_pMsg;
		m_pMsg = NULL;
	}
}

TCHAR * CStringFormat::format(CONST TCHAR * fmt,...)
{
	size_t nLen = 0;
	va_list args;

	va_start(args, fmt);
	nLen = _vscwprintf(fmt, args) + 1;

	if(m_pMsg)
	{
		delete m_pMsg;
		m_pMsg = NULL;
	}

	m_pMsg = new TCHAR[nLen + 1];
	_vsnwprintf(m_pMsg, nLen, fmt, args); 

	return m_pMsg;
}

CFileVersionInfo::CFileVersionInfo()
{
	Reset();
}

CFileVersionInfo::~CFileVersionInfo()
{

}

BOOL CFileVersionInfo::GetTranslationId(LPVOID lpData, UINT unBlockSize, WORD wLangId, DWORD &dwId, BOOL bPrimaryEnough/*= FALSE*/)
{
	LPWORD lpwData = 0;

	for (lpwData = (LPWORD)lpData; (LPBYTE)lpwData < ((LPBYTE)lpData)+unBlockSize; lpwData+=sizeof(DWORD))
	{
		if (*lpwData == wLangId)
		{
			dwId = *((DWORD*)lpwData);
			return TRUE;
		}
	}

	if (!bPrimaryEnough)
		return FALSE;

	for (lpwData = (LPWORD)lpData; (LPBYTE)lpwData < ((LPBYTE)lpData) + unBlockSize; lpwData+=sizeof(DWORD))
	{
		if (((*lpwData)&0x00FF) == (wLangId&0x00FF))
		{
			dwId = *((DWORD*)lpwData);
			return TRUE;
		}
	}

	return FALSE;
}

tstring CFileVersionInfo::QueryValue(LPVOID lpData, tstring & strSubBlock, CONST TCHAR * lpcSubBlockKeyName, LPVOID lpInfo, UINT unInfoLen)
{
	tstring strTmp;

	strTmp = strSubBlock; 
	strTmp += lpcSubBlockKeyName;
	strTmp = (VerQueryValue(lpData, ( LPWSTR )strTmp.c_str(), &lpInfo, &unInfoLen)) ? (TCHAR *)lpInfo : _T("");

	return strTmp;
}

BOOL CFileVersionInfo::Create(HMODULE hModule /*= NULL*/)
{
	tstring	strFilePathName;

	strFilePathName.resize(_MAX_PATH);
	GetModuleFileName(hModule, (TCHAR *)strFilePathName.c_str(), ( ULONG )strFilePathName.size());

	return Create(strFilePathName.c_str());
}

BOOL CFileVersionInfo::Create(CONST TCHAR * lpcFilePathName)
{
	UINT	unInfoLen = 0;
	DWORD	dwHandle = 0;
	DWORD	dwFileVersionInfoSize = 0;
	DWORD	dwLangCode = 0;
	LPVOID	lpData = NULL;
	LPVOID	lpInfo = NULL;
	tstring strSubBlock;
	tstring strTmp;
	CStringFormat fmt;

	Reset();

	m_strFilePathName = lpcFilePathName;
	dwFileVersionInfoSize = GetFileVersionInfoSize(m_strFilePathName.c_str(), &dwHandle);
	if( !dwFileVersionInfoSize )
	{
		return FALSE;
	}

	lpData = (LPVOID)new BYTE[dwFileVersionInfoSize];
	if (!lpData)
		return FALSE;

	try
	{
		if (!GetFileVersionInfo(m_strFilePathName.c_str(), dwHandle, dwFileVersionInfoSize, lpData))
			throw FALSE;

		if (VerQueryValue(lpData, _T("\\"), &lpInfo, &unInfoLen))
		{
			_ASSERT(unInfoLen == sizeof(VS_FIXEDFILEINFO));
			if (unInfoLen == sizeof(VS_FIXEDFILEINFO))
			{
				::memcpy(&m_FileInfo, lpInfo, unInfoLen);
			}
		}
		VerQueryValue(lpData, _T("\\VarFileInfo\\Translation"), &lpInfo, &unInfoLen);
		if (!GetTranslationId(lpInfo, unInfoLen, GetUserDefaultLangID(), dwLangCode, FALSE))
		{
			if (!GetTranslationId(lpInfo, unInfoLen, GetUserDefaultLangID(), dwLangCode, TRUE))
			{
				if (!GetTranslationId(lpInfo, unInfoLen, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), dwLangCode, TRUE))
				{
					if (!GetTranslationId(lpInfo, unInfoLen, MAKELANGID(LANG_ENGLISH, SUBLANG_NEUTRAL), dwLangCode, TRUE))
						dwLangCode = *((DWORD*)lpInfo);
				}
			}
		}

		strSubBlock = fmt.format(_T("\\StringFileInfo\\%04X%04X\\"), dwLangCode&0x0000FFFF, (dwLangCode&0xFFFF0000)>>16);

		m_strCompanyName =		QueryValue(lpData, strSubBlock, FILE_VERSION_COMPANY_NAME, lpInfo, unInfoLen);
		m_strFileDescription =	QueryValue(lpData, strSubBlock, FILE_VERSION_FILE_DESCRIPTION, lpInfo, unInfoLen);
		m_strFileVersion =		QueryValue(lpData, strSubBlock, FILE_VERSION_FILE_VERSION, lpInfo, unInfoLen);
		m_strInternalName =		QueryValue(lpData, strSubBlock, FILE_VERSION_INTERNAL_NAME, lpInfo, unInfoLen);
		m_strLegalCopyright =	QueryValue(lpData, strSubBlock, FILE_VERSION_LEGAL_COPYRIGHT, lpInfo, unInfoLen);
		m_strOriginalFileName =	QueryValue(lpData, strSubBlock, FILE_VERSION_ORIGINAL_FILENAME, lpInfo, unInfoLen);
		m_strProductName =		QueryValue(lpData, strSubBlock, FILE_VERSION_PRODUCT_NAME, lpInfo, unInfoLen);
		m_strProductVersion =	QueryValue(lpData, strSubBlock, FILE_VERSION_PRODUCT_VERSION, lpInfo, unInfoLen);
		m_strComments =			QueryValue(lpData, strSubBlock, FILE_VERSION_COMMENTS, lpInfo, unInfoLen);
		m_strLegalTrademarks =	QueryValue(lpData, strSubBlock, FILE_VERSION_LEGAL_TRADEMARKS, lpInfo, unInfoLen);
		m_strPrivateBuild =		QueryValue(lpData, strSubBlock, FILE_VERSION_PRIVATE_BUILD, lpInfo, unInfoLen);
		m_strSpecialBuild =		QueryValue(lpData, strSubBlock, FILE_VERSION_SPECIAL_BUILD, lpInfo, unInfoLen);

		delete[] lpData;
	}
	catch (BOOL)
	{
		delete[] lpData;
		return FALSE;
	}

	return TRUE;
}

WORD CFileVersionInfo::GetFileVersion(int iIndex) CONST
{
	WORD wRc = 0;
	switch(iIndex)
	{
	case 0:
		{
			wRc = (WORD)(m_FileInfo.dwFileVersionLS & 0x0000FFFF);
		}
		break;
	case 1:
		{
			wRc = (WORD)((m_FileInfo.dwFileVersionLS & 0xFFFF0000) >> 16);
		}
		break;
	case 2:
		{
			wRc = (WORD)(m_FileInfo.dwFileVersionMS & 0x0000FFFF);
		}
		break;
	case 3:
		{
			wRc = (WORD)((m_FileInfo.dwFileVersionMS & 0xFFFF0000) >> 16);
		}
		break;
	default:
		{
			wRc = 0;
		}
		break;
	}

	return wRc;
}

WORD CFileVersionInfo::GetProductVersion(int iIndex) CONST
{
	WORD wRc = 0;
	switch(iIndex)
	{
	case 0:
		{
			wRc = (WORD)(m_FileInfo.dwProductVersionLS & 0x0000FFFF);
		}
		break;
	case 1:
		{
			wRc = (WORD)((m_FileInfo.dwProductVersionLS & 0xFFFF0000) >> 16);
		}
		break;
	case 2:
		{
			wRc = (WORD)(m_FileInfo.dwProductVersionMS & 0x0000FFFF);
		}
		break;
	case 3:
		{
			wRc = (WORD)((m_FileInfo.dwProductVersionMS & 0xFFFF0000) >> 16);
		}
		break;
	default:
		{
			wRc = 0;
		}
		break;
	}

	return wRc;
}

DWORD CFileVersionInfo::GetFileFlagsMask() CONST
{
	return m_FileInfo.dwFileFlagsMask;
}

DWORD CFileVersionInfo::GetFileFlags() CONST
{
	return m_FileInfo.dwFileFlags;
}

DWORD CFileVersionInfo::GetFileOs() CONST
{
	return m_FileInfo.dwFileOS;
}

DWORD CFileVersionInfo::GetFileType() CONST
{
	return m_FileInfo.dwFileType;
}

DWORD CFileVersionInfo::GetFileSubtype() CONST
{
	return m_FileInfo.dwFileSubtype;
}

VOID CFileVersionInfo::GetFileDate(FILETIME * ft) CONST
{
	ft->dwLowDateTime = m_FileInfo.dwFileDateLS;
	ft->dwHighDateTime = m_FileInfo.dwFileDateMS;
}

TCHAR * CFileVersionInfo::GetCompanyName() CONST
{
	return (TCHAR *)m_strCompanyName.c_str();
}

TCHAR * CFileVersionInfo::GetFileDescription() CONST
{
	return (TCHAR *)m_strFileDescription.c_str();
}

TCHAR * CFileVersionInfo::GetFileVersion() CONST
{
	return (TCHAR *)m_strFileVersion.c_str();
}

TCHAR * CFileVersionInfo::GetInternalName() CONST
{
	return (TCHAR *)m_strInternalName.c_str();
}

TCHAR * CFileVersionInfo::GetLegalCopyright() CONST
{
	return (TCHAR *)m_strLegalCopyright.c_str();
}

TCHAR * CFileVersionInfo::GetOriginalFileName() CONST
{
	return (TCHAR *)m_strOriginalFileName.c_str();
}

TCHAR * CFileVersionInfo::GetProductName() CONST
{
	return (TCHAR *)m_strProductName.c_str();
}

TCHAR * CFileVersionInfo::GetProductVersion() CONST
{
	return (TCHAR *)m_strProductVersion.c_str();
}

TCHAR * CFileVersionInfo::GetComments() CONST
{
	return (TCHAR *)m_strComments.c_str();
}

TCHAR * CFileVersionInfo::GetLegalTrademarks() CONST
{
	return (TCHAR *)m_strLegalTrademarks.c_str();
}

TCHAR * CFileVersionInfo::GetPrivateBuild() CONST
{
	return (TCHAR *)m_strPrivateBuild.c_str();
}

TCHAR * CFileVersionInfo::GetSpecialBuild() CONST
{
	return (TCHAR *)m_strSpecialBuild.c_str();
}

TCHAR * CFileVersionInfo::GetFilePathName()	CONST
{
	return (TCHAR *)m_strFilePathName.c_str();
}

void CFileVersionInfo::Reset()
{
	m_strFilePathName = _T("");
	ZeroMemory(&m_FileInfo, sizeof(VS_FIXEDFILEINFO));
	m_strCompanyName = _T("");
	m_strFileDescription = _T("");
	m_strFileVersion = _T("");
	m_strInternalName = _T("");
	m_strLegalCopyright = _T("");
	m_strOriginalFileName = _T("");
	m_strProductName = _T("");
	m_strProductVersion = _T("");
	m_strComments = _T("");
	m_strLegalTrademarks = _T("");
	m_strPrivateBuild = _T("");
	m_strSpecialBuild = _T("");
}

VOID CFileVersionInfo::ShowFileVersionInfo()
{
	tstring strMsg;
	CStringFormat fmt;

	strMsg = fmt.format(_T("File [%s] information below:\n")\
		_T("%s = %s\n")\
		_T("%s = %s\n")\
		_T("%s = %s\n")\
		_T("%s = %s\n")\
		_T("%s = %s\n")\
		_T("%s = %s\n")\
		_T("%s = %s\n")\
		_T("%s = %s\n")\
		_T("%s = %s\n")\
		_T("%s = %s\n")\
		_T("%s = %s\n"),
		GetFilePathName(),
		_T("m_strCompanyName"), GetCompanyName(),
		_T("m_strFileDescription"), GetFileDescription(),
		_T("m_strFileVersion"), GetFileVersion(),
		_T("m_strInternalName"), GetInternalName(),
		_T("m_strLegalCopyright"), GetLegalCopyright(),
		_T("m_strOriginalFileName"), GetOriginalFileName(),
		_T("m_strProductName"), GetProductName(),
		_T("m_strProductVersion"), GetProductVersion(),
		_T("m_strComments"), GetComments(),
		_T("m_strLegalTrademarks"), GetLegalTrademarks(),
		_T("m_strPrivateBuild"), GetPrivateBuild(),
		_T("m_strSpecialBuild"), GetSpecialBuild());

	_tprintf(_T("%s"), strMsg.c_str());
}

LRESULT CFileVersionInfo::CompareFileVersion( LPCWSTR lpcFileVersion, INT32 *result )
{
	LRESULT ret = ERROR_SUCCESS; 
#define G_DOT_CNT_MAX 5	
	INT iRc = 1; 
	size_t nPos = 0;
	size_t nIndex = 0;
	size_t ver_part_count; 
	ULONGLONG ullMyVersion = ((ULONGLONG)m_FileInfo.dwFileVersionMS << (sizeof(DWORD) * 8)) | (ULONGLONG)m_FileInfo.dwFileVersionLS;
	ULONGLONG ullFileVersion = 0;
	tstring strFileVerPart[G_DOT_CNT_MAX + 1];

	ASSERT( result != NULL ); 

	*result = -1; 

	if(NULL == lpcFileVersion)
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _CompareFileVersion_END;
	}

	while((nIndex <= G_DOT_CNT_MAX) && (L'\0' != *(lpcFileVersion + nPos)))
	{
		if( TRUE == CHECK_VERSION_DELIM( *(lpcFileVersion + nPos ) ) )
		{
			nPos++;
			nIndex++;
			continue;
		}

		strFileVerPart[nIndex] += *(lpcFileVersion + nPos++);
	};

	if( nIndex > G_DOT_CNT_MAX )
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _CompareFileVersion_END; 
	}

	ver_part_count = nIndex; 

	nIndex = 0;
	while(nIndex <= ver_part_count)
	{
		ullFileVersion = (ullFileVersion << (sizeof(WORD) * 8)) + _ttoi(strFileVerPart[nIndex++].c_str());
	}

	iRc = ( INT32 )( ullMyVersion - ullFileVersion ); 

	*result = iRc; 

_CompareFileVersion_END:
	return ret;
}