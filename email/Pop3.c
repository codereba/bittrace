/*
 * ERC
 *
 * Pop3.c (RFC 1939)
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
#include "global.h"
#include "md5.h"

/* Define */
#define OK_LEN					3			// "+OK" のバイト数

#ifdef _WIN32_WCE
#define MAIL_BUF_SIZE			4096		// メール受信用バッファの初期サイズ
#else
#define MAIL_BUF_SIZE			32768		// メール受信用バッファの初期サイズ
#endif
#define DOWNLOAD_SIZE			65535		// RETR未使用時の全文受信サイズ

#define HEAD_LINE				30			// ヘッダー長
#define LINE_LEN				80

#define REDRAWCNT				100			// ステータスバー再設定数

#define CMD_USER				"USER"
#define CMD_PASS				"PASS"
#define CMD_APOP				"APOP"
#define CMD_STLS				"STLS"
#define CMD_STAT				"STAT"
#define CMD_LIST				"LIST"
#define CMD_UIDL				"UIDL"
#define CMD_TOP					"TOP"
#define CMD_RETR				"RETR"
#define CMD_DELE				"DELE"
#define CMD_RSET				"RSET"
#define CMD_QUIT				"QUIT"

/* Global Variables */
static char *mail_buf = NULL;				// メール受信用バッファ
static int mail_buf_size;					// メール受信用バッファの実サイズ
static int mail_buf_len;					// メール受信用バッファ内の文字列長
static int mail_size = -1;					// メールサイズ
//static int list_get_no;						// 受信メール位置
//static int download_get_no;					// ダウンロードメール位置
//static int delete_get_no;					// 削除メール位置
static BOOL init_recv;						// 新着取得位置初期化フラグ
static BOOL mail_received;					// １件目受信フラグ
static BOOL receiving_uidl;					// UIDLレスポンス受信中
static BOOL receiving_data;					// メールデータ受信中
static BOOL disable_uidl;					// UIDLサポートフラグ
static BOOL disable_top;					// TOPサポートフラグ (op.ListDownload が 1 or TOPが未サポート)

typedef struct _UIDL_INFO {
	int no;
	TCHAR *uidl;
} UIDL_INFO;

static UIDL_INFO *ui;						// UIDLリスト
static int ui_size;							// UIDLリスト数
static int ui_pt;							// UIDL追加位置
static MAILITEM *uidl_item;					// UIDLをセットするメールアイテム

// 外部参照
//extern OPTION op;

//extern TCHAR *g_Pass;
//extern HWND hViewWnd;						// 表示ウィンドウ

//extern BOOL ShowMsgFlag;
//extern BOOL NewMail_Flag;
//extern BOOL EndThreadSortFlag;

//extern int PopBeforeSmtpFlag;

/* Local Function Prototypes */
static BOOL mail_buf_init( ULONG size );
static BOOL mail_buf_set(char *buf, int len);
static void mail_buf_free(void);
static BOOL uidl_init(int size);
static BOOL uidl_set(char *buf, int len);
static TCHAR *uidl_get(int get_no);
static int uidl_check(TCHAR *buf);
static void uidl_free(void);
//static void init_mailbox(HWND hWnd, email_box *mail_box, BOOL ShowFlag);
static char *skip_response(char *p);
static BOOL check_response(char *buf);
static BOOL check_message_id(char *buf, MAILITEM *mail_item, TCHAR *ErrStr, email_box *mail_box);
static int check_last_mail(email_handler *handler, SOCKET soc, BOOL chek_flag, TCHAR *ErrStr, email_box *mail_box, BOOL ShowFlag);
static BOOL send_command(email_handler *handler, SOCKET soc, TCHAR *Command, int Cnt, TCHAR *ErrStr);
static ULONG send_command_top(email_handler *handler, SOCKET soc, int Cnt, TCHAR *ErrStr, int len, int ret);
static LRESULT create_apop( char *buf, email_box *mail_box, TCHAR *sPass, TCHAR **digest );
static ULONG login_proc( email_handler *handler, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, email_box *mail_box);
static int list_proc_stat( email_handler *handler, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, email_box *mail_box, BOOL ShowFlag);
static int list_proc_uidl_all( email_handler *handler, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, email_box *mail_box, BOOL ShowFlag);
static int list_proc_uidl_check( email_handler *handler, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, email_box *mail_box, BOOL ShowFlag);
static int list_proc_uidl_set( email_handler *handler, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, email_box *mail_box, BOOL ShowFlag);
static ULONG list_proc_list( email_handler *handler, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, email_box *mail_box, BOOL ShowFlag);
static int list_proc_top( email_handler *handler, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, email_box *mail_box, BOOL ShowFlag);
static int exec_send_check_command( email_handler *handler, SOCKET soc, int get_no, TCHAR *ErrStr, email_box *mail_box);
static int exec_proc_init( email_handler *handler, SOCKET soc, INT32 mail_index, char *buf, int buflen, TCHAR *ErrStr, email_box *mail_box, BOOL ShowFlag);
static int exec_proc_retr( email_handler *handler, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, email_box *mail_box, BOOL ShowFlag);
static int exec_proc_uidl( email_handler *handler, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, email_box *mail_box, BOOL ShowFlag);
static int exec_proc_top( email_handler *handler, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, email_box *mail_box, BOOL ShowFlag);
static int exec_proc_dele( email_handler *handler, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, email_box *mail_box, BOOL ShowFlag);

/*
 * mail_buf_init - 
 */
static BOOL mail_buf_init( ULONG size )
{
	if( mail_buf == NULL || mail_buf_size < size )
	{
		mem_free( &mail_buf );
		
		mail_buf_size = size + 3; 

		mail_buf = ( CHAR* )mem_alloc( mail_buf_size ); 
	}

	mail_buf_len = 0; 
	return ( ( mail_buf == NULL ) ? FALSE : TRUE ); 
}

/*
 * mail_buf_set - メールバッファに文字列を追加
 */
static BOOL mail_buf_set(char *buf, int len)
{
	char *tmp;

	if( mail_buf_size < ( mail_buf_len + len + 2 + 1 ) )
	{
		mail_buf_size += ( MAIL_BUF_SIZE < len + 2 + 1 ) ? ( len + 2 + 1 ) : MAIL_BUF_SIZE;
		tmp = ( char* )mem_alloc( mail_buf_size ); 

		if( tmp == NULL )
		{
			mail_buf_size = 0;
			return FALSE;
		}

		CopyMemory( tmp, mail_buf, mail_buf_len );
		CopyMemory( tmp + mail_buf_len, buf, len );
		
		*( tmp + mail_buf_len + len + 0 ) = '\r';
		*( tmp + mail_buf_len + len + 1 ) = '\n';
		*( tmp + mail_buf_len + len + 2 ) = '\0';
		
		mail_buf_len += ( len + 2 );
		
		mem_free( &mail_buf );
		mail_buf = tmp;
	} 
	else 
	{
		CopyMemory( mail_buf + mail_buf_len, buf, len );
		*( mail_buf + mail_buf_len + len + 0 ) = '\r';
		*( mail_buf + mail_buf_len + len + 1 ) = '\n';
		*( mail_buf + mail_buf_len + len + 2 ) = '\0';
		
		mail_buf_len += ( len + 2 );
	}

	return TRUE;
}

/*
 * mail_buf_free - メールバッファを解放
 */
static void mail_buf_free(void)
{
	if (mail_buf != NULL) {
		mem_free(&mail_buf);
		mail_buf = NULL;
	}
}

/*
 * uidl_init - UIDLリストを初期化
 */
static BOOL uidl_init(int size)
{
	uidl_free();
	ui_size = size;
	ui_pt = 0;
	ui = (UIDL_INFO *)mem_calloc(sizeof(UIDL_INFO) * ui_size);
	return ((ui == NULL) ? FALSE : TRUE);
}

/*
 * uidl_set - UIDLリストに文字列を追加
 */
static BOOL uidl_set(char *buf, int len)
{
	char *p;

	if (ui_pt >= ui_size) {
		return FALSE;
	}

	ui[ui_pt].no = a2i(buf);
	for (p = buf; *p != ' ' && *p != '\0'; p++);		// 番号
	for (; *p == ' '; p++);								// 空白

	ui[ui_pt].uidl = alloc_char_to_tchar(p);
	if (ui[ui_pt].uidl == NULL) {
		return FALSE;
	}
	ui_pt++;
	return TRUE;
}

/*
 * uidl_get - UIDL取得
 */
static TCHAR *uidl_get(int get_no)
{
	int i;

	for (i = 0; i < ui_size; i++) {
		if (ui[i].no == get_no) {
			return ui[i].uidl;
		}
	}
	return NULL;
}

/*
 * uidl_check - UIDLがUIDLの一覧に存在するかチェック
 */
static int uidl_check(TCHAR *buf)
{
	int i;

	if (buf == NULL) {
		return 0;
	}
	for (i = 0; i < ui_size; i++) {
		if (lstrcmp(ui[i].uidl, buf) == 0) {
			return ui[i].no;
		}
	}
	return 0;
}

