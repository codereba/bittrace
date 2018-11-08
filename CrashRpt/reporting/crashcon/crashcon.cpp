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

// crashcon.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <assert.h>
#include <process.h>
#include "CrashRpt.h" // Include CrashRpt header

#pragma comment( lib, "crashrpt.lib" )
#pragma comment( lib, "Dbghelp.lib" )
#pragma comment( lib, "Version.lib" )
#pragma comment( lib, "Rpcrt4.lib" )
#pragma comment( lib, "ws2_32.lib" )
#pragma comment( lib, "jpeg.lib" )
#pragma comment( lib, "libpng.lib" )
#pragma comment( lib, "zlib.lib" )
#pragma comment( lib, "Dnsapi.lib" )
#pragma comment( lib, "minizip.lib" )
#pragma comment( lib, "Wininet.lib" )
#pragma comment( lib, "Psapi.lib" )

LPVOID lpvState = NULL; // Not used, deprecated

int main(int argc, char* argv[])
{
    argc; // this is to avoid C4100 unreferenced formal parameter warning
    argv; // this is to avoid C4100 unreferenced formal parameter warning

    // Install crash reporting

#ifdef TEST_DEPRECATED_FUNCS
    lpvState = Install(
        NULL, 
        _T("test@hotmail.com"), 
        NULL);
#else
    CR_INSTALL_INFO info;
    memset(&info, 0, sizeof(CR_INSTALL_INFO));
    info.cb = sizeof(CR_INSTALL_INFO);             // Size of the structure
    info.pszAppName = _T("CrashRpt Console Test"); // App name
    info.pszAppVersion = _T("1.0.0");              // App version
    info.pszEmailSubject = _T("CrashRpt Console Test 1.0.0 Error Report"); // Email subject
    info.pszEmailTo = _T("test@hotmail.com");      // Email recipient address

    // Install crash handlers
    int nInstResult = crInstall(&info);            
    assert(nInstResult==0);

    // Check result
    if(nInstResult!=0)
    {
        TCHAR buff[256];
        crGetLastErrorMsg(buff, 256); // Get last error
        _tprintf(_T("%s\n"), buff); // and output it to the screen
        return FALSE;
    }

#endif //TEST_DEPRECATED_FUNCS

    printf("Press Enter to simulate a null pointer exception or any other key to exit...\n");
    int n = _getch();
    if(n==13)
    {

#ifdef _DEBUG
        __try
        {
            RaiseException(123, EXCEPTION_NONCONTINUABLE, 0, NULL);
        } 
        __except(crExceptionFilter(GetExceptionCode(), GetExceptionInformation()))
        {
        }
#else
        int *p = 0;
        *p = 0; // Access violation
#endif // _DEBUG

    }

#ifdef TEST_DEPRECATED_FUNCS
    Uninstall(lpvState); // Uninstall exception handlers
#else
    int nUninstRes = crUninstall(); // Uninstall exception handlers
    assert(nUninstRes==0);
    nUninstRes;
#endif //TEST_DEPRECATED_FUNCS

    // Exit
    return 0;
}

