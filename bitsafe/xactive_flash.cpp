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
 
 #ifdef _DEBUG_MEM_LEAKS
#undef _DEBUG_MEM_LEAKS
#endif //_DEBUG_MEM_LEAKS

#include "StdAfx.h"
#include "flash10a.tlh"
#include "xactive_flash.h"

//xml text: <ActiveX name="flash" clsid="{D27CDB6E-AE6D-11CF-96B8-444553540000}" mouse="false"/>

LRESULT play_flash( CActiveXUI* pActiveX, LPCTSTR flash_file_name )
{
	LRESULT ret = ERROR_SUCCESS; 
	IShockwaveFlash* pFlash = NULL; 
	BSTR response;

	ASSERT( pActiveX != NULL ); 
	ASSERT( flash_file_name != NULL ); 

	ret = pActiveX->GetControl( IID_IUnknown, ( void** )&pFlash ); 

	if( pFlash != NULL ) 
	{
		pFlash->put_WMode( _bstr_t( _T( "Transparent" ) ) );
		pFlash->put_Movie( _bstr_t( flash_file_name ) ); 

		pFlash->DisableLocalSecurity();
		pFlash->put_AllowScriptAccess( L"always" );
		
		pFlash->CallFunction( L"<invoke name=\"setButtonText\" returntype=\"xml\"><arguments><string>Click me!</string></arguments></invoke>", &response ); 
		
		pFlash->Release();
	} 

	return ret; 
}
