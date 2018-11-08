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

// File: CrashInfoReader.h
// Description: Retrieves crash information passed from CrashRpt.dll in form of XML files.
// Authors: zexspectrum
// Date: 2010

#pragma once
#include "stdafx.h"
#include "tinyxml.h"
#include "SharedMem.h"
#include "ScreenCap.h"

// The structure describing a file item.
struct ERIFileItem
{
    ERIFileItem()
    {
        m_bMakeCopy = FALSE;
    }

    CString m_sDestFile;  // Destination file name (not including directory name).
    CString m_sSrcFile;   // Absolute source file path.
    CString m_sDesc;      // File description.
    BOOL m_bMakeCopy;     // Should we copy source file to error report folder?
    CString m_sErrorStatus; // Empty if OK, non-empty if error occurred.
};

// Error report delivery status
enum DELIVERY_STATUS
{  
    PENDING    = 0,  // Status pending  
    DELIVERED  = 1,  // Error report was delivered ok
    FAILED     = 2,  // Error report delivery failed
	DELETED    = 3   // Error report was deleted by user
};

// Error report info
struct ErrorReportInfo
{
    ErrorReportInfo()
    {
        m_bSelected = TRUE;
        m_DeliveryStatus = PENDING;    
        m_dwGuiResources = 0;
        m_dwProcessHandleCount = 0;
        m_uTotalSize = 0;
    }

    CString     m_sErrorReportDirName; // Name of the directory where error report files are located.
    CString     m_sCrashGUID;          // Crash GUID.
    CString     m_sAppName;            // Application name.
    CString     m_sAppVersion;         // Application version.
    CString     m_sImageName;          // Path to the application executable file.
    CString     m_sEmailFrom;          // E-mail sender address.
    CString     m_sDescription;        // User-provided problem description.
    CString     m_sSystemTimeUTC;      // The time when crash occurred (UTC).
    DWORD       m_dwGuiResources;      // GUI resource count.
    DWORD       m_dwProcessHandleCount; // Process handle count.
    CString     m_sMemUsage;           // Memory usage.
    CString     m_sOSName;             // Operating system friendly name.
    BOOL        m_bOSIs64Bit;          // Is operating system 64-bit?
    CString     m_sGeoLocation;        // Geographic location.
    ScreenshotInfo m_ScreenshotInfo;   // Screenshot info.
    ULONG64     m_uTotalSize;          // Summary size of this (uncompressed) report.
    BOOL        m_bSelected;           // Is this report selected for delivery.
    DELIVERY_STATUS m_DeliveryStatus;  // Delivery status.

    std::map<CString, ERIFileItem>  m_FileItems; // The list of files that are included into this report.
    std::map<CString, CString> m_RegKeys; // The list of registry keys
    std::map<CString, CString> m_Props;   // The list of custom properties
};

// Remind policy.
enum REMIND_POLICY 
{
    REMIND_LATER,   // Remind later.
    NEVER_REMIND    // Never remind.
};

// Class responsible for reading the crash info.
class CCrashInfoReader
{
public:

    /* Member variables. */

    int         m_nCrashRptVersion;     // CrashRpt version sent through shared memory
    CString     m_sUnsentCrashReportsFolder; // Path to UnsentCrashReports folder for the application.
    CString     m_sLangFileName;        // Path to language INI file.
    CString     m_sDbgHelpPath;         // Path to dbghelp.dll.
    CString     m_sAppName;             // Application name.
    CString     m_sCustomSenderIcon;    // Custom icon resource for Error Report dialog.
    CString     m_sEmailTo;             // E-mail recipient address.
    CString     m_sEmailSubject;        // E-mail subject.
    CString     m_sEmailText;           // E-mail text.
    int         m_nSmtpPort;            // SMTP port.
    CString     m_sSmtpProxyServer;     // SMTP proxy server.
    int         m_nSmtpProxyPort;       // SMTP proxy port.
    CString     m_sUrl;                 // URL (used for HTTP connection).
    BOOL        m_bHttpBinaryEncoding;  // Should we use binary transfer encoding (HTTP).
    BOOL        m_bSilentMode;          // Should we show GUI?
    BOOL        m_bSendErrorReport;     // Should we send error report now?
    BOOL        m_bStoreZIPArchives;    // Should we store zipped error report files?
    BOOL        m_bSendRecentReports;   // Should we send recent reports now?
    BOOL        m_bAppRestart;          // Should we restart the application?
    CString     m_sRestartCmdLine;      // Command line for app restart.
    UINT        m_uPriorities[3];       // Delivery priorities.
    CString     m_sPrivacyPolicyURL;    // Privacy policy URL.
    BOOL        m_bGenerateMinidump;    // Should we generate crash minidump file?
    MINIDUMP_TYPE m_MinidumpType;       // Minidump type.
    BOOL        m_bAddScreenshot;       // Should we add a desktop screenshot?
    DWORD       m_dwScreenshotFlags;    // Screenshot options.
    int         m_nJpegQuality;         // Jpeg image quality.
    CPoint      m_ptCursorPos;          // Mouse cursor position on crash.
    CRect       m_rcAppWnd;             // Rectangle of the application's main window.  
    BOOL        m_bQueueEnabled;        // Can reports be send later or not (queue enabled)?

