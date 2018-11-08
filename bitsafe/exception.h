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
/**
 * \file        exception.h
 * \author      Plugware Solutions.com, Ltd.
 * \date        
 * \brief       [brief here]
 *
 *              Based on code by Matt Pietrek
 *              Microsoft Systems Journal, April 1997
 *
 *              Copyright (c) 2003
 *              Plugware Solutions.com, Ltd.
 *
 *              Permission to use, copy, modify, distribute and sell 
 *              this software and its documentation for any purpose 
 *              is hereby granted without fee, provided that the above 
 *              copyright notice appear in all copies and the both that 
 *              copyright notice and this permission notice appear 
 *              in supporting documentation.  Plugware Solutions.com, Ltd. 
 *              makes no reresentations about the suitability of this 
 *              software for any purpose.  It is provided "as is" 
 *              without express or implied warranty.
 */
 
#ifndef _plugware_exception_h
#define _plugware_exception_h

/*
**  Pragmas
*/

#if (_WIN32_WINNT < 0x0500)
#error Requires Windows Server 2003, Windows 2000 Server, or Windows NT Server 3.5 and later.
#endif //_WIN32_WINNT < 0x0500

#pragma comment(lib, "imagehlp.lib")

/*
**  Includes
*/

#include <string>
#include <crtdbg.h>
#include <windows.h>
#include <imagehlp.h>

/*
**  Declarations
*/

namespace plugware 
{

namespace exception 
{

    // helper functions
    static void generate_report(HANDLE h_file, PEXCEPTION_POINTERS p_info);
    static const char* get_exception_string(DWORD dw_code);
    static void get_logical_address(PVOID addr, char* sz_module, DWORD len, UINT_PTR& section, UINT_PTR& offset);
    static void stack_walk(HANDLE h_file, PCONTEXT p_context);
    static int write(HANDLE h_file, const char* format, ...);

    /**
    * \class        shutdown
    * \author        
    * \date        
    * \brief        *QUICKLY* shuts down the process.
    *
    *               [details here]
    */
    struct shutdown 
    {
        static void handler(const char*, PEXCEPTION_POINTERS) { ::exit(-1); }
    };
    
    /**
    * \class        report
    * \author        
    * \date        
    * \brief        Write an exception report to disk.
    *
    *               [details here]
    */
    struct report 
    {
        static void handler(const char* sz_log, PEXCEPTION_POINTERS p_info)
        {
            HANDLE h_file(
                ::CreateFileA(sz_log,
                            GENERIC_WRITE,
                            0,
                            0,
                            OPEN_ALWAYS,
                            FILE_FLAG_WRITE_THROUGH,
                            0));

            if (h_file != INVALID_HANDLE_VALUE)
            {
                ::SetFilePointer(h_file, 0, 0, FILE_END);
                generate_report(h_file, p_info);
                ::CloseHandle(h_file);
            }

        ::OutputDebugStringA("An unhandled exception has occurred.\r\n"\
                                "Information regarding the exception has been logged.\r\n"\
                                "Please contact your application vendor for further assistance.\r\n");                               
        }
    }; // struct report
    
    /**
    * \class        filter
    * \author        
    * \date        
    * \brief        Windows exception filter implementation
    *
    *               T_handler describes the action to be taken 
    *               during an exception.  Filters may be chained.
    */
    template <typename T_handler>
    class filter
    {
    public:
        
        static void install();
        static void set_log_file(const std::string&);
        static void uninstall();
        ~filter() { uninstall(); }

    private:    // implementation

        static LONG WINAPI callback(EXCEPTION_POINTERS*);

        // variables used by the class
        static char* get_file_name();
        static LPTOP_LEVEL_EXCEPTION_FILTER& get_previous_filter();

    }; // class filter

    /*
    **  Implementation
    */

    //////////////////////////////////////////////////////////////////////////
    // filter
    //////////////////////////////////////////////////////////////////////////
    template <typename T_handler>
    inline char* filter<T_handler>::get_file_name()
    {
        static char log_file_name[MAX_PATH] = {0};
        return log_file_name; 
    }
                
    template <typename T_handler>
    inline LPTOP_LEVEL_EXCEPTION_FILTER& filter<T_handler>::get_previous_filter() 
    {
        static LPTOP_LEVEL_EXCEPTION_FILTER previous_filter = 0;
        return previous_filter;
    }            

    template <typename T_handler>
    inline void filter<T_handler>::install()
    {
        // Install the unhandled exception filter function
        get_previous_filter() = ::SetUnhandledExceptionFilter(callback);

        // Figure out what the report file will be named, and store it away
        ::GetModuleFileNameA(0, get_file_name(), MAX_PATH);

        // Look for the '.' before the "exe" extension.  
        char* psz_dot = ::strchr(get_file_name(), '.');

        // Replace the extension with "rpt"
        (psz_dot) && 
        (++psz_dot) && 
        (::strlen(psz_dot) >= 3) && 
        (::strcpy(psz_dot, "rpt"));
    }