/*
 * uidl_free - UIDL
 */
static void uidl_free(void)
{
	int i;

	if( ui == NULL )
	{
		return;
	}

	for( i = 0; i < ui_size; i++ )
	{
		mem_free( &ui[ i ].uidl ); 
	}

	mem_free( &ui ); 
	ui = NULL; 
	ui_size = 0; 
}

/*
 * init_mailbox - 
 */
//static void init_mailbox(HWND hWnd, email_box *mail_box, BOOL ShowFlag)
//{
//	if (ShowFlag == TRUE) {
//		ListView_DeleteAllItems(GetDlgItem(hWnd, IDC_LISTVIEW));
//	}
//	item_free(mail_box->mail_item, mail_box->mail_num);
//	mem_free((void **)&mail_box->mail_item);
//	mail_box->mail_item = NULL;
//	mail_box->AllocCnt = mail_box->mail_num = 0;
//	mail_box->mail_index = 0;
//}

/*
 * skip_response - skip the one message of the all messages of one response  of pop3 protocol.
 */
static char *skip_response( CHAR *p )
{
	for(; *p != ' ' && *p != '\0'; p++);	// +OK
	for(; *p == ' '; p++);					// whitespace
	for(; *p != ' ' && *p != '\0'; p++ );	// message
	for( ; *p == ' '; p++ );				// whitespace
	
	return p;
}

/*
 * check_response - check response message of pop3 server
 */
static BOOL check_response( CHAR *buf )
{
	return ( ( tstrlen( buf ) < OK_LEN 
		|| *buf != '+' 
		|| !( *( buf + 1 ) == 'O' || *( buf + 1 ) == 'o' ) 
		|| !( *( buf + 2 ) == 'K' || *( buf + 2 ) == 'k' ) ) 
		
		? FALSE : TRUE ); 
}

/*
 * check_message_id - メッセージIDのチェック
 */
static BOOL check_message_id(char *buf, MAILITEM *mail_item, TCHAR *ErrStr, email_box *mail_box)
{
	char *content;
#ifdef UNICODE
	char *p;
#endif

	// Message-Id
	content = item_get_message_id(buf);
	if (content == NULL) {
		lstrcpy(ErrStr, STR_ERR_SOCK_NOMESSAGEID);
		return FALSE;
	}

#ifdef UNICODE
	p = NULL;
	if (mail_item->MessageID != NULL) {
		p = alloc_tchar_to_char(mail_item->MessageID);
		if (p == NULL) {
			mem_free(&content);
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return FALSE;
		}
	}
	if (mail_item->MessageID == NULL || tstrcmp(p, content) != 0) {
		mem_free(&content);
		mem_free(&p);
		lstrcpy(ErrStr, STR_ERR_SOCK_MAILSYNC);
		return FALSE;
	}
	mem_free(&p);
#else
	if (mail_item->MessageID == NULL || tstrcmp(mail_item->MessageID, content) != 0) {
		mem_free(&content);
		lstrcpy(ErrStr, STR_ERR_SOCK_MAILSYNC);
		return FALSE;
	}
#endif
	mem_free(&content);
	return TRUE;
}

/*
 * check_last_mail - 
 */
static int check_last_mail(email_handler *handler, SOCKET soc, BOOL chek_flag, TCHAR *ErrStr, email_box *mail_box, BOOL ShowFlag)
{
	if( chek_flag != FALSE ) 
	{ 
		// 
		if( disable_uidl == FALSE )
		{
			// UIDLで全メールの同期を取る
			receiving_uidl = FALSE;
			set_email_process_state(handler, TEXT(CMD_UIDL)TEXT("\r\n"));
			if( send_buf( handler, soc, CMD_UIDL"\r\n" ) == -1 )
			{
				lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
				return FALSE;
			}
			return POP_UIDL_ALL;
		}
		// 1
		//init_mailbox(hWnd, mail_box, ShowFlag);
		mail_box->mail_index = 0;
	} 
	else 
	{
		// 
		mail_box->mail_index++;
		ASSERT( mail_box->mail_index <= mail_box->mail_num ); 

		if( mail_box->mail_index == mail_box->mail_num )
		{
			return POP_QUIT;
		}
	}

	if( send_command( handler, soc, TEXT(CMD_LIST), mail_box->mail_index, ErrStr ) == FALSE )
	{
		return POP_ERR;
	}
	
	return POP_LIST;
}

/*
 * send_command - 
 */
static BOOL send_command(email_handler *handler, SOCKET soc, TCHAR *Command, int mail_index, TCHAR *ErrStr)
{
	TCHAR wBuf[BUF_SIZE];

	wsprintf(wBuf, TEXT("%s %d\r\n"), Command, mail_index );
	set_email_process_state(handler, wBuf);

	if( send_buf_t( handler, soc, wBuf ) == -1 )
	{
		lstrcpy( ErrStr, STR_ERR_SOCK_SEND );
		return FALSE;
	}

	return TRUE;
}

/*
 * send_command_top - send TOP command
 */
static ULONG send_command_top( email_handler *handler, SOCKET soc, int Cnt, TCHAR *ErrStr, int len, int ret)
{
	LRESULT _ret; 
	TCHAR wBuf[ BUF_SIZE ];

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	wsprintf( wBuf, TEXT( CMD_TOP ) TEXT(" %d %d\r\n"), Cnt, len );

	set_email_process_state( handler, wBuf );

	_ret = send_buf_t( handler, soc, wBuf ); 
	if( _ret != ERROR_SUCCESS ) 
	{
		log_trace( ( MSG_ERROR, "send top command failed \n" ) ); 
		//lstrcpy( ErrStr, STR_ERR_SOCK_SEND );
		ret = POP_ERR;
	}

	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
	return ret;
}

/*
 * create_apop - APOP create password digest.
 */
static LRESULT create_apop( char *buf, email_box *mail_box, TCHAR *sPass, TCHAR **digest )
{
	LRESULT ret = ERROR_SUCCESS; 
	MD5_CTX context;
	TCHAR *wbuf;
	unsigned char _digest[16];
	CHAR *hidx = NULL;
	CHAR *tidx = NULL;
	CHAR *pass = NULL;
	CHAR *p = NULL;
	INT32 len;
	INT32 i;

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	ASSERT( digest != NULL ); 

	*digest = NULL; 

	for( hidx = NULL, p = buf; *p != '\0'; p++ )
	{
		if( *p == '<' )
		{
			hidx = p;
		}
	}

	if( hidx != NULL )
	{
		for( tidx = NULL, p = hidx; *p != '\0';p++ )
		{
			if( *p == '>' )
			{
				tidx = p;
			}
		}
	}

	if( hidx == NULL || tidx == NULL )
	{
		//lstrcpy( ErrStr, STR_ERR_SOCK_NOAPOP ); 
		log_trace( ( MSG_ERROR, "%s \n", STR_ERR_SOCK_NOAPOP ) ); 
		ret = ERROR_NOT_SUPPORTED; 
		goto _return; 
	}

	pass = alloc_tchar_to_char( sPass );
	
	if( pass == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		//lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		//wbuf = NULL; 
		goto _return; 
	}

	p = ( CHAR* )mem_alloc( ( tidx - hidx + 2 ) + tstrlen( pass ) + 1 );
	
	if( p == NULL )
	{
		mem_free( &pass );
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		//lstrcpy( ErrStr, STR_ERR_MEMALLOC );
		//wbuf = NULL;
		goto _return; 
	}

	str_cpy_n( p, hidx, tidx - hidx + 2 );
	tstrcat( p, pass );
	mem_free( &pass );

	// digest 
	MD5Init( &context );
	MD5Update( &context, p, tstrlen( p ) );
	MD5Final( _digest, &context );

	mem_free( &p );

	wbuf = ( TCHAR* )mem_alloc(
		sizeof( TCHAR ) * ( lstrlen( TEXT( CMD_APOP ) ) + 1 + lstrlen( mail_box->email ) + 1 + ( 16 * 2 ) + 2 + 1 ) );
	
	if( wbuf == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		//lstrcpy( ErrStr, STR_ERR_MEMALLOC );
		goto _return; 
	}

	len = wsprintf( wbuf, TEXT( CMD_APOP ) TEXT( " %s " ), mail_box->email );
	
	for( i = 0; i < 16; i++, len += 2 ) 
	{
		wsprintf( wbuf + len, TEXT( "%02x" ), _digest[ i ] );
	}

	lstrcat( wbuf, TEXT( "\r\n" ) );
	
	log_trace( ( MSG_INFO, "apop command is %ws \n", wbuf ) ); 

	*digest = wbuf; 

_return:
	
	if( p != NULL )
	{
		mem_free( &p ); 
	}

	if( pass != NULL )
	{
		mem_free( &pass ); 
	}

	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
	return ret;
}

