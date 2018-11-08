#include "common_func.h"


INT32 GetFileVersionNum( LPCWSTR FileName, 	VS_FIXEDFILEINFO* FixedFileInfo )
{
	INT32 Ret = 0; 
	INT32 _Ret; 
	ULONG ErrorCode; 
	LPVOID VersionInfo = NULL; 
	ULONG FileVersionSize; 

	do 
	{
		if( NULL == FixedFileInfo )
		{
			Ret = ERROR_INVALID_PARAMETER; 
			break;
		}

		FileVersionSize = GetFileVersionInfoSize( FileName, NULL ); 

		if( FileVersionSize == 0 )
		{
			ErrorCode = GetLastError(); 

			dbg_print( MSG_FATAL_ERROR, "get file %ws version information size error %u\n", 
				FileName, 
				ErrorCode ); 

			Ret = ERROR_ERRORS_ENCOUNTERED; 
			break;
		}

		VersionInfo = malloc( FileVersionSize ); 
		if( VersionInfo == NULL )
		{
			Ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		_Ret = GetFileVersionInfo( FileName, NULL, FileVersionSize, VersionInfo );

		if( _Ret == FALSE )
		{
			ErrorCode = GetLastError(); 

			dbg_print( MSG_FATAL_ERROR, "get file %ws version information error %u\n", 
				FileName, 
				ErrorCode ); 

			Ret = ErrorCode; 
			break; 
		}

		{

			UINT FixedFileInfoSize; 
			VS_FIXEDFILEINFO* _FixedFileInfo; 

			_Ret = VerQueryValueW( VersionInfo, L"\\", ( LPVOID* )&_FixedFileInfo, &FixedFileInfoSize ); 

			if( _Ret == FALSE )
			{
				ErrorCode = GetLastError(); 

				dbg_print( MSG_FATAL_ERROR, "get the detail of the file %ws version information error %u\n", 
					FileName, 
					ErrorCode ); 

				Ret = ErrorCode; 
				break; 
			}

			if( FixedFileInfoSize < sizeof( *FixedFileInfo ) )
			{
				memcpy( FixedFileInfo, _FixedFileInfo, FixedFileInfoSize ); 

				if( FIELD_OFFSET( VS_FIXEDFILEINFO, dwProductVersionMS ) > FixedFileInfoSize )
				{
					dbg_print( MSG_FATAL_ERROR, "get the size %u of the file %ws version information too small\n", 
						FileName, 
						FixedFileInfoSize ); 

					Ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}
			}
			else
			{
				memcpy( FixedFileInfo, _FixedFileInfo, sizeof( *FixedFileInfo ) ); 
			}

		}

	}while( FALSE );

	if( VersionInfo != NULL )
	{
		free( VersionInfo ); 
	}

	return Ret; 
}


