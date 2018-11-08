#ifndef _PROCESS_H_
#define _PROCESS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#define PSAPI_VERSION 1 

#include "process_module.h"
#include <TLHELP32.H>
#include "file_version_info.h"
#include <psapi.h>
#include "process_io_counter.h"
#include "process_times.h"


#define PROC_FULL_ACCESS ( PROCESS_ALL_ACCESS | SYNCHRONIZE )
#define PROC_SAFE_ACCESS ( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | SYNCHRONIZE | PROCESS_VM_WRITE )
#define PROC_READ_ONLY_ACCESS ( PROCESS_QUERY_INFORMATION | PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ )

enum PROCESS_TYPE_e
{
    PRT_WIN32_CONSOLE = 10,
    PRT_WIN32_WINDOWS,
    PRT_MSDOS,
    PRT_INVALID
};

#ifdef __cplusplus
extern "C" 
{
#endif

    typedef LONG NTSTATUS;
    
    
    
    typedef LONG KPRIORITY;

#pragma pack( push )
#pragma pack( 8 )

	typedef struct _RTL_DRIVE_LETTER_CURDIR {
		USHORT                  Flags;
		USHORT                  Length;
		ULONG                   TimeStamp;
		UNICODE_STRING          DosPath;
	} RTL_DRIVE_LETTER_CURDIR, *PRTL_DRIVE_LETTER_CURDIR;

	typedef struct _RTL_USER_PROCESS_PARAMETERS
	{
		ULONG                   MaximumLength;
		ULONG                   Length;
		ULONG                   Flags;
		ULONG                   DebugFlags;
		PVOID                   ConsoleHandle;
		ULONG                   ConsoleFlags;
		HANDLE                  StdInputHandle;
		HANDLE                  StdOutputHandle;
		HANDLE                  StdErrorHandle;
		UNICODE_STRING          CurrentDirectoryPath;
		HANDLE                  CurrentDirectoryHandle;
		UNICODE_STRING          DllPath;
		UNICODE_STRING          ImagePathName;
		UNICODE_STRING          CommandLine;
		PVOID                   Environment;
		ULONG                   StartingPositionLeft;
		ULONG                   StartingPositionTop;
		ULONG                   Width;
		ULONG                   Height;
		ULONG                   CharWidth;
		ULONG                   CharHeight;
		ULONG                   ConsoleTextAttributes;
		ULONG                   WindowFlags;
		ULONG                   ShowWindowFlags;
		UNICODE_STRING          WindowTitle;
		UNICODE_STRING          DesktopName;
		UNICODE_STRING          ShellInfo;
		UNICODE_STRING          RuntimeData;
		RTL_DRIVE_LETTER_CURDIR DLCurrentDirectory[0x20];
	} RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;

	typedef struct _PEB_LDR_DATA {
		ULONG                   Length;
		BOOLEAN                 Initialized;
		PVOID                   SsHandle;
		LIST_ENTRY              InLoadOrderModuleList;
		LIST_ENTRY              InMemoryOrderModuleList;
		LIST_ENTRY              InInitializationOrderModuleList;
	} PEB_LDR_DATA, *PPEB_LDR_DATA;

	typedef struct _PEB_FREE_BLOCK
	{
		struct _PEB_FREE_BLOCK* Next;
		ULONG Size;
	} PEB_FREE_BLOCK, *PPEB_FREE_BLOCK;

	typedef VOID ( *PPEBLOCKROUTINE)(PVOID);

	typedef struct _PEB {
		BOOLEAN                 InheritedAddressSpace;
		BOOLEAN                 ReadImageFileExecOptions;
		BOOLEAN                 BeingDebugged;
		BOOLEAN                 Spare;
		HANDLE                  Mutant;
		PVOID                   ImageBaseAddress;
		PPEB_LDR_DATA           LoaderData;
		PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
		PVOID                   SubSystemData;
		PVOID                   ProcessHeap;
		PVOID                   FastPebLock;
		PPEBLOCKROUTINE         FastPebLockRoutine;
		PPEBLOCKROUTINE         FastPebUnlockRoutine;
		ULONG                   EnvironmentUpdateCount;
		PVOID                  *KernelCallbackTable;
		PVOID                   EventLogSection;
		PVOID                   EventLog;
		PPEB_FREE_BLOCK         FreeList;
		ULONG                   TlsExpansionCounter;
		PVOID                   TlsBitmap;
		ULONG                   TlsBitmapBits[0x2];
		PVOID                   ReadOnlySharedMemoryBase;
		PVOID                   ReadOnlySharedMemoryHeap;
		PVOID                  *ReadOnlyStaticServerData;
		PVOID                   AnsiCodePageData;
		PVOID                   OemCodePageData;
		PVOID                   UnicodeCaseTableData;
		ULONG                   NumberOfProcessors;
		ULONG                   NtGlobalFlag;
		BYTE                    Spare2[0x4];
		LARGE_INTEGER           CriticalSectionTimeout;
		ULONG                   HeapSegmentReserve;
		ULONG                   HeapSegmentCommit;
		ULONG                   HeapDeCommitTotalFreeThreshold;
		ULONG                   HeapDeCommitFreeBlockThreshold;
		ULONG                   NumberOfHeaps;
		ULONG                   MaximumNumberOfHeaps;
		PVOID                  **ProcessHeaps;
		PVOID                   GdiSharedHandleTable;
		PVOID                   ProcessStarterHelper;
		PVOID                   GdiDCAttributeList;
		PVOID                   LoaderLock;
		ULONG                   OSMajorVersion;
		ULONG                   OSMinorVersion;
		ULONG                   OSBuildNumber;
		ULONG                   OSPlatformId;
		ULONG                   ImageSubSystem;
		ULONG                   ImageSubSystemMajorVersion;
		ULONG                   ImageSubSystemMinorVersion;
		ULONG                   GdiHandleBuffer[0x22];
		ULONG                   PostProcessInitRoutine;
		ULONG                   TlsExpansionBitmap;
		BYTE                    TlsExpansionBitmapBits[0x80];
		ULONG                   SessionId;
	} PEB, *PPEB;

	typedef struct _PROCESS_BASIC_INFORMATION {
		NTSTATUS ExitStatus;
		PPEB PebBaseAddress;
		ULONG_PTR AffinityMask;
		KPRIORITY BasePriority;
		ULONG_PTR UniqueProcessId;
		ULONG_PTR InheritedFromUniqueProcessId;
	} PROCESS_BASIC_INFORMATION,*PPROCESS_BASIC_INFORMATION;

#pragma pack( pop )

#ifdef __cplusplus
}
#endif