/*
 * login_proc - log to pop3 server, 2 mode APOP, USER PASS 
 */
static ULONG login_proc( email_handler *handler, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, email_box *mail_box )
{
	LRESULT _ret = ERROR_SUCCESS; 
	TCHAR *wbuf;
	static TCHAR *pass;
	int ret = POP_ERR;
	BOOL PopSTARTTLS = FALSE;

	ASSERT( handler != NULL ); 

	//if (op.SocLog == 1) SetSocStatusText(hWnd, buf);

	switch( handler->command_status )
	{
	case POP_STARTTLS:	
		if( check_response( buf ) == FALSE )
		{
			log_trace( ( "server given error response: %s\n", buf ) ); 

			//lstrcpy( ErrStr, STR_ERR_SOCK_RESPONSE );
			//str_cat_n( ErrStr, buf, BUF_SIZE - 1 );
			ret = POP_ERR;
			goto _return; 
		}

		handler->ssl_type = 1; 

		if( init_ssl( handler, soc, ErrStr ) == -1 )
		{
			ret = POP_ERR; 
			goto _return; 
		}

		handler->pop_start_tls = TRUE;

		//pop3 server has connected.
	case POP_START:

		disable_top = FALSE;
		disable_uidl = TRUE; //( mail_box->no_uidl == FALSE ) ? FALSE : TRUE;

		if( check_response( buf ) == FALSE )
		{
			log_trace( ( MSG_ERROR, "pop3 server response for start failed: %s\n", buf ) ); 

			//lstrcpy( ErrStr, STR_ERR_SOCK_RESPONSE );
			//str_cat_n( ErrStr, buf, BUF_SIZE - 1 );
			ret = POP_ERR;
			goto _return; 
		}

		if( /*mail_box->email == NULL ||*/ *mail_box->email == TEXT( '\0' ) )
		{
			ASSERT( FALSE && "account email is null" ); 

			//lstrcpy( ErrStr, STR_ERR_SOCK_NOUSERID );
			ret = POP_ERR;
			goto _return; 
		}

		// STARTTLS
		if( PopSTARTTLS == FALSE && mail_box->pop_use_ssl == TRUE 
			&& mail_box->pop_ssl_info.Type == 4 ) 
		{
			set_email_process_state( handler, TEXT( CMD_STLS ) );
			
			_ret = send_buf( handler, soc, CMD_STLS "\r\n" ); 

			if( _ret != ERROR_SUCCESS )
			{
				log_trace( ( MSG_ERROR, "send the start tls command failed 0x%0.8x\n", ret ) ); 

				//lstrcpy( ErrStr, STR_ERR_SOCK_SEND );
				ret = POP_ERR;
				goto _return; 
			}

			ret = POP_STARTTLS;
			goto _return; 
		}

		pass = mail_box->pwd;
		//if( pass == NULL || *pass == TEXT( '\0' ) ) 
		//{
		//	pass = mail_box->TmpPass;
		//}
		
		/*if( pass == NULL || *pass == TEXT( '\0' ) )
		{
			pass = g_Pass;
		}
		*/
		
		if( /*pass == NULL || */*pass == TEXT( '\0' ) ) 
		{
			ASSERT( FALSE ); 
			log_trace( ( MSG_ERROR, "account password can't is null\n" ) ); 

			//lstrcpy( ErrStr, STR_ERR_SOCK_NOPASSWORD );
			ret = POP_ERR;
			goto _return; 
		}
		
		// APOP
		if( mail_box->use_apop == TRUE )
		{
			_ret = create_apop( buf, mail_box, pass, &wbuf ); 

			if( _ret != ERROR_SUCCESS )
			{
				ret = POP_ERR;
				goto _return; 
			}

			ASSERT( wbuf != NULL ); 

			set_email_process_state( handler, TEXT( CMD_APOP ) TEXT( " ****" ) );

			ret = send_buf_t( handler, soc, wbuf ); 

			if( ret != ERROR_SUCCESS )
			{
				mem_free( &wbuf );
				//lstrcpy( ErrStr, STR_ERR_SOCK_SEND );
				log_trace( ( "send the apop command failed 0x%0.8x\n", ret ) ); 

				ret = POP_ERR;
				goto _return; 
			}

			mem_free( &wbuf );
			ret = POP_PASS;
			break;
		}

		// USER 
		wbuf = ( TCHAR* )mem_alloc( sizeof( TCHAR ) * ( lstrlen( TEXT( CMD_USER ) ) + 1 + lstrlen( mail_box->email ) + 2 + 1 ) );
		
		if( wbuf == NULL ) 
		{
			//lstrcpy( ErrStr, STR_ERR_MEMALLOC );
			log_trace( ( MSG_ERROR, "allocate the user command buffer failed \n" ) ); 

			ret = POP_ERR;
			goto _return; 
		}

		str_join_t( wbuf, 
			TEXT( CMD_USER ), 
			TEXT( " " ), 
			mail_box->email, 
			TEXT( "\r\n" ), 
			( TCHAR* )-1 ); 

		set_email_process_state( handler, wbuf );

		_ret = send_buf_t( handler, soc, wbuf ); 
		if( _ret != ERROR_SUCCESS )
		{
			mem_free( &wbuf );
			//lstrcpy( ErrStr, STR_ERR_SOCK_SEND );
			log_trace( ( MSG_ERROR, "user command failed\n" ) ); 
			
			ret = POP_ERR; 
			goto _return; 
		}

		mem_free( &wbuf );

		ret = POP_USER;
		break;

	case POP_USER:
		//AUTHORAZATION state step 2: PASS command.

		if( check_response( buf ) == FALSE )
		{
			log_trace( ( MSG_ERROR, "the pop3 server response for USER failed: %s\n", buf ) ); 
			//lstrcpy( ErrStr, STR_ERR_SOCK_ACCOUNT );
			//str_cat_n( ErrStr, buf, BUF_SIZE - 1 );
			ret = POP_ERR;
			goto _return; 
		}

		// send PASS command 
		wbuf = ( TCHAR* )mem_alloc( sizeof( TCHAR ) * ( lstrlen( TEXT( CMD_PASS ) ) + 1 + lstrlen( pass ) + 2 + 1 ) );
		if( wbuf == NULL ) 
		{
			//lstrcpy( ErrStr, STR_ERR_MEMALLOC );
			
			ret = POP_ERR; 
			goto _return; 
		}

		str_join_t( wbuf, TEXT( CMD_PASS ), TEXT( " " ), pass, TEXT( "\r\n" ), ( TCHAR * )-1 ); 

#ifdef _DEBUG
		set_email_process_state( handler, wbuf ); 

#else
		set_email_process_state( handler, TEXT( CMD_PASS ) TEXT( " ****" ) );
#endif //_DEBUG

		_ret = send_buf_t( handler, soc, wbuf ); 

		if( _ret != ERROR_SUCCESS )
		{
			mem_free( &wbuf );
			log_trace( ( MSG_ERROR, "send the PASS command failed \n" ) ); 
			//lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			ret = POP_ERR;
			goto _return; 
		}
		
		mem_free( &wbuf );

		ret = POP_PASS; 
		break;

	case POP_PASS:
		//pop3 server has responsed for PASS command
		if( check_response( buf ) == FALSE )
		{
			log_trace( ( MSG_ERROR, "server can't accept the password: %s\n", buf ) ); 

			//lstrcpy( ErrStr, STR_ERR_SOCK_BADPASSWORD );
			//str_cat_n( ErrStr, buf, BUF_SIZE - 1 );
			ret = POP_ERR;
			goto _return; 
		}

		ret = POP_LOGIN;
		break;
	}

_return:
	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 

	return ret;
}

/*
 * list_proc_stat - STAT
 */
