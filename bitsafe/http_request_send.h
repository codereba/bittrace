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

// File: httpsend.h
// Description: Sends error report over HTTP connection.
// Authors: zexspectrum
// Date: 2009

#pragma once

#include <WinInet.h>

struct CHttpRequestFile
{  
    CStdString m_sSrcFileName;  // Name of the file attachment.
    CStdString m_sContentType;  // Content type.
};

typedef struct _http_content
{
	CStdString text_name; 
	CStdString text_cont; 
} http_content, *phttp_content; 

#define MAX_HTTP_CONT_NUM 10
// HTTP request information
class http_request
{
public:
	ULONG text_count; 
    CStdString m_sUrl;      // Script URL  
    http_content m_aTextFields[ MAX_HTTP_CONT_NUM ];    // Array of text fields to include into POST data
};

// Sends HTTP request
// See also: RFC 1867 - Form-based File Upload in HTML (http://www.ietf.org/rfc/rfc1867.txt)
class http_data_sender
{
public:

    http_data_sender();

	LRESULT add_text_count( LPCTSTR name, LPCTSTR text )
	{
		LRESULT ret = ERROR_SUCCESS; 

		if( m_Request.text_count >= MAX_HTTP_CONT_NUM )
		{
			ret = ERROR_NOT_ENOUGH_QUOTA; 
			goto _return; 
		}

		m_Request.m_aTextFields[ m_Request.text_count ].text_name = name; 
		m_Request.m_aTextFields[ m_Request.text_count ].text_cont = text; 

		m_Request.text_count ++; 
_return:
		return ret; 
	}

	BOOL send_all_data( LPCTSTR url ); 

    // Sends HTTP request assynchroniously
    BOOL SendAssync(http_request& Request );

private:

    // Worker thread procedure
    static DWORD WINAPI WorkerThread(VOID* pParam);  

    BOOL InternalSend();

    // Used to calculate summary size of the request
    BOOL CalcRequestSize(LONGLONG& lSize);
	BOOL FormatTextPartHeader(CStdString sName, CStdString& sText);
	BOOL FormatTextPartFooter(CStdString sName, CStdString& sText);  
	BOOL FormatTrailingBoundary(CStdString& sBoundary);
	BOOL CalcTextPartSize(CStdString sName, CStdString sData, LONGLONG& lSize); 
	BOOL WriteTextPart(HINTERNET hRequest, CStdString sName, CStdString sData);
	BOOL WriteTrailingBoundary(HINTERNET hRequest);
	void UploadProgress(DWORD dwBytesWritten);

    // This helper function is used to split URL into several parts
    void ParseURL(LPCTSTR szURL, LPTSTR szProtocol, UINT cbProtocol,
        LPTSTR szAddress, UINT cbAddress, DWORD &dwPort, LPTSTR szURI, UINT cbURI);

    http_request m_Request;       // HTTP request being sent

    CStdString m_sFilePartHeaderFmt;
    CStdString m_sFilePartFooterFmt;
    CStdString m_sTextPartHeaderFmt;
    CStdString m_sTextPartFooterFmt;
    CStdString m_sBoundary;
    DWORD m_dwPostSize;
    DWORD m_dwUploaded;
};


