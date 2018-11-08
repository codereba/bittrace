/************************************************************************************* 
This file is a part of CrashRpt library.

Copyright (c) 2003, Michael Carruth
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this 
list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice, 
this list of conditions and the following disclaimer in the documentation 
and/or other materials provided with the distribution.

* Neither the name of the author nor the names of its contributors 
may be used to endorse or promote products derived from this software without 
specific prior written permission.


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED 
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR 
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***************************************************************************************/

#include "stdafx.h"

#include "http_request_send.h"
#include "base64.h"
#include "md5.h"
#include "strconv.h"

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

// Constructor
http_data_sender::http_data_sender()
{
    // Init variables
	m_Request.text_count = 0; 

    m_sBoundary = _T("AaB03x5fs1045fcc7");

    m_sTextPartHeaderFmt = _T("--%s\r\nContent-disposition: form-data; name=\"%s\"\r\n\r\n");
    m_sTextPartFooterFmt = _T("\r\n");   
    m_sFilePartHeaderFmt = _T("--%s\r\nContent-disposition: form-data; name=\"%s\"; filename=\"%s\"\r\nContent-Type: %s\r\nContent-Transfer-Encoding: binary\r\n\r\n");
    m_sFilePartFooterFmt = _T("\r\n");  
}

// Sends HTTP request assyncronously (in a working thread)
BOOL http_data_sender::SendAssync(http_request& Request )
{
    // Copy parameters
    m_Request = Request;

    // Create worker thread
    HANDLE hThread = CreateThread(NULL, 0, WorkerThread, (void*)this, 0, NULL);
    if(hThread==NULL)
        return FALSE;

    return TRUE;
}

BOOL http_data_sender::send_all_data( LPCTSTR url )
{
	BOOL ret = FALSE; 

	if( m_Request.text_count == 0 )
	{
		ret = TRUE; 
		goto _return; 
	}

	m_Request.m_sUrl = url; 

	ret = InternalSend(); 

_return:
	return ret; 
}


// Thread procedure.
DWORD WINAPI http_data_sender::WorkerThread(VOID* pParam)
{
    http_data_sender* pSender = (http_data_sender*)pParam;
    // Delegate further actions to http_data_sender class
    pSender->InternalSend();  

    return 0;
}

