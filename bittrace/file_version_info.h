// FileVersionInfo.h: interface for the FileVersionInfo class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __FILE_VERSION_INFO_H__
#define __FILE_VERSION_INFO_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#define DELETE_PTR( ptr ) delete ptr;\
                          ptr = 0;

#define DELETE_PTR_ARR( ptrArr ) delete [] ptrArr;\
                                 ptrArr = 0;

typedef unsigned char VersionData;

#include <atlstr.h>

class FileVersionInfo  
{
    public:

	    FileVersionInfo();
        FileVersionInfo( const FileVersionInfo& );
        FileVersionInfo& operator = ( const FileVersionInfo& );
	    virtual ~FileVersionInfo();

        void SetFilePath( LPCTSTR lpctszFilePath_i ){ m_csFilePath = lpctszFilePath_i; }
        const CString& GetFilePath() const { return m_csFilePath; }

		bool IsVersionRetrieved() const
		{
			return m_pVersionData != 0;
		}

        bool GetInfo();
        bool GetProductVersion( CString& csVersion_o ) const;
        bool GetFileVersion( CString& csVersion_o ) const;
        bool GetFileDescription( CString& csFileDesc_o ) const;
        bool GetCompanyName( CString& csCompanyName_o ) const;
        bool GetProductName( CString& csProductName_o ) const;
        bool GetComments( CString& csComments_o ) const;
        bool GetInternalName( CString& csInternalName_o ) const;
        bool GetLegalCopyright( CString& csLegalCopyright_o ) const;
        bool GetLegalTrademarks( CString& csLegalTrademarks_o ) const;
        bool GetPrivateBuild( CString& csPrivateBuild_o ) const;
        bool GetOriginalFileName( CString& csOriginalFileName_o ) const;
        bool GetSpecialBuild( CString& csSpecialBuild_o ) const;

        ULONGLONG GetFileSize() const;

    private:

        bool GetLanguageList();
        bool ExtractInfo( LPCTSTR lpctszBlockString_i, 
                          LPVOID* ppvData_io ) const;
        bool ExtractStringInfo( LPCTSTR lpctszBlockString_o, 
                                CString& csInfo_o ) const;

    private:

        typedef struct LANG_CP_t
        {
            WORD wLanguage; 
            WORD wCodePage;
        }*PLANG_CP_t;

        CString m_csFilePath;
        VersionData* m_pVersionData;
        DWORD m_dwVersionSize;
        PLANG_CP_t m_pLang;

};

#endif //__FILE_VERSION_INFO_H__
