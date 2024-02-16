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