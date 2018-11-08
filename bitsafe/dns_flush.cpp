/*
 *
 * Copyright 2010 JiJie Shi(wx:AIChangeLife)
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

