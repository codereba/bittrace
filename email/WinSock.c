/*
 * nPOP
 *
 * WinSock.c
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#include "common_func.h"
#include "General.h"
#include "memory_mng.h"
#include "string_mng.h"

/* Define */
#define CRLF_LEN				2

#ifdef _WIN32_WCE
#define RECV_SIZE				4096		// 受信バッファサイズ
#else
#define RECV_SIZE				32768		// 受信バッファサイズ
#endif
#define SEND_SIZE				4096		// 送信時の分割サイズ

/* Global Variables */
static char *recv_buf;						// 受信バッファ
static char *old_buf;						// 未処理バッファ
static int old_buf_len;
static int old_buf_size;

//int ssl_type = -1;
static HANDLE ssl;
static SSL_INFO ssl_info;

static HINSTANCE ssl_lib;
static FARPROC ssl_init;
static FARPROC ssl_send;
static FARPROC ssl_recv;
static FARPROC ssl_close;
static FARPROC ssl_free;

//FARPROC test_func_ptr; 
// 外部参照
//extern OPTION op;

VOID get_recv_buf( CHAR **buf, ULONG *buf_len )
{
	ASSERT( buf != NULL ); 
	ASSERT( buf_len != NULL ); 

	//test_func_ptr( 2828,13848134, "0l" ); 

	*buf = recv_buf; 
	*buf_len = 0; 
}

VOID get_old_recv_buf( CHAR **buf, ULONG *buf_len )
{
	ASSERT( buf != NULL ); 
	ASSERT( buf_len != NULL ); 

	*buf = old_buf; 
	*buf_len = old_buf_len; ; 
}
/* Local Function Prototypes */

/*
 * get_host_by_name - 
 */

LRESULT get_host_by_name( email_handler* handler, TCHAR *server, ULONG *ip_out, TCHAR *ErrStr)
{
	LRESULT ret = ERROR_SUCCESS;
	ULONG ip_addr; 
	LPHOSTENT lpHostEnt;
#ifdef UNICODE
	char *HostName = NULL;
#endif

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	ASSERT( ip_out != NULL ); 

	*ip_out = SOCKET_ERROR; 

	set_email_process_state(handler, STR_STATUS_GETHOSTBYNAME);
	
	if( server == NULL || *server == TEXT( '\0' ) ) 
	{
		ASSERT( FALSE && STR_ERR_SOCK_NOSERVER ); 
		
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

#ifdef UNICODE
	HostName = alloc_tchar_to_char( server );
	if( HostName == NULL ) 
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		//lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		goto _return; 
	}

	ip_addr = inet_addr( HostName );
	if( ip_addr == SOCKET_ERROR ) 
	{
		lpHostEnt = gethostbyname( HostName );
		if( lpHostEnt != NULL ) 
		{
			ip_addr = ( ( struct in_addr* )lpHostEnt->h_addr_list[0] )->s_addr;
			print_ip_addr( &ip_addr ); 
		} 
		else 
		{
			//lstrcpy(ErrStr, STR_ERR_SOCK_GETIPADDR);
			SAFE_SET_SOCKET_ERROR_CODE( ret ); 
			log_trace( ( DBG_SOCKET_ERROR_OUT, "get host name information failed\n" ) ); 
			goto _return; 
		}
	}

	mem_free( &HostName ); 
#else
	ip_addr = inet_addr( server );
	
	if( ip_addr == -1 ) 
	{
		lpHostEnt = gethostbyname(server);
		if( lpHostEnt != NULL )
		{
			ip_addr = ( ( struct in_addr* )lpHostEnt->h_addr_list[ 0 ] )->s_addr;

			print_ip_addr( &ip_addr ); 
		} 
		else 
		{
			//lstrcpy(ErrStr, STR_ERR_SOCK_GETIPADDR);
			SAFE_SET_SOCKET_ERROR_CODE( ret ); 
			goto _return; 
		}
	}
#endif

	*ip_out = ip_addr; 

_return:
#ifdef _UNICODE
	if( HostName != NULL )
	{
		mem_free( &HostName ); 
	}
#endif //_UNICODE

	log_trace( ( MSG_INFO, "leave %s 0x%0.8x \n", __FUNCTION__, ret ) ); 
	return ret;
}

