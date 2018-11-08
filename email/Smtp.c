/*
 * ERC
 *
 * Smtp.c (RFC 821, RFC 2821)
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
#include "code.h"
#include "mime.h"
#include "multipart.h"

#include "global.h"
#include "md5.h"

/* Define */
#define SEND_SIZE				4096			
#define SMTP_ERRORSTATUS		400				// SMTP
#define MIME_VERSION			"1.0"			// MIME

#define AUTH_CRAM_MD5			0				// SMTP-AUTH の認証タイプ
#define AUTH_LOGIN				1
#define AUTH_PLAIN				2
#define AUTH_NON				99

#define CMD_HELO				"HELO"
#define CMD_EHLO				"EHLO"
#define CMD_STARTTLS			"STARTTLS"
#define CMD_AUTH				"AUTH"
#define CMD_AUTH_CRAM_MD5		"CRAM-MD5"
#define CMD_AUTH_LOGIN			"LOGIN"
#define CMD_AUTH_PLAIN			"PLAIN"
#define CMD_RSET				"RSET"
#define CMD_MAIL_FROM			"MAIL FROM"
#define CMD_RCPT_TO				"RCPT TO"
#define CMD_DATA				"DATA"
#define CMD_DATA_END			"\r\n.\r\n"
#define CMD_QUIT				"QUIT"

//extern OPTION op;

//extern MAILBOX *MailBox;
int MailBoxCnt = 1;
int SelBox = 1;

//extern BOOL GetHostFlag;
extern int ssl_type;

//extern int command_status;
extern TCHAR *g_Pass;

/* Local Function Prototypes */
static int check_response(char *buf);
static BOOL check_starttls(char *buf);
static BOOL auth_get_type(char *buf, int *type);
static TCHAR *auth_create_cram_md5(char *buf, TCHAR *user, TCHAR *pass, TCHAR *ErrStr);
static TCHAR *auth_create_plain(TCHAR *user, TCHAR *pass, TCHAR *ErrStr);
static TCHAR *auth_get_password(email_box *mail_box);
static BOOL send_address( email_handler *handler, SOCKET soc, TCHAR *command, TCHAR *address, TCHAR *ErrStr);
static TCHAR *send_rcpt_to(email_handler *handler, SOCKET soc, TCHAR *address, TCHAR *ErrStr);
static BOOL send_header_t( email_handler *handler, SOCKET soc, TCHAR *header, TCHAR *content, TCHAR *ErrStr);
#ifdef UNICODE
static BOOL send_header( email_handler *handler, SOCKET soc, char *header, char *content, TCHAR *ErrStr);
#else
#define send_header send_header_t
#endif
static BOOL send_mime_header( email_handler* handler, SOCKET soc, MAILITEM *mail_item, TCHAR *header, TCHAR *content, BOOL address, TCHAR *ErrStr);
static BOOL send_mail_data( email_handler *handler, SOCKET soc, MAILITEM *mail_item, TCHAR *ErrStr);
static LRESULT send_mail_proc( email_handler *handler, 
					   //email_box *user_info, 
					   SOCKET soc, 
					   char *buf, 
					   TCHAR *ErrStr, 
					   MAILITEM *mail_item, 
					   BOOL ShowFlag ); 

/*
 * HMAC_MD5 - MD5
 */
void HMAC_MD5(unsigned char *input, int len, unsigned char *key, int keylen, unsigned char *digest)
{
    MD5_CTX context;
    MD5_CTX tctx;
    unsigned char k_ipad[65];
    unsigned char k_opad[65];
    unsigned char tk[16];
    int i;

	// キーが64バイトより大きい場合はキーのダイジェストをキーにする
    if (keylen > 64) {
        MD5Init(&tctx);
        MD5Update(&tctx, key, keylen);
        MD5Final(tk, &tctx);

        key = tk;
        keylen = 16;
    }

	// MD5(key XOR opad, MD5(key XOR ipad, input)) 
    FillMemory(k_ipad, sizeof(k_ipad), 0x36);
    FillMemory(k_opad, sizeof(k_opad), 0x5c);
    for (i = 0; i < keylen; i++) {
        *(k_ipad + i) ^= *(key + i);
        *(k_opad + i) ^= *(key + i);
    }

    MD5Init(&context);
    MD5Update(&context, k_ipad, 64);
    MD5Update(&context, input, len);
    MD5Final(digest, &context);

    MD5Init(&context);
    MD5Update(&context, k_opad, 64);
    MD5Update(&context, digest, 16);
    MD5Final(digest, &context);
}

/*
 * check_response - SMTP
 */
static int check_response(char *buf)
{
	char stat[5];
	int ret;

	if (tstrlen(buf) > 3 && *(buf + 3) == '-') {
		return -1;
	}

	str_cpy_n(stat, buf, 4);
	ret = a2i(stat);
	if (ret <= 0) {
		ret = SMTP_ERRORSTATUS;
	}
	return ret;
}

/*
 * check_starttls - STARTTLSが利用可能かチェック
 */
static BOOL check_starttls(char *buf)
{
	char *p;

	for (p = buf; *p != '\0' && *p != ' ' && *p != '-'; p++);
	for (; *p == ' ' || *p == '-'; p++);
	if (*p == '\0') {
		return FALSE;
	}
	// STARTTLS
	if (str_cmp_ni(p, CMD_STARTTLS, tstrlen(CMD_STARTTLS)) == 0) {
		return TRUE;
	}
	return FALSE;
}

/*
 * auth_get_type - 利用可能な SMTP AUTH の種類を取得 (RFC 2554)
 */
static BOOL auth_get_type(char *buf, int *type)
{
	char *p, *r;

	for (p = buf; *p != '\0' && *p != ' ' && *p != '-'; p++);
	for (; *p == ' ' || *p == '-'; p++);
	if (*p == '\0') {
		return FALSE;
	}
	// AUTH
	if (str_cmp_ni(p, CMD_AUTH, tstrlen(CMD_AUTH)) != 0) {
		return FALSE;
	}
	for (; *p != '\0' && *p != ' ' && *p != '='; p++);
	for (; *p == ' ' || *p == '='; p++);
	if (*p == '\0') {
		return FALSE;
	}

	// CRAM-MD5
	r = p;
	while (*r != '\0') {
		if (str_cmp_ni(r, CMD_AUTH_CRAM_MD5, tstrlen(CMD_AUTH_CRAM_MD5)) == 0) {
			*type = AUTH_CRAM_MD5;
			break;
		}
		for (; *r != '\0' && *r != ' '; r++);
		for (; *r == ' '; r++);
	}
	if (*type == AUTH_CRAM_MD5) {
		return TRUE;
	}

	// LOGIN
	r = p;
	while (*r != '\0') {
		if (str_cmp_ni(r, CMD_AUTH_LOGIN, tstrlen(CMD_AUTH_LOGIN)) == 0) {
			*type = AUTH_LOGIN;
			break;
		}
		for (; *r != '\0' && *r != ' '; r++);
		for (; *r == ' '; r++);
	}
	if (*type == AUTH_LOGIN) {
		return TRUE;
	}

	// PLAIN
	r = p;
	while (*r != '\0') {
		if (str_cmp_ni(r, CMD_AUTH_PLAIN, tstrlen(CMD_AUTH_PLAIN)) == 0) {
			*type = AUTH_PLAIN;
			break;
		}
		for (; *r != '\0' && *r != ' '; r++);
		for (; *r == ' '; r++);
	}
	if (*type == AUTH_PLAIN) {
		return TRUE;
	}
	return FALSE;
}

/*
 * auth_create_cram_md5 - AUTH CRAM-MD5 のパスワードを生成する (RFC 2554)
 */
static TCHAR *auth_create_cram_md5(char *buf, TCHAR *user, TCHAR *pass, TCHAR *ErrStr)
{
	TCHAR *ret;
	char *input, *key;
	char *p;
	unsigned char digest[16];
	int len;
	int i;

#ifdef UNICODE
	key = alloc_tchar_to_char(pass);
	if (key == NULL) {
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return NULL;
	}
#else
	key = pass;
#endif

	for (p = buf; *p != '\0' && *p != ' '; p++);
	for (; *p == ' '; p++);

	input = (char *)mem_alloc(tstrlen(p) + 1);
	if (input == NULL) {
#ifdef UNICODE
		mem_free(&key);
#endif
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return NULL;
	}
	base64_decode(p, input);

	// ダイジェスト値の取得
	HMAC_MD5(input, tstrlen(input), key, tstrlen(key), digest);
#ifdef UNICODE
	mem_free(&key);
#endif
	mem_free(&input);

	ret = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(TEXT(" \r\n")) + lstrlen(user) + (16 * 2) + 1));
	if (ret == NULL) {
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return NULL;
	}
	len = wsprintf(ret, TEXT("%s "), user);
	for (i = 0; i < 16; i++, len += 2) {
		wsprintf(ret + len, TEXT("%02x"), digest[i]);
	}
	return ret;
}

/*
 * auth_create_plain - AUTH PLAIN の文字列を生成する (RFC 2554)
 */