static int list_proc_stat( email_handler *handler, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, email_box *mail_box, BOOL ShowFlag)
{
	MAILITEM *mail_item = NULL;
	char *p, *r, *t;
	int get_no;
	int ret;

	//if (op.SocLog == 1) SetSocStatusText(hWnd, buf);
	// '.'
	if( *buf == '.' && *( buf + 1 ) == '\0' )
	{
		ret = POP_STAT;
		goto _return; 
	}

	// 
	if( check_response( buf ) == FALSE ) 
	{
		lstrcpy( ErrStr, STR_ERR_SOCK_STAT );
		str_cat_n( ErrStr, buf, BUF_SIZE - 1 );
		ret = POP_ERR;
		goto _return; 
	}

	// response format: "+OK mail_num mail_size"
	p = buf;
	t = NULL;
	for( ; *p != ' ' && *p != '\0'; p++ );

	for( ; *p == ' '; p++ );

	for( r = p; *r != '\0'; r++ )
	{
		if( *r == ' ' ) 
		{
			t = r + 1;
			*r = '\0';
			break;
		}
	}

	mail_box->mail_num = a2i( p );
	
	if( t != NULL )
	{
		mail_box->mail_size = a2i( t );
	}

	log_trace( ( MSG_INFO, "mail number is %d, mail size is %d\n", 
		mail_box->mail_num, 
		mail_box->mail_size ) ); 

	if( mail_box->mail_num == 0 || mail_box->mail_index == 0 ) 
	{
		if( mail_box->mail_num == 0 )
		{
			ret = POP_QUIT;
			goto _return; 
		}
	}

	//SetItemCntStatusText(hWnd, mail_box);
	//( op.ListDownload != 0 ) ? TRUE : 

	init_recv = FALSE;
	mail_received = FALSE;
	disable_top = FALSE;

	// UIDL
	if( uidl_init( mail_box->mail_num ) == FALSE )
	{
		lstrcpy( ErrStr, STR_ERR_MEMALLOC );
		ret = POP_ERR;
		goto _return; 
	}
	
	// 
	if( disable_uidl == FALSE ) 
	{
		receiving_uidl = FALSE;
		set_email_process_state( handler, TEXT( CMD_UIDL ) TEXT( "\r\n" ) );
		if( send_buf( handler, soc, CMD_UIDL "\r\n" ) == -1 )
		{
			lstrcpy( ErrStr, STR_ERR_SOCK_SEND );
			ret = POP_ERR;
			goto _return; 
		}

		ret = POP_UIDL_ALL;
		goto _return; 
	}
	else
	{

		init_recv = TRUE; 

		mail_box->mail_index = 0; 

		if( send_command( handler, soc, TEXT(CMD_LIST), mail_box->mail_index + 1, ErrStr ) == FALSE )
		{
			ret = POP_ERR;
			goto _return; 
		}

		ret = POP_LIST;
		goto _return; 
	}


	if( disable_uidl == FALSE )
	{
		receiving_uidl = FALSE;
		set_email_process_state( handler, TEXT( CMD_UIDL ) TEXT("\r\n") );

		if( send_buf( handler, soc, CMD_UIDL "\r\n" ) == -1 )
		{
			lstrcpy( ErrStr, STR_ERR_SOCK_SEND );
			ret = POP_ERR;
			goto _return; 
		}

		ret = POP_UIDL_ALL;
		goto _return; 
	} 
	else 
	{
		if( send_command( handler, soc, TEXT( CMD_LIST ), mail_box->mail_index + 1, ErrStr ) == FALSE )
		{
			ret = POP_ERR;
			goto _return; 
		}

		ret = POP_LIST;
		goto _return; 
	}

	//list_get_no = mail_box->mail_index;
	get_no = item_get_number_to_index( mail_box, mail_box->mail_index ); 

	if( get_no != -1 )
	{
		mail_item = *( mail_box->mail_item + get_no ); 
	}
	
	if( disable_uidl == FALSE && mail_item != NULL && mail_item->UIDL != NULL )
	{
		if( send_command( handler, soc, TEXT( CMD_UIDL ), mail_box->mail_num + 1, ErrStr ) == FALSE )
		{
			ret = POP_ERR;
			goto _return; 
		}

		ret = POP_UIDL_CHECK;
		goto _return; 
	} 
	else 
	{
		receiving_data = FALSE;
		if( mail_buf_init( MAIL_BUF_SIZE ) == FALSE )
		{
			lstrcpy( ErrStr, STR_ERR_MEMALLOC );
			ret = POP_ERR;
			goto _return; 
		}

		ret = send_command_top( handler, soc, mail_box->mail_num + 1, ErrStr, 0, POP_TOP );
		goto _return; 
	}

_return:
	return ret;
}

/*
 * list_proc_uidl_all - UIDL
 */
static int list_proc_uidl_all( email_handler *handler, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, email_box *mail_box, BOOL ShowFlag)
{
	MAILITEM *mail_item;
	MAILITEM *tpLastMailItem;
	HWND hListView;
	int No;
	int i;

	// UIDLレスポンスの1行目
	if (receiving_uidl == FALSE) {
		//SetSocStatusText(hWnd, buf);
		// 前コマンド結果に'.'が付いている場合はスキップする
		if (*buf == '.' && *(buf + 1) == '\0') {
			return POP_UIDL_ALL;
		}
		// レスポンスの解析
		if (check_response(buf) == TRUE) {
			receiving_uidl = TRUE;
			return POP_UIDL_ALL;
		}
		// UIDL未サポート
		disable_uidl = TRUE;
		set_email_process_state( handler, TEXT(CMD_STAT)TEXT("\r\n"));
		if (send_buf(handler, soc, CMD_STAT"\r\n") == -1) {
			lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			return POP_ERR;
		}
		return POP_STAT;
	}
	// UIDLの終わりではない場合
	if (*buf != '.' || *(buf + 1) != '\0') {
		// 受信文字列を保存する
		if (uidl_set(buf, buflen) == FALSE) {
			uidl_free();
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return POP_ERR;
		}
		return POP_UIDL_ALL;
	}

	if (mail_box->mail_index == 0 || mail_box->last_message_id == NULL) 
	{
		init_recv = TRUE;
		if (send_command( handler, soc, TEXT(CMD_LIST), mail_box->mail_num + 1, ErrStr) == FALSE) {
			return POP_ERR;
		}
		return POP_LIST;
	}

	//hListView = GetDlgItem(hWnd, IDC_LISTVIEW);
	//if (ShowFlag == TRUE) {
	//	ListView_SetRedraw(hListView, FALSE);
	//}
	//SwitchCursor(FALSE);
	// UIDL
	tpLastMailItem = NULL;
	for (i = 0; ( ULONG )i < mail_box->mail_num; i++) {
		mail_item = *mail_box->mail_item;
		if (mail_item == NULL) 
		{
			continue;
		}

		switch ((No = uidl_check(mail_item->UIDL))) {
#ifdef UNICODE
		case -1:
			uidl_free();
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			//if (ShowFlag == TRUE) {
			//	ListView_SetRedraw(hListView, TRUE);
			//}
			//SwitchCursor(TRUE);
			return POP_ERR;
#endif
		case 0:
			// UIDL
			//if (ShowFlag == TRUE) {
			//	No = ListView_GetMemToItem(hListView, mail_item);
			//	ListView_DeleteItem(hListView, No);
			//}
			//item_free((mail_box->mail_item + i), 1);
			//break;

		default:
			// 
			mail_item->No = No;
			if (tpLastMailItem == NULL || tpLastMailItem->No < mail_item->No) {
				tpLastMailItem = mail_item;
			}
			break;
		}
	}
	// 削除されたメールを一覧から消す
	item_resize_mailbox(mail_box);
	//if (ShowFlag == TRUE) {
	//	ListView_SetRedraw(hListView, TRUE);
	//}
	//SwitchCursor(TRUE);
	//SetItemCntStatusText(hWnd, mail_box);

	// 最後に受信したメールのメッセージIDを保存する
	mem_free(&mail_box->last_message_id);
	mail_box->last_message_id = NULL;
	mail_box->mail_index = 0;
	if (tpLastMailItem != NULL) {
		mail_box->last_message_id = alloc_tchar_to_char(tpLastMailItem->MessageID);
		mail_box->mail_index = tpLastMailItem->No;
	}

	ASSERT( mail_box->mail_index <= mail_box->mail_num ); 

	if( mail_box->mail_index == mail_box->mail_num ) 
	{
		return POP_QUIT;
	}

	if(send_command( handler, soc, TEXT(CMD_LIST), mail_box->mail_index + 1, ErrStr) == FALSE) 
	{
		return POP_ERR;
	}
	return POP_LIST;
}

/*
 * list_proc_uidl_check - UIDLのレスポンスの解析
 */
static int list_proc_uidl_check( email_handler *handler, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, email_box *mail_box, BOOL ShowFlag)
{
	MAILITEM *mail_item = NULL;
	TCHAR *UIDL = NULL;
	int get_no;
	int ret;

	//'.'
	if (*buf == '.' && *(buf + 1) == '\0') {
		return POP_UIDL_CHECK;
	}
	//
	if (check_response(buf) == TRUE) {
		UIDL = alloc_char_to_tchar(skip_response(buf));
	} else {
		// UIDL
		disable_uidl = TRUE;
		receiving_data = FALSE;
		if (mail_buf_init(MAIL_BUF_SIZE) == FALSE) {
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return POP_ERR;
		}
		return send_command_top( handler, soc, mail_box->mail_index + 1, ErrStr, 0, POP_TOP);
	}
	get_no = item_get_number_to_index(mail_box, mail_box->mail_index );
	if (get_no != -1) {
		mail_item = *mail_box->mail_item;
	}
	// UIDL
	ret = check_last_mail(handler, soc,
		(mail_item ==NULL || mail_item->UIDL == NULL || UIDL == NULL || lstrcmp(mail_item->UIDL, UIDL) != 0),
		ErrStr, mail_box, ShowFlag);
	mem_free(&UIDL);
	return ret;
}

/*
 * list_proc_uidl_set - UIDLのレスポンスの解析
 */