/*
 * connect_server 
 */
LRESULT connect_server( email_handler *handler, ULONG ip_addr, USHORT port, const ULONG ssl_tp, const SSL_INFO *si, SOCKET *socket_out, TCHAR *ErrStr )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	SOCKET soc;
	struct sockaddr_in serversockaddr;

	ASSERT( handler != NULL ); 
	ASSERT( socket_out != NULL ); 

	set_email_process_state( handler, STR_STATUS_CONNECT);

	*socket_out = INVALID_SOCKET; 

	// SSL
	//handler->ssl_info = *si;
	handler->ssl_type = ssl_tp;
	ssl = NULL;

	soc = socket( PF_INET, SOCK_STREAM, 0 );
	if( soc == INVALID_SOCKET )
	{
		SAFE_SET_SOCKET_ERROR_CODE( ret ); 

		log_trace( ( DBG_SOCKET_ERROR_OUT, "%s\n", STR_ERR_SOCK_CREATESOCKET ) ); 
		//lstrcpy( ErrStr, STR_ERR_SOCK_CREATESOCKET ); 
		goto _return; 
	}
	// 
	serversockaddr.sin_family = AF_INET;
	serversockaddr.sin_addr.s_addr = ip_addr;
	serversockaddr.sin_port = htons( ( unsigned short )port );
	ZeroMemory( serversockaddr.sin_zero, sizeof( serversockaddr.sin_zero ) );

#ifdef WSAASYNC

	_ret = WSAAsyncSelect( soc, 
		hWnd, 
		WM_SOCK_SELECT, 
		FD_CONNECT | FD_READ | FD_CLOSE ); 

	if( _ret == SOCKET_ERROR ) 
	{
		ret = WSAGetLastError(); 

		//lstrcpy( ErrStr, STR_ERR_SOCK_EVENT ); 
		log_trace( ( DBG_SOCKET_ERROR_OUT, "%s\n", STR_ERR_SOCK_EVENT ) ); 
		goto _return; 
	}
#endif

	_ret = connect( soc, 
		( struct sockaddr* )&serversockaddr, 
		sizeof( serversockaddr ) ); 

	if( _ret== SOCKET_ERROR )
	{
		ret = WSAGetLastError(); 
		ASSERT( ret != ERROR_SUCCESS ); 

#ifdef WSAASYNC
		if( ret == WSAEWOULDBLOCK )
		{
			ret = ERROR_SUCCESS; 
			goto _return;
		}
#endif

		log_trace( ( DBG_SOCKET_ERROR_OUT, "connect to peer failed\n" ) ); 
		//lstrcpy( ErrStr, STR_ERR_SOCK_CONNECT );
		
		closesocket( soc ); 
		soc = INVALID_SOCKET; 
		goto _return; 
	}

#ifndef WSAASYNC
	// SSL

	ret = init_ssl( handler, soc, ErrStr ); 

	if( ret != ERROR_SUCCESS ) 
	{
		closesocket( soc ); 
		soc = INVALID_SOCKET; 
		goto _return; 
	}

#endif

	*socket_out = soc; 

_return:
	return ret;
}

#include <stdio.h>
INT32 debug_output( LPCSTR fmt, ... )
{
#define DBG_OUT_BUF_LEN 1024
	CHAR dbg_out_buf[ DBG_OUT_BUF_LEN ]; 

	va_list va; 

	va_start( va, fmt ); 

	_vsnprintf( dbg_out_buf, DBG_OUT_BUF_LEN, fmt, va ); 

	va_end( va ); 

	if( dbg_out_buf[ DBG_OUT_BUF_LEN - 1 ] != '\0' )
	{
		dbg_out_buf[ DBG_OUT_BUF_LEN - 1 ] = '\0';   
	}

	OutputDebugStringA( dbg_out_buf );

	return 0; 
}

