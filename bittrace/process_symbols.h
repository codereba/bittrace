/**
 * Copyright(c) 2007 Nibu babu thomas
 *
 * ProcessSymbolCollection.h - Interface declaration for ProcessSymbolCollection
 *
 * @author :    Nibu babu thomas
 * @version:    1.0            Date:  2007-06-19
 */


#ifndef __PROCESS_SYMBOLS_H__
#define __PROCESS_SYMBOLS_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SymbolCollection.h"
#include <dbghelp.h>
#include <afxtempl.h>

// Module symbol list
typedef CMap<DWORD, DWORD, SymbolCollection*, SymbolCollection*&> ModuleSymbolList;


/**
 * Copyright(c) 2007 Nibu babu thomas
 *
 * ProcessSymbolCollection - ProcessSymbolCollection class.
 *
 * @author :    Nibu babu thomas
 * @version:    1.0            Date:  2007-06-19
 */
class ProcessSymbolCollection  
{
    public:

        ProcessSymbolCollection();
        virtual ~ProcessSymbolCollection();

        // Initialize process handle
        bool Init( HANDLE hProcess_i, LPCTSTR lpctszProcessName_i );

        // EnumerateModuleSymbols through modules for given process
        bool EnumerateModuleSymbols( LPDWORD lpdwDllBases_i, 
                                     const DWORD dwCount_i,
                                     LPCTSTR lpctszModuleFileName_i,
                                     const DWORD dwModuleSize_i );
        bool EnumerateModuleSymbols( DWORD dwDllBases_i, 
                                     SymbolCollection*& pModSymColl_o, 
                                     LPCTSTR lpctszModuleFileName_i,
                                     const DWORD dwModuleSize_i,
                                     const bool bRefresh_i );

        // Clear symbols
        void Clear();

        // Returns status of initialization
        bool IsInitialized() const
        {
            return m_bInitialized;
        }

        static bool GetValidSymbolPath()
        {
            if( !Utils::GetEnvironmentVarValue( _T( "_NT_SYMBOL_PATH" ), GetSymbolPath() ) || 
                !Utils::GetEnvironmentVarValue( _T( "_NT_ALTERNATE_SYMBOL_PATH" ), GetSymbolPath() ))
            {
                Utils::GetEnvironmentVarValue( _T( "SYSTEMROOT" ), GetSymbolPath() );
                GetSymbolPath() += _T( "\\System32" );
                return false;
            }

            return true;
        }


        // Return a reference to symbol path
        static CString& GetSymbolPath() { return m_csSymbolPath; }
        
        
    private:

        // Symbol callback function
		static BOOL CALLBACK EnumModuleSymbolsCB( PSYMBOL_INFO pSymInfo,
												  ULONG  ulSymbolSize_i,
												  LPVOID lpvdUserContext_i );

        // List of symbols
        ModuleSymbolList m_ModSymMap;

        // Position for list traversal
        POSITION m_pstPos;

        // Handle to process
        HANDLE m_hProcess;

        // Initialization status flag
        bool m_bInitialized;

        // bool that indicates environment variable status
        static bool m_bNTSymbolPathEnvVarDefined;

        // Symbol path
        static CString m_csSymbolPath;

        // Name of the process for which we are loading the symbols
        CString m_csProcessName;
};// End ProcessSymbolCollection

#endif // __PROCESS_SYMBOLS_H__