static int list_proc_uidl_set( email_handler *handler, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, email_box *mail_box, BOOL ShowFlag)
{
	//if (op.SocLog == 1) SetSocStatusText(hWnd, buf);
	// '.'
	if( *buf == '.' && *( buf + 1 ) == '\0')
	{
		return POP_UIDL;
	}
	
	// 
	if(check_response( buf ) == TRUE )
	{
		uidl_item->UIDL = alloc_char_to_tchar(skip_response(buf));
	} 
	else
	{
		disable_uidl = TRUE;
	}

	// 
	mail_box->mail_index++;
	ASSERT( mail_box->mail_index <= mail_box->mail_num ); 

	if( mail_box->mail_index == mail_box->mail_num )
	{
		return POP_QUIT;
	}

	if( send_command( handler, soc, TEXT(CMD_LIST), mail_box->mail_index + 1, ErrStr ) == FALSE)
	{
		return POP_ERR;
	}

	return POP_LIST;
}

/*
 * list_proc_list - LIST
 */
static ULONG list_proc_list( email_handler *handler, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, email_box *mail_box, BOOL ShowFlag)
{
	char *p, *r;
	int len = 0;
	ULONG ret; 

	//if (op.SocLog == 1) SetSocStatusText(hWnd, buf);
	//'.'
	
	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 
	log_trace( ( MSG_INFO, "server response is %s\n", buf ) ); 

	if( *buf == '.' && *( buf + 1 ) == '\0' )
	{
		ret = POP_LIST;
		log_trace( ( MSG_INFO, "encounter the the end of command\n" ) ); 
		goto _return; 
	}

	mail_size = -1; 

	if( check_response( buf ) == TRUE )
	{
		p = skip_response( buf ); 

 		for( r = p; *r != ' ' && *r != '\0'; r++ ); 

		mail_size = len = a2i( p ); 

		if( disable_top == FALSE )
		{
			len = ( len > 0 && len < HEAD_LINE * LINE_LEN )
				? len : ( HEAD_LINE * LINE_LEN );
		}
	}

	receiving_data = FALSE; 

	if( mail_buf_init( ( len > 0 ) ? len : MAIL_BUF_SIZE ) == FALSE )
	{
		log_trace( ( MSG_ERROR, "allocate the buffer for list command of pop3 failed\n" ) ); 
		//lstrcpy( ErrStr, STR_ERR_MEMALLOC );
		ret = POP_ERR; 
		goto _return; 
	}

	if( disable_top == TRUE )
	{
		if( mail_box->no_retr == TRUE ) 
		{
			ret = send_command_top( handler, soc, mail_box->mail_index + 1, ErrStr, ( len > 0 ) ? len : DOWNLOAD_SIZE, POP_RETR ); 
			goto _return; 
		}

		if( send_command( handler, soc, TEXT( CMD_RETR ), mail_box->mail_index + 1, ErrStr ) == FALSE )
		{
			ret = POP_ERR; 
			goto _return; 
		}

		ret = POP_RETR; 
		goto _return; 
	}

#define DEF_EMAIL_BODY_LINE 10
	ret = send_command_top( handler, soc, mail_box->mail_index + 1, ErrStr, DEF_EMAIL_BODY_LINE, POP_TOP );
	
_return: 
	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
	return ret; 
}

/*
 * list_proc_top - TOP
 */
static int list_proc_top( email_handler *handler, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, email_box *mail_box, BOOL ShowFlag )
{
	LRESULT ret; 
	MAILITEM *mail_item;
	LV_ITEM lvi;
	HWND hListView;
	TCHAR *p;
	int i;
	int st;
	//PVOID cmd_context = NULL; 

	// TOP 1
	if( receiving_data == FALSE )
	{
		//SetSocStatusText(hWnd, buf);
		// '.'
		if( *buf == '.' && *( buf + 1 ) == '\0' )
		{
			return POP_TOP;
		}

		// 
		if( check_response( buf ) == TRUE ) 
		{
			receiving_data = TRUE;
			return POP_TOP;
		}

		if( disable_top == FALSE && mail_box->no_retr == 0 )
		{
			// TOP RETR
			disable_top = TRUE;
			if( send_command( handler, soc, TEXT( CMD_RETR ), mail_box->mail_index + 1, ErrStr ) == FALSE ) 
			{
				return POP_ERR;
			}

			return POP_RETR;
		}

		lstrcpy( ErrStr, ( disable_top == FALSE ) ? STR_ERR_SOCK_TOP : STR_ERR_SOCK_RETR );
		
		str_cat_n( ErrStr, buf, BUF_SIZE - 1 );
		return POP_ERR;
	}
	
	// TOP
	if( *buf != '.' || *( buf + 1) != '\0' ) 
	{
		// 
		if( mail_buf_set( buf, buflen) == FALSE )
		{
			lstrcpy( ErrStr, STR_ERR_MEMALLOC );
			return POP_ERR;
		}

		return POP_TOP;
	}

	ASSERT( handler->command_check != NULL ); 
	ASSERT( handler->erc_cmd_record == NULL ); 
	ret = handler->command_check( handler, mail_buf, buflen, &handler->erc_cmd_record ); 
	if( ret != ERROR_SUCCESS )
	{
		ASSERT( handler->erc_cmd_record == NULL ); 
		mail_box->mail_index ++; 
	}
	else
	{
		ASSERT( handler->erc_cmd_record != NULL ); 

		if( send_command( handler, soc, TEXT( CMD_DELE ), mail_box->mail_index + 1, ErrStr ) == FALSE )
		{
			return POP_ERR;
		}

		return POP_DELE; 
	}

	if( mail_box->mail_index == mail_box->mail_num )
	{
		//send_buf( handler, soc, CMD_QUIT "\r\n" );
		return POP_QUIT; 		; 
	}

	if( send_command( handler, soc, TEXT( CMD_LIST ), mail_box->mail_index + 1, ErrStr ) == FALSE )
	{
		return POP_ERR;
	}

	return POP_LIST;

#if 0
	if( list_get_no == mail_box->mail_index )
	{
		if( init_recv == TRUE )
		{ 
			//init_mailbox(hWnd, mail_box, ShowFlag);
		}
		else
		{
			// Message-ID
			char *content = item_get_message_id( mail_buf ); 

			i = check_last_mail( 
				handler, 
				soc,
				TRUE,
				ErrStr, 
				mail_box, 
				ShowFlag ); 

			mem_free( &content );
			return i;
		}
	}

	// 
	//if( mail_received == FALSE )
	//{
	//	//if (ShowFlag == TRUE) {
	//	//	ListView_SetItemCount(GetDlgItem(hWnd, IDC_LISTVIEW), mail_box->mail_num);
	//	//}
	//	if( item_set_count( mail_box, mail_box->mail_num ) == FALSE )
	//	{
	//		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
	//		return POP_ERR;
	//	}
	//}

	// 
	ret = item_header_to_item( &mail_item, mail_buf, mail_size );
	
	if( ret != ERROR_SUCCESS )
	{
		lstrcpy( ErrStr, STR_ERR_MEMALLOC );
		return POP_ERR;
	}

	ASSERT( mail_item != NULL ); 

	if( ( int )mail_item != -1 
		&& mail_item->Body == NULL 
		&& disable_top == TRUE )
	{
		mail_item->Body = (char *)mem_alloc(sizeof(char));
		if( mail_item->Body != NULL )
		{
			*mail_item->Body = '\0';
			mail_item->Status = mail_item->MailStatus = ICON_MAIL;
		}
	}

	if( ( int )mail_item != -1 ) 
	{
		//if (mail_received == FALSE && NewMail_Flag == FALSE && ShowMsgFlag == FALSE) {
		//	for (i = 0; i < mail_box->mail_num; i++) {
		//		if (*(mail_box->mail_item + i) == NULL) {
		//			continue;
		//		}
		//		(*(mail_box->mail_item + i))->New = FALSE;
		//	}
		//}
		mail_item->New = TRUE;
		mail_item->Download = disable_top;
		mail_item->No = mail_box->mail_index + 1;

		if( ShowFlag == TRUE )
		{
			st = INDEXTOOVERLAYMASK(1);
			st |= ( ( mail_item->Multipart == TRUE ) ? INDEXTOSTATEIMAGEMASK( 1 ) : 0 );

			//if( mail_received == FALSE && NewMail_Flag == FALSE && ShowMsgFlag == FALSE) 
			//{
			//	ListView_SetItemState(hListView, -1, 0, LVIS_OVERLAYMASK);
			//	ListView_RedrawItems(hListView, 0, ListView_GetItemCount(hListView));
			//	ListView_SetItemState(hListView, -1, 0, LVIS_FOCUSED | LVIS_SELECTED);
			//	st |= (LVIS_FOCUSED | LVIS_SELECTED);
			//}
			//st |= ((mail_item->Download == FALSE && mail_item->Status != ICON_DOWN && mail_item->Status != ICON_DEL)
			//	? LVIS_CUT : 0);
			//lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
			//lvi.iItem = ListView_GetItemCount(hListView);
			//lvi.iSubItem = 0;
			//lvi.state = st;
			//lvi.stateMask = LVIS_OVERLAYMASK | LVIS_STATEIMAGEMASK | LVIS_CUT | LVIS_FOCUSED | LVIS_SELECTED;
			//lvi.pszText = LPSTR_TEXTCALLBACK;
			//lvi.cchTextMax = 0;
			//lvi.iImage = I_IMAGECALLBACK;
			//lvi.lParam = (long)mail_item;
			//i = ListView_InsertItem(hListView, &lvi);
			//if (mail_received == FALSE) {
			//	ListView_EnsureVisible(hListView, i, TRUE);
			//}
			//if (op.RecvScroll == 1) {
			//	SendMessage(hListView, WM_VSCROLL, SB_LINEDOWN, 0);
			//}

			//SetItemCntStatusText(hWnd, mail_box);
			//EndThreadSortFlag = TRUE;
		}
		mail_received = TRUE;

		handler->new_mail_count++;
		//mail_box->NewMail = TRUE;
	}

	//ID
	//mail_box->mail_index = list_get_no;
	//mem_free( &mail_box->LastMessageId );
	//mail_box->LastMessageId = item_get_message_id( mail_buf );
	
	//if( mail_box->LastMessageId == NULL )
	//{
		//lstrcpy( ErrStr, STR_ERR_SOCK_NOMESSAGEID );
		//return POP_ERR;
	//}

	if( ( int )mail_item != -1 && disable_uidl == FALSE )
	{
		// UIDL
		if( ( p = uidl_get( mail_box->mail_index + 1 ) ) != NULL )
		{
			mail_item->UIDL = alloc_copy_t( p );
		} 
		else 
		{
			uidl_item = mail_item;
			if( send_command( handler, soc, TEXT( CMD_UIDL ), mail_box->mail_index + 1, ErrStr ) == FALSE ) 
			{
				return POP_ERR;
			}
			
			return POP_UIDL;
		}
	}

	mail_box->mail_index ++;
	//if( list_get_no > mail_box->mail_num )
	//{
	//	return POP_QUIT;
	//}
	
	if( send_command( handler, soc, TEXT( CMD_LIST ), mail_box->mail_index + 1, ErrStr ) == FALSE )
	{
		return POP_ERR;
	}
	
	return POP_LIST;
#endif //0
}