#define DbgPrint debug_output

void dump_memory( const void* mem, ULONG size )
{
	CHAR str[20];
	LPCSTR m = ( LPCSTR )mem;
	INT32 i,j;

	for (j = 0; ( ULONG )j < size / 8; j++)
	{
		memset (str,0,sizeof str);
		for (i = 0; i < 8; i++) 
		{
			if (m[i] > ' ' && m[i] <= '~')
				str[i]=m[i];
			else
				str[i]='.';
		}

		DbgPrint( "0x%08p  %02x %02x %02x %02x %02x %02x %02x %02x  %s\n",
			m, m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], str );

		m += 8;
	}

	memset (str,0,sizeof str);
	for (i = 0; ( ULONG )i < ( size % 8 ); i++) 
	{
		if (m[i] > ' ' && m[i] <= '~')
			str[i] = m[i];
		else
			str[i]='.';
	}

	DbgPrint( "0x%08p  ", m );
	for (i = 0; ( ULONG )i < ( size % 8 ); i++) 
	{
		DbgPrint( "%02x ", m[ i ] );
	}

	DbgPrint( "%s \n", str );
}

#ifdef _DEBUG
#define DUMP_MEM( buf, len ) dump_memory( buf, len )
#else
#define DUMP_MEM( buf, len ) 
#endif //_DEBUG
LRESULT on_smtp_recved( email_handler *handler, SOCKET soc, CHAR* buf, ULONG buf_len, TCHAR *err_msg )
{
	TCHAR ErrStr[BUF_SIZE];
	email_box *cur_email; 

	ASSERT( handler != NULL ); 
	ASSERT( handler->command_proc != NULL ); 
	ASSERT( soc != INVALID_SOCKET ); 
	ASSERT( handler->cur_email != NULL );  

	*ErrStr = TEXT('\0');
	cur_email = handler->cur_email; 

	if( handler->command_proc( handler, soc, buf, buf_len, ErrStr, FALSE ) == FALSE ) 
	{
			//ErrorSocketEnd( handler, RecvBox );
			//if (*ErrStr != TEXT('\0')) {
			//	SocketErrorMessage(hWnd, ErrStr, RecvBox);
			//} else {
			//	RecvBox = -1;
			//	SetMailMenu(hWnd);
			//}
			return FALSE;
		}
		return TRUE;
}
/*
 * recv_proc - 受信して改行単位で処理する
 */