static TCHAR *auth_create_plain(TCHAR *user, TCHAR *pass, TCHAR *ErrStr)
{
#ifdef UNICODE
	TCHAR *ret;
#endif
	char *c_user, *c_pass;
	char *buf, *tmp;
	int len;

#ifdef UNICODE
	c_user = alloc_tchar_to_char(user);
	if (c_user == NULL) {
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return NULL;
	}
	c_pass = alloc_tchar_to_char(pass);
	if (c_pass == NULL) {
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return NULL;
	}
#else
	c_user = user;
	c_pass = pass;
#endif

	len = tstrlen(c_user) + tstrlen(c_pass) + 2;

	tmp = (char *)mem_calloc(len + 1);
	if (tmp == NULL) {
#ifdef UNICODE
		mem_free(&c_user);
		mem_free(&c_pass);
#endif
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return NULL;
	}
	tstrcpy(tmp + 1, c_user);
	tstrcpy(tmp + 1 + tstrlen(c_user) + 1, c_pass);

#ifdef UNICODE
	mem_free(&c_user);
	mem_free(&c_pass);
#endif

	buf = (char *)mem_calloc(len * 2 + 4);
	if (buf == NULL) {
		mem_free(&tmp);
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return NULL;
	}
	base64_encode(tmp, buf, len);
	mem_free(&tmp);
#ifdef UNICODE
	ret = alloc_char_to_tchar(buf);
	mem_free(&buf);
	return ret;
#else
	return buf;
#endif
}

/*
 * auth_get_password 
 */
static TCHAR *auth_get_password( email_box *user_info )
{
	TCHAR *r;

	r = user_info->pwd; 

	return r;
}

/*
 * send_address 
 */
static BOOL send_address( email_handler *handler, SOCKET soc, TCHAR *command, TCHAR *address, TCHAR *ErrStr)
{
	TCHAR *wbuf;

	wbuf = ( TCHAR* )mem_alloc( sizeof( TCHAR ) * ( lstrlen( TEXT( ":<>\r\n" ) ) + lstrlen( command ) + lstrlen( address ) + 1 ) );
	if( wbuf == NULL )
	{
		lstrcpy( ErrStr, STR_ERR_MEMALLOC );
		return FALSE;
	}

	// command:<address>\r\n
	str_join_t( wbuf, command, TEXT( ":<" ), address, TEXT( ">\r\n" ), ( TCHAR* )-1 );

	set_email_process_state( handler, wbuf);
	
	if( send_buf_t( handler, soc, wbuf ) == -1 )
	{
		mem_free(&wbuf);
		lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
		return FALSE;
	}

	mem_free( &wbuf ); 
	return TRUE;
}

/*
 * send_rcpt_to - RCPT TO
 */
static TCHAR *send_rcpt_to( email_handler *handler, SOCKET soc, TCHAR *address, TCHAR *ErrStr)
{
	TCHAR *p;

	// メールアドレスの抽出
	p = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(address) + 1));
	if (p == NULL) {
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return (TCHAR *)-1;
	}
	address = GetMailAddress(address, p, FALSE);

	// メールアドレスの送信
	if (send_address( handler, soc, TEXT( CMD_RCPT_TO ), p, ErrStr ) == FALSE )
	{
		mem_free( &p );
		return ( TCHAR * )-1;
	}

	mem_free( &p );
	return ( ( *address != TEXT( '\0' ) ) ? address + 1 : address );
}

/*
 * send_header_t - 
 */
static BOOL send_header_t( email_handler *handler, SOCKET soc, TCHAR *header, TCHAR *content, TCHAR *ErrStr )
{
	TCHAR *buf;

	if( content == NULL || *content == TEXT( '\0' ) ) 
	{
		return TRUE;
	}
	
	buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(header) + 1 + lstrlen(content) + 2 + 1));
	if (buf == NULL) {
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return FALSE;
	}

	str_join_t(buf, header, TEXT(" "), content, TEXT("\r\n"), (TCHAR *)-1);
	if (send_buf_t( handler, soc, buf) == -1) {
		mem_free(&buf);
		lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
		return FALSE;
	}

	mem_free(&buf);
	return TRUE;
}

/*
 * send_header_t - ヘッダ項目の送信
 */
#ifdef UNICODE
static BOOL send_header( email_handler *handler, SOCKET soc, char *header, char *content, TCHAR *ErrStr)
{
	char *buf;

	if (content == NULL || *content == '\0') {
		return TRUE;
	}
	buf = (char *)mem_alloc(sizeof(char) * (tstrlen(header) + 1 + tstrlen(content) + 2 + 1));
	if (buf == NULL) {
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return FALSE;
	}
	str_join(buf, header, " ", content, "\r\n", (char *)-1);
	if (send_buf( handler, soc, buf) == -1) {
		mem_free(&buf);
		lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
		return FALSE;
	}
	mem_free(&buf);
	return TRUE;
}
#endif

/*
 * send_mime_header - ヘッダ項目をMIMEエンコードして送信
 */
static BOOL send_mime_header( email_handler* handler, SOCKET soc, MAILITEM *mail_item, TCHAR *header, TCHAR *content, BOOL address, TCHAR *ErrStr)
{
	TCHAR *p = NULL;
	BOOL ret;

	if (content == NULL || *content == TEXT('\0')) {
		return TRUE;
	}
	if( mail_item->HeadCharset != NULL )
	{
		p = MIME_encode(content, address, mail_item->HeadCharset, mail_item->HeadEncoding);
	}
	else
	{
		p = MIME_encode(content, address, CHARSET_GB2312, 2 );
	}
	
	if( p == NULL )
	{
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return FALSE;
	}
	
	ret = send_header_t( handler, soc, header, p, ErrStr);
	mem_free(&p);
	return ret;
}

/*
 * send_mail_data - ヘッダと本文の送信 (RFC 822, RFC 2822, RFC 2045, RFC 2076)
 */
static BOOL send_mail_data(email_handler *handler, SOCKET soc, MAILITEM *mail_item, TCHAR *ErrStr)
{
#ifdef UNICODE
	TCHAR *body;
#endif
	TCHAR buf[BUF_SIZE];
	TCHAR *p, *r;
	char ctype[BUF_SIZE], enc_type[BUF_SIZE];
	char *mctypr;
	char *mbody;
	CHAR *send_body; 
	email_box *cur_email; 
	int len;

	ASSERT( handler != NULL ); 
	ASSERT( handler->cur_email != NULL ); 

	cur_email = handler->cur_email; 

	// From
	mem_free(&mail_item->From);
	mail_item->From = NULL;

	if( cur_email->email != NULL && *cur_email->email != TEXT( '\0' ) ) 
	{
		len = lstrlen(TEXT(" <>"));
		p = NULL;

		if( *cur_email->name != '\0' )
		{
			p = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen( cur_email->name ) + 1 ) );
			if (p != NULL) {
				SetUserName( cur_email->name, p );
				len += lstrlen(p);
			}
		}
		len += lstrlen( cur_email->email ); 

		// From
		mail_item->From = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
		
		if( mail_item->From != NULL ) 
		{
			r = mail_item->From;
			if( p != NULL && *p != TEXT( '\0' ) )
			{
				r = str_join_t( r, p, TEXT( " " ), ( TCHAR* )-1 );
			}

			str_join_t( r, TEXT( "<" ), cur_email->email, TEXT( ">" ), ( TCHAR* )-1 );
			
			if( send_mime_header( handler, soc, mail_item, TEXT( HEAD_FROM ), mail_item->From, TRUE, ErrStr ) == FALSE ) 
			{
				return FALSE;
			}
		}
		mem_free(&p);
	}

	// To
	if (send_mime_header( handler, soc, mail_item, TEXT(HEAD_TO), mail_item->To, TRUE, ErrStr) == FALSE) {
		return FALSE;
	}
	// Cc
	if (send_mime_header( handler, soc, mail_item, TEXT(HEAD_CC), mail_item->Cc, TRUE, ErrStr) == FALSE) {
		return FALSE;
	}

	// Date
	GetTimeString( buf );
	if( handler->send_date == TRUE 
		&& send_header_t( handler, soc, TEXT(HEAD_DATE), buf, ErrStr ) == FALSE) 
	{
		return FALSE;
	}

	if( mail_item->Date != NULL )
	{
		mem_free( &mail_item->Date );
	}

#ifdef UNICODE
	{
		char *cbuf, *dbuf;

		cbuf = alloc_tchar_to_char(buf);
		if (cbuf != NULL) {
			dbuf = (char *)mem_alloc(BUF_SIZE);
			if (dbuf != NULL) {
				DateConv(cbuf, dbuf);
				mail_item->Date = alloc_char_to_tchar(dbuf);
				mem_free(&dbuf);
			}
			mem_free(&cbuf);
		}
	}
#else
	mail_item->Date = (char *)mem_alloc(BUF_SIZE);
	if (mail_item->Date != NULL) {
		DateConv(buf, mail_item->Date);
	}
