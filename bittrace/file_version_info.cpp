#include "stdafx.h"
#include "file_version_info.h"

FileVersionInfo::FileVersionInfo() : m_pVersionData( 0 ), 
                                     m_dwVersionSize( 0 ),
                                     m_pLang( 0 )
{}


FileVersionInfo::FileVersionInfo( const FileVersionInfo& fviVersionInfo_i )
{
    
    this->FileVersionInfo::FileVersionInfo();
    *this = fviVersionInfo_i;
}


FileVersionInfo& FileVersionInfo::operator = ( const FileVersionInfo& fviVersionInfo_i )
{
    
    if( this != &fviVersionInfo_i )
    {
        
        if( m_pVersionData )
        {
            DELETE_PTR_ARR( m_pVersionData );
        }

        m_pVersionData = new VersionData[fviVersionInfo_i.m_dwVersionSize];
        if( m_pVersionData )
        {
            memcpy( m_pVersionData, 
                    fviVersionInfo_i.m_pVersionData, 
                    fviVersionInfo_i.m_dwVersionSize ); 
        }
    }

    return *this;
}


FileVersionInfo::~FileVersionInfo()
{
    if( m_pVersionData )
    {
        DELETE_PTR_ARR( m_pVersionData );
    }
}


bool FileVersionInfo::GetInfo()
{
    
    

    
    LPTSTR lptszFileName = m_csFilePath.GetBuffer( 0 );

    
    DWORD dwHandle = 0;

    
    const DWORD dwSize = GetFileVersionInfoSize( lptszFileName, &dwHandle );

    
    if( !dwSize )
    {
        return false;
    }

    
    if( m_pVersionData )
    {
        DELETE_PTR_ARR( m_pVersionData );
    }

    
    m_pVersionData = new VersionData[ dwSize ];
    if( !m_pVersionData )
    {
        return false;
    }
    
    if( !GetFileVersionInfo( lptszFileName, dwHandle, dwSize, m_pVersionData ))
    {
        DELETE_PTR_ARR( m_pVersionData );
        return false;
    }

    
    if( !GetLanguageList() )
    {
        return false;
    }


    return true;
}

bool FileVersionInfo::GetProductVersion( CString& csVersion_o ) const
{
    return ExtractStringInfo( _T( "ProductVersion" ), csVersion_o );
}


bool FileVersionInfo::GetFileVersion( CString& csVersion_o ) const
{
    return ExtractStringInfo( _T( "FileVersion" ), csVersion_o );
}


bool FileVersionInfo::GetFileDescription( CString& csFileDesc_o ) const
{
    return ExtractStringInfo( _T( "FileDescription" ), csFileDesc_o );
}


bool FileVersionInfo::GetCompanyName( CString& csCompanyName_o ) const
{
    return ExtractStringInfo( _T( "CompanyName" ), csCompanyName_o );
}

bool FileVersionInfo::GetProductName( CString& csProductName_o ) const
{
    return ExtractStringInfo( _T( "ProductName" ), csProductName_o );
}


bool FileVersionInfo::GetComments( CString& csComments_o ) const
{
    return ExtractStringInfo( _T( "Comments" ), csComments_o );
}

bool FileVersionInfo::GetInternalName( CString& csInternalName_o ) const
{
    return ExtractStringInfo( _T( "InternalName" ), csInternalName_o );
}

bool FileVersionInfo::GetLegalCopyright( CString& csLegalCopyright_o ) const
{
    return ExtractStringInfo( _T( "LegalCopyright" ), csLegalCopyright_o );
}

bool FileVersionInfo::GetLegalTrademarks( CString& csLegalTrademarks_o ) const
{
    return ExtractStringInfo( _T( "LegalTrademarks" ), csLegalTrademarks_o );
}

bool FileVersionInfo::GetPrivateBuild( CString& csPrivateBuild_o ) const
{
    return ExtractStringInfo( _T( "PrivateBuild" ), csPrivateBuild_o );
}

bool FileVersionInfo::GetOriginalFileName( CString& csOriginalFileName_o ) const
{
    return ExtractStringInfo( _T( "OrignalFileName" ), csOriginalFileName_o );
}

bool FileVersionInfo::GetSpecialBuild( CString& csSpecialBuild_o ) const
{
    return ExtractStringInfo( _T( "SpecialBuild" ), csSpecialBuild_o );
}

ULONGLONG FileVersionInfo::GetFileSize() const
{
    return 0; 
}






bool FileVersionInfo::GetLanguageList()
{
    
    if( !m_pVersionData )
    {
        return false;
    }

    
    CString csBlock = _T( "\\VarFileInfo\\Translation" );

    
    UINT uLength = 0;
    const bool bResult =  ( VerQueryValue( m_pVersionData, 
                                           csBlock.GetBuffer( 0 ), 
                                           reinterpret_cast<void**>( &m_pLang ), 
                                           &uLength ) && uLength );

    csBlock.ReleaseBuffer();
    return bResult;
}

bool FileVersionInfo::ExtractInfo( LPCTSTR lpctszBlockString_i, LPVOID* ppvData_io ) const
{
    if( !lpctszBlockString_i || !ppvData_io || !m_pVersionData || !m_pLang )
    {
        return false;
    }

    CString csBlock;
    csBlock.Format( _T( "\\StringFileInfo\\%04X%04X\\%s" ), 
                        m_pLang[0].wLanguage, 
                        m_pLang[0].wCodePage,
                        lpctszBlockString_i );

    UINT uVersionDataLength = 0;
    const BOOL bResult = VerQueryValue( m_pVersionData, 
                                        csBlock.GetBuffer( 0 ),
                                        ppvData_io,
                                        &uVersionDataLength );
    csBlock.ReleaseBuffer();
    return uVersionDataLength != 0 && bResult != 0;
}

bool FileVersionInfo::ExtractStringInfo( LPCTSTR lpctszBlockString_o, CString& csInfo_o ) const
{
    LPTSTR lptszInfo = 0;
    
    
    if( !ExtractInfo( lpctszBlockString_o, ( LPVOID* )&lptszInfo ))
    {
        return false;
    }

    
    csInfo_o = lptszInfo;
    return true;
}