    DWORD       m_dwProcessId;          // Parent process ID (used for minidump generation).
    DWORD       m_dwThreadId;           // Parent thread ID (used for minidump generation).
    PEXCEPTION_POINTERS m_pExInfo;      // Address of exception info (used for minidump generation).
    int         m_nExceptionType;       // Exception type (what handler caught the exception).
    DWORD       m_dwExceptionCode;      // SEH exception code
    UINT        m_uFPESubcode;          // FPE exception subcode
    CString     m_sInvParamExpr;        // Invalid parameter expression
    CString     m_sInvParamFunction;    // Invalid parameter function
    CString     m_sInvParamFile;        // Invalid parameter file
    UINT        m_uInvParamLine;        // Invalid parameter line

    /* Member functions */

    // Gets crash info from shared memory
    int Init(LPCTSTR szFileMappingName);

    // Loads custom icon (if defined)
    HICON GetCustomIcon();

    // Returns report by its index in the list
    ErrorReportInfo& GetReport(int nIndex);

    // Returns count of error reports
    int GetReportCount();

	// Deletes n-th report
	void DeleteReport(int nIndex);

	// Deletes all reports
	void DeleteAllReports();

    // Returns TRUE if it is time to remind user
    BOOL IsRemindNowOK();

    // Sets remind policy
    BOOL SetRemindPolicy(REMIND_POLICY Policy);

    // Updates last remind date
    BOOL SetLastRemindDateToday();

    // Adds user information
    BOOL AddUserInfoToCrashDescriptionXML(CString sEmail, CString sDesc);

private:

    // Retrieves some crash info from crash description XML
    int ParseCrashDescription(CString sFileName, BOOL bParseFileItems, ErrorReportInfo& eri);  

    // Adds the list of files
    BOOL AddFilesToCrashDescriptionXML(std::vector<ERIFileItem>);

    // Returns last remind date
    BOOL GetLastRemindDate(SYSTEMTIME& LastDate);

    // Returns current remind policy
    REMIND_POLICY GetRemindPolicy();

    // Unpacks crash description from shared memory
    int UnpackCrashDescription(ErrorReportInfo& eri);

    // Unpacks string
    int UnpackString(DWORD dwOffset, CString& str);

    // Collects misc info about the crash
    void CollectMiscCrashInfo(ErrorReportInfo& eri);

    // Gets the list of file items 
    int ParseFileList(TiXmlHandle& hRoot, ErrorReportInfo& eri);

    // Gets the list of registry keys
    int ParseRegKeyList(TiXmlHandle& hRoot, ErrorReportInfo& eri);

    // Calculates size of an uncompressed error report.
    LONG64 GetUncompressedReportSize(ErrorReportInfo& eri);

    // Array of error reports
    std::vector<ErrorReportInfo> m_Reports;

    // Path to ~CrashRpt.ini file.
    CString m_sINIFile; 

    CSharedMem m_SharedMem;          // Shared memory
    CRASH_DESCRIPTION* m_pCrashDesc; // Pointer to crash descritpion
};

// Declare globally available object.
extern CCrashInfoReader g_CrashInfo;