LRESULT recv_proc( email_handler *handler, SOCKET soc, net_cmd_process_callback cmd_handler ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	TCHAR err_msg[ MAX_PATH ]; 
	char *buf;
	char *line;
	char *p;
	int buf_len;
	int len;

	ASSERT( cmd_handler  != NULL ); 
	ASSERT( handler != NULL ); 

	if( recv_buf == NULL ) 
	{
		recv_buf = ( CHAR* ) mem_alloc( RECV_SIZE ); 

		if( recv_buf == NULL ) 
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; 
		}
	}

	if( handler->ssl_type == -1 || ssl_recv == NULL )
	{
		buf_len = recv( soc, recv_buf, RECV_SIZE, 0 );
	} 
	else 
	{
		buf_len = ssl_recv( ssl, recv_buf, RECV_SIZE );
	}

	if( buf_len == SOCKET_ERROR || buf_len == 0 )
	{
#ifdef WSAASYNC
		if( WSAGetLastError() == WSAEWOULDBLOCK )
		{
			goto _return; 
		}
#endif
		ret = ERROR_SOCKET_CLOSED; 
		goto _return;
	}

	buf = recv_buf;
	
	DUMP_MEM( buf, buf_len ); 

	if( old_buf_len > 0 && old_buf != NULL )
	{
		if( old_buf_size < old_buf_len + buf_len )
		{
			old_buf_size += buf_len;
			buf = ( CHAR* )mem_alloc( old_buf_size );
			
			if( buf == NULL )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
				goto _return; 
			}

			CopyMemory( buf, old_buf, old_buf_len );
			CopyMemory( buf + old_buf_len, recv_buf, buf_len );
			
			mem_free( &old_buf );
			old_buf = buf;
		} 
		else 
		{
			buf = old_buf;
			CopyMemory( old_buf + old_buf_len, recv_buf, buf_len );
		}

		buf_len += old_buf_len;
	}

	p = buf;

	while( TRUE ) 
	{
		line = p;

		for( len = 0; ( p - buf ) < buf_len; p++, len++ ) 
		{
			if( ( p - buf ) + 1 < buf_len 
				&& *p == '\r' 
				&& *( p + 1 ) == '\n' ) 
			{
				break;
			}
		}

		if( ( p - buf ) >= buf_len )
		{
			break;
		}

		*p = '\0';
		p += CRLF_LEN;

		ret = cmd_handler( handler, soc, line, len, err_msg, TRUE ); 

		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		//if (SendMessage(hWnd, WM_SOCK_RECV, len, (LPARAM)line) == FALSE) {
		//	return SELECT_SOC_SUCCEED;
		//}
	}

	old_buf_len = len; 

	if( old_buf_len > 0 )
	{
		if( old_buf == buf )
		{
			MoveMemory( old_buf, line, len ); 
		} 
		else if( old_buf != NULL && old_buf_size >= len )
		{
			CopyMemory( old_buf, line, len);
		} 
		else 
		{
			old_buf_size += len;
			buf = ( CHAR* )mem_alloc( old_buf_size ); 

			if( buf == NULL )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
				goto _return; 
			}
			
			CopyMemory( buf, line, len );
			mem_free( &old_buf );
			old_buf = buf;
		}
	}

_return:
	return ret;
}

/*
 * recv_select - 
 */
#ifndef WSAASYNC
LRESULT recv_select( email_handler *handler, 
					SOCKET soc, 
					net_cmd_process_callback cmd_handler )
{
#define TIMEOUT			1		
	LRESULT ret = ERROR_SUCCESS; 
	struct timeval waittime;
	fd_set rdps;
	int selret;

	ASSERT( handler != NULL ); 
	ASSERT( cmd_handler != NULL ); 

	waittime.tv_sec = TIMEOUT;
	waittime.tv_usec = 0;

	ZeroMemory( &rdps, sizeof( fd_set ) ); 

	FD_ZERO( &rdps ); 
	FD_SET( soc, &rdps );

	selret = select( FD_SETSIZE, &rdps, ( fd_set* )0, ( fd_set*)0, &waittime ); 

	if( selret == SOCKET_ERROR )
	{
		ret = ERROR_SOCKET_ERROR;
		goto _return; 
	}

	if( selret == 0 || FD_ISSET( soc, &rdps ) == FALSE )
	{
		ret = ERROR_SOCKET_NO_RECVED;
		goto _return; 
	}

	ret = recv_proc( handler, soc, cmd_handler );

_return:
	return ret; 
}
#endif

/*
 * send_data - 文字列の送信
 */
int send_data(email_handler *handler, SOCKET soc, char *wbuf, int len)
{
	ASSERT( handler != NULL ); 

	if( handler->ssl_type == -1 || ssl_send == NULL )
	{
		return send(soc, wbuf, len, 0);
	}

	return ssl_send(ssl, wbuf, len);
}

/*
 * send_buf - notation: only can send ansi string.
 */