LRESULT WINAPI init_symbol_mapping(); 

LRESULT WINAPI convert_symbolic_file_path( LPCWSTR file_path, 
										  ULONG cc_path_len, 
										  LPWSTR path_out, 
										  ULONG cc_buf_len, 
										  ULONG *cc_ret_len ); 

/**
 * Copyright(c) 2007 Nibu babu thomas
 *
 * Process - Denotes a single process running in a system
 *
 * @author :    Nibu babu thomas
 * @version:    1.0            Date:  2007-06-15
 */
class Process  
{
    public:

        Process();
        virtual ~Process();

		LRESULT SetProcessId( ULONG ProcessId ); 

		LRESULT GetMainModuleFullName(); 

		LRESULT SetProcessFullName( LPCWSTR ImagePath )
		{
			LRESULT ret = ERROR_SUCCESS; 
			ULONG path_len; 

			do 
			{
				m_csFullPath = ImagePath; 

				ret = convert_symbolic_file_path( m_csFullPath.GetBuffer(), 
					m_csFullPath.GetLength(), 
					m_csFullPath.GetBufferSetLength( MAX_PATH ), 
					MAX_PATH, 
					&path_len ); 

				m_csFullPath.ReleaseBuffer(); 
			}while( FALSE );

			return ret; 
		}

