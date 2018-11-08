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
#include "crash_rpt_supp.h"
#include "CrashRpt.h"

#ifdef _DEBUG
#pragma comment( lib, "crashrpt_d.lib" )
#else
#pragma comment( lib, "crashrpt.lib" )
#endif //_DEBUG

BOOL WINAPI CrashCallback(LPVOID lpvState)
{
	UNREFERENCED_PARAMETER(lpvState);

	// Crash happened!

	return TRUE;
}

#define MAIL_CRUSH_REPORT_TO _T( "codereba@gmail.com" )
#define HTTP_URL_CRUSH_REPORT_TO _T( "http://codereba.com:80/crashrpt.php" ) 

LRESULT init_crash_rpt()
{
	LRESULT ret = ERROR_SUCCESS; 
	TCHAR app_path[ MAX_PATH ]; 

	INT32 nResult; 
	CR_INSTALL_INFO info;
	memset(&info, 0, sizeof(CR_INSTALL_INFO));
	info.cb = sizeof(CR_INSTALL_INFO);  
	info.pszAppName = _T("BitSafe"); // Define application name.
	//info.pszAppVersion = _T("1.2.7");     // Define application version.
	info.pszEmailSubject = _T("Error from bitsafe"); // Define subject for email.
	info.pszEmailTo = MAIL_CRUSH_REPORT_TO;   // Define E-mail recipient address.  
	info.pszUrl =  HTTP_URL_CRUSH_REPORT_TO; // URL for sending reports over HTTP.		
	info.pfnCrashCallback = CrashCallback; // Define crash callback function.   
	// Define delivery transport priorities. 
	info.uPriorities[CR_HTTP] = 3;         // Use HTTP the first.
	info.uPriorities[CR_SMTP] = 2;         // Use SMTP the second.
	info.uPriorities[CR_SMAPI] = 1;        // Use Simple MAPI the last.  
	info.dwFlags = 0;                    
	info.dwFlags |= CR_INST_ALL_POSSIBLE_HANDLERS; // Install all available exception handlers.
	info.dwFlags |= CR_INST_HTTP_BINARY_ENCODING;   // Use binary encoding for HTTP uploads (recommended).  
	info.dwFlags |= CR_INST_APP_RESTART;            // Restart the application on crash.  
	//info.dwFlags |= CR_INST_NO_MINIDUMP;            // Do not include minidump.
	//info.dwFlags |= CR_INST_NO_GUI;               // Don't display GUI.
	//info.dwFlags |= CR_INST_DONT_SEND_REPORT;     // Don't send report immediately, just queue for later delivery.
	//info.dwFlags |= CR_INST_STORE_ZIP_ARCHIVES;   / Store ZIP archives along with uncompressed files (to be used with CR_INST_DONT_SEND_REPORT)
	info.dwFlags |= CR_INST_SEND_QUEUED_REPORTS;    // Send reports that were failed to send recently.
	info.pszDebugHelpDLL = NULL;                    // Search for dbghelp.dll using default search sequence.
	info.uMiniDumpType = MiniDumpNormal;            // Define minidump size.
	// Define privacy policy URL.
	info.pszPrivacyPolicyURL = _T("http://code.google.com/p/crashrpt/wiki/PrivacyPolicyTemplate");
	info.pszErrorReportSaveDir = NULL;       // Save error reports to the default location.
	info.pszRestartCmdLine = _T("/restart"); // Command line for automatic app restart.
	//info.pszLangFilePath = _T("D:\\");       // Specify custom dir or filename for language file.
	//info.pszSmtpProxy = _T("127.0.0.1:2525");  // Use SMTP proxy.
	CStdString sCustomSenderIcon; 
#ifdef _DEBUG
	sCustomSenderIcon = _T("\\bitsafe_ud.exe, -203"); // Use custom icon
#else
	sCustomSenderIcon = _T("\\bitsafe.exe, -203"); // Use custom icon
#endif

	ret = add_app_path_to_file_name( app_path, MAX_PATH, sCustomSenderIcon.GetData(), FALSE );
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	sCustomSenderIcon = app_path; 

	//info.pszCustomSenderIcon = sCustomSenderIcon.GetBuffer(0);

	// Install crash handlers.
	nResult = crInstallW(&info);
	if( nResult != 0 )
	{
		TCHAR buff[ 256 ];
		crGetLastErrorMsg( buff, 256 );
		MessageBox( NULL, buff, _T( "crInstall error" ), MB_OK );
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	//ASSERT( nResult == 0); 

	//CString sLogFile = GetAppDir() + _T("\\dummy.log");
	//CString sIniFile = GetAppDir() + _T("\\dummy.ini");

	//int nResult = crAddFile2(sLogFile, NULL, _T("Dummy Log File"), CR_AF_MAKE_FILE_COPY);
	//ATLASSERT(nResult==0);

	//nResult = crAddFile(sIniFile, _T("Dummy INI File"));
	//ATLASSERT(nResult==0);

	nResult = crAddScreenshot2( CR_AS_PROCESS_WINDOWS | CR_AS_USE_JPEG_FORMAT, 10 );
	//nResult = crAddScreenshot(CR_AS_MAIN_WINDOW);
	ASSERT( nResult == 0 );

	//nResult = crAddProperty(_T("VideoCard"),_T("nVidia GeForce 9800"));
	//ASSERT(nResult==0);

	//nResult = crAddRegKey(_T("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer"), _T("regkey.xml"), 0);
	//ASSERT(nResult==0);

	//nResult = crAddRegKey(_T("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings"), _T("regkey.xml"), 0);
	//ASSERT(nResult==0);

_return:
	return ret; 
}

void test_generate_report()
{
	CR_EXCEPTION_INFO ei;
	memset(&ei, 0, sizeof(CR_EXCEPTION_INFO));
	ei.cb = sizeof(CR_EXCEPTION_INFO);
	ei.exctype = CR_SEH_EXCEPTION;
	ei.code = 0x1234;
	ei.pexcptrs = NULL;
	ei.bManual = TRUE; // Signal the report is being generated manually.

	int nResult = crGenerateErrorReport(&ei);
	if(nResult!=0)
	{
		//TCHAR szErrorMsg[256];
		//CString sError = _T("Error generating error report!\nErrorMsg:");
		//crGetLastErrorMsg(szErrorMsg, 256);
		//sError+=szErrorMsg;
		//MessageBox(NULL, sError, 0, 0);
	}
}