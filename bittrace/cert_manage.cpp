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
#include "cert_manage.h"
#include <Wincrypt.h>
#pragma comment(lib, "crypt32.lib")


LRESULT WINAPI check_cedereba_cert_installed()
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	HCERTSTORE sys_store = NULL; 
	PCCERT_CONTEXT cert_context = NULL; 

	do 
	{
		sys_store = CertOpenStore( CERT_STORE_PROV_SYSTEM, 
			0, 
			NULL, 
			CERT_STORE_OPEN_EXISTING_FLAG | CERT_SYSTEM_STORE_LOCAL_MACHINE, 
			L"ROOT" ); 

		if( NULL == sys_store )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			
			dbg_print( DBG_MSG_AND_ERROR_OUT, 
				"get the system store codereba of local host error\n" ); 
			break; 
		}

		cert_context = CertFindCertificateInStore( sys_store, 
			CODEREBA_CERT_ENCODE_TYPE,            // Use X509_ASN_ENCODING
			0,                           // No dwFlags needed 
			CERT_FIND_SUBJECT_STR,       // Find a certificate with a
			// subject that matches the string
			// in the next parameter
			L"codereba",// The Unicode string to be found
			// in a certificate's subject
			NULL); // NULL for the first call to the
			// function
			// In all subsequent
			// calls, it is the last pointer
			// returned by the function
		
		if( NULL == cert_context )
		{
			dbg_print( DBG_MSG_AND_ERROR_OUT, "codereba certificate was found in codereba store. \n");
			
			SAFE_SET_ERROR_CODE( ret ); 
			//ret = ERROR_NOT_FOUND; 
			break; 
		}

		CertFreeCertificateContext( cert_context ); 
		_ret = CertCloseStore( sys_store, CERT_CLOSE_STORE_FORCE_FLAG ); 
		if( _ret == FALSE )
		{
			dbg_print( DBG_MSG_AND_ERROR_OUT, "close certicate store error" ); 
		}

		cert_context = NULL; 
		sys_store = NULL; 

		sys_store = CertOpenStore( CERT_STORE_PROV_SYSTEM, 
			0, 
			NULL, 
			CERT_STORE_OPEN_EXISTING_FLAG | CERT_SYSTEM_STORE_LOCAL_MACHINE, 
			L"TRUSTEDPUBLISHER" ); 

		if( NULL == sys_store )
		{
			SAFE_SET_ERROR_CODE( ret ); 

			dbg_print( DBG_MSG_AND_ERROR_OUT, 
				"get the system store codereba of local host error\n" ); 
			break; 
		}

		cert_context = CertFindCertificateInStore( sys_store, 
			CODEREBA_CERT_ENCODE_TYPE,            // Use X509_ASN_ENCODING
			0,                           // No dwFlags needed 
			CERT_FIND_SUBJECT_STR,       // Find a certificate with a
			// subject that matches the string
			// in the next parameter
			L"codereba",// The Unicode string to be found
			// in a certificate's subject
			NULL); // NULL for the first call to the
		// function
		// In all subsequent
		// calls, it is the last pointer
		// returned by the function

		if( NULL == cert_context )
		{
			dbg_print( DBG_MSG_AND_ERROR_OUT, "codereba certificate was found in codereba store. \n");

			SAFE_SET_ERROR_CODE( ret ); 
			//ret = ERROR_NOT_FOUND; 
			break; 
		}

	} while ( FALSE );

	if( cert_context != NULL )
	{
		CertFreeCertificateContext( cert_context ); 
	}

	if( NULL != sys_store )
	{
		_ret = CertCloseStore( sys_store, CERT_CLOSE_STORE_FORCE_FLAG ); 
		if( _ret == FALSE )
		{
			dbg_print( DBG_MSG_AND_ERROR_OUT, "close certicate store error" ); 
		}
	}

	return ret; 
}