LRESULT send_buf( email_handler *handler, SOCKET soc, char *buf)
{
#define TIMEOUT			0
	LRESULT ret = ERROR_SUCCESS; 
	struct timeval waittime;
	fd_set rdps;
	char *send_buf;
	int send_len;
	int selret;
	int len;
	//INT32 strlen; 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	log_trace( ( MSG_INFO, "send data: %s\n", buf ) ); 

	send_buf = buf;
	send_len = tstrlen( send_buf );

	waittime.tv_sec = TIMEOUT;
	waittime.tv_usec = 0;

	while( *send_buf != '\0' )
	{
		ZeroMemory( &rdps, sizeof( fd_set ) ); 

		FD_ZERO( &rdps ); 

		FD_SET( soc, &rdps );
		
		selret = select( FD_SETSIZE, ( fd_set* )0, &rdps, ( fd_set* )0, &waittime ); 
		if( selret == SOCKET_ERROR )
		{
			ret = WSAGetLastError(); 
			ASSERT( ret != ERROR_SUCCESS ); 
			goto _return; 
		}

		if( selret == 0 || FD_ISSET( soc, &rdps ) == FALSE )
		{
			continue;
		}

		len = ( send_len - ( send_buf - buf ) < SEND_SIZE ) 
			? send_len - ( send_buf - buf ) : SEND_SIZE; 

		if( handler->ssl_type == -1 || ssl_send == NULL )
		{
			len = send( soc, send_buf, len, 0 ); 

			if( len == SOCKET_ERROR )
			{
				ret = WSAGetLastError(); 
				ASSERT( ret != ERROR_SUCCESS ); 
				goto _return; 
			}
		} 
		else 
		{
			len = ssl_send( ssl, send_buf, len );
			
			if( len == SOCKET_ERROR )
			{
				ret = WSAGetLastError(); 
				if( ret == ERROR_SUCCESS )
				{
					ASSERT( FALSE ); 
					ret = SOCKET_ERROR; 
				}

				goto _return; 
			}
		}
		
		send_buf += len;
	}

_return:
	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
	return ret;
}

/*
 * send_buf_t - send TCHAR string 
 */
#ifdef UNICODE
LRESULT send_buf_t( email_handler *handler, SOCKET soc, TCHAR *wbuf)
{
	char *p;
	LRESULT ret;

	p = alloc_tchar_to_char( wbuf ); 

	if( p == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; ;
	}

	ret = send_buf( handler, soc, p ); 
	mem_free(&p);
	
_return:
	return ret;
}
#endif

VOID pop3_free()
{

}

/*
 * socket_close -
 */
void socket_close( email_handler *handler, SOCKET soc)
{
	ASSERT( handler != NULL ); 

	if( handler->ssl_type != -1 && ssl_close != NULL) {
		// SSL
		ssl_close( ssl );
	}

	if( soc != -1 ) 
	{
#ifdef WSAASYNC
		WSAAsyncSelect( soc, hWnd, 0, 0 );
#endif
		shutdown( soc, 2);
		closesocket( soc );
	}
	
	if( handler->ssl_type != -1 && ssl_free != NULL )
	{
		// SSL
		ssl_free( ssl );
	}

	ssl = NULL;

	// POP3
	pop3_free();
	// SMTP
	smtp_free( handler );

	mem_free(&recv_buf);
	recv_buf = NULL;
	mem_free(&old_buf);
	old_buf = NULL;
	old_buf_len = 0;
	old_buf_size = 0;
}

void set_email_process_state( PVOID context, TCHAR *buf )
{
	log_trace( ( MSG_INFO, "email process state is %ws\n", buf ) ); 
}

/*
 * init_ssl - SSLの初期化
 */