/*
 * exec_send_check_command - 
 */
static int exec_send_check_command( email_handler *handler, SOCKET soc, INT32 get_no, TCHAR *ErrStr, email_box *mail_box )
{
	MAILITEM *mail_item = NULL;
	
	mail_item = *mail_box->mail_item;

	if( disable_uidl == TRUE 
		|| mail_item == NULL 
		|| mail_item->UIDL == NULL )
	{
		receiving_data = FALSE;
		
		if( mail_buf_init( MAIL_BUF_SIZE ) == FALSE )
		{
			log_trace( ( MSG_ERROR, "init smtp buffer failed " ) ); 
			//lstrcpy( ErrStr, STR_ERR_MEMALLOC );
			return POP_ERR;
		}
		
		if( disable_top == TRUE 
			&& mail_box->no_retr == 0 )
		{
			// RETR
			if( send_command( handler, soc, TEXT( CMD_RETR ), get_no, ErrStr ) == FALSE )
			{
				return POP_ERR;
			}

			return POP_TOP;
		}

		// TOP
		return send_command_top(handler, soc, get_no, ErrStr, 0, POP_TOP);
	}

	// UIDL
	if( send_command( handler, soc, TEXT(CMD_UIDL), get_no, ErrStr ) == FALSE )
	{
		return POP_ERR;
	}

	return POP_UIDL;
}

/*
 * exec_proc_init - 
 */
static int exec_proc_init( email_handler *handler, 
	SOCKET soc, 
	INT32 mail_index, 
	char *buf, 
	int buflen, 
	TCHAR *ErrStr, 
	email_box *mail_box, 
	BOOL ShowFlag )
{
	MAILITEM *mail_item;
	int size;
	int get_no;

	get_no = item_get_next_download_mark(mail_box, -1, &mail_index );
	if (get_no == -1) {
		// 削除
		get_no = item_get_next_delete_mark(mail_box, -1, &mail_index );
		if (get_no == -1) {
			return POP_QUIT;
		}
		// 削除メールの確認コマンド(UIDL)を送信
		return exec_send_check_command(handler, soc, get_no, ErrStr, mail_box);
	}

	// ダウンロード
	mail_item = mail_box->mail_item;
	if (mail_item == NULL) {
		lstrcpy(ErrStr, STR_ERR_SOCK_GETITEMINFO);
		return POP_ERR;
	}
	size = (mail_item->Size == NULL) ? 0 : _ttoi(mail_item->Size);
	receiving_data = FALSE;
	if (mail_buf_init((size > 0) ? size : MAIL_BUF_SIZE) == FALSE) {
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return POP_ERR;
	}
	// 
	if( mail_box->no_retr == 1 )
	{
		return send_command_top(handler, soc, mail_index, ErrStr, (size > 0) ? size : DOWNLOAD_SIZE, POP_RETR);
	}
	
	if( send_command( handler, soc, TEXT( CMD_RETR ), mail_index, ErrStr ) == FALSE )
	{
		return POP_ERR;
	}

	return POP_RETR;
}

/*
 * exec_proc_retr - RETRのレスポンスの解析
 */
static int exec_proc_retr( email_handler *handler, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, email_box *mail_box, BOOL ShowFlag)
{
	MAILITEM *mail_item;
	HWND hListView;
	int i, size;
	int get_no = 0;
	static int recvlen;
	static int recvcnt;

	if (receiving_data == FALSE) {
		// 前コマンド結果に'.'が付いている場合はスキップする
		if (*buf == '.' && *(buf + 1) == '\0') {
			return POP_RETR;
		}
		// レスポンスの解析
		if (check_response(buf) == FALSE) {
			lstrcpy(ErrStr, (mail_box->no_retr == 1) ? STR_ERR_SOCK_TOP : STR_ERR_SOCK_RETR);
			str_cat_n(ErrStr, buf, BUF_SIZE - 1);
			return POP_ERR;
		}
		recvlen = 0;
		recvcnt = REDRAWCNT;
		set_email_process_state(handler, STR_STATUS_RECV);
		receiving_data = TRUE;
		return POP_RETR;
	}

	// RETRの終わりではない場合
	if (*buf != '.' || *(buf + 1) != '\0') {
		// 受信データを保存する
		if (mail_buf_set(buf, buflen) == FALSE) {
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return POP_ERR;
		}
		recvlen += buflen;
		recvcnt++;
		if (recvcnt > REDRAWCNT) {
			recvcnt = 0;
			//SetStatusRecvLen(handler, recvlen, STR_STATUS_SOCKINFO_RECV);
		}
		return POP_RETR;
	}

	//get_no = item_get_number_to_index(mail_box, download_get_no);
	//if( get_no == -1 )
	//{
	//	lstrcpy(ErrStr, STR_ERR_SOCK_MAILSYNC);
	//	return POP_ERR;
	//}

	mail_item = *mail_box->mail_item;
	if (mail_item == NULL) {
		lstrcpy(ErrStr, STR_ERR_SOCK_GETITEMINFO);
		return POP_ERR;
	}
	// メッセージIDで要求したメールかどうかチェックする
	if (check_message_id(mail_buf, mail_item, ErrStr, mail_box) == FALSE) {
		return POP_ERR;
	}

	// 本文を取得
	item_mail_to_item(mail_item, mail_buf, -1, TRUE);
	mail_item->Download = TRUE;

	//if (ShowFlag == TRUE) {
	//	hListView = GetDlgItem(hWnd, IDC_LISTVIEW);
	//	// リストビューの更新
	//	i = ListView_GetMemToItem(hListView, mail_item);
	//	if (i != -1) {
	//		ListView_SetItemState(hListView, i, 0, LVIS_CUT);
	//		ListView_RedrawItems(hListView, i, i);
	//		UpdateWindow(hListView);
	//		SetItemCntStatusText(hWnd, mail_box);
	//	}
	//}
	//if (hViewWnd != NULL) {
	//	SendMessage(hViewWnd, WM_CHANGE_MARK, 0, 0);
	//}

	//get_no = item_get_next_download_mark(mail_box, -1, &download_get_no);
	//if (get_no == -1) {
	//	get_no = item_get_next_delete_mark(mail_box, -1, &delete_get_no);
	//	if (get_no == -1) {
	//		return POP_QUIT;
	//	}
	//	// 削除メールの確認コマンド(UIDL)を送信
	//	return exec_send_check_command(handler, soc, get_no, ErrStr, mail_box);
	//}
	mail_item = *( mail_box->mail_item + get_no );
	if (mail_item == NULL) {
		lstrcpy(ErrStr, STR_ERR_SOCK_GETITEMINFO);
		return POP_ERR;
	}
	size = (mail_item->Size == NULL) ? 0 : _ttoi(mail_item->Size);

	// 次のヘッダの取得
	receiving_data = FALSE;
	if (mail_buf_init((size > 0) ? size : MAIL_BUF_SIZE) == FALSE) {
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return POP_ERR;
	}
	// 全文受信するコマンドを送信
	//if (mail_box->no_retr == 1) {
	//	return send_command_top(handler, soc, download_get_no, ErrStr, (size > 0) ? size : DOWNLOAD_SIZE, POP_RETR);
	//}
	//if (send_command(handler, soc, TEXT(CMD_RETR), download_get_no, ErrStr) == FALSE) {
	//	return POP_ERR;
	//}
	return POP_RETR;
}

