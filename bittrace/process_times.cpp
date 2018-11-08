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

#include "StdAfx.h"
#include "process_times.h"
#include "common_func.h"

ProcessTimes::ProcessTimes()
{}

ProcessTimes::~ProcessTimes()
{}

#define MAX_TIME_STRING 64
LRESULT ProcessTimes::GetProcessTimes( HANDLE hProcess )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG wait_ret; 

    FILETIME ftStartTime = { 0 },
             ftExitTime = { 0 },
             ftKernelModeTime = { 0 },
             ftUserModeTime = { 0 };

	do 
	{
		//_ret = ::GetExitCodeProcess( hProcess, &exit_code )
		//if( FALSE == _ret )
		//{
		//}

		wait_ret = WaitForSingleObject( hProcess, 0 ); 

		if( ::GetProcessTimes( hProcess, 
			&ftStartTime, 
			&ftExitTime, 
			&ftKernelModeTime, 
			&ftUserModeTime ) == FALSE )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		sprint_file_time( m_csProcessStartTime.GetBufferSetLength( MAX_TIME_STRING ), MAX_TIME_STRING, &ftStartTime );
		
		if( wait_ret != WAIT_OBJECT_0 )
		{
			m_csProcessExitTime = L""; 
		}
		else
		{
			sprint_file_time( m_csProcessExitTime.GetBufferSetLength( MAX_TIME_STRING ), MAX_TIME_STRING, &ftExitTime );
		}

		sprint_file_time( m_csProcessKernelTime.GetBufferSetLength( MAX_TIME_STRING ), MAX_TIME_STRING, &ftKernelModeTime );
		sprint_file_time( m_csProcessUserTime.GetBufferSetLength( MAX_TIME_STRING ), MAX_TIME_STRING, &ftUserModeTime );
	}while( FALSE );

    return ret;
}