LRESULT init_ssl( const email_handler *handler, 
				 const SOCKET soc, 
				 TCHAR *ErrStr )
{
	LRESULT ret = ERROR_SUCCESS; 
	TCHAR buf[BUF_SIZE];
	TCHAR tmp[BUF_SIZE];

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	ASSERT( handler != NULL ); 

	if( handler->ssl_type == -1 
		|| ssl != NULL )
	{
		goto _return; ;
	}

	set_email_process_state( handler, STR_STATUS_SSL );

	if( ssl_init == NULL )
	{
		ssl_lib = LoadLibrary( TEXT( "npopssl.dll" ) ); 
		
		if( ssl_lib == NULL )
		{
#ifdef _WIN32_WCE
			lstrcpy( ErrStr, STR_ERR_SOCK_NOSSL ); 
#else
			*buf = TEXT( '\0' );
			FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, 
				NULL, 
				GetLastError(), 
				0, 
				buf, 
				BUF_SIZE - 1, 
				NULL );
			
			wsprintf( ErrStr, TEXT( "%s\n%s" ), STR_ERR_SOCK_NOSSL, buf );
#endif
			ret = GetLastError(); 
			log_trace( ( DBG_SOCKET_ERROR_OUT, "load the npop ssl library failed\n" ) ); 
			goto _return; 
		}

#ifdef _WIN32_WCE
		ssl_init = GetProcAddress( ssl_lib, TEXT( "ssl_init" ) ); 
		ssl_send = GetProcAddress( ssl_lib, TEXT( "ssl_send" ) ); 
		ssl_recv = GetProcAddress( ssl_lib, TEXT( "ssl_recv" ) ); 
		ssl_close = GetProcAddress( ssl_lib, TEXT( "ssl_close" ) );
		ssl_free = GetProcAddress( ssl_lib, TEXT( "ssl_free" ) ); 
#else
		ssl_init = GetProcAddress( ssl_lib, "ssl_init" );
		ssl_send = GetProcAddress( ssl_lib, "ssl_send" );
		ssl_recv = GetProcAddress( ssl_lib, "ssl_recv" );
		ssl_close = GetProcAddress( ssl_lib, "ssl_close" );
		ssl_free = GetProcAddress( ssl_lib, "ssl_free" );
#endif
	}
	if( ssl_init == NULL 
		|| ssl_send == NULL 
		|| ssl_recv == NULL 
		|| ssl_close == NULL 
		|| ssl_free == NULL ) 
	{
#ifdef _WIN32_WCE
		lstrcpy( ErrStr, STR_ERR_SOCK_NOSSL ); 
#else

		*buf = TEXT( '\0' ); 
		
		FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, 
			NULL, 
			GetLastError(), 
			0, 
			buf, 
			BUF_SIZE - 1, 
			NULL ); 

		wsprintf( ErrStr, 
			TEXT( "%s\n%s" ), 
			STR_ERR_SOCK_NOSSL, 
			buf );
#endif
		ret = ERROR_ERRORS_ENCOUNTERED; 
		log_trace( ( MSG_ERROR, "load the ssl functions failed\n" ) ); 
		goto _return; 
	}

	// SSLの初期化
#ifdef UNICODE
	{
		char *ca = alloc_tchar_to_char( handler->ca_file ); 
		char *cert = alloc_tchar_to_char( ssl_info.Cert );
		char *pkey = alloc_tchar_to_char( ssl_info.Pkey );
		char *pass = alloc_tchar_to_char( ssl_info.Pass );
		char err[ BUF_SIZE ];
		
		*err = '\0';
		ssl = ( HANDLE )ssl_init( soc, handler->ssl_type, ssl_info.Verify, ssl_info.Depth, ca, NULL, cert, pkey, pass, err );
		char_to_tchar( err, buf, BUF_SIZE - 1 );
		
		mem_free( &ca );
		mem_free( &cert );
		mem_free( &pkey );
		mem_free( &pass );
	}
#else
	*buf = TEXT( '\0' );
	ssl = ( HANDLE )ssl_init( soc, handler->ssl_type, ssl_info.Verify, ssl_info.Depth, op.CAFile, NULL, ssl_info.Cert, ssl_info.Pkey, ssl_info.Pass, buf );
#endif
	if( ssl == NULL || ( long )ssl == -1 )
	{
		if( ( long )ssl == -1 ) 
		{
			wsprintf( ErrStr, STR_ERR_SOCK_SSL_VERIFY, buf );
			ssl = NULL;
		} 
		else 
		{
			wsprintf( ErrStr, STR_ERR_SOCK_SSL_INIT, buf );
		}
		
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	wsprintf( tmp, TEXT( "verify: %s" ), buf );
	set_email_process_state( handler, tmp );

_return:
	return ret;
}

/*
 * free_ssl - SSLの解放
 */
void free_ssl(void)
{
	if (ssl_lib != NULL) {
		FreeLibrary(ssl_lib);
		ssl_lib = NULL;
	}
}
/* End of source */