/*
 * exec_proc_uidl - 削除メール確認用UIDLのレスポンスの解析
 */
static int exec_proc_uidl( email_handler *handler, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, email_box *mail_box, BOOL ShowFlag)
{
	MAILITEM *mail_item = NULL;
	TCHAR *UIDL = NULL;
	int get_no;

	ASSERT( handler != NULL ); 

	//if (op.SocLog == 1) SetSocStatusText(hWnd, buf);
	// 前コマンド結果に'.'が付いている場合はスキップする
	if (*buf == '.' && *(buf + 1) == '\0') {
		return POP_UIDL;
	}
	// レスポンスの解析
	if (check_response(buf) == TRUE) {
		UIDL = alloc_char_to_tchar(skip_response(buf));
	} else {
		// UIDL未サポート
		disable_uidl = TRUE;
		// 削除メールの確認コマンド(TOP)を送信
		return exec_send_check_command(handler, soc, -1, ErrStr, mail_box);
	}
	//get_no = item_get_number_to_index(mail_box, delete_get_no);
	//if (get_no != -1) {
	//	mail_item = *(mail_box->mail_item + get_no);
	//}
	if (UIDL == NULL || mail_item == NULL || mail_item->UIDL == NULL) {
		mem_free(&UIDL);
		// 削除メールの確認コマンド(TOP)を送信
		return exec_send_check_command(handler, soc, -1, ErrStr, mail_box);
	}
	// メッセージIDで要求したメールかどうかチェックする
	if (lstrcmp(mail_item->UIDL, UIDL) != 0) {
		mem_free(&UIDL);
		lstrcpy(ErrStr, STR_ERR_SOCK_MAILSYNC);
		return POP_ERR;
	}
	mem_free(&UIDL);
	
	// 
	//if( send_command( handler, soc, TEXT( CMD_DELE ), delete_get_no, ErrStr ) == FALSE )
	//{
	//	return POP_ERR;
	//}

	return POP_DELE;
}

/*
 * exec_proc_top - TOP
 */
static int exec_proc_top( email_handler *handler, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, email_box *mail_box, BOOL ShowFlag)
{
	MAILITEM *mail_item;
	int get_no;

	ASSERT( handler != NULL ); 

	if (receiving_data == FALSE) {
		//SetSocStatusText(hWnd, buf);
		// 前コマンド結果に'.'が付いている場合はスキップする
		if (*buf == '.' && *(buf + 1) == '\0') {
			return POP_TOP;
		}
		// レスポンスの解析
		if (check_response(buf) == FALSE) {
			// 削除確認のTOPとRETRが両方失敗した場合はエラー
			if (disable_top == TRUE || mail_box->no_retr == 1) {
				lstrcpy(ErrStr, (mail_box->no_retr == 1) ? STR_ERR_SOCK_TOP : STR_ERR_SOCK_RETR);
				str_cat_n(ErrStr, buf, BUF_SIZE - 1);
				return POP_ERR;
			}
			// 削除確認のTOPで失敗したためRETRで削除確認を行う
			disable_top = TRUE;
			// 削除メールの確認コマンド(RETR)を送信
			return exec_send_check_command(handler, soc, -1, ErrStr, mail_box);
		}
		receiving_data = TRUE;
		return POP_TOP;
	}

	// TOP
	if (*buf != '.' || *(buf + 1) != '\0') {
		// 
		if (mail_buf_set(buf, buflen) == FALSE) {
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return POP_ERR;
		}
		return POP_TOP;
	}

	//get_no = item_get_number_to_index(mail_box, delete_get_no);
	//if (get_no == -1) {
	//	lstrcpy(ErrStr, STR_ERR_SOCK_MAILSYNC);
	//	return POP_ERR;
	//}
	mail_item = *(mail_box->mail_item + get_no);
	if (mail_item == NULL) {
		lstrcpy(ErrStr, STR_ERR_SOCK_GETITEMINFO);
		return POP_ERR;
	}
	// ID
	if (check_message_id(mail_buf, mail_item, ErrStr, mail_box) == FALSE) {
		return POP_ERR;
	}
	// 
	//if (send_command(handler, soc, TEXT(CMD_DELE), delete_get_no, ErrStr) == FALSE) {
	//	return POP_ERR;
	//}
	return POP_DELE;
}

/*
 * exec_proc_dele - DELE
 */
static int exec_proc_dele( email_handler *handler, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, email_box *mail_box, BOOL ShowFlag)
{
	MAILITEM *mail_item;
	//HWND hListView = NULL;
	int i, j;
	int get_no;

	ASSERT( handler != NULL ); 

	//if (op.SocLog == 1) SetSocStatusText(hWnd, buf);
	if (*buf == '.' && *(buf + 1) == '\0') {
		return POP_DELE;
	}
	// 
	if (check_response(buf) == FALSE) {
		lstrcpy(ErrStr, STR_ERR_SOCK_DELE);
		str_cat_n(ErrStr, buf, BUF_SIZE - 1);
		send_buf( handler, soc, CMD_RSET"\r\n");
		return POP_ERR;
	}

	//get_no = item_get_number_to_index(mail_box, delete_get_no);
	//if( get_no == -1 )
	//{
	//	lstrcpy(ErrStr, STR_ERR_SOCK_MAILSYNC);
	//	return POP_ERR;
	//}

	//get_no = item_get_next_delete_mark(mail_box, get_no, &delete_get_no);
	//if( get_no != -1 )
	//{
	//	// 
	//	return exec_send_check_command(handler, soc, get_no, ErrStr, mail_box);
	//}

	//if( ShowFlag == TRUE ) 
	//{
	//	hListView = GetDlgItem(hWnd, IDC_LISTVIEW);
	//	ListView_SetRedraw(hListView, FALSE);
	//	while( ( get_no =  ListView_GetNextDeleteItem( hListView, -1 ) ) != -1 )
	//	{
	//		ListView_DeleteItem( hListView, get_no );
	//	}
	//	
	//	ListView_SetRedraw( hListView, TRUE );
	//}

	for( i = 0; i < mail_box->mail_num; i++ )
	{
		mail_item = mail_box->mail_item;
		if (mail_item == NULL || mail_item->Status != ICON_DEL) {
			continue;
		}
		item_free(mail_box->mail_item, 1);
	
		for( j = i + 1; j < mail_box->mail_num; j++ )
		{
			mail_item = mail_box->mail_item ;
			
			if( mail_item == NULL )
			{
				continue;
			}

			mail_item->No--;
		}

		mail_box->mail_num--;
		mail_box->mail_index--;
	}
	
	if( mail_box->mail_item == NULL )
	{
		mail_item = NULL;
		
		for( i = 0; i < mail_box->mail_num; i++ )
		{
			if( mail_box->mail_item == NULL )
			{
				ASSERT( FALSE ); 			
				continue;
			}
			
			if( mail_item == NULL || mail_item->No < ( *( mail_box->mail_item + i ) )->No )
			{
				mail_item = *mail_box->mail_item;
			}
		}

		if( mail_item != NULL )
		{
			mem_free( ( PVOID* )&mail_box->last_message_id ); 

			mail_box->last_message_id = alloc_tchar_to_char( mail_item->MessageID );
			
			mail_box->mail_index = mail_item->No;
		}
	}

	item_resize_mailbox( mail_box );
	//SetItemCntStatusText(hWnd, mail_box);
	return POP_QUIT;
}

LRESULT pop3_delete_mail( email_handler *handler, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, email_box *mail_box, BOOL ShowFlag)
{
	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	ASSERT( handler != NULL ); 

	//if( *buf == '.' && *( buf + 1 ) == '\0' )
	//{
	//	return POP_DELE;
	//}

	log_trace ( ( MSG_INFO, "response is %s\n", buf ) ); 

	if( check_response( buf ) == FALSE )
	{
		//lstrcpy( ErrStr, STR_ERR_SOCK_DELE );
		//str_cat_n( ErrStr, buf, BUF_SIZE - 1 );
		send_buf( handler, soc, CMD_RSET "\r\n" );
		return POP_ERR;
	}

	//mail_box->mail_index ++; 
	//mail_box->need_update = TRUE; 

	mail_box->mail_index++;
	ASSERT( mail_box->mail_index <= mail_box->mail_num ); 

	if( mail_box->mail_index == mail_box->mail_num )
	{
		return POP_QUIT;
	}

	//if( send_command( handler, soc, TEXT( CMD_LIST ), mail_box->mail_index + 1, ErrStr ) == FALSE )
	//{
	//	return POP_ERR;
	//}

	return POP_QUIT;
}