#define CREATE_NEW_CERT_STORE_IF_NOT_EXIST 0x00000001
LRESULT WINAPI add_cert_to_system_store( PCCERT_CONTEXT cert_context, LPCWSTR store_name, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	ULONG error_code; 

	HCERTSTORE sys_store = NULL; 

	do 
	{

		if( flags & CREATE_NEW_CERT_STORE_IF_NOT_EXIST )
		{
			sys_store = CertOpenStore( CERT_STORE_PROV_SYSTEM, 0, NULL, CERT_STORE_CREATE_NEW_FLAG | CERT_SYSTEM_STORE_LOCAL_MACHINE, store_name );
		}
		else
		{
			sys_store = CertOpenStore( CERT_STORE_PROV_SYSTEM, 0, NULL, CERT_STORE_OPEN_EXISTING_FLAG | CERT_SYSTEM_STORE_LOCAL_MACHINE, store_name );
		}

		if( NULL == sys_store )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			dbg_print( DBG_MSG_AND_ERROR_OUT, "get the system store %ws of local host error\n", store_name ); 
			break; 
		}

		_ret = CertAddCertificateContextToStore( sys_store, cert_context, CERT_STORE_ADD_NEW, NULL ); 

		if( _ret == TRUE )
		{
			break; 
		}

		error_code = GetLastError();

		if( CRYPT_E_EXISTS != error_code )
		{
			SAFE_SET_ERROR_CODE( ret ); 

			dbg_print( DBG_MSG_AND_ERROR_OUT, "add the certificate to system store error\n" ); 
			break; 
		}

		_ret = CertAddCertificateContextToStore( sys_store, cert_context, CERT_STORE_ADD_REPLACE_EXISTING, NULL ); 

		if( FALSE == _ret )
		{
			SAFE_SET_ERROR_CODE( ret ); 

			dbg_print( DBG_MSG_AND_ERROR_OUT, "add the certificate to system store error\n" ); 
			break; 
		}

	}while( FALSE );

	if( NULL != sys_store )
	{
		_ret = CertCloseStore( sys_store, CERT_CLOSE_STORE_FORCE_FLAG ); 
		if( _ret == FALSE )
		{
			dbg_print( DBG_MSG_AND_ERROR_OUT, "close certicate store error" ); 
		}
	}

	return ret; 
}

LRESULT WINAPI install_cedereba_cert()
{

	LRESULT ret = ERROR_SUCCESS; 
	LRESULT __ret; 
	ULONG error_code; 

	WCHAR file_name[ MAX_PATH ]; 
	HANDLE file_handle = INVALID_HANDLE_VALUE; 
	HANDLE map_section = NULL;
	PVOID cert_data = NULL; 
	ULONG file_size; 
	ULONG file_size_high; 
	PCCERT_CONTEXT cert_context = NULL; 

	do 
	{
		ret = add_app_path_to_file_name( file_name, ARRAYSIZE( file_name ), CODEREBA_CERTIFICATE_FILE_NAME, TRUE ); 
		if( ret != ERROR_SUCCESS )
		{
			dbg_print( MSG_FATAL_ERROR, "get file path in the application directory error\n" ); 
			break; 
		}

		file_handle = CreateFile( file_name, FILE_READ_DATA, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
		if( INVALID_HANDLE_VALUE == file_handle )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			dbg_print( DBG_MSG_AND_ERROR_OUT, "open the certification store file of codereba error" ); 
			break; 
		}

		map_section = CreateFileMapping( file_handle, 0, PAGE_READONLY, 0, 0, 0 ); 

		if( NULL == map_section )
		{
			dbg_print( DBG_MSG_AND_ERROR_OUT, "map the certification store file of codereba error" ); 

			break; 
		}

		cert_data = MapViewOfFile( map_section, FILE_MAP_READ, 0, 0, 0 ); 

		if( NULL == cert_data )
		{
			dbg_print( DBG_MSG_AND_ERROR_OUT, "get the mapped view of the certification store file of codereba error" ); 

			break; 
		}

		file_size = GetFileSize( file_handle, &file_size_high ); 
		if( file_size == INVALID_FILE_SIZE )
		{	 
			error_code = GetLastError(); 

			if( error_code != ERROR_SUCCESS )
			{
				dbg_print( DBG_MSG_AND_ERROR_OUT, "get the size of the certification store file of codereba error" ); 
				break; 
			}
		}

		if( file_size == 0 
			|| file_size_high != 0 )
		{
			dbg_print( DBG_MSG_AND_ERROR_OUT, "the size of the certification store file of codereba is invalid" ); 

			break; 
		}

		cert_context = CertCreateCertificateContext( CODEREBA_CERT_ENCODE_TYPE, ( BYTE* )cert_data, file_size ); 

		if( NULL == cert_context )
		{
			dbg_print( DBG_MSG_AND_ERROR_OUT, "get the context of the certification store file of codereba is invalid" ); 
			break; 
		}

		__ret = add_cert_to_system_store( cert_context, L"ROOT", 0 ); 
		if( __ret != ERROR_SUCCESS )
		{
			ret = __ret; 
			dbg_print( DBG_MSG_AND_ERROR_OUT, "add the certificate to system root store error" ); 

			//break; 
		}

		__ret = add_cert_to_system_store( cert_context, L"TRUSTEDPUBLISHER", 0 ); 
		if( __ret != ERROR_SUCCESS )
		{
			ret = __ret; 
			dbg_print( DBG_MSG_AND_ERROR_OUT, "add the certificate to system trusted publisher store error" ); 

			//break; 
		}

	}while( FALSE );

	if( NULL != cert_data )
	{
		UnmapViewOfFile( cert_data ); 
	}

	if( NULL != map_section )
	{
		CloseHandle( map_section ); 
	}

	if( INVALID_HANDLE_VALUE != file_handle )
	{
		CloseHandle( file_handle ); 
	}

	if( NULL != cert_context )
	{
		CertFreeCertificateContext( cert_context ); 
	}

	return ret; 
}
