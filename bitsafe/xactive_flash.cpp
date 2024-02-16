/*
 * Copyright 2010-2024 JiJie.Shi.
 *
 * This file is part of bittrace.
 * Licensed under the Gangoo License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