/*
 * pop3_list_proc 
 */
LRESULT pop3_list_proc( email_handler *handler, SOCKET soc, char *buf, int len, TCHAR *ErrStr, BOOL ShowFlag)
{
	LRESULT ret = ERROR_SUCCESS; 
	email_box *mail_box; 

	ASSERT( handler != NULL ); 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	log_trace( ( MSG_INFO, "pop3 response is %s, current action is %ws \n", buf, get_pop3_action_desc( handler->command_status ) ) ); 

	
	mail_box = handler->cur_email; 

	switch( handler->command_status )
	{
	case POP_START:
	case POP_STARTTLS:
	case POP_USER:
	case POP_PASS:
		handler->command_status = login_proc( handler, soc, buf, len, ErrStr, mail_box );
		
		if( handler->command_status == POP_LOGIN ) 
		{
			if( handler->pop_before_smtp == TRUE )
			{
				handler->command_status = POP_QUIT;
			} 
			else
			{
				set_email_process_state( handler, TEXT( CMD_STAT ) TEXT( "\r\n" ) );
				if( send_buf( handler, soc, CMD_STAT "\r\n" ) == -1 )
				{
					lstrcpy( ErrStr, STR_ERR_SOCK_SEND );
					ret = ERROR_SOCKET_ERROR; 
				}
				
				handler->command_status = POP_STAT;
			}
		}
		break;

	case POP_STAT:
		DateAdd( NULL, NULL ); 
		handler->command_status = list_proc_stat( handler, soc, buf, len, ErrStr, mail_box, ShowFlag );
		break;

	case POP_LIST:
		handler->command_status = list_proc_list( handler, soc, buf, len, ErrStr, mail_box, ShowFlag );
		break;

	case POP_UIDL_ALL:
		handler->command_status = list_proc_uidl_all( handler, soc, buf, len, ErrStr, mail_box, ShowFlag );
		break;

	case POP_UIDL_CHECK:
		handler->command_status = list_proc_uidl_check( handler, soc, buf, len, ErrStr, mail_box, ShowFlag );
		break;

	case POP_UIDL:
		handler->command_status = list_proc_uidl_set( handler, soc, buf, len, ErrStr, mail_box, ShowFlag );
		break;

	case POP_TOP:
	case POP_RETR:
		handler->command_status = list_proc_top( handler, soc, buf, len, ErrStr, mail_box, ShowFlag );
		break;

	case POP_DELE:
		{
			handler->command_status = pop3_delete_mail( handler, soc, buf, len, ErrStr, mail_box, ShowFlag );
			break;
		}

	case POP_QUIT:
		if( str_cmp_ni( buf, "+OK", 3 ) == 0 
			|| str_cmp_ni( buf, "-ERR", 3 ) == 0 )
		{
			ASSERT( handler->command_handler != NULL ); 
			if( handler->erc_cmd_record != NULL )
			{
				ret = handler->command_handler( handler, handler->erc_cmd_record ); 
				handler->erc_cmd_record = NULL; 
			}
			ret = ERROR_SUCCESS; 
			goto _return; 
			//SetSocStatusText(hWnd, buf);
		}

		goto _return; 
	}

	switch( handler->command_status )
	{
	case POP_ERR:
		//item_resize_mailbox( mail_box );
		send_buf( handler, soc, CMD_QUIT "\r\n" );
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 

	case POP_QUIT:
		//item_resize_mailbox( mail_box );
		set_email_process_state( handler, TEXT( CMD_QUIT ) );
		
		if( send_buf( handler, soc, CMD_QUIT "\r\n" ) == -1 )
		{
			//lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}
		break;
	}

_return:
	log_trace( ( MSG_INFO, "leave %s\n", __FUNCTION__ ) ); 
	return ret;
}

/*
 * pop3_test_login_proc 
 */
LRESULT pop3_test_login_proc( email_handler *handler, SOCKET soc, char *buf, int len, TCHAR *ErrStr, BOOL ShowFlag)
{
	LRESULT ret = ERROR_SUCCESS; 
	email_box *mail_box; 

	ASSERT( handler != NULL ); 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	log_trace( ( MSG_INFO, "pop3 response is %s, current action is %ws \n", buf, get_pop3_action_desc( handler->command_status ) ) ); 

	
	mail_box = handler->cur_email; 

	switch( handler->command_status )
	{
	case POP_START:
	case POP_STARTTLS:
	case POP_USER:
	case POP_PASS:
		handler->command_status = login_proc( handler, soc, buf, len, ErrStr, mail_box );
		
		if( handler->command_status == POP_LOGIN ) 
		{
			if( handler->pop_before_smtp == TRUE )
			{
				handler->command_status = POP_QUIT;
			} 
			else
			{
				set_email_process_state( handler, TEXT( CMD_STAT ) TEXT( "\r\n" ) );
				if( send_buf( handler, soc, CMD_STAT "\r\n" ) == -1 )
				{
					lstrcpy( ErrStr, STR_ERR_SOCK_SEND );
					ret = ERROR_SOCKET_ERROR; 
				}
				
				handler->command_status = POP_STAT;
			}
		}
		break;
	case POP_STAT: 
		send_buf( handler, soc, CMD_QUIT "\r\n" );
		handler->command_status = POP_QUIT;
		goto _return; 
		break; 
	case POP_QUIT:
		if( str_cmp_ni( buf, "+OK", 3 ) != 0 )
		{
			ASSERT( str_cmp_ni( buf, "-ERR", 3 ) == 0 ); 

			//ret = ERROR_ERRORS_ENCOUNTERED; 
			closesocket( soc ); 
		}

		goto _return; 
	default:
		ASSERT( FALSE && "invalid pop action " ); 
		break; 
	}

	switch( handler->command_status )
	{
	case POP_ERR:
		//item_resize_mailbox( mail_box );
		send_buf( handler, soc, CMD_QUIT "\r\n" );
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 

	case POP_QUIT:
		//item_resize_mailbox( mail_box );
		set_email_process_state( handler, TEXT( CMD_QUIT ) );
		
		if( send_buf( handler, soc, CMD_QUIT "\r\n" ) == -1 )
		{
			//lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}
		break;
	}

_return:
	log_trace( ( MSG_INFO, "leave %s\n", __FUNCTION__ ) ); 
	return ret;
}


/*
 * pop3_exec_proc - 
 */
LRESULT pop3_exec_proc( email_handler *handler, SOCKET soc, char *buf, int len, TCHAR *ErrStr, BOOL ShowFlag)
{
	LRESULT ret = ERROR_SUCCESS; 

	email_box *mail_box;  
	ASSERT( handler != NULL ); 

	mail_box = handler->cur_email; 

	switch( handler->command_status )
	{
	case POP_START:
	case POP_STARTTLS:
	case POP_USER:
	case POP_PASS:
		handler->command_status = login_proc( handler, soc, buf, len, ErrStr, mail_box );
		
		if( handler->command_status == POP_LOGIN )
		{
			handler->command_status = exec_proc_init( handler, soc, 0, buf, len, ErrStr, mail_box, ShowFlag );
		}
		break;

	case POP_RETR:
		handler->command_status = exec_proc_retr(handler, soc, buf, len, ErrStr, mail_box, ShowFlag);
		break;

	case POP_UIDL:
		handler->command_status = exec_proc_uidl(handler, soc, buf, len, ErrStr, mail_box, ShowFlag);
		break;

	case POP_TOP:
		handler->command_status = exec_proc_top(handler, soc, buf, len, ErrStr, mail_box, ShowFlag);
		break;

	case POP_DELE:
		handler->command_status = exec_proc_dele(handler, soc, buf, len, ErrStr, mail_box, ShowFlag);
		break;

	case POP_QUIT:
		if (str_cmp_ni(buf, "+OK", 3) == 0 || str_cmp_ni(buf, "-ERR", 3) == 0) {
			//SetSocStatusText(hWnd, buf);
		}
		goto _return; 
	}

	switch( handler->command_status ) 
	{
	case POP_ERR:
		send_buf( handler, soc, CMD_QUIT"\r\n");
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
		break; 

	case POP_QUIT:
		set_email_process_state( handler, TEXT( CMD_QUIT ) );
		if( send_buf( handler, soc, CMD_QUIT "\r\n" ) == -1 ) 
		{
			//lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}
		break;
	}

_return:
	return ret;
}

/*
 * pop3_free - POP3
 */
//void pop3_free(void)
//{
//	mail_buf_free();
//	uidl_free();
//}
/* End of source */
