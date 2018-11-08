/*
 *
 * Copyright 2010 JiJie Shi(weixin:AIChangeLife)
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
 * Copyright(c) 2007 Nibu babu thomas
 *
 * ProcessSymbolCollection.cpp - Implementation file for ProcessSymbolCollection.
 *
 * @author :    Nibu babu thomas
 * @version:    1.0            Date:  2007-06-19
 */


#include "stdafx.h"
#include "process_symbols.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif




CString ProcessSymbolCollection::m_csSymbolPath;

bool ProcessSymbolCollection::m_bNTSymbolPathEnvVarDefined = ProcessSymbolCollection::GetValidSymbolPath();

ProcessSymbolCollection::ProcessSymbolCollection() : m_hProcess( 0 ), m_bInitialized( false )
{
    m_ModSymMap.InitHashTable( 53, FALSE );
}

ProcessSymbolCollection::~ProcessSymbolCollection()
{
    
    Clear();
}

void ProcessSymbolCollection::Clear()
{
    if( m_bInitialized && m_hProcess )
    {
        VERIFY( SymCleanup( m_hProcess ));
        m_bInitialized = false;
        m_hProcess = 0;
    }

    POSITION pstPos = m_ModSymMap.GetStartPosition();
    while( pstPos )
    {
        
        SymbolCollection* pSymData = 0;
        DWORD dwKey = 0;
        m_ModSymMap.GetNextAssoc( pstPos, dwKey, pSymData );

        delete pSymData;
        pSymData = 0;
    }

    
    m_ModSymMap.RemoveAll();
    m_bInitialized = false;
    m_hProcess = 0;
    m_csProcessName.Empty();
}

bool ProcessSymbolCollection::Init( HANDLE hProcess_i, 
                                    LPCTSTR lpctszProcessName_i )
{
    
    m_bInitialized = false;

    
    if( !hProcess_i )
    {
        ASSERT( hProcess_i );
        return false;
    }

	
	m_hProcess = hProcess_i;

    
    SymSetOptions( SYMOPT_DEFERRED_LOADS | 
                   SYMOPT_LOAD_ANYTHING | 
                   SYMOPT_ALLOW_ABSOLUTE_SYMBOLS | 
				   SYMOPT_UNDNAME );

    
    USES_CONVERSION;
    if( !SymInitialize( m_hProcess, T2A( GetSymbolPath() ), TRUE ))
    {
        return false;
    }

    m_bInitialized = true;
    m_csProcessName = lpctszProcessName_i;

    
    return true;
}

bool ProcessSymbolCollection::EnumerateModuleSymbols( DWORD dwDllBases_i, 
                                                      SymbolCollection*& pModSymColl_o,
                                                      LPCTSTR lpctszModuleFileName_i,
                                                      const DWORD dwModuleSize_i,
                                                      const bool bRefresh_i )
{
    UNREFERENCED_PARAMETER( lpctszModuleFileName_i );
    UNREFERENCED_PARAMETER( dwModuleSize_i );

    bool bFound = false;
    if( bRefresh_i )
    {
        
        bFound = m_ModSymMap.Lookup( dwDllBases_i, pModSymColl_o ) != FALSE;

        if( bFound )
        {
            
            m_ModSymMap.RemoveKey( dwDllBases_i );
            delete pModSymColl_o;
        }
    }

    if( bRefresh_i || !m_ModSymMap.Lookup( dwDllBases_i, pModSymColl_o ))
    {
        
        pModSymColl_o = new SymbolCollection;
    	SymEnumSymbols( m_hProcess, 
						dwDllBases_i, 
						0, 
						EnumModuleSymbolsCB,  
						pModSymColl_o );

        
        m_ModSymMap[dwDllBases_i] = pModSymColl_o;
    }

    return true;
}

bool ProcessSymbolCollection::EnumerateModuleSymbols( LPDWORD lpdwDllBases_i, 
                                                      const DWORD dwCount_i,
                                                      LPCTSTR lpctszModuleFileName_i,
                                                      const DWORD dwModuleSize_i )
{
    
    if( !m_bInitialized || !m_hProcess )
    {
        ASSERT( false );
        return false;
    }

    
    for( DWORD dwIndex = 0; dwIndex < dwCount_i; ++dwIndex )
    {
        
        SymbolCollection* pModSymColl = 0;

        
        VERIFY( EnumerateModuleSymbols( lpdwDllBases_i[dwIndex], 
                                        pModSymColl, 
                                        lpctszModuleFileName_i, 
                                        dwModuleSize_i, 
                                        false ));
    }

    
    return true;
}


BOOL ProcessSymbolCollection::EnumModuleSymbolsCB( PSYMBOL_INFO pSymInfo_i,
												   ULONG ulSymbolSize_i,
												   LPVOID lpvdUserContext_i )
{
	UNREFERENCED_PARAMETER( ulSymbolSize_i );
    SymbolCollection* pModSymColl = RCAST( SymbolCollection*, lpvdUserContext_i );
    if( !pModSymColl )
    {
        
        return TRUE;
    }

	PSymbolData pSymbolData = new SymbolData;
	if( !pSymbolData )
	{
		return TRUE;
	}

	pSymbolData->SetSymbolData( *pSymInfo_i );

    
    pModSymColl->AddSymbol( pSymbolData );

    
    return TRUE;

}