		ULONG GetSessionId()
		{
			return SessionId; 
		}

        
        void SetProcessHandle( HANDLE hProcess_i )
        {
            m_ahmProcess = hProcess_i;
			m_dwProcId = GetProcessId( hProcess_i ); 
        }

        
        HANDLE GetProcessHandle() const
        {
            return m_ahmProcess;
        }

        
        DWORD GetProcessID() const
        {
            return m_dwProcId;
        }

        
        const PROCESSENTRY32& GetProcessEntry32() const
        {
            return m_stPrcEntry32;
        }


        
        LRESULT GetAllInfo();

        
        bool SetProcessEntry32( const PROCESSENTRY32& stpePrcEntry32_i );

        
        bool AddKid( Process*& pPrcKid_i );

        
        bool IsKid() const
        {
            return m_stPrcEntry32.th32ParentProcessID != 0;
        }

        
        const CString& GetBaseName() const
        {
            return m_csBaseName;
        }

        const CString& GetFullPath() const
        {
            return m_csFullPath;
        }

		const CString& GetUserName() const 
		{
			return m_csUserName; 
		}

		const ULONG GetParentProcessId() const
		{
			return m_dwParentProcId; 
		}

		const CString GetFileVersion() const 
		{
			bool ret; 
			CString Text; 
			
			ret = m_fviFileVersion.GetFileVersion( Text ); 
			if( false == ret )
			{
				Text = L""; 
			}

			return Text; 
		}

		LRESULT _GetVersionInfo(); 

        
        bool IsProcessAlive() const
        {
            if( m_ahmProcess != NULL )
            {
                
                return WaitForSingleObject( m_ahmProcess, 0 ) == WAIT_TIMEOUT;
            }

            return false;
        }

        
        LRESULT EnumModules();

        
        void Clear();

        
        PROCESS_TYPE_e ExtractProcessType( CString* pcsProcessType_o = 0 ) const;
        static PROCESS_TYPE_e ExtractProcessType( LPCTSTR lpctszProcessPath_i, CString* pcsProcessType_o = 0 );

        
        HICON ExtractAssociatedProcessIcon();

        
        LRESULT Kill( const UINT uExitCode_i ) const;

        
        LRESULT ExtractGDIInfo();

        
        LRESULT ExtractProcessIOCounters();

        
        LRESULT ExtractProcessTimes();

        
        LRESULT ExtractProcessPriority( CString& csPriority_o ) const;

        
        static LRESULT ExtractProcessOwner( HANDLE hProcess_i, CString& csOwner_o );

        
        LRESULT ExtractProcessCommandLine( HANDLE hProcess, CString& csCommandLine_o );

		CString & GetProcessCommandLine()
		{
			return m_csProcessCommandLine; 
		}
        
        const PROCESS_MEMORY_COUNTERS& ExtractProcessMemoryCounters();

        
        static bool ChangeProcessPriority( const HANDLE& hProcess_i, const UINT uPriority_i );
        static void EnableDisableAutoStartup( LPCTSTR lpctszProcessPath_i );

    protected:

        
		void ParseOutFilePath( CString& csFileName_io ) const;

	public:
		
		ProcessTimes m_ProcTimes; 
	
	public: 
		std::vector< Module* > m_ProcessModules; 

    private:

        
		
		
		HANDLE m_ahmProcess; 

        
        HMODULE m_hProcessModule;

        
        PROCESSENTRY32 m_stPrcEntry32;

        
        FileVersionInfo m_fviFileVersion;

        
        ProcessIOCounters m_ProcIoCounters;

        
        
        

        
        CString m_csBaseName;

        
        CString m_csFullPath;
		CString m_csUserName;


        
        DWORD m_dwGDIObjs;
        DWORD m_dwUserObjs;

        
        CString m_csProcessType;
    
        
        CString m_csPriority;

        
        CString m_csProcessOwner;

        
        CString m_csProcessCommandLine;

        
        DWORD m_dwProcId; 

		ULONG m_dwParentProcId; 

		ULONG SessionId; 

        
        PROCESS_MEMORY_COUNTERS m_stpmcMemCounters;

};

#endif //_PROCESS_H_