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
 
#include "common_func.h"
#include "dns_flush.h"

typedef BOOL ( WINAPI *dns_flush_func )( VOID );

HMODULE dns_mod = NULL; 

dns_flush_func dns_flush = NULL; 

LRESULT init_dns_mod()
{
	LRESULT ret = ERROR_SUCCESS; 

	dns_mod = LoadLibrary( _T( DNS_MOD_NAME ) ); 
	if( dns_mod == NULL )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	dns_flush = ( dns_flush_func )GetProcAddress( dns_mod, DNS_FLUSH_FUNC_NAME ); 
	if( dns_flush == NULL )
	{
		ret = ERROR_NOT_FOUND; 
		goto _return; 
	}

_return:
	return ret; 
}

LRESULT release_dns_mod()
{
	LRESULT ret = ERROR_SUCCESS; 
	if( dns_mod == NULL ) 
	{
		goto _return; 
	}

	ret = FreeLibrary( dns_mod ); 

_return:
	return ret; 
}

INT32 flush_dns()
{
	INT32 ret = FALSE; 
	if( dns_flush == NULL )
	{
		goto _return; 
	}

	ret = dns_flush(); 

_return:
	return ret; 
}

