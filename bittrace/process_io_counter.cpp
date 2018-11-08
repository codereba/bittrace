#include "stdafx.h"
#include "process_io_counter.h"

ProcessIOCounters::ProcessIOCounters()
{}

ProcessIOCounters::~ProcessIOCounters()
{}


LRESULT ProcessIOCounters::GetProcessIoCounters( HANDLE ProcessHandle )
{
	LRESULT ret = ERROR_SUCCESS; 
    // Check process validity status

    do 
	{
		IO_COUNTERS sticCounter = { 0 };
		if( ProcessHandle != INVALID_HANDLE_VALUE)
		{
			break; 
		}

		if( !::GetProcessIoCounters( ProcessHandle, &sticCounter ))
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		// Get information
		GetReadOperationCount().Format( _T( "Read operation: %I64d" ), sticCounter.ReadOperationCount );
		GetWriteOperationCount().Format( _T( "Write operation: %I64d" ), sticCounter.WriteOperationCount );
		GetOtherOperationCount().Format( _T( "Other operation: %I64d" ), sticCounter.OtherOperationCount );
		GetReadTransferCount().Format( _T( "Read transfer: %I64d" ), sticCounter.ReadTransferCount );
		GetWriteTransferCount().Format( _T( "Write transfer: %I64d" ), sticCounter.WriteTransferCount );
		GetOtherTransferCount().Format( _T( "Other transfer: %I64d" ), sticCounter.OtherTransferCount );
	}while( FALSE );

    return ret; 
}