#endif
	// Subject
	if (send_mime_header( handler, soc, mail_item, TEXT(HEAD_SUBJECT), mail_item->Subject, FALSE, ErrStr) == FALSE) {
		return FALSE;
	}
	// Reply-To
	if (mail_item->ReplyTo != NULL && *mail_item->ReplyTo != TEXT('\0')) {
		// Reply-To
		if (send_mime_header( handler, soc, mail_item, TEXT(HEAD_REPLYTO), mail_item->ReplyTo, TRUE, ErrStr) == FALSE) {
			return FALSE;
		}
	} 
	//else 
	//{
	//	// メールボックスに設定されている Reply-To
	//	if (send_mime_header(soc, mail_item, TEXT(HEAD_REPLYTO), send_mail_box->ReplyTo, TRUE, ErrStr) == FALSE) {
	//		return FALSE;
	//	}
	//}
	// In-Reply-To
	if( send_header_t( handler, soc, TEXT( HEAD_INREPLYTO ), mail_item->InReplyTo, ErrStr ) == FALSE ) {
		return FALSE;
	}

	// References
	if( send_header_t( handler, soc, TEXT( HEAD_REFERENCES ), mail_item->References, ErrStr ) == FALSE ) {
		return FALSE;
	}

	if( handler->send_msg_id == TRUE ) 
	{
		// Message-Id
		mem_free(&mail_item->MessageID);
		mail_item->MessageID = CreateMessageId((long)mail_item, cur_email->email );
		
		if (mail_item->MessageID == NULL) {
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return FALSE;
		}

		if (send_header_t( handler, soc, TEXT(HEAD_MESSAGEID), mail_item->MessageID, ErrStr) == FALSE) {
			return FALSE;
		}
	}

	// MIME-Version
	if (send_header_t( handler, soc, TEXT(HEAD_MIMEVERSION), TEXT(MIME_VERSION), ErrStr) == FALSE) {
		return FALSE;
	}

	if( mail_item->BodyCharset != NULL ) {
		send_body = alloc_copy(mail_item->Body);
		MIME_create_encode_header(mail_item->BodyCharset, mail_item->BodyEncoding, ctype, enc_type);
	} 
	else 
	{
		//__asm int 3; 
//#ifdef UNICODE
//		body = alloc_char_to_tchar(mail_item->Body);
//		send_body = MIME_body_encode(body, handler->body_charset, op.BodyEncoding, ctype, enc_type, ErrStr);
//		mem_free(&body);
//#else
//		send_body = MIME_body_encode(mail_item->Body, op.BodyCharset, op.BodyEncoding, ctype, enc_type, ErrStr);
//#endif
	}

	if (send_body == NULL) {
		return FALSE;
	}
	
	switch( multipart_create( mail_item->Attach, ctype, enc_type, &mctypr, send_body, &mbody ) ) 
	{
	case MP_ERROR_FILE:
		mem_free(&send_body);
		send_body = NULL;
		lstrcpy(ErrStr, STR_ERR_SOCK_NOATTACH);
		return FALSE;

	case MP_ERROR_ALLOC:
		mem_free(&send_body);
		send_body = NULL;
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return FALSE;

	case MP_NO_ATTACH:
		// Content-Type
		if (send_header(handler, soc, HEAD_CONTENTTYPE, ctype, ErrStr) == FALSE) {
			mem_free(&send_body);
			send_body = NULL;
			return FALSE;
		}
		// Content-Transfer-Encoding
		if (send_header(handler, soc, HEAD_ENCODING, enc_type, ErrStr) == FALSE) {
			mem_free(&send_body);
			send_body = NULL;
			return FALSE;
		}
		break;

	case MP_ATTACH:
		mem_free(&send_body);
		send_body = mbody;
		// Content-Type
		if (send_header(handler, soc, HEAD_CONTENTTYPE, mctypr, ErrStr) == FALSE) {
			mem_free(&send_body);
			send_body = NULL;
			mem_free(&mctypr);
			return FALSE;
		}
		mem_free(&mctypr);
		break;
	}

	// X-Mailer
	if (send_header_t(handler, soc, TEXT(HEAD_X_MAILER), APP_NAME, ErrStr) == FALSE) {
		mem_free(&send_body);
		send_body = NULL;
		return FALSE;
	}
	
	if (send_buf(handler, soc, "\r\n") == -1) {
		mem_free(&send_body);
		send_body = NULL;
		lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
		return FALSE;
	}

#ifdef WSAASYNC
	handler->send_pt = send_body;
	handler->send_len = tstrlen(send_body);

	if (WSAAsyncSelect(soc, NULL, WM_SOCK_SELECT, FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR) {
		mem_free(&send_body);
		send_body = NULL;
		lstrcpy(ErrStr, STR_ERR_SOCK_EVENT);
		return FALSE;
	}
#else
	// 本文送信
	if (send_body != NULL && send_buf(handler, soc, send_body) == -1) {
		mem_free(&send_body);
		send_body = NULL;
		lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
		return FALSE;
	}
	mem_free( &send_body );
	send_body = NULL;

	if( send_buf( handler, soc, CMD_DATA_END ) == -1 )
	{
		lstrcpy( ErrStr, STR_ERR_SOCK_SEND );
		return FALSE;
	}

#endif
	return TRUE;
}

/*
 * send_mail_proc - SMTP (RFC 821, RFC 2821)
 */