    template <typename T_handler>
    inline void filter<T_handler>::uninstall()
    {
        ::SetUnhandledExceptionFilter(get_previous_filter());
    }
    
    template <typename T_handler>
    inline void filter<T_handler>::set_log_file(const std::string& s_file)
    {
        if (true == s_file.empty()) return;
        ::strncpy(get_file_name(), s_file.c_str(), min(s_file.size(), MAX_PATH));
    }

    template <typename T_handler>
    inline LONG WINAPI filter<T_handler>::callback(PEXCEPTION_POINTERS p_info)
    {
        T_handler::handler(get_file_name(), p_info);
        if (get_previous_filter()) return (get_previous_filter())(p_info);
        return EXCEPTION_CONTINUE_SEARCH;
    }

    //////////////////////////////////////////////////////////////////////////
    // helpers
    //////////////////////////////////////////////////////////////////////////
    inline void generate_report(HANDLE h_file, PEXCEPTION_POINTERS p_info)
    {
        // Start out with a banner
        SYSTEMTIME st = {0};
        ::GetLocalTime(&st);

        write(h_file, "Unhandled exception: %02d/%02d/%04d %02d:%02d:%02d\r\n", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond);
        PEXCEPTION_RECORD p_record = p_info->ExceptionRecord;

        // First print information about the type of fault
        write(h_file, "Exception code: %08X %s\r\n", p_record->ExceptionCode, get_exception_string(p_record->ExceptionCode));

        // Now print information about where the fault occured
        char sz_module[MAX_PATH] = {0};
        UINT_PTR section = 0, offset = 0;
        get_logical_address(p_record->ExceptionAddress, sz_module, sizeof(sz_module), section, offset);
        write(h_file, "Fault address:  %08X %02X:%08X %s\r\n", p_record->ExceptionAddress, section, offset, sz_module);
        PCONTEXT p_context = p_info->ContextRecord;


#ifdef _M_IX86  // Intel Only!
        // Show the registers
        write(h_file, "\nRegisters:\r\n");
        write(h_file, "EAX:%08X\r\nEBX:%08X\r\nECX:%08X\r\nEDX:%08X\r\nESI:%08X\r\nEDI:%08X\r\n", p_context->Eax, p_context->Ebx, p_context->Ecx, p_context->Edx, p_context->Esi, p_context->Edi );
        write(h_file, "CS:EIP:%04X:%08X\r\n", p_context->SegCs, p_context->Eip);
        write(h_file, "SS:ESP:%04X:%08X  EBP:%08X\r\n", p_context->SegSs, p_context->Esp, p_context->Ebp );
        write(h_file, "DS:%04X  ES:%04X  FS:%04X  GS:%04X\r\n", p_context->SegDs, p_context->SegEs, p_context->SegFs, p_context->SegGs );
        write(h_file, "Flags:%08X\r\n", p_context->EFlags);
#endif //_M_IX86

        stack_walk(h_file, p_context);
        write(h_file, "\r\n");
    }

    //======================================================================
    // Given an exception code, returns a pointer to a static string with a 
    // description of the exception                                         
    //======================================================================
    inline const char* get_exception_string(DWORD dw_code)
    {
        #define EXCEPTION( x ) case EXCEPTION_##x: return (#x);

        switch (dw_code)
        {
            EXCEPTION( ACCESS_VIOLATION )
            EXCEPTION( DATATYPE_MISALIGNMENT )
            EXCEPTION( BREAKPOINT )
            EXCEPTION( SINGLE_STEP )
            EXCEPTION( ARRAY_BOUNDS_EXCEEDED )
            EXCEPTION( FLT_DENORMAL_OPERAND )
            EXCEPTION( FLT_DIVIDE_BY_ZERO )
            EXCEPTION( FLT_INEXACT_RESULT )
            EXCEPTION( FLT_INVALID_OPERATION )
            EXCEPTION( FLT_OVERFLOW )
            EXCEPTION( FLT_STACK_CHECK )
            EXCEPTION( FLT_UNDERFLOW )
            EXCEPTION( INT_DIVIDE_BY_ZERO )
            EXCEPTION( INT_OVERFLOW )
            EXCEPTION( PRIV_INSTRUCTION )
            EXCEPTION( IN_PAGE_ERROR )
            EXCEPTION( ILLEGAL_INSTRUCTION )
            EXCEPTION( NONCONTINUABLE_EXCEPTION )
            EXCEPTION( STACK_OVERFLOW )
            EXCEPTION( INVALID_DISPOSITION )
            EXCEPTION( GUARD_PAGE )
            EXCEPTION( INVALID_HANDLE )
        }

        // If not one of the "known" exceptions, try to get the string from ntdll.dll's message table.
        static char sz_buffer[512] = {0};
        ::FormatMessageA(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE,
                         ::GetModuleHandleA("NTDLL.DLL"),
                         dw_code, 0, sz_buffer, sizeof(sz_buffer), 0);

        return sz_buffer;
    }