// Sends HTTP request and checks responce
BOOL http_data_sender::InternalSend()
{ 
    BOOL bStatus = FALSE;      // Resulting status
    strconv_t strconv;         // String conversion
    HINTERNET hSession = NULL; // Internet session
    HINTERNET hConnect = NULL; // Internet connection
    HINTERNET hRequest = NULL; // Handle to HTTP request
    TCHAR szProtocol[512];     // Protocol
    TCHAR szServer[512];       // Server name
    TCHAR szURI[1024];         // URI
    DWORD dwPort=0;            // Port
    CStdString sHeaders = _T("Content-type: multipart/form-data; boundary="); 
	sHeaders += m_sBoundary;
    LPCTSTR szAccept[2]={_T("*/*"), NULL};
    INTERNET_BUFFERS BufferIn;
    BYTE pBuffer[4096] = {0};
    BOOL bRet = FALSE;
    DWORD dwBuffSize = 0;
    CStdString sMsg;
    LONGLONG lPostSize = 0;  
	INT32 i; 

    // Calculate size of data to send
    bRet = CalcRequestSize(lPostSize);
    if(!bRet)
    {
        goto cleanup;
    }

    // Create Internet session
    hSession = InternetOpen(NULL, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if(hSession==NULL)
    {
        goto cleanup; 
    }

    // Parse application-provided URL
    ParseURL(m_Request.m_sUrl, szProtocol, 512, szServer, 512, dwPort, szURI, 1024);

    // Connect to HTTP server

    hConnect = InternetConnect(
        hSession,     // InternetOpen handle
        szServer,     // Server  name
        (WORD)dwPort, // Default HTTPS port - 443
        NULL,         // User name
        NULL,         //  User password
        INTERNET_SERVICE_HTTP, // Service
        0,            // Flags
        0             // Context
        );
    if(hConnect==NULL)
    {
        goto cleanup; 
    }

    // Check if canceled

    // Add a message to log

    // Configure flags for HttpOpenRequest
    DWORD dwFlags = INTERNET_FLAG_NO_CACHE_WRITE;
    if(dwPort==INTERNET_DEFAULT_HTTPS_PORT)
        dwFlags |= INTERNET_FLAG_SECURE; // Use SSL

    // Open HTTP request
    hRequest = HttpOpenRequest(
        hConnect, 
        _T("POST"), 
        szURI, 
        NULL, 
        NULL, 
        szAccept, 
        dwFlags, 
        0
        );
    if (!hRequest)
    {
        goto cleanup;
    }

    // Fill in buffer
    BufferIn.dwStructSize = sizeof( INTERNET_BUFFERS ); // Must be set or error will occur
    BufferIn.Next = NULL; 
    BufferIn.lpcszHeader = sHeaders;
    BufferIn.dwHeadersLength = sHeaders.GetLength();
    BufferIn.dwHeadersTotal = 0;
    BufferIn.lpvBuffer = NULL;                
    BufferIn.dwBufferLength = 0;
    BufferIn.dwBufferTotal = (DWORD)lPostSize; // This is the only member used other than dwStructSize
    BufferIn.dwOffsetLow = 0;
    BufferIn.dwOffsetHigh = 0;

    m_dwPostSize = (DWORD)lPostSize;
    m_dwUploaded = 0;

    // Add a message to log
    // Send request
    if(!HttpSendRequestEx( hRequest, &BufferIn, NULL, 0, 0))
    {
        goto cleanup;
    }

    // Write text fields
    for( i = 0; ( ULONG )i < m_Request.text_count; i ++ )
    {
        bRet = WriteTextPart(hRequest, 
			m_Request.m_aTextFields[ i ].text_name, 
			m_Request.m_aTextFields[ i ].text_cont ); 
        if( !bRet )
		{
            goto cleanup;
		}
	}

    // Write boundary
    bRet = WriteTrailingBoundary(hRequest);
    if(!bRet)
        goto cleanup;

    // Add a message to log

    // End request
    if(!HttpEndRequest(hRequest, NULL, 0, 0))
    {
        goto cleanup;
    }

    // Add a message to log
	
	// Get HTTP responce code from HTTP headers
	DWORD lHttpStatus = 0;
	DWORD lHttpStatusSize = sizeof(lHttpStatus);
	BOOL bQueryInfo = HttpQueryInfo( hRequest, HTTP_QUERY_STATUS_CODE|HTTP_QUERY_FLAG_NUMBER, 
									&lHttpStatus, &lHttpStatusSize, 0 );
	
	if(bQueryInfo)
	{
		sMsg.Format(_T("Server responce code: %ld"), lHttpStatus);
	}

	// Read HTTP responce
	InternetReadFile(hRequest, pBuffer, 4095, &dwBuffSize);
	pBuffer[dwBuffSize] = 0;
	//sMsg = CStdString((LPCSTR)pBuffer, dwBuffSize);

	log_trace( ( MSG_INFO, "Server response body: %s\n", pBuffer ) ); 

	// If the first byte of HTTP responce is a digit, than assume a legacy way
	// of determining delivery status - the HTTP responce starts with a delivery status code
	if(dwBuffSize>0 && pBuffer[0]>='0' && pBuffer[0]<='9')
	{
		// Get status code from HTTP responce
		if(atoi((LPCSTR)pBuffer)!=200)
		{
			goto cleanup;
		}
	}
	else
	{
		// If the first byte of HTTP responce is not a digit, assume that
		// the delivery status should be read from HTTP header
		if(!bQueryInfo || lHttpStatus!=200)
		{
			goto cleanup;
		}
	}

	// Add a message to log
    bStatus = TRUE;

cleanup:

    if(!bStatus)
    {
        // Add a message to log
    }

    // Clean up
    if(hRequest) 
        InternetCloseHandle(hRequest);

    // Clean up internet handle
    if(hConnect) 
        InternetCloseHandle(hConnect);

    // Clean up internet session
    if(hSession) 
        InternetCloseHandle(hSession);

    // Notify about completion

    return bStatus;
}

#include <string>
BOOL http_data_sender::WriteTextPart(HINTERNET hRequest, CStdString sName, CStdString sData )
{
    BOOL bRet = FALSE;
	std::string tmp_str; 
	LPCSTR data_ansi; 

    /* Write part header */
    CStdString sHeader;
    bRet= FormatTextPartHeader(sName, sHeader);
    if(!bRet)
    {
        return FALSE;
    }

    strconv_t strconv;
    LPCSTR pszHeader = strconv.t2a(sHeader);
    if(pszHeader==NULL)
    {
        return FALSE;
    }

    DWORD dwBytesWritten = 0;
    bRet=InternetWriteFile(hRequest, pszHeader, (DWORD)strlen(pszHeader), &dwBytesWritten);
    if(!bRet)
    {
        return FALSE;
    }
    UploadProgress(dwBytesWritten);

	data_ansi = strconv.t2a( sData.GetData() ); 

    /* Write form data */

	tmp_str = base64_encode( ( const unsigned char* )data_ansi, ( UINT32 )strlen( data_ansi ), 100 ); 

	//sData = tmp_str.c_str(); 

    size_t nDataSize = tmp_str.size(); 
    int pos = 0;    
    DWORD dwBytesRead = 0;
    for(;;)
    {
        dwBytesRead = (DWORD)MIN(1024, nDataSize-pos);    
        if(dwBytesRead==0)
            break; // EOF

		std::string sBuffer = tmp_str.substr( pos, dwBytesRead);

        DWORD dwBytesWritten = 0;
        bRet=InternetWriteFile(hRequest, sBuffer.c_str(), dwBytesRead, &dwBytesWritten);
        if(!bRet)
        {
            return FALSE;
        }
        UploadProgress(dwBytesWritten);

        pos += dwBytesRead;    
    }

    /* Write part footer */

    CStdString sFooter;
    bRet= FormatTextPartFooter(sName, sFooter);
    if(!bRet)
    {
        return FALSE;
    }

    LPCSTR pszFooter = strconv.t2a(sFooter);
    if(pszFooter==NULL)
    {
        return FALSE;
    }

    bRet=InternetWriteFile(hRequest, pszFooter, (DWORD)strlen(pszFooter), &dwBytesWritten);
    if(!bRet)
    {
        return FALSE;
    }
    UploadProgress(dwBytesWritten);

    return TRUE;
}

BOOL http_data_sender::WriteTrailingBoundary(HINTERNET hRequest)
{
    BOOL bRet = FALSE;

    CStdString sText;
    bRet= FormatTrailingBoundary(sText);
    if(!bRet)
        return FALSE;

    strconv_t strconv;
    LPCSTR pszText = strconv.t2a(sText);
    if(pszText==NULL)
        return FALSE;

    DWORD dwBytesWritten = 0;
    bRet=InternetWriteFile(hRequest, pszText, (DWORD)strlen(pszText), &dwBytesWritten);
    if(!bRet)
        return FALSE;

    UploadProgress(dwBytesWritten);

    return TRUE;
}


BOOL http_data_sender::FormatTextPartHeader(CStdString sName, CStdString& sPart)
{
    sPart.Format(m_sTextPartHeaderFmt, m_sBoundary.GetData(), sName.GetData() );    

    return TRUE;
}

BOOL http_data_sender::FormatTextPartFooter(CStdString sName, CStdString& sText)
{
    sText = m_sTextPartFooterFmt;
    return TRUE;
}


BOOL http_data_sender::FormatTrailingBoundary(CStdString& sText)
{
    sText.Format(_T("--%s--\r\n"), m_sBoundary);
    return TRUE;
}

BOOL http_data_sender::CalcRequestSize(LONGLONG& lSize)
{
	INT32 i; 
    lSize = 0;

    // Calculate summary size of all text fields included into request
    //std::map<CStdString, std::string>::iterator it;
    for(i = 0; ( ULONG )i < m_Request.text_count; i++ )
    {
        LONGLONG lPartSize;
        BOOL bCalc = CalcTextPartSize( m_Request.m_aTextFields[ i ].text_name, 
			m_Request.m_aTextFields[ i ].text_cont, 
			lPartSize);        
        if(!bCalc)
            return FALSE;
        lSize += lPartSize;
    }


    CStdString sTrailingBoundary;
    FormatTrailingBoundary(sTrailingBoundary);
    lSize += sTrailingBoundary.GetLength();

    return TRUE;
}

BOOL http_data_sender::CalcTextPartSize(CStdString sName, CStdString sData, LONGLONG& lSize)
{
    lSize = 0;

    CStdString sPartHeader;
    BOOL bFormat = FormatTextPartHeader(sName, sPartHeader);
    if(!bFormat)
        return FALSE;
    lSize += sPartHeader.GetLength();

    lSize += sData.GetLength(); 

    CStdString sPartFooter;
    bFormat = FormatTextPartFooter(sName, sPartFooter);
    if(!bFormat)
        return FALSE;
    lSize += sPartFooter.GetLength();

    return TRUE;
}

// This method updates upload progress status
void http_data_sender::UploadProgress(DWORD dwBytesWritten)
{
    m_dwUploaded += dwBytesWritten;

    float progress = 100*(float)m_dwUploaded/m_dwPostSize;
    //m_Assync->SetProgress((int)progress, false);
}

#if _MSC_VER<1400
#define _TCSCPY_S(strDestination, numberOfElements, strSource) _tcscpy(strDestination, strSource)
#define _TCSNCPY_S(strDest, sizeInBytes, strSource, count) _tcsncpy(strDest, strSource, count)
#define STRCPY_S(strDestination, numberOfElements, strSource) strcpy(strDestination, strSource)
#define _TFOPEN_S(_File, _Filename, _Mode) _File = _tfopen(_Filename, _Mode);
#else
#define _TCSCPY_S(strDestination, numberOfElements, strSource) _tcscpy_s(strDestination, numberOfElements, strSource)
#define _TCSNCPY_S(strDest, sizeInBytes, strSource, count) _tcsncpy_s(strDest, sizeInBytes, strSource, count)
#define STRCPY_S(strDestination, numberOfElements, strSource) strcpy_s(strDestination, numberOfElements, strSource)
#define _TFOPEN_S(_File, _Filename, _Mode) _tfopen_s(&(_File), _Filename, _Mode);
#endif

// Parses URL and splits it into URL, port, protocol and so on. This method's code was taken from 
// http://www.codeproject.com/KB/IP/simplehttpclient.aspx
void http_data_sender::ParseURL(LPCTSTR szURL, LPTSTR szProtocol, UINT cbProtocol, 
                                  LPTSTR szAddress, UINT cbAddress, DWORD &dwPort, LPTSTR szURI, UINT cbURI)
{  
    cbURI;
    cbAddress;
    cbProtocol;

    DWORD dwPosition=0;
    BOOL bFlag=FALSE;

    while(_tcslen(szURL)>0 && dwPosition<_tcslen(szURL) && _tcsncmp((szURL+dwPosition), _T(":"), 1))
        ++dwPosition;

    if(!_tcsncmp((szURL+dwPosition+1), _T("/"), 1)){	// is PROTOCOL
        if(szProtocol){
            _TCSNCPY_S(szProtocol, cbProtocol, szURL, dwPosition);
            szProtocol[dwPosition]=0;
        }
        bFlag=TRUE;
    }else{	// is HOST 
        if(szProtocol){
            _TCSNCPY_S(szProtocol, cbProtocol, _T("http"), 4);
            szProtocol[5]=0;
        }
    }

    DWORD dwStartPosition=0;

    if(bFlag){
        dwStartPosition=dwPosition+=3;				
    }else{
        dwStartPosition=dwPosition=0;
    }

    bFlag=FALSE;
    while(_tcslen(szURL)>0 && dwPosition<_tcslen(szURL) && _tcsncmp(szURL+dwPosition, _T("/"), 1))
        ++dwPosition;

    DWORD dwFind=dwStartPosition;

    for(;dwFind<=dwPosition;dwFind++){
        if(!_tcsncmp((szURL+dwFind), _T(":"), 1)){ // find PORT
            bFlag=TRUE;
            break;
        }
    }

    if(bFlag)
    {
        TCHAR sztmp[256]=_T("");
        _TCSNCPY_S(sztmp, 256, (szURL+dwFind+1), dwPosition-dwFind);
        dwPort=_ttol(sztmp);
        int len = dwFind-dwStartPosition;
        _TCSNCPY_S(szAddress, cbAddress, (szURL+dwStartPosition), len);
        szAddress[len]=0;
    }
    else if(!_tcscmp(szProtocol,_T("https")))
    {
        dwPort=INTERNET_DEFAULT_HTTPS_PORT;
        int len = dwPosition-dwStartPosition;
        _TCSNCPY_S(szAddress, cbAddress, (szURL+dwStartPosition), len);
        szAddress[len]=0;
    }
    else 
    {
        dwPort=INTERNET_DEFAULT_HTTP_PORT;
        int len = dwPosition-dwStartPosition;
        _TCSNCPY_S(szAddress, cbAddress, (szURL+dwStartPosition), len);    
        szAddress[len]=0;
    }

    if(dwPosition<_tcslen(szURL))
    { 
        // find URI
        int len = (int)(_tcslen(szURL)-dwPosition);
        _TCSNCPY_S(szURI, cbURI, (szURL+dwPosition), len);
        szURI[len] = 0;
    }
    else
    {
        szURI[0]=0;
    }

    return;
}