LRESULT send_mail_proc( email_handler *handler, 
					   //email_box *user_info, 
					   SOCKET soc, 
					   char *buf, 
					   TCHAR *ErrStr, 
					   MAILITEM *mail_item, 
					   BOOL ShowFlag ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	static TCHAR *To, *Cc, *Bcc, *MyBcc;
	//HWND hListView;
	TCHAR *wbuf;
	TCHAR *user;
	TCHAR *pass;
	TCHAR *p, *r;
	ULONG command_status; 
	email_box *user_info; 
	int status = 0;
	int i;

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	log_trace( ( MSG_INFO, "smtp response is %s\n", buf ) ) ; 

	if( handler == NULL )
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	if( handler->cur_email == NULL )
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	user_info = handler->cur_email; 

	command_status = handler->command_status; 

	if( buf != NULL )
	{
		set_email_process_state( handler, ( LPTSTR )buf );

		if( handler->auth_flag == TRUE 
			&& command_status == SMTP_EHLO )
		{
			auth_get_type( buf, &handler->auth_type );
		}

		if( command_status == SMTP_EHLO ) 
		{
			if( check_starttls( buf ) == TRUE ) 
			{
				handler->starttls_flag = TRUE;
			}
		}

		status = check_response( buf ); 
		if( status == -1 ) 
		{
			goto _return; 
		}
	}


	log_trace( ( MSG_INFO, "current smtp action is %d(%ws) \n", 
		command_status, 
		get_smtp_action_desc( command_status ) ) ); 

	switch( command_status ) 
	{
	case SMTP_START:
	case SMTP_STARTTLS:
		if( status >= SMTP_ERRORSTATUS ) 
		{
			lstrcpy( ErrStr, STR_ERR_SOCK_RESPONSE ); 
			str_cat_n( ErrStr, buf, BUF_SIZE - 1 ); 

			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}

		if( /*user_info->email == NULL || */ *user_info->email == TEXT( '\0' ) ) 
		{
			lstrcpy( ErrStr, STR_ERR_SOCK_BADFROM ); 

			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}

		p = user_info->email;

		for( ; *p != TEXT( '@' ) 
			&& *p != TEXT( '\0' ); 
			p++ );

		if( *p == TEXT( '\0' ) )
		{
			lstrcpy( ErrStr, STR_ERR_SOCK_BADFROM );
			ret = ERROR_INVALID_PARAMETER; 
		}
		p++;

		// STARTTLS
		if( command_status == SMTP_STARTTLS )
		{
			handler->ssl_type = 1;

			ret = init_ssl( handler, soc, ErrStr ); 
			if( ret != ERROR_SUCCESS ) 
			{
				goto _return; 
			}
		}

		// HELO \r\n
		r = TEXT( CMD_HELO )TEXT( " " ); 

		// HELO \r\n
		//r = TEXT(CMD_HELO)TEXT(" ");
		//command_status = SMTP_HELO;
		//if (send_mail_box->SmtpSSL == 1 && send_mail_box->SmtpSSLInfo.Type == 4) {
		//	r = TEXT(CMD_EHLO)TEXT(" ");
		//	command_status = SMTP_EHLO;
		//}
		//if (auth_flag == FALSE && send_mail_box->SmtpAuth == 1) {
		//	// SMTP認証
		//	auth_flag = TRUE;
		//	auth_type = AUTH_NON;
		//	r = TEXT(CMD_EHLO)TEXT(" ");
		//	command_status = SMTP_EHLO;
		//} else {
		//	auth_flag = FALSE;
		//	if (op.ESMTP != 0) {
		//		// ESMTP
		//		r = TEXT(CMD_EHLO)TEXT(" ");
		//		command_status = SMTP_EHLO;
		//	}
		//}

		handler->command_status = SMTP_HELO;
		if( user_info->use_ssl == TRUE && user_info->ssl_info.Type == 4 ) 
		{
			r = TEXT( CMD_EHLO )TEXT( " " ); 
			handler->command_status = SMTP_EHLO; 
		}

		if( handler->auth_flag == FALSE \
			&& *user_info->pwd != _T( '\0' ) ) 
		{
			// SMTP
			handler->auth_flag = TRUE; 
			handler->auth_type = AUTH_NON;
			r = TEXT( CMD_EHLO )TEXT( " " );
			handler->command_status = SMTP_EHLO;
		} 
		else 
		{
			if( handler->is_esmtp == TRUE )
			{
				// ESMTP
				r = TEXT( CMD_EHLO )TEXT( " " );
				handler->command_status = SMTP_EHLO;
			}
		}

		wbuf = ( TCHAR * )mem_alloc( sizeof( TCHAR ) * ( lstrlen( r ) + lstrlen( p ) + lstrlen( TEXT( "\r\n" ) ) + 1 ) );

		if( wbuf == NULL )
		{
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return FALSE;
		}

		str_join_t( wbuf, r, p, TEXT( "\r\n" ), ( TCHAR* )-1 );
		set_email_process_state( handler, wbuf );

		if( send_buf_t( handler, soc, wbuf ) == -1 )
		{
			mem_free(&wbuf);
			lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		mem_free( &wbuf );
		break;

	case SMTP_EHLO:
		if( status >= SMTP_ERRORSTATUS )
		{
			// EHLO HELO 
			command_status = SMTP_START;
			return send_mail_proc( handler, soc, NULL, ErrStr, mail_item, ShowFlag );
		}

		if( handler->starttls_flag == TRUE 
			&& user_info->use_ssl == TRUE 
			&& user_info->ssl_info.Type == 4 )
		{
			// STARTTLS
			set_email_process_state( handler, TEXT( CMD_STARTTLS ) );

			if( send_buf( handler, soc, CMD_STARTTLS "\r\n" ) == -1 )
			{
				lstrcpy( ErrStr, STR_ERR_SOCK_SEND ); 
				ret = ERROR_ERRORS_ENCOUNTERED; 
				goto _return; 
			}

			handler->auth_flag = FALSE;
			handler->starttls_flag = FALSE;
			handler->command_status = SMTP_STARTTLS;
			break;
		}
		if( *user_info->pwd == _T( '\0' ) )
		{
			set_email_process_state( handler, TEXT( CMD_RSET ) ); 

			if( send_buf( handler, soc, CMD_RSET "\r\n" ) == -1 )
			{
				lstrcpy( ErrStr, STR_ERR_SOCK_SEND );
				ret = ERROR_ERRORS_ENCOUNTERED; 
				goto _return; 
			}

			handler->command_status = SMTP_RSET;
			break;
		}

		if( user_info->smtp_auth_type != 0 ) 
		{
			handler->auth_type = user_info->smtp_auth_type - 1;
		}

		switch( handler->auth_type )
		{
		case AUTH_CRAM_MD5:
		case AUTH_NON:
		default:
			// AUTH CRAM-MD5
			set_email_process_state( handler, TEXT( CMD_AUTH )TEXT( " " )TEXT( CMD_AUTH_CRAM_MD5 ) );
			if( send_buf( handler, soc, CMD_AUTH" "CMD_AUTH_CRAM_MD5"\r\n" ) == -1 ) 
			{
				lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
				ret = ERROR_ERRORS_ENCOUNTERED; 
				goto _return; 
			}

			handler->command_status = SMTP_AUTH_CRAM_MD5;
			break;

		case AUTH_LOGIN:
			// AUTH LOGIN
			set_email_process_state( handler, TEXT(CMD_AUTH)TEXT(" ")TEXT(CMD_AUTH_LOGIN));

			if( send_buf( handler, soc, CMD_AUTH " " CMD_AUTH_LOGIN "\r\n" ) == -1 ) 
			{
				lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
				ret = ERROR_ERRORS_ENCOUNTERED; 
				goto _return; 
			}

			handler->command_status = SMTP_AUTH_LOGIN;
			break;

		case AUTH_PLAIN:
			// AUTH PLAIN
			user = user_info->email; 

			pass = auth_get_password( user_info );
			
			ASSERT( pass != NULL ); 

			if( *pass == TEXT( '\0' ) ) 
			{
				lstrcpy( ErrStr, STR_ERR_SOCK_NOPASSWORD );
				ret = ERROR_ERRORS_ENCOUNTERED; 
				goto _return; 
			}

			p = auth_create_plain( user, pass, ErrStr ); 

			if( p == NULL ) 
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				goto _return; 
			}

			wbuf = ( TCHAR* )mem_alloc( sizeof( TCHAR ) *
				( lstrlen( TEXT( CMD_AUTH ) ) 
				+ 1 + lstrlen( TEXT( CMD_AUTH_PLAIN ) ) 
				+ 1 + lstrlen( p ) + 2 + 1 ) ); 

			if( wbuf == NULL ) 
			{
				mem_free( &p );
				lstrcpy( ErrStr, STR_ERR_MEMALLOC );
				ret = ERROR_ERRORS_ENCOUNTERED; 
				goto _return; 
			}

			// AUTH PLAIN Base64(<NULL>User<NULL>Pass)[CRLF] 
			str_join_t( wbuf, TEXT( CMD_AUTH )TEXT( " " ) TEXT( CMD_AUTH_PLAIN ) TEXT( " " ), p, TEXT( "\r\n" ), ( TCHAR* )-1 ); 

			set_email_process_state( handler, TEXT( CMD_AUTH ) TEXT( " " ) TEXT( CMD_AUTH_PLAIN ) TEXT( " ****" ) ); 
			
			if( send_buf_t( handler, soc, wbuf ) == -1 ) 
			{
				mem_free( &p );
				mem_free( &wbuf );
				lstrcpy( ErrStr, STR_ERR_SOCK_SEND );
				ret = ERROR_ERRORS_ENCOUNTERED; 
				goto _return; 
			}

			mem_free( &p );
			mem_free( &wbuf );
			
			handler->command_status = SMTP_HELO;
			break;
		}
		break;

	case SMTP_AUTH_CRAM_MD5:
		if( status >= SMTP_ERRORSTATUS ) 
		{
			handler->auth_flag = FALSE;
			lstrcpy( ErrStr, STR_ERR_SOCK_SMTPAUTH );
			str_cat_n( ErrStr, buf, BUF_SIZE - 1 );
			
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		// AUTH CRAM-MD5

		user = user_info->email; 

		pass = auth_get_password( user_info ); 

		if( pass == NULL || *pass == TEXT( '\0' ) ) \
		{
			lstrcpy( ErrStr, STR_ERR_SOCK_NOPASSWORD );
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		wbuf = auth_create_cram_md5( buf, user, pass, ErrStr );
		if( wbuf == NULL )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		p = ( TCHAR* )mem_calloc( sizeof( TCHAR ) * ( lstrlen( wbuf ) * 2 + 4 ) );
		
		if( p == NULL )
		{
			mem_free( &wbuf );
			lstrcpy( ErrStr, STR_ERR_MEMALLOC );
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		//Base64
		base64_encode_t( wbuf, p, 0 );
		mem_free( &wbuf );

		wbuf = ( TCHAR* )mem_alloc( sizeof( TCHAR ) * ( lstrlen( TEXT( "\r\n" ) ) + lstrlen( p ) + 1 ) );
		
		if( wbuf == NULL )
		{
			mem_free( &p );
			lstrcpy( ErrStr, STR_ERR_MEMALLOC );
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		str_join_t( wbuf, p, TEXT( "\r\n" ), ( TCHAR* )-1 );
		
		set_email_process_state( handler, STR_STATUS_SEND_PASS );
		
		if( send_buf_t( handler, soc, wbuf ) == -1 ) 
		{
			mem_free( &p );
			mem_free( &wbuf );
			lstrcpy( ErrStr, STR_ERR_SOCK_SEND );
			
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		mem_free( &p );
		mem_free( &wbuf );
		handler->command_status = SMTP_HELO;
		break;

	case SMTP_AUTH_LOGIN:
		if( status >= SMTP_ERRORSTATUS )
		{
			handler->auth_flag = FALSE;
			lstrcpy(ErrStr, STR_ERR_SOCK_SMTPAUTH);
			str_cat_n(ErrStr, buf, BUF_SIZE - 1);
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		user = user_info->email; 

		p = ( TCHAR* )mem_calloc( sizeof( TCHAR ) * ( lstrlen( user ) * 2 + 4 ) );
		
		if( p == NULL ) 
		{
			lstrcpy( ErrStr, STR_ERR_MEMALLOC );
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		// Base64
		base64_encode_t( user, p, 0 );
		
		wbuf = ( TCHAR* )mem_alloc( sizeof( TCHAR ) * ( lstrlen( TEXT( "\r\n" ) ) + lstrlen( p ) + 1 ) );
		
		if( wbuf == NULL )
		{
			mem_free( &p );
			lstrcpy( ErrStr, STR_ERR_MEMALLOC );
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		str_join_t( wbuf, p, TEXT( "\r\n" ), ( TCHAR* )-1 );
		
		set_email_process_state( handler, STR_STATUS_SEND_USER );
		
		if( send_buf_t( handler, soc, wbuf ) == -1 )
		{
			mem_free( &p );
			mem_free( &wbuf );
			lstrcpy( ErrStr, STR_ERR_SOCK_SEND );
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		mem_free( &p );
		mem_free( &wbuf );
		handler->command_status = SMTP_AUTH_LOGIN_PASS;
		break;

	case SMTP_AUTH_LOGIN_PASS:

		if( status >= SMTP_ERRORSTATUS )
		{
			handler->auth_flag = FALSE;
			
			lstrcpy( ErrStr, STR_ERR_SOCK_SMTPAUTH );
			str_cat_n( ErrStr, buf, BUF_SIZE - 1 );
			
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}
		
		pass = auth_get_password( user_info );
		
		ASSERT( pass != NULL ); 

		if( *pass == TEXT( '\0' ) ) 
		{
			lstrcpy( ErrStr, STR_ERR_SOCK_NOPASSWORD );
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		p = ( TCHAR* )mem_calloc( sizeof( TCHAR ) * ( lstrlen( pass ) * 2 + 4 ) );
		
		if( p == NULL )
		{
			lstrcpy( ErrStr, STR_ERR_MEMALLOC );
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; 
		}

		// Base64
		base64_encode_t( pass, p, 0 );
		
		wbuf = ( TCHAR* )mem_alloc( sizeof( TCHAR ) * ( lstrlen( TEXT( "\r\n" ) ) + lstrlen( p ) + 1 ) );
		if( wbuf == NULL )
		{
			mem_free( &p );
			lstrcpy( ErrStr, STR_ERR_MEMALLOC );
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		str_join_t( wbuf, p, TEXT( "\r\n" ), ( TCHAR* )-1 );
		set_email_process_state( handler, STR_STATUS_SEND_PASS );
		
		if( send_buf_t( handler, soc, wbuf ) == -1 )
		{
			mem_free( &p );
			mem_free( &wbuf );
			
			lstrcpy( ErrStr, STR_ERR_SOCK_SEND );
		
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		mem_free( &p );
		mem_free( &wbuf );
		handler->command_status = SMTP_HELO;
		break;

	case SMTP_HELO:
		if( status >= SMTP_ERRORSTATUS )
		{
			if( handler->auth_flag == FALSE ) {
				lstrcpy(ErrStr, STR_ERR_SOCK_HELO);
			} else {
				handler->auth_flag = FALSE;
				lstrcpy(ErrStr, STR_ERR_SOCK_SMTPAUTH);
			}
			str_cat_n(ErrStr, buf, BUF_SIZE - 1);
			return FALSE;
		}
		

		set_email_process_state( handler, TEXT( CMD_RSET ) );
	
		if( send_buf( handler, soc, CMD_RSET "\r\n" ) == -1 )
		{
			lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			return FALSE;
		}
		
		handler->command_status = SMTP_RSET;
		break;

	case SMTP_RSET:
		if (status >= SMTP_ERRORSTATUS)
		{
			lstrcpy(ErrStr, STR_ERR_SOCK_RSET);
			str_cat_n(ErrStr, buf, BUF_SIZE - 1);
			return FALSE;
		}

		if( send_address( handler, soc, TEXT( CMD_MAIL_FROM ), user_info->email, ErrStr ) == FALSE )
		{
			return FALSE;
		}

		handler->command_status = SMTP_MAILFROM;
		break;

	case SMTP_MAILFROM:
		if (status >= SMTP_ERRORSTATUS) {
			lstrcpy(ErrStr, STR_ERR_SOCK_MAILFROM);
			str_cat_n(ErrStr, buf, BUF_SIZE - 1);
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		if (mail_item->To == NULL) {

			lstrcpy(ErrStr, STR_ERR_SOCK_NOTO);
			ret = ERROR_ERRORS_ENCOUNTERED;
			goto _return; 
		}

		To = mail_item->To;
		Cc = mail_item->Cc;
		Bcc = mail_item->Bcc;

		MyBcc = NULL;
		//if(send_mail_box->MyAddr2Bcc == 1) 
		{
			MyBcc = user_info->email; //(send_mail_box->BccAddr != NULL && *send_mail_box->BccAddr != TEXT('\0')) ? send_mail_box->BccAddr : send_mail_box->MailAddress;
		}

		handler->command_status = SMTP_RCPTTO;
		ret = send_mail_proc( handler, soc, NULL, ErrStr, mail_item, ShowFlag); 
		goto _return; 

	case SMTP_RCPTTO:
		if (status >= SMTP_ERRORSTATUS) {
			lstrcpy(ErrStr, STR_ERR_SOCK_RCPTTO);
			str_cat_n(ErrStr, buf, BUF_SIZE - 1);
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		// To 
		if( To != NULL && *To != TEXT( '\0' ) )
		{
			if( ( To = send_rcpt_to( handler, soc, To, ErrStr )) == ( TCHAR* )-1 ) 
			{
				ret = ERROR_ERRORS_ENCOUNTERED;
				goto _return; 
			}

			handler->command_status = SMTP_RCPTTO;
			break;
		}

		// Cc 
		if( Cc != NULL && *Cc != TEXT( '\0' ) )
		{
			if( ( Cc = send_rcpt_to( handler, soc, Cc, ErrStr ) ) == ( TCHAR* )-1 ) 
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				goto _return; 
			}

			handler->command_status = SMTP_RCPTTO;
			break;
		}

		if( Bcc != NULL && *Bcc != TEXT( '\0' ) )
		{
			if( ( Bcc = send_rcpt_to( handler, soc, Bcc, ErrStr ) ) == ( TCHAR* )-1 )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				goto _return; 
			}

			handler->command_status = SMTP_RCPTTO;
			break;
		}

		if( MyBcc != NULL && *MyBcc != TEXT( '\0' ) )
		{
			if( ( MyBcc = send_rcpt_to( handler, soc, MyBcc, ErrStr ) ) == ( TCHAR* )-1 ) 
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				goto _return; 
			}

			handler->command_status = SMTP_RCPTTO;
			break;
		}
		
		// DATA
		set_email_process_state( handler, TEXT( CMD_DATA ) );
		
		if( send_buf( handler, soc, CMD_DATA "\r\n" ) == -1 )
		{
			lstrcpy( ErrStr, STR_ERR_SOCK_SEND );
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		handler->command_status = SMTP_DATA;
		break;

	case SMTP_DATA:
		if (status >= SMTP_ERRORSTATUS) 
		{
			lstrcpy(ErrStr, STR_ERR_SOCK_DATA);
			str_cat_n(ErrStr, buf, BUF_SIZE - 1);
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		set_email_process_state( handler, STR_STATUS_SENDBODY );
		//SwitchCursor( FALSE );
		if( send_mail_data( handler, soc, mail_item, ErrStr ) == FALSE )
		{
			//SwitchCursor(TRUE);
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		//SwitchCursor( TRUE );

#ifdef WSAASYNC
		handler->command_status = SMTP_SENDBODY;
		break;

	case SMTP_SENDBODY:
		break;
#else
		handler->command_status = handler->send_end_cmd;
		break;
#endif

	case SMTP_NEXTSEND:
		if( status >= SMTP_ERRORSTATUS )
		{
			lstrcpy( ErrStr, STR_ERR_SOCK_MAILSEND );
			str_cat_n( ErrStr, buf, BUF_SIZE - 1 );
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		//mail_item->Status = mail_item->MailStatus = ICON_SENDMAIL;
		//if (ShowFlag == TRUE) {
		//	hListView = GetDlgItem(hWnd, IDC_LISTVIEW);
		//	i = ListView_GetMemToItem(hListView, mail_item);
		//	if (i != -1) {
		//		ListView_RedrawItems(hListView, i, i);
		//		UpdateWindow(hListView);
		//	}
		//}
		//if (op.AutoSave == 1) {
		//	file_save_mailbox(SENDBOX_FILE, MailBox + MAILBOX_SEND, 2);
		//}
		
		//item_get_next_send_mark_mailbox((MailBox + MAILBOX_SEND), -1, mailbox_name_to_index( user_info->name ) );

		set_email_process_state(handler, TEXT(CMD_QUIT));
		if( send_buf( handler, soc, CMD_QUIT "\r\n" ) == -1 )
		{
			lstrcpy( ErrStr, STR_ERR_SOCK_SEND );
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}
		
		handler->command_status = SMTP_QUIT;

		log_trace( ( MSG_INFO, "email send successfully\n" ) ); 

		break;
		//if (send_mail_item == *((MailBox + MAILBOX_SEND)->mail_item + i)) {
		//	lstrcpy(ErrStr, STR_ERR_SOCK_GETITEMINFO);
		//	return FALSE;
		//}
		//send_mail_item = *((MailBox + MAILBOX_SEND)->mail_item + i);
		//if ((send_mail_item->To == NULL || *send_mail_item->To == TEXT('\0')) &&
		//	(send_mail_item->Cc == NULL || *send_mail_item->Cc == TEXT('\0')) &&
		//	(send_mail_item->Bcc == NULL || *send_mail_item->Bcc == TEXT('\0'))) {
		//		lstrcpy(ErrStr, STR_ERR_SOCK_NOTO);
		//		return FALSE;
		//	}

		//set_email_process_state( handler, TEXT( CMD_RSET ) );
		//
		//if( send_buf( handler, soc, CMD_RSET "\r\n" ) == -1 )
		//{
		//	lstrcpy( ErrStr, STR_ERR_SOCK_SEND ); 
		//	ret = ERROR_ERRORS_ENCOUNTERED; 
		//	goto _return; 
		//}

		//handler->command_status = SMTP_RSET;
		//break;

	case SMTP_SENDEND:
		if( status >= SMTP_ERRORSTATUS )
		{
			lstrcpy( ErrStr, STR_ERR_SOCK_MAILSEND );
			str_cat_n( ErrStr, buf, BUF_SIZE - 1 );
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		set_email_process_state( handler, TEXT( CMD_QUIT ) ); 

		if( send_buf( handler, soc, CMD_QUIT "\r\n" ) == -1 )
		{
			lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		//mail_item->Status = mail_item->MailStatus = ICON_SENDMAIL;
		//if (ShowFlag == TRUE) {
		//	hListView = GetDlgItem(hWnd, IDC_LISTVIEW);
		//	i = ListView_GetMemToItem(hListView, mail_item);
		//	if (i != -1) {
		//		ListView_RedrawItems(hListView, i, i);
		//		UpdateWindow(hListView);
		//	}
		//}

		//if( op.AutoSave == TRUE ) 
		//{
		//	file_save_mailbox(SENDBOX_FILE, MailBox + MAILBOX_SEND, 2);
		//}

		handler->command_status = SMTP_QUIT;
		break;

	case SMTP_QUIT:
		
		break;
	}

_return:
	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 

	return ret; 
}

LRESULT smtp_proc( email_handler *handler, 
			   SOCKET soc, 
			   char *buf, 
			   int len, 
			   TCHAR *ErrStr, 
			   BOOL ShowFlag )
{
	LRESULT ret = ERROR_SUCCESS;

	ASSERT( handler != NULL ); 
	ASSERT( handler->cur_email != NULL ); 

	if( handler->send_mail_item == NULL )
	{
		ASSERT( FALSE ); 
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	ret = send_mail_proc( handler, soc, buf, ErrStr, handler->send_mail_item, ShowFlag ); 
	if( ret != ERROR_SUCCESS )
	{
		send_buf( handler, soc, CMD_QUIT"\r\n" ); 

		smtp_set_error( handler );
	}

_return:
	return ret;
}

LRESULT smtp_login_proc( email_handler *handler, 
				  SOCKET soc, 
				  char *buf, 
				  int len, 
				  TCHAR *ErrStr, 
				  BOOL ShowFlag )
{
	LRESULT ret = ERROR_SUCCESS; 
	static TCHAR *To, *Cc, *Bcc, *MyBcc;
	//HWND hListView;
	TCHAR *wbuf;
	TCHAR *user;
	TCHAR *pass;
	TCHAR *p, *r;
	ULONG command_status; 
	email_box *user_info; 
	int status = 0;
	int i;

	ASSERT( handler != NULL ); 
	ASSERT( handler->cur_email != NULL ); 

	if( handler->send_mail_item == NULL )
	{
		ASSERT( FALSE ); 
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	log_trace( ( MSG_INFO, "smtp response is %s\n", buf ) ) ; 

	if( handler == NULL )
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	if( handler->cur_email == NULL )
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	user_info = handler->cur_email; 

	command_status = handler->command_status; 

	if( buf != NULL )
	{
		set_email_process_state( handler, ( LPTSTR )buf );

		if( handler->auth_flag == TRUE 
			&& command_status == SMTP_EHLO )
		{
			auth_get_type( buf, &handler->auth_type );
		}

		if( command_status == SMTP_EHLO ) 
		{
			if( check_starttls( buf ) == TRUE ) 
			{
				handler->starttls_flag = TRUE;
			}
		}

		status = check_response( buf ); 
		if( status == -1 ) 
		{
			goto _return; 
		}
	}

	log_trace( ( MSG_INFO, "current smtp action is %d(%ws) \n", 
		command_status, 
		get_smtp_action_desc( command_status ) ) ); 

	switch( command_status ) 
	{
	case SMTP_START:
	case SMTP_STARTTLS:
		if( status >= SMTP_ERRORSTATUS ) 
		{
			lstrcpy( ErrStr, STR_ERR_SOCK_RESPONSE ); 
			str_cat_n( ErrStr, buf, BUF_SIZE - 1 ); 

			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}

		if( /*user_info->email == NULL || */ *user_info->email == TEXT( '\0' ) ) 
		{
			lstrcpy( ErrStr, STR_ERR_SOCK_BADFROM ); 

			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}

		p = user_info->email;

		for( ; *p != TEXT( '@' ) 
			&& *p != TEXT( '\0' ); 
			p++ );

		if( *p == TEXT( '\0' ) )
		{
			lstrcpy( ErrStr, STR_ERR_SOCK_BADFROM );
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}

		p++;

		// STARTTLS
		if( command_status == SMTP_STARTTLS )
		{
			handler->ssl_type = 1;

			ret = init_ssl( handler, soc, ErrStr ); 
			if( ret != ERROR_SUCCESS ) 
			{
				goto _return; 
			}
		}

		// HELO \r\n
		r = TEXT( CMD_HELO )TEXT( " " ); 

		// HELO \r\n
		//r = TEXT(CMD_HELO)TEXT(" ");
		//command_status = SMTP_HELO;
		//if (send_mail_box->SmtpSSL == 1 && send_mail_box->SmtpSSLInfo.Type == 4) {
		//	r = TEXT(CMD_EHLO)TEXT(" ");
		//	command_status = SMTP_EHLO;
		//}
		//if (auth_flag == FALSE && send_mail_box->SmtpAuth == 1) {
		//	// SMTP
		//	auth_flag = TRUE;
		//	auth_type = AUTH_NON;
		//	r = TEXT(CMD_EHLO)TEXT(" ");
		//	command_status = SMTP_EHLO;
		//} else {
		//	auth_flag = FALSE;
		//	if (op.ESMTP != 0) {
		//		// ESMTP
		//		r = TEXT(CMD_EHLO)TEXT(" ");
		//		command_status = SMTP_EHLO;
		//	}
		//}

		handler->command_status = SMTP_QUIT;
		if( user_info->use_ssl == TRUE && user_info->ssl_info.Type == 4 ) 
		{
			r = TEXT( CMD_EHLO )TEXT( " " ); 
			handler->command_status = SMTP_EHLO; 
		}

		if( handler->auth_flag == FALSE 
			&& *user_info->pwd != '\0' ) 
		{
			// SMTP
			handler->auth_flag = TRUE; 
			handler->auth_type = AUTH_NON;
			r = TEXT( CMD_EHLO )TEXT( " " );
			handler->command_status = SMTP_EHLO;
		} 
		else 
		{
			if( handler->is_esmtp == TRUE )
			{
				// ESMTP
				r = TEXT( CMD_EHLO )TEXT( " " );
				handler->command_status = SMTP_EHLO;
			}
		}

		wbuf = ( TCHAR * )mem_alloc( sizeof( TCHAR ) * ( lstrlen( r ) + lstrlen( p ) + lstrlen( TEXT( "\r\n" ) ) + 1 ) );

		if( wbuf == NULL )
		{
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return FALSE;
		}

		str_join_t( wbuf, r, p, TEXT( "\r\n" ), ( TCHAR* )-1 );
		set_email_process_state( handler, wbuf );

		if( send_buf_t( handler, soc, wbuf ) == -1 )
		{
			mem_free(&wbuf);
			lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		mem_free( &wbuf );
		break;

	case SMTP_EHLO:
		if( status >= SMTP_ERRORSTATUS )
		{
			// EHLO HELO 
			command_status = SMTP_QUIT;
			ret = ERROR_SUCCESS; 
			goto _return; ;
		}

		if( handler->starttls_flag == TRUE 
			&& user_info->use_ssl == TRUE 
			&& user_info->ssl_info.Type == 4 )
		{
			// STARTTLS
			set_email_process_state( handler, TEXT( CMD_STARTTLS ) );

			if( send_buf( handler, soc, CMD_STARTTLS "\r\n" ) == -1 )
			{
				lstrcpy( ErrStr, STR_ERR_SOCK_SEND ); 
				ret = ERROR_ERRORS_ENCOUNTERED; 
				goto _return; 
			}

			handler->auth_flag = FALSE;
			handler->starttls_flag = FALSE;
			handler->command_status = SMTP_STARTTLS;
			break;
		}

		if( *user_info->pwd == _T( '\0' ) )
		{
			set_email_process_state( handler, TEXT( CMD_RSET ) ); 

			if( send_buf( handler, soc, CMD_RSET "\r\n" ) == -1 )
			{
				lstrcpy( ErrStr, STR_ERR_SOCK_SEND );
				ret = ERROR_ERRORS_ENCOUNTERED; 
				goto _return; 
			}

			handler->command_status = SMTP_RSET;
			break;
		}

		if( user_info->smtp_auth_type != 0 ) 
		{
			handler->auth_type = user_info->smtp_auth_type - 1;
		}

		switch( handler->auth_type )
		{
		case AUTH_CRAM_MD5:
		case AUTH_NON:
		default:
			// AUTH CRAM-MD5
			set_email_process_state( handler, TEXT( CMD_AUTH )TEXT( " " )TEXT( CMD_AUTH_CRAM_MD5 ) );
			if( send_buf( handler, soc, CMD_AUTH" "CMD_AUTH_CRAM_MD5"\r\n" ) == -1 ) 
			{
				lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
				ret = ERROR_ERRORS_ENCOUNTERED; 
				goto _return; 
			}

			handler->command_status = SMTP_AUTH_CRAM_MD5;
			break;

		case AUTH_LOGIN:
			// AUTH LOGIN
			set_email_process_state( handler, TEXT(CMD_AUTH)TEXT(" ")TEXT(CMD_AUTH_LOGIN));

			if( send_buf( handler, soc, CMD_AUTH " " CMD_AUTH_LOGIN "\r\n" ) == -1 ) 
			{
				lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
				ret = ERROR_ERRORS_ENCOUNTERED; 
				goto _return; 
			}

			handler->command_status = SMTP_AUTH_LOGIN;
			break;

		case AUTH_PLAIN:
			// AUTH PLAIN
			user = user_info->email; 

			pass = auth_get_password( user_info );

			ASSERT( pass != NULL ); 

			if( *pass == TEXT( '\0' ) ) 
			{
				lstrcpy( ErrStr, STR_ERR_SOCK_NOPASSWORD );
				ret = ERROR_ERRORS_ENCOUNTERED; 
				goto _return; 
			}

			p = auth_create_plain( user, pass, ErrStr ); 

			if( p == NULL ) 
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				goto _return; 
			}

			wbuf = ( TCHAR* )mem_alloc( sizeof( TCHAR ) *
				( lstrlen( TEXT( CMD_AUTH ) ) 
				+ 1 + lstrlen( TEXT( CMD_AUTH_PLAIN ) ) 
				+ 1 + lstrlen( p ) + 2 + 1 ) ); 

			if( wbuf == NULL ) 
			{
				mem_free( &p );
				lstrcpy( ErrStr, STR_ERR_MEMALLOC );
				ret = ERROR_ERRORS_ENCOUNTERED; 
				goto _return; 
			}

			// AUTH PLAIN Base64(<NULL>User<NULL>Pass)[CRLF] 
			str_join_t( wbuf, TEXT( CMD_AUTH )TEXT( " " ) TEXT( CMD_AUTH_PLAIN ) TEXT( " " ), p, TEXT( "\r\n" ), ( TCHAR* )-1 ); 

			set_email_process_state( handler, TEXT( CMD_AUTH ) TEXT( " " ) TEXT( CMD_AUTH_PLAIN ) TEXT( " ****" ) ); 

			if( send_buf_t( handler, soc, wbuf ) == -1 ) 
			{
				mem_free( &p );
				mem_free( &wbuf );
				lstrcpy( ErrStr, STR_ERR_SOCK_SEND );
				ret = ERROR_ERRORS_ENCOUNTERED; 
				goto _return; 
			}

			mem_free( &p );
			mem_free( &wbuf );

			handler->command_status = SMTP_HELO;
			break;
		}
		break;

	case SMTP_AUTH_CRAM_MD5:
		if( status >= SMTP_ERRORSTATUS ) 
		{
			handler->auth_flag = FALSE;
			lstrcpy( ErrStr, STR_ERR_SOCK_SMTPAUTH );
			str_cat_n( ErrStr, buf, BUF_SIZE - 1 );

			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		// AUTH CRAM-MD5

		user = user_info->email; 

		pass = auth_get_password( user_info ); 

		if( pass == NULL || *pass == TEXT( '\0' ) ) \
		{
			lstrcpy( ErrStr, STR_ERR_SOCK_NOPASSWORD );
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		wbuf = auth_create_cram_md5( buf, user, pass, ErrStr );
		if( wbuf == NULL )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		p = ( TCHAR* )mem_calloc( sizeof( TCHAR ) * ( lstrlen( wbuf ) * 2 + 4 ) );

		if( p == NULL )
		{
			mem_free( &wbuf );
			lstrcpy( ErrStr, STR_ERR_MEMALLOC );
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		//Base64
		base64_encode_t( wbuf, p, 0 );
		mem_free( &wbuf );

		wbuf = ( TCHAR* )mem_alloc( sizeof( TCHAR ) * ( lstrlen( TEXT( "\r\n" ) ) + lstrlen( p ) + 1 ) );

		if( wbuf == NULL )
		{
			mem_free( &p );
			lstrcpy( ErrStr, STR_ERR_MEMALLOC );
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		str_join_t( wbuf, p, TEXT( "\r\n" ), ( TCHAR* )-1 );

		set_email_process_state( handler, STR_STATUS_SEND_PASS );

		if( send_buf_t( handler, soc, wbuf ) == -1 ) 
		{
			mem_free( &p );
			mem_free( &wbuf );
			lstrcpy( ErrStr, STR_ERR_SOCK_SEND );

			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		mem_free( &p );
		mem_free( &wbuf );
		handler->command_status = SMTP_HELO;
		break;

	case SMTP_AUTH_LOGIN:
		if( status >= SMTP_ERRORSTATUS )
		{
			handler->auth_flag = FALSE;
			lstrcpy(ErrStr, STR_ERR_SOCK_SMTPAUTH);
			str_cat_n(ErrStr, buf, BUF_SIZE - 1);
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		user = user_info->email; 

		p = ( TCHAR* )mem_calloc( sizeof( TCHAR ) * ( lstrlen( user ) * 2 + 4 ) );

		if( p == NULL ) 
		{
			lstrcpy( ErrStr, STR_ERR_MEMALLOC );
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		// Base64
		base64_encode_t( user, p, 0 );

		wbuf = ( TCHAR* )mem_alloc( sizeof( TCHAR ) * ( lstrlen( TEXT( "\r\n" ) ) + lstrlen( p ) + 1 ) );

		if( wbuf == NULL )
		{
			mem_free( &p );
			lstrcpy( ErrStr, STR_ERR_MEMALLOC );
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		str_join_t( wbuf, p, TEXT( "\r\n" ), ( TCHAR* )-1 );

		set_email_process_state( handler, STR_STATUS_SEND_USER );

		if( send_buf_t( handler, soc, wbuf ) == -1 )
		{
			mem_free( &p );
			mem_free( &wbuf );
			lstrcpy( ErrStr, STR_ERR_SOCK_SEND );
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		mem_free( &p );
		mem_free( &wbuf );
		handler->command_status = SMTP_AUTH_LOGIN_PASS;
		break;

	case SMTP_AUTH_LOGIN_PASS:

		if( status >= SMTP_ERRORSTATUS )
		{
			handler->auth_flag = FALSE;

			lstrcpy( ErrStr, STR_ERR_SOCK_SMTPAUTH );
			str_cat_n( ErrStr, buf, BUF_SIZE - 1 );

			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		pass = auth_get_password( user_info );

		ASSERT( pass != NULL ); 

		if( *pass == TEXT( '\0' ) ) 
		{
			lstrcpy( ErrStr, STR_ERR_SOCK_NOPASSWORD );
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		p = ( TCHAR* )mem_calloc( sizeof( TCHAR ) * ( lstrlen( pass ) * 2 + 4 ) );

		if( p == NULL )
		{
			lstrcpy( ErrStr, STR_ERR_MEMALLOC );
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; 
		}

		// Base64
		base64_encode_t( pass, p, 0 );

		wbuf = ( TCHAR* )mem_alloc( sizeof( TCHAR ) * ( lstrlen( TEXT( "\r\n" ) ) + lstrlen( p ) + 1 ) );
		if( wbuf == NULL )
		{
			mem_free( &p );
			lstrcpy( ErrStr, STR_ERR_MEMALLOC );
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		str_join_t( wbuf, p, TEXT( "\r\n" ), ( TCHAR* )-1 );
		set_email_process_state( handler, STR_STATUS_SEND_PASS );

		if( send_buf_t( handler, soc, wbuf ) == -1 )
		{
			mem_free( &p );
			mem_free( &wbuf );

			lstrcpy( ErrStr, STR_ERR_SOCK_SEND );

			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		mem_free( &p );
		mem_free( &wbuf );
		handler->command_status = SMTP_HELO;
		break;

	case SMTP_HELO:
		send_buf( handler, soc, CMD_QUIT"\r\n" ); 			
		handler->command_status = SMTP_QUIT; 		
		break; 
	case SMTP_QUIT:
		//closesocket( soc ); 
		break; 
	default:
		ASSERT( FALSE ); 
		break; 	
	}

	if( ret != ERROR_SUCCESS )
	{
		send_buf( handler, soc, CMD_QUIT"\r\n" ); 
		smtp_set_error( handler );
	}

_return:
	log_trace( ( MSG_INFO, "leave %s, 0x%0.8x\n", __FUNCTION__, ret ) ); 

	return ret;
}

/*
 * smtp_send_proc -
 */
#ifdef WSAASYNC
BOOL smtp_send_proc(HWND hWnd, SOCKET soc, TCHAR *ErrStr, email_box *mail_box)
{
	int len;

	if (send_body == NULL && send_pt != NULL) {
		lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
		return FALSE;
	}
	// 送信終了
	if (send_body == NULL || *send_pt == '\0') {
		if (send_data(soc, CMD_DATA_END, tstrlen(CMD_DATA_END)) == -1) {
			if (WSAGetLastError() == WSAEWOULDBLOCK) {
				return TRUE;
			}
			lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			return FALSE;
		}
		mem_free(&send_body);
		send_body = NULL;
		command_status = send_end_cmd;
		if (WSAAsyncSelect(soc, hWnd, WM_SOCK_SELECT, FD_CONNECT | FD_READ | FD_CLOSE) == SOCKET_ERROR) {
			mem_free(&send_body);
			send_body = NULL;
			lstrcpy(ErrStr, STR_ERR_SOCK_EVENT);
			return FALSE;
		}
		return TRUE;
	}
	// 分割して送信
	len = (send_len - (send_pt - send_body) < SEND_SIZE) ? send_len - (send_pt - send_body) : SEND_SIZE;
	if ((len = send_data(soc, send_pt, len)) == -1) {
		if (WSAGetLastError() == WSAEWOULDBLOCK) {
			return TRUE;
		}
		mem_free(&send_body);
		send_body = NULL;
		lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
		return FALSE;
	}
	send_pt += len;
	SetStatusRecvLen(hWnd, send_pt - send_body, STR_STATUS_SOCKINFO_SEND);
	if (WSAAsyncSelect(soc, hWnd, WM_SOCK_SELECT, FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR) {
		mem_free(&send_body);
		send_body = NULL;
		lstrcpy(ErrStr, STR_ERR_SOCK_EVENT);
		return FALSE;
	}
	return TRUE;
}
#endif

/*
 * smtp_proc - SMTP
 */
//BOOL smtp_proc( email_handler *handler, SOCKET soc, char *buf, int len, TCHAR *ErrStr, email_box *mail_box, MAILITEM *send_mail_item, BOOL ShowFlag)
//{
//	BOOL ret;
//
//	ASSERT( handler != NULL ); 
//
//	if( send_mail_item == NULL )
//	{
//		return FALSE;
//	}
//	
//	ret = send_mail_proc( handler, soc, buf, ErrStr, send_mail_item, ShowFlag);
//	if (ret == FALSE) {
//		send_buf(soc, CMD_QUIT"\r\n");
//		smtp_set_error( handler );
//	}
//
//	return ret;
//}

/*
 * smtp_send_mail - 
 */
LRESULT smtp_send_mail( email_handler *handler, email_box *mail_box, MAILITEM *mail_item, int end_cmd, TCHAR *err_msg)
{
	LRESULT ret = ERROR_SUCCESS; 
	SOCKET soc = INVALID_SOCKET;
	ULONG smtp_ip; 
	ULONG try_time; 

	handler->cur_email = mail_box;
	handler->send_mail_item = mail_item;
	handler->auth_flag = FALSE;
	handler->starttls_flag = FALSE;
	handler->send_end_cmd = end_cmd;

	if( ( mail_item->To == NULL || *mail_item->To == TEXT('\0' ) ) &&
		( mail_item->Cc == NULL || *mail_item->Cc == TEXT('\0' ) ) &&
		( mail_item->Bcc == NULL || *mail_item->Bcc == TEXT( '\0' ) ) ) 
	{
		lstrcpy( err_msg, STR_ERR_SOCK_NOTO );
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	ret = get_host_by_name( handler, mail_box->server, &smtp_ip, err_msg );

	if( ret != ERROR_SUCCESS ) 
	{
		goto _return; 
	}

	ASSERT( smtp_ip != SOCKET_ERROR ); 

	ret = connect_server( handler,
		smtp_ip, (unsigned short)mail_box->port,
		(mail_box->use_ssl == FALSE || mail_box->ssl_info.Type == 4) ? -1 : mail_box->ssl_info.Type,
		&mail_box->ssl_info,
		&soc, 
		err_msg );

	if( ret != ERROR_SUCCESS ) 
	{
		goto _return; 
	}

	ASSERT( smtp_ip != SOCKET_ERROR ); 

	try_time = 0; 
	for( ; ; )
	{
		Sleep( RECVTIME ); 

		ret = recv_select( handler, soc, smtp_proc ); 
		
		switch( ret ) 
		{
		case ERROR_NOT_ENOUGH_MEMORY:
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "%s\n", STR_ERR_MEMALLOC ) );
			goto _return; 
			break;

		case ERROR_SOCKET_ERROR:
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "%s\n", STR_ERR_SOCK_SELECT ) ); 
			goto _return; 
			break;

		case ERROR_SOCKET_CLOSED:
			if( handler->command_status != POP_QUIT ) 
			{
				log_trace( ( DBG_MSG_AND_ERROR_OUT, "%s\n", "socket is connected when the action have not done"	) );
			} 
			else 
			{
				socket_close( handler, soc ); 
				soc = INVALID_SOCKET; 
				ret = ERROR_SUCCESS; 
			}

			goto _return; 
			break;

		case ERROR_SUCCESS:
			{
				//CHAR *buf; 
				//ULONG buf_len; 

				//get_old_recv_buf( &buf, &buf_len ); 
				//dump_mem( buf, buf_len );

				//get_recv_buf( &buf, &buf_len ); 
				//dump_mem( buf, buf_len ); 

				//ret = smtp_proc( handler, soc, buf, buf_len, err_msg, TRUE ); 

				//if( ret != ERROR_SUCCESS )
				//{
				//	goto _return; 
				//}
			}
			break;

		case ERROR_SOCKET_NO_RECVED:
			{
				try_time += 1; 

				if( try_time == SMTP_RETRY_TIME )
				{
					ret = ERROR_TIMEOUT; 
					goto _return; 
				}

				break; 
			}
		default:
			log_trace( ( MSG_ERROR, "select receive state of the seocket failed 0x%0.8x\n", ret ) ); 
			goto _return; 
			break; 
		}
	}

_return:
	if( soc != INVALID_SOCKET )
	{
		socket_close( handler, soc ); 
		//soc = INVALID_SOCKET;
	}

	return ret;
}

LRESULT smtp_test_login( email_handler *handler, email_box *mail_box, MAILITEM *mail_item, int end_cmd, TCHAR *err_msg)
{
	LRESULT ret = ERROR_SUCCESS; 
	SOCKET soc = INVALID_SOCKET;
	ULONG smtp_ip; 
	ULONG try_time; 

	handler->cur_email = mail_box;
	handler->send_mail_item = mail_item;
	handler->auth_flag = FALSE;
	handler->starttls_flag = FALSE;
	handler->send_end_cmd = end_cmd;

	if( ( mail_item->To == NULL || *mail_item->To == TEXT('\0' ) ) &&
		( mail_item->Cc == NULL || *mail_item->Cc == TEXT('\0' ) ) &&
		( mail_item->Bcc == NULL || *mail_item->Bcc == TEXT( '\0' ) ) ) 
	{
		lstrcpy( err_msg, STR_ERR_SOCK_NOTO );
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	ret = get_host_by_name( handler, mail_box->server, &smtp_ip, err_msg );

	if( ret != ERROR_SUCCESS ) 
	{
		goto _return; 
	}

	ASSERT( smtp_ip != SOCKET_ERROR ); 

	ret = connect_server( handler,
		smtp_ip, (unsigned short)mail_box->port,
		(mail_box->use_ssl == FALSE || mail_box->ssl_info.Type == 4) ? -1 : mail_box->ssl_info.Type,
		&mail_box->ssl_info,
		&soc, 
		err_msg );

	if( ret != ERROR_SUCCESS ) 
	{
		goto _return; 
	}

	ASSERT( smtp_ip != SOCKET_ERROR ); 

	try_time = 0; 
	for( ; ; )
	{
		Sleep( RECVTIME ); 

		ret = recv_select( handler, soc, smtp_login_proc ); 

		switch( ret ) 
		{
		case ERROR_NOT_ENOUGH_MEMORY:
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "%s\n", STR_ERR_MEMALLOC ) );
			goto _return; 
			break;

		case ERROR_SOCKET_ERROR:
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "%s\n", STR_ERR_SOCK_SELECT ) ); 
			goto _return; 
			break;

		case ERROR_SOCKET_CLOSED:
			if( handler->command_status != POP_QUIT ) 
			{
				log_trace( ( DBG_MSG_AND_ERROR_OUT, "%s\n", "socket is connected when the action have not done"	) );
			} 
			else 
			{
				socket_close( handler, soc ); 
				soc = INVALID_SOCKET;
				ret = ERROR_SUCCESS; 
			}

			goto _return; 
			break;

		case ERROR_SUCCESS:
			{
				//CHAR *buf; 
				//ULONG buf_len; 

				//get_old_recv_buf( &buf, &buf_len ); 
				//dump_mem( buf, buf_len );

				//get_recv_buf( &buf, &buf_len ); 
				//dump_mem( buf, buf_len ); 

				//ret = smtp_proc( handler, soc, buf, buf_len, err_msg, TRUE ); 

				//if( ret != ERROR_SUCCESS )
				//{
				//	goto _return; 
				//}
			}
			break;

		case ERROR_SOCKET_NO_RECVED:
			{
				try_time += 1; 

				if( try_time == SMTP_TEST_RETRY_TIME )
				{
					ret = ERROR_TIMEOUT; 
					goto _return; 
				}

				break; 
			}
		default:
			log_trace( ( MSG_ERROR, "select receive state of the seocket failed 0x%0.8x\n", ret ) ); 
			goto _return; 
			break; 
		}
	}

_return:
	if( soc != INVALID_SOCKET )
	{
		socket_close( handler, soc ); 
		//soc = INVALID_SOCKET;
	}

	return ret;
}

/*
 * smtp_set_error - 送信メールにエラーステータスを付加する
 */
void smtp_set_error( email_handler *handler )
{
	//HWND hListView;
	//int i;

	if( handler == NULL )
	{
		return; 
	}

	smtp_free( handler );

	if( handler->send_mail_item == NULL ) 
	{
		return;
	}

	//send_mail_item->Status = send_mail_item->MailStatus = ICON_ERROR;

	//if (SelBox == MAILBOX_SEND) {
	//	hListView = GetDlgItem(hWnd, IDC_LISTVIEW);
	//	i = ListView_GetMemToItem(hListView, send_mail_item);
	//	if (i != -1) {
	//		ListView_RedrawItems(hListView, i, i);
	//		UpdateWindow(hListView);
	//	}
	//}

	handler->send_mail_item = NULL;
}

/*
 * smtp_free - SMTP
 */
void smtp_free( email_handler *handler )
{
	if( handler != NULL )
	{
		return; 
	}

	mem_free( &handler->send_body );
	handler->send_body = NULL;
}
/* End of source */
