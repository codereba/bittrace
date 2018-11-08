/********************************************************************
*
* Copyright (C) 1999-2000 Sven Wiegand
* Copyright (C) 2000-2001 ToolsCenter
* 
* This file is free software; you can redistribute it and/or
* modify, but leave the headers intact and do not remove any 
* copyrights from the source.
*
* If you have further questions, suggestions or bug fixes, visit 
* our homepage
*
*    http://www.ToolsCenter.org
*
********************************************************************/

//////////////////////////////////////////////////////////////////////////
/// @file FileVersionInfo.h
/// @brief modify Sven Wiegand's CFileVersionInfo to no mfc progam version
/// @author LostSpeed
/// @date 2012/4/21
/// @history
/// 2012/4/21 ÐÞ¸ÄCString to std::wstring, add CompareFileVersion
//////////////////////////////////////////////////////////////////////////

/*
 *
 * Copyright 2010 JiJie Shi(weixin:AIChangeLife)
 *
 */

#ifndef __SAFE_INSTALL_H__
#define __SAFE_INSTALL_H__

INLINE LRESULT get_file_version( LPCTSTR file_name )
{
	LRESULT ret = ERROR_SUCCESS; 

	return ret; 
}

#ifndef _UNICODE
#error "only support unicode"
#endif //_UNICODE

#ifndef tstring
#ifdef _UNICODE
#define tstring std::wstring
#else
#define tstring std::string
#endif //_UNICODE
#endif //tstring

#define FILE_VERSION_COMMENTS			_T("Comments")
#define FILE_VERSION_INTERNAL_NAME		_T("InternalName")
#define FILE_VERSION_PRODUCT_NAME		_T("ProductName")
#define FILE_VERSION_COMPANY_NAME		_T("CompanyName")
#define FILE_VERSION_LEGAL_COPYRIGHT	_T("LegalCopyright")
#define FILE_VERSION_PRODUCT_VERSION	_T("ProductVersion")
#define FILE_VERSION_FILE_DESCRIPTION	_T("FileDescription")
#define FILE_VERSION_LEGAL_TRADEMARKS	_T("LegalTrademarks")
#define FILE_VERSION_PRIVATE_BUILD		_T("PrivateBuild")
#define FILE_VERSION_FILE_VERSION		_T("FileVersion")
#define FILE_VERSION_ORIGINAL_FILENAME	_T("OriginalFilename")
#define FILE_VERSION_SPECIAL_BUILD		_T("SpecialBuild")

class CFileVersionInfo
{
	// construction/destruction
public:
	CFileVersionInfo();
	virtual ~CFileVersionInfo();

	// operations
public:
	BOOL	Create(HMODULE hModule = NULL);
	BOOL	Create(CONST TCHAR * lpcFilePathName);
	VOID	ShowFileVersionInfo();

	// attribute operations
public:
	TCHAR * GetFilePathName()	CONST;
	WORD	GetFileVersion(INT iIndex)	CONST;
	WORD	GetProductVersion(INT iIndex)	CONST;
	DWORD	GetFileFlagsMask()	CONST;
	DWORD	GetFileFlags()	CONST;
	DWORD	GetFileOs()	CONST;
	DWORD	GetFileType()	CONST;
	DWORD	GetFileSubtype()	CONST;
	VOID	GetFileDate(FILETIME * ft)	CONST;

	TCHAR * GetCompanyName()	CONST;
	TCHAR * GetFileDescription()	CONST;
	TCHAR * GetFileVersion()	CONST;
	TCHAR * GetInternalName()	CONST;
	TCHAR * GetLegalCopyright()	CONST;
	TCHAR * GetOriginalFileName()	CONST;
	TCHAR * GetProductName()	CONST;
	TCHAR * GetProductVersion()	CONST;
	TCHAR * GetComments()	CONST;
	TCHAR * GetLegalTrademarks()	CONST;
	TCHAR * GetPrivateBuild()	CONST;
	TCHAR * GetSpecialBuild()	CONST;

	LRESULT CompareFileVersion( LPCWSTR lpcFileVersion, INT32 *result ); 

protected:
	virtual	VOID	Reset();
	BOOL	GetTranslationId(LPVOID lpData, UINT uBlockSize, WORD wLangId, DWORD &dwId, BOOL bPrimaryEnough = FALSE);
	tstring QueryValue(LPVOID lpData, tstring & strSubBlock, CONST TCHAR * lpcSubBlockKeyName, LPVOID lpInfo, UINT unInfoLen);

private:
	tstring m_strFilePathName;
	VS_FIXEDFILEINFO m_FileInfo;

	tstring m_strCompanyName;
	tstring m_strFileDescription;
	tstring m_strFileVersion;
	tstring m_strInternalName;
	tstring m_strLegalCopyright;
	tstring m_strOriginalFileName;
	tstring m_strProductName;
	tstring m_strProductVersion;
	tstring m_strComments;
	tstring m_strLegalTrademarks;
	tstring m_strPrivateBuild;
	tstring m_strSpecialBuild;
};


class CStringFormat
{
public:
	CStringFormat(void);
	virtual ~CStringFormat(void);

public:
	TCHAR * format(CONST TCHAR * fmt,...);

private:
	TCHAR * m_pMsg;
};

#endif //__SAFE_INSTALL_H__