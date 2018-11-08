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

#include "common_func.h"
#include "performance_test.h"

typedef struct _performance_data
{
	LPCWSTR desc; 
	ULONG begin; 
	ULONG end; 
	PVOID info; 
} performance_data, *pperformance_data; 

#ifdef new
#undef new 
#include <vector>
#endif //new 

std::vector< performance_data > performance_datas; 

LRESULT WINAPI perf_test_begin( LPCWSTR desc, PVOID info, PVOID *context )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG count; 

	do 
	{
		ASSERT( context != NULL 
			|| info != NULL ); 

#ifndef _DEBUG
		break; 
#endif //_DEBUG

		count = performance_datas.size(); 

		performance_datas.resize( count + 1 ); 

		if( performance_datas.size() <= count )
		{
			ASSERT( FALSE ); 
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		performance_datas[ count ].begin = GetTickCount(); 
		performance_datas[ count ].info = info; 
		performance_datas[ count ].desc = desc; 

		if( context != NULL )
		{
			*context = &performance_datas[ count ]; 
		}

	}while( FALSE );

	return ret; 
}

LRESULT WINAPI perf_test_end( PVOID context, PVOID info )
{
	LRESULT ret = ERROR_SUCCESS; 
	performance_data *performance_test; 
	std::vector<performance_data>::iterator it; 

	do 
	{
		ASSERT( context != NULL 
			|| info != NULL ); 

#ifndef _DEBUG
		break; 
#endif //_DEBUG

		if( context != NULL )
		{
			performance_test = ( performance_data* )context; 

			performance_test->end = GetTickCount(); 

			dbg_print( MSG_IMPORTANT, "%ws sign:0x%0.8x elapse:%u\n", 
				performance_test->desc == NULL ? L"" : performance_test->desc, 
				performance_test->info, 
				performance_test->end - performance_test->begin ); 

			for( it = performance_datas.begin(); it != performance_datas.end(); it ++ )
			{
				if( &( *it ) == context )
				{
					break; 
				}
			}

			if( it == performance_datas.end() )
			{
				ASSERT( FALSE ); 
			}
			else
			{
				performance_datas.erase( it ); 
			}
		}
		else
		{
			for( it = performance_datas.begin(); it != performance_datas.end(); it ++ )
			{
				if( it->info == info )
				{
					it->end = GetTickCount(); 

					dbg_print( MSG_IMPORTANT, "%ws sign:0x%0.8x elapse:%u\n", 
						it->desc == NULL ? L"" : it->desc, 
						it->info, 
						it->end - it->begin ); 
					break; 
				}
			}

			if( it == performance_datas.end() )
			{
				ASSERT( FALSE ); 
			}
			else
			{
				performance_datas.erase( it ); 
			}
		}

	}while( FALSE );

	return ret; 
}