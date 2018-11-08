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

#pragma once
#include "AssyncNotification.h"
#include "MailMsg.h"
#include "smtpclient.h"
#include "HttpRequestSender.h"
#include "tinyxml.h"
#include "CrashInfoReader.h"

// Action type
enum ActionType  
{
    COLLECT_CRASH_INFO = 0x01, // Crash info should be collected
    COMPRESS_REPORT    = 0x02, // Error report files should be packed into ZIP archive
    RESTART_APP        = 0x04, // Crashed app should be restarted
    SEND_REPORT        = 0x08  // Report should be sent
};

// The class that sends error reports
class CErrorReportSender
{
public:

    // Construction/destruction
    CErrorReportSender();
    ~CErrorReportSender();

    // Does an action assynchronously
    BOOL DoWork(int action);

    // Allows to specify file name for exporting error report
    void SetExportFlag(BOOL bExport, CString sExportFile);

    // Blocks until an assync operation finishes
    void WaitForCompletion();

    // Returns global status
    int GetGlobalStatus();

    // Gets local operation status
    void GetStatus(int& nProgressPct, std::vector<CString>& msg_log);

    // Cancels the assyn operation
    void Cancel();

    // Unblocks waiting worker thread
    void FeedbackReady(int code);

    // Returns size in bytes of error report files
    LONG64 GetUncompressedReportSize(ErrorReportInfo& eri);

    // Returns current error report's index
    int GetCurReport();

    // Sets current error report's index
    BOOL SetCurReport(int nCurReport);

    // Sets log file name
    BOOL SetLogFile(LPCTSTR szFileName);

    // Cleans up all temp files and does other finalizing work
    BOOL Finalize();

private:

    // Runs an operation in assync mode
    void DoWorkAssync();

    // Worker thread proc
    static DWORD WINAPI WorkerThread(LPVOID lpParam);  

    // Collects crash report files
    BOOL CollectCrashFiles();  

    // Calculates MD5 hash for a file
    int CalcFileMD5Hash(CString sFileName, CString& sMD5Hash);

    // Dumps registry key to the XML file
    int DumpRegKey(CString sRegKey, CString sDestFile, CString& sErrorMsg);

    // Used internally
    int DumpRegKey(HKEY hKeyParent, CString sSubKey, TiXmlElement* elem);

    // Takes desktop screenshot
    BOOL TakeDesktopScreenshot();

    // Creates crash dump file
    BOOL CreateMiniDump();  

    // Creates crash description XML file
    BOOL CreateCrashDescriptionXML(ErrorReportInfo& eri);

    // Adds an element to XML file
    void AddElemToXML(CString sName, CString sValue, TiXmlNode* root);

    // Minidump callback
    static BOOL CALLBACK MiniDumpCallback(PVOID CallbackParam, PMINIDUMP_CALLBACK_INPUT CallbackInput,
        PMINIDUMP_CALLBACK_OUTPUT CallbackOutput); 

    // Minidump callback
    BOOL OnMinidumpProgress(const PMINIDUMP_CALLBACK_INPUT CallbackInput,
        PMINIDUMP_CALLBACK_OUTPUT CallbackOutput);

    // Restarts the application
    BOOL RestartApp();

    // Packs error report files to ZIP archive
    BOOL CompressReportFiles(ErrorReportInfo& eri);

    // Unblocks parent process
    void UnblockParentProcess();

    // Sends error report
    BOOL SendReport();

    // Sends error report over HTTP
    BOOL SendOverHTTP();

    // Encodes attachment file with Base-64 encoding
    int Base64EncodeAttachment(CString sFileName, std::string& sEncodedFileData);

    // Formats Email text
    CString FormatEmailText();

    // Sends error report over SMTP
    BOOL SendOverSMTP();

    // Sends error report over Simple MAPI
    BOOL SendOverSMAPI();

    int m_nGlobalStatus;                // Global status
    int m_nCurReport;                   // Index of current error report
    HANDLE m_hThread;                   // Handle to the worker thread
    int m_SendAttempt;                  // Number of current sending attempt
    AssyncNotification m_Assync;        // Used for communication with the main thread
    CEmailMessage m_EmailMsg;           // Email message to send
    CSmtpClient m_SmtpClient;           // Used to send report over SMTP 
    CHttpRequestSender m_HttpSender;    // Used to send report over HTTP
    CMailMsg m_MapiSender;              // Used to send report over SMAPI
    CString m_sZipName;                 // Name of the ZIP archive to send
    int m_Action;                       // Current action
    BOOL m_bExport;                     // If TRUE than export should be performed
    CString m_sExportFileName;          // File name for exporting
};

extern CErrorReportSender g_ErrorReportSender;