    inline void get_logical_address(PVOID addr, char* sz_module, DWORD len, UINT_PTR& section, UINT_PTR& offset)
    {
        MEMORY_BASIC_INFORMATION mbi = {0};
        if (FALSE == ::VirtualQuery( addr, &mbi, sizeof(mbi) ) ) return;

        UINT_PTR h_module = (UINT_PTR)mbi.AllocationBase;
        if (FALSE == ::GetModuleFileNameA((HMODULE)h_module, sz_module, len)) return;

        // Point to the DOS header in memory
        PIMAGE_DOS_HEADER p_dos_hdr = (PIMAGE_DOS_HEADER)h_module;

        // From the DOS header, find the NT (PE) header
        PIMAGE_NT_HEADERS p_nt_hdr = (PIMAGE_NT_HEADERS)(h_module + p_dos_hdr->e_lfanew);
        PIMAGE_SECTION_HEADER p_section = IMAGE_FIRST_SECTION( p_nt_hdr );
        UINT_PTR rva = (UINT_PTR)addr - h_module; // RVA is offset from module load address

        // Iterate through the section table, looking for the one that encompasses
        // the linear address.
        for (unsigned i = 0; 
             i < p_nt_hdr->FileHeader.NumberOfSections;
             ++i, ++p_section)
        {
            UINT_PTR section_start = p_section->VirtualAddress;
            UINT_PTR section_end   = section_start  + max(p_section->SizeOfRawData, p_section->Misc.VirtualSize);

            // Is the address in this section?
            if ((rva >= section_start) && (rva <= section_end))
            {
                // Yes, address is in the section.  Calculate section and offset,
                // and store in the "section" & "offset" params, which were
                // passed by reference.
                section = i + 1;
                offset = rva - section_start;
                return;
            }
        }

        _ASSERT(false); // Should never get here!
    }

    inline void stack_walk(HANDLE h_file, PCONTEXT p_context)
    {
        write(h_file, "Call stack:\r\n");
        write(h_file, "Address\tFrame\r\n");

        // Could use SymSetOptions here to add the SYMOPT_DEFERRED_LOADS flag
        STACKFRAME sf = {0};

        // Initialize the STACKFRAME structure for the first call.  This is only
        // necessary for Intel CPUs, and isn't mentioned in the documentation.
        sf.AddrPC.Offset    = p_context->Eip;
        sf.AddrPC.Mode      = AddrModeFlat;
        sf.AddrStack.Offset = p_context->Esp;
        sf.AddrStack.Mode   = AddrModeFlat;
        sf.AddrFrame.Offset = p_context->Ebp;
        sf.AddrFrame.Mode   = AddrModeFlat;

        for (;;)
        {
            if (FALSE == 
                StackWalk(IMAGE_FILE_MACHINE_I386,
                          GetCurrentProcess(),
                          GetCurrentThread(),
                          &sf,
                          p_context,
                          0,
                          SymFunctionTableAccess,
                          SymGetModuleBase,
                          0)) break;

            // Basic sanity check to make sure the frame is OK.  Bail if not.
            if (0 == sf.AddrFrame.Offset) break;

            write(h_file, "%08X\t%08X", sf.AddrPC.Offset, sf.AddrFrame.Offset);
            BYTE sym_buffer[sizeof(IMAGEHLP_SYMBOL) + 512];
            PIMAGEHLP_SYMBOL p_symbol = (PIMAGEHLP_SYMBOL)sym_buffer;
            p_symbol->SizeOfStruct = sizeof(sym_buffer);
            p_symbol->MaxNameLength = 512;
                            
            // Displacement of the input address, relative to the start of the symbol
            DWORD sym_displacement = 0;  
            if (SymGetSymFromAddr(::GetCurrentProcess(), sf.AddrPC.Offset, &sym_displacement, p_symbol))
            {
                write("%hs+%X\r\n", p_symbol->Name, sym_displacement );
            }
            else // No symbol found.  Print out the logical address instead.
            {
                char sz_module[MAX_PATH] = {0};
                UINT_PTR section = 0, offset = 0;
                get_logical_address((PVOID)(UINT_PTR)sf.AddrPC.Offset, sz_module, sizeof(sz_module), section, offset);
                write(h_file, "%04X:%08X %s\r\n", section, offset, sz_module );
            }
        }
    }

    inline int __cdecl write(HANDLE h_file, const char* format, ...)
    {
        int n_ret = 0;
        char sz_buf[1024] = {0};
        DWORD dw_written = 0;
        va_list argptr;
              
        va_start(argptr, format);
        n_ret = ::vsprintf(sz_buf, format, argptr);
        va_end(argptr);

        ::OutputDebugStringA(sz_buf);
        ::WriteFile(h_file, sz_buf, n_ret, &dw_written, 0 );
        return n_ret;
    }
} // namespace exception    
} // namespace plugware
#endif //_plugware_exception_h