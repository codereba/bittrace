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
 
 #pragma once

#pragma once

#pragma warning(disable: 4267)  // conversion from 'size_t' to 'type', possible loss of data
#pragma warning(disable: 4996)
//#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS
#define _WTL_NO_CSTRING

#include <atlbase.h>
#include <atlstr.h>
#include <string>
#include <vector>

enum E_WinVersion 
{
	WINUNKNOWN = 0,
	WIN32S,
	WIN95,
	WIN98,
	WINME,
	WINNT,
	WIN2K,
	WINXP,
	WIN2003,
	WINVISTA,
	WIN7,
	WIN2005,
	WIN2008,
	WIN2008R2,
};
#define KB_VER_BASE (10*1000000)

DWORD TransSysCode(LPCTSTR lpszSysName);
BOOL TransHKey( LPCTSTR lpKey, HKEY& hKey );

// �ļ�����
BOOL GetFileVersion( LPCTSTR szPath, LARGE_INTEGER &lgVersion);
BOOL IsFileExist(LPCTSTR pszFile);
BOOL MyMoveFileA(LPCSTR lpszFileFrom, LPCSTR lpszFileto);
BOOL IsFileUsing(LPCTSTR szFilename);
BOOL GetFileFullInfo(LPCTSTR lpFile, LPWIN32_FIND_DATA lpfd );
BOOL file_get_contents( LPCTSTR lpszFilename, CStringA &strA );
BOOL file_get_contents(LPCTSTR lpszFilename, CString &strBuffer);
BOOL file_put_contents(LPCTSTR lpszFilename, BYTE *pBuffer, INT nLen);
BOOL file_put_contents(LPCTSTR lpszFilename, LPCTSTR lpszBuffer);
INT64 file_get_size(LPCTSTR lpszFilename);
INT64 GetFolderSize(LPCTSTR szFolder);

CString &CompletePathWithModulePath(CString &strPath, LPCTSTR lpszFilename);
BOOL CreateDirectoryNested( LPCTSTR  lpszDir );
BOOL CreateDirEx (const char *lpFullPath, DWORD dwFileAttributes = INVALID_FILE_ATTRIBUTES) ;
void CreateDirs (const char *lpFileName);

LARGE_INTEGER ParseVersion( LPCTSTR szVer );
void GenVersionStr( LARGE_INTEGER ll, CString &str );
BOOL VersionInRange(LARGE_INTEGER lVersion, LPCTSTR lpszLowerVersion, LPCTSTR lpszUpperVersion );
BOOL EvalCompareResult( INT nRet, LPCTSTR sOpt, BOOL &bResult );	// left-right, opt
BOOL CompareVersion( const LARGE_INTEGER &v1, const LARGE_INTEGER &v2, LPCTSTR sOpt, BOOL &bResult );
BOOL CompareString( LPCTSTR lpLeft, LPCTSTR lpRight, LPCTSTR sOpt, BOOL& bResult );
void GetDateString( CString &strDate );
BOOL ParseDateString(LPCTSTR szDate, INT &year, INT &month, INT &day);

LRESULT GetDirectXVerionViaFileVersions( DWORD* pdwDirectXVersionMajor, DWORD* pdwDirectXVersionMinor, TCHAR* pcDirectXVersionLetter );
LARGE_INTEGER MakeInt64( WORD a, WORD b, WORD c, WORD d );
int CompareLargeInts( LARGE_INTEGER ullParam1, LARGE_INTEGER ullParam2 );

// Reg Operate 
BOOL IsWindows64();
BOOL RegKeyExists( HKEY hRoot, LPCTSTR lpSubKey );
BOOL RegKeyExistsEx(HKEY hRoot, LPCTSTR lpSubKey, BOOL bNoDirect);
BOOL RegValueExists( HKEY hRoot, LPCTSTR lpSubKey, LPCTSTR lpValueName );
BOOL RegValueExistsEx( HKEY hRoot, LPCTSTR lpSubKey, LPCTSTR lpValueName, BOOL bNoDirect );
BOOL RegHasSubValue(HKEY hKey, LPCTSTR lpszSubKey);
BOOL RegHasSubKey(HKEY hKey, LPCTSTR lpszSubKey);

BOOL ReadRegString(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, CString &str);
BOOL ReadRegStringEx(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, CString &str, BOOL bNoDirect);
BOOL ReadRegDWord(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, DWORD &dw);
BOOL ReadRegMString(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, CSimpleArray<CString> &ms);

BOOL WriteRegString(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, LPCTSTR strValue);
BOOL WriteRegExString( HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, LPCTSTR strValue );
BOOL WriteRegDWord(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, DWORD dw);
BOOL WriteRegMString(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, const CSimpleArray<CString> &ms);

INT ParseKBString(LPCTSTR sz);
void FormatKBString(INT nKB, CString &str);
void FormatKBWebUrl( CString &strUrl, INT nKB );
INT GetRealKBID(INT nKB);
BOOL IsNumber( LPTSTR lpszName );
BOOL Split( LPCTSTR lpString, CSimpleArray<CString>& arr, TCHAR delimiter=_T(',') );

BOOL ExecuteFile(LPCTSTR lpszFilename, LPCTSTR lpszInstallParam, DWORD &dwExitCode, BOOL &bHijacked);
INT GetWinVer( );
BOOL IsCurrentUserAdmin();

BOOL GetWinVerInfo(INT &nWindowsVersion, INT &nSP );
LCID SetLocale2GBK(LCID id=0);

// 
const char * SkipXmlHeader( const char * szXml );
const char *SkipXmlHeaderForWin2K(const char *szXml);


// Special Folder
BOOL GetCLSIDFilename( LPCTSTR lpszClsid, CString &strFilename );
void ExpandFilePath( CString &strPath );
BOOL ParseSpecialFolder( CString &strSpecialFolder );
BOOL ParseSpecialFolder(LPCTSTR lpSpecialFolder, CString& strPath); //�����ַ���ȡ����Ӧ��·��

// Evaluate Expression 
typedef bool (*Function_EvaluateCallback)(const char *funcname, const std::vector<std::string> &params, double &evalResult);
bool DefaultEvaluateFunction(const char *funcname, const std::vector<std::string> &params, double &evalResult);
BOOL EvaluateCondition(LPCTSTR lpszExpression);
BOOL EvaluateCondition(LPCTSTR lpszExpression, Function_EvaluateCallback pfnCallback);

BOOL InstallCryptService();
BOOL IsCryptServiceExists(BOOL &bExists);
BOOL IsCryptServiceCorrupt();
BOOL EnableUpdateRelatedService();
BOOL FixRegByResource(LPCTSTR lpName, LPCTSTR lpType);
BOOL IsWindowInSafeMode();

struct T_WindowsInfo
{
	OSVERSIONINFOEX osvi;
	INT nWindowsVersion;
	INT nSP;

	BOOL Init();

protected:
	BOOL GetWindowsVersionInfo( OSVERSIONINFOEX &osvi );
	BOOL FixVersionInfo( OSVERSIONINFOEX &osvi );
	int _ParseWinVer( OSVERSIONINFOEX &osvi );
};
LANGID GetLangID();
void ShutDownComputer(BOOL toReboot=FALSE);

class CWowFsRedirectDisableHelper
{
public:
	CWowFsRedirectDisableHelper(BOOL bSet);
	~CWowFsRedirectDisableHelper();
	BOOL Init();

	BOOL m_bSet;
	PVOID m_pOldValue;

protected:
	typedef int (__stdcall * Wow64DisableWow64FsRedirection)(LPVOID *); 
	typedef int (__stdcall * Wow64RevertWow64FsRedirection)(LPVOID); 

	static char bIs64; 
	static BOOL bInited;
	static Wow64DisableWow64FsRedirection f_Wow64DisableWow64FsRedirection;
	static Wow64RevertWow64FsRedirection f_Wow64RevertWow64FsRedirection;  
};