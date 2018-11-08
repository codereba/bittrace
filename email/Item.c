/*
 * ERC
 *
 * Item.c
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

/* Global Variables */
//extern OPTION op;
BOOL KeyShowHeader = TRUE;

//extern email_box *MailBox;
//extern email_box *AddressBox;

/* Local Function Prototypes */
static int item_get_content(char *buf, char *header, char **ret);
static void item_get_content_t(char *buf, char *header, TCHAR **ret);
static int item_get_content_int(char *buf, char *header, int DefaultRet);
static int item_get_multi_content(char *buf, char *header, char **ret);
static int item_get_mime_content(char *buf, char *header, TCHAR **ret, BOOL multi_flag);
static void item_set_body(MAILITEM *mail_item, char *buf, BOOL download);
static int item_save_header_size(TCHAR *header, TCHAR *buf);
static char *item_save_header(TCHAR *header, TCHAR *buf, char *ret);
static int item_filter_check(CHAR *buf);

//int mailbox_name_to_index( email_box *mail_box, TCHAR *Name)
//{
//	int i;
//
//	if(Name == NULL){
//		return -1;
//	}
//	for(i = 0; i < MailBoxCnt; i++){
//		if(lstrcmpi( ( mail_box + i )->, Name) == 0){
//			return i;
//		}
//	}
//	return -1;
//}

/*
 * item_is_mailbox - メールボックス内のメールリストに指定のメールが存在するか調べる
 */
BOOL item_is_mailbox(email_box *mail_box, MAILITEM *mail_item)
{
	int i;

	for (i = 0; i < mail_box->mail_num; i++) {
		if( *( mail_box->mail_item + i )== mail_item) {
			return TRUE;
		}
	}
	return FALSE;
}

/*
 * item_set_count - 
 */
LRESULT item_set_count( email_box *mail_box, int i)
{
	LRESULT ret = ERROR_SUCCESS; 
	MAILITEM **tpMailList;

	if( i <= mail_box->max_mail_count ) 
	{
		goto _return; 
	}
	
	tpMailList = ( MAILITEM** )mem_calloc( sizeof( MAILITEM* ) * i );
	
	if( tpMailList == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}
	
	if( mail_box->mail_item != NULL ) 
	{
		CopyMemory( tpMailList, mail_box->mail_item,
			sizeof( MAILITEM* ) * mail_box->mail_num );
	}

	mem_free( ( void ** )&mail_box->mail_item );
	
	mail_box->mail_item = tpMailList;

_return:
	return ret;
}

/*
 * item_add - 
 */
//BOOL item_add(email_box *mail_box, MAILITEM *tpNewMailItem)
//{
//	MAILITEM **tpMailList = mail_box->mail_item;
//
//	if (mail_box->max_mail_count <= mail_box->mail_num) {
//		tpMailList = (MAILITEM **)mem_alloc(sizeof(MAILITEM *) * (mail_box->mail_num + 1));
//		if (tpMailList == NULL) {
//			return FALSE;
//		}
//		if (mail_box->mail_item != NULL) {
//			CopyMemory(tpMailList, mail_box->mail_item,
//				sizeof(MAILITEM *) * mail_box->mail_num);
//			mem_free((void **)&mail_box->mail_item);
//		}
//		mail_box->mail_item = tpMailList;
//		mail_box->max_mail_count = mail_box->mail_num + 1;
//	}
//	*(tpMailList + mail_box->mail_num) = tpNewMailItem;
//	mail_box->mail_num++;
//	if (*mail_box->mail_item != NULL) {
//		(*mail_box->mail_item)->NextNo = 0;
//	}
//	return TRUE;
//}

/*
 * item_copy - アイテムのコピー
 */
void item_copy(MAILITEM *tpFromMailItem, MAILITEM *tpToMailItem)
{
	CopyMemory(tpToMailItem, tpFromMailItem, sizeof(MAILITEM));
	tpToMailItem->Status = tpToMailItem->MailStatus;
	tpToMailItem->New = FALSE;
	tpToMailItem->No = 0;
	tpToMailItem->UIDL = NULL;

	tpToMailItem->From = alloc_copy_t(tpFromMailItem->From);
	tpToMailItem->To = alloc_copy_t(tpFromMailItem->To);
	tpToMailItem->Cc = alloc_copy_t(tpFromMailItem->Cc);
	tpToMailItem->Bcc = alloc_copy_t(tpFromMailItem->Bcc);
	tpToMailItem->Subject = alloc_copy_t(tpFromMailItem->Subject);
	tpToMailItem->Date = alloc_copy_t(tpFromMailItem->Date);
	tpToMailItem->Size = alloc_copy_t(tpFromMailItem->Size);
	tpToMailItem->ReplyTo = alloc_copy_t(tpFromMailItem->ReplyTo);
	tpToMailItem->ContentType = alloc_copy_t(tpFromMailItem->ContentType);
	tpToMailItem->Encoding = alloc_copy_t(tpFromMailItem->Encoding);
	tpToMailItem->MessageID = alloc_copy_t(tpFromMailItem->MessageID);
	tpToMailItem->InReplyTo = alloc_copy_t(tpFromMailItem->InReplyTo);
	tpToMailItem->References = alloc_copy_t(tpFromMailItem->References);
	tpToMailItem->Body = alloc_copy(tpFromMailItem->Body);
	tpToMailItem->MailBox = alloc_copy_t(tpFromMailItem->MailBox);
	tpToMailItem->Attach = alloc_copy_t(tpFromMailItem->Attach);

	tpToMailItem->HeadCharset = alloc_copy_t(tpFromMailItem->HeadCharset);
	tpToMailItem->BodyCharset = alloc_copy_t(tpFromMailItem->BodyCharset);
}

/*
 * item_to_mailbox - アイテムをメールボックスに追加
 */
MAILITEM *item_to_mailbox(email_box *mail_box, MAILITEM *tpNewMailItem, TCHAR *MailBoxName, BOOL SendClear)
{
	MAILITEM **tpMailList;
	int i = 0;

	tpMailList = (MAILITEM **)mem_calloc(sizeof(MAILITEM *) * (mail_box->mail_num + 1));
	if (tpMailList == NULL) {
		return NULL;
	}
	if (mail_box->mail_item != NULL) {
		for (i = 0; i < mail_box->mail_num; i++) {
			*tpMailList = mail_box->mail_item;
		}
	}

	*(tpMailList + i) = (MAILITEM *)mem_calloc(sizeof(MAILITEM));
	if (*(tpMailList + i) == NULL) {
		mem_free((void **)&tpMailList);
		return NULL;
	}
	item_copy(tpNewMailItem, *(tpMailList + i));
	if ((*(tpMailList + i))->MailBox == NULL) {
		(*(tpMailList + i))->MailBox = alloc_copy_t(MailBoxName);
	}
	if (SendClear == TRUE) {
		(*(tpMailList + i))->MailStatus = (*(tpMailList + i))->Status = ICON_NON;
		mem_free(&(*(tpMailList + i))->Date);
		(*(tpMailList + i))->Date = NULL;
		mem_free(&(*(tpMailList + i))->MessageID);
		(*(tpMailList + i))->MessageID = NULL;
		(*(tpMailList + i))->hEditWnd = NULL;
	}

	mem_free((void **)&mail_box->mail_item);
	mail_box->mail_item = tpMailList;
	mail_box->mail_num++;
	mail_box->max_mail_count = mail_box->mail_num;
	if (*mail_box->mail_item != NULL) {
		(*mail_box->mail_item)->NextNo = 0;
	}
	return *(tpMailList + i);
}

/*
 * item_resize_mailbox - アイテム情報の整理
 */
BOOL item_resize_mailbox(email_box *mail_box)
{
	MAILITEM **tpMailList;
	int i, cnt = 0;

	if (mail_box->mail_item == NULL) {
		mail_box->max_mail_count = mail_box->mail_num = 0;
		return FALSE;
	}

	for (i = 0; i < mail_box->mail_num; i++) {
		if (*(mail_box->mail_item + i) == NULL) {
			continue;
		}
		cnt++;
	}

	tpMailList = (MAILITEM **)mem_calloc(sizeof(MAILITEM *) * cnt);
	if (tpMailList == NULL) {
		mem_free((void **)&mail_box->mail_item);
		mail_box->mail_item = NULL;
		mail_box->max_mail_count = mail_box->mail_num = 0;
		return FALSE;
	}
	cnt = 0;
	for (i = 0; i < mail_box->mail_num; i++) {
		if (*(mail_box->mail_item + i) == NULL) {
			continue;
		}
		*(tpMailList + cnt) = *(mail_box->mail_item + i);
		cnt++;
	}

	mem_free((void **)&mail_box->mail_item);
	mail_box->mail_item = tpMailList;
	mail_box->max_mail_count = mail_box->mail_num = cnt;
	if (cnt != 0 && *mail_box->mail_item != NULL) {
		(*mail_box->mail_item)->NextNo = 0;
	}
	return TRUE;
}

/*
 * item_free - メールアイテムの解放
 */
void item_free(MAILITEM **mail_item, int cnt)
{
	int i;

	for (i = 0; i < cnt; i++) {
		if (*(mail_item + i) == NULL) {
			continue;
		}
		if ((*(mail_item + i))->hEditWnd != NULL) {
			(*(mail_item + i)) = NULL;
			continue;
		}
		mem_free(&(*(mail_item + i))->From);
		mem_free(&(*(mail_item + i))->To);
		mem_free(&(*(mail_item + i))->Cc);
		mem_free(&(*(mail_item + i))->Bcc);
		mem_free(&(*(mail_item + i))->Date);
		mem_free(&(*(mail_item + i))->Size);
		mem_free(&(*(mail_item + i))->Subject);
		mem_free(&(*(mail_item + i))->ReplyTo);
		mem_free(&(*(mail_item + i))->ContentType);
		mem_free(&(*(mail_item + i))->Encoding);
		mem_free(&(*(mail_item + i))->MessageID);
		mem_free(&(*(mail_item + i))->UIDL);
		mem_free(&(*(mail_item + i))->InReplyTo);
		mem_free(&(*(mail_item + i))->References);
		mem_free(&(*(mail_item + i))->Body);
		mem_free(&(*(mail_item + i))->MailBox);
		mem_free(&(*(mail_item + i))->Attach);
		mem_free(&(*(mail_item + i))->HeadCharset);
		mem_free(&(*(mail_item + i))->BodyCharset);

		mem_free(&*(mail_item + i));
		(*(mail_item + i)) = NULL;
	}
}

/*
 * item_get_content - コンテンツの取得
 */
static int item_get_content(char *buf, char *header, char **ret)
{
	char *p;
	int len;

	// 位置の取得
	p = GetHeaderStringPoint(buf, header);
	if (p == NULL) {
		*ret = NULL;
		return 0;
	}
	// サイズの取得
	len = GetHeaderStringSize(p, FALSE);
	*ret = (char *)mem_alloc(len + 1);
	if (*ret == NULL) {
		return 0;
	}
	GetHeaderString(p, *ret, FALSE);
	return len;
}

/*
 * item_get_content_t - コンテンツの取得
 */
static void item_get_content_t(char *buf, char *header, TCHAR **ret)
{
#ifdef UNICODE
	char *cret;
#endif
	char *p;
	int len;

	// 位置の取得
	p = GetHeaderStringPoint(buf, header);
	if (p == NULL) {
		*ret = NULL;
		return;
	}
	// サイズの取得
	len = GetHeaderStringSize(p, TRUE);
#ifdef UNICODE
	cret = (char *)mem_alloc(sizeof(char) * (len + 1));
	if (cret == NULL) {
		return;
	}
	GetHeaderString(p, cret, TRUE);
	*ret = alloc_char_to_tchar(cret);
	mem_free(&cret);
#else
	*ret = (char *)mem_alloc(sizeof(char) * (len + 1));
	if (*ret == NULL) {
		return;
	}
	GetHeaderString(p, *ret, TRUE);
#endif
}

/*
 * item_get_content_int - コンテンツの取得
 */
static int item_get_content_int(char *buf, char *header, int DefaultRet)
{
	TCHAR *Content;
	int ret;

	item_get_content_t(buf, header, &Content);
	if (Content == NULL) {
		return DefaultRet;
	}
	ret = _ttoi(Content);
	mem_free(&Content);
	return ret;
}

/*
 * item_get_multi_content - 複数ある場合は一つにまとめてコンテンツの取得
 */
static int item_get_multi_content(char *buf, char *header, char **ret)
{
	char *tmp;
	char *p, *r;
	int len;
	int ret_len = 0;

	*ret = NULL;
	p = buf;
	while (1) {
		// 位置の取得
		p = GetHeaderStringPoint(p, header);
		if (p == NULL) {
			return ret_len;
		}
		// サイズの取得
		len = GetHeaderStringSize(p, FALSE);
		if (*ret != NULL) {
			r = tmp = (char *)mem_alloc(ret_len + len + 2);
			if (tmp == NULL) {
				return ret_len;
			}
			r = str_cpy(r, *ret);
			r = str_cpy(r, ",");
			mem_free(&*ret);
			*ret = tmp;
			ret_len += (len + 1);
		} else {
			r = *ret = (char *)mem_alloc(len + 1);
			if (*ret == NULL) {
				return ret_len;
			}
			ret_len = len;
		}
		GetHeaderString(p, r, FALSE);
		p += len;
	}
}

/*
 * item_get_mime_content - MIME
 */
static int item_get_mime_content(char *buf, char *header, TCHAR **ret, BOOL multi_flag)
{
	char *Content;
	int len;

	*ret = NULL;

	len = ( ( multi_flag == TRUE ) ? item_get_multi_content : item_get_content )( buf, header, &Content );
	if( Content != NULL )
	{
		len = MIME_decode( Content, NULL );
		
		*ret = ( TCHAR* )mem_alloc( sizeof( TCHAR ) * ( len + 1 ) );
		
		if( *ret != NULL )
		{
			MIME_decode( Content, *ret );
		}

		mem_free( &Content );
	}

	return len;
}

/*
 * item_get_message_id - メッセージIDの取得
 */
char *item_get_message_id(char *buf)
{
	char *Content, *p;
	MD5_CTX context;
	unsigned char digest[16];
	int len;

	// Message-Id取得
	Content = NULL;
	item_get_content(buf, HEAD_MESSAGEID, &Content);
	TrimMessageId(Content);
	if (Content != NULL && *Content != '\0') {
		return Content;
	}
	mem_free(&Content);

	// UIDLを取得
	Content = NULL;
	item_get_content(buf, HEAD_X_UIDL, &Content);
	if (Content != NULL && *Content != '\0') {
		return Content;
	}
	mem_free(&Content);

	// Dateを取得
	Content = NULL;
	item_get_content(buf, HEAD_DATE, &Content);
	if (Content != NULL && *Content != '\0') {
		return Content;
	}
	mem_free(&Content);

	// ヘッダのハッシュ値を取得
	p = GetBodyPointa(buf);
	if (p != NULL) {
		len = p - buf;
	} else {
		len = tstrlen(buf);
	}
	MD5Init(&context);
	MD5Update(&context, buf, len);
	MD5Final(digest, &context);

	Content = (char *)mem_alloc(16 * 2 + 1);
	if (Content == NULL) {
		return NULL;
	}
	base64_encode(digest, Content, 16);
	return Content;
}

/*
 * item_get_number_to_index - メール番号からアイテムのインデックスを取得
 */
int item_get_number_to_index(email_box *mail_box, int No)
{
	int i;

	for (i = 0; i < mail_box->mail_num; i++) {
		if (*(mail_box->mail_item + i) == NULL) {
			continue;
		}
		if ((*(mail_box->mail_item + i))->No == No) {
			return i;
		}
	}
	return -1;
}

/*
 * item_get_next_download_mark - ダウンロードマークのアイテムのインデックスを取得
 */
int item_get_next_download_mark(email_box *mail_box, int Index, int *No)
{
	MAILITEM *mail_item;
	int i;

	for (i = Index + 1; i < mail_box->mail_num; i++) {
		mail_item = *(mail_box->mail_item + i);
		if (mail_item == NULL) {
			continue;
		}
		if (mail_item->Status == ICON_DOWN) {
			if (No != NULL) {
				*No = mail_item->No;
			}
			return i;
		}
	}
	return -1;
}

/*
 * item_get_next_delete_mark - 削除マークのアイテムのインデックスを取得
 */
int item_get_next_delete_mark(email_box *mail_box, int Index, int *No)
{
	MAILITEM *mail_item;
	int i;

	for (i = Index + 1; i < mail_box->mail_num; i++) {
		mail_item = *(mail_box->mail_item + i);
		if (mail_item == NULL) {
			continue;
		}
		if (mail_item->Status == ICON_DEL) {
			if (No != NULL) {
				*No = mail_item->No;
			}
			return i;
		}
	}
	return -1;
}

/*
 * item_get_next_send_mark - 送信マークの付いたアイテムのインデックスを取得
 */
int item_get_next_send_mark(email_box *mail_box, int Index, int *MailBoxIndex)
{
	MAILITEM *mail_item;
	int BoxIndex;
	int i;
	int wkIndex = -1;
	int wkBoxIndex = -1;

	for (i = Index + 1; i < mail_box->mail_num; i++) {
		mail_item = *(mail_box->mail_item + i);
		if (mail_item == NULL || mail_item->Status != ICON_SEND) {
			continue;
		}
		
		if (MailBoxIndex == NULL) {
			return i;
		}
		
		BoxIndex = 0; //mailbox_name_to_index(mail_item->MailBox);
		if (*MailBoxIndex == -1 || *MailBoxIndex == BoxIndex) {
			wkIndex = i;
			wkBoxIndex = BoxIndex;
			break;
		}
		if (wkIndex == -1) {
			wkIndex = i;
			wkBoxIndex = BoxIndex;
		}
	}
	if (MailBoxIndex != NULL) {
		*MailBoxIndex = wkBoxIndex;
	}
	return wkIndex;
}

/*
 * item_get_next_send_mark_mailbox - 指定のメールボックスの送信マークの付いたアイテムのインデックスを取得
 */
int item_get_next_send_mark_mailbox(email_box *mail_box, int Index, int MailBoxIndex)
{
	MAILITEM *mail_item;
	int BoxIndex;
	int i;

	if (MailBoxIndex == -1) {
		return -1;
	}
	for (i = Index + 1; i < mail_box->mail_num; i++) {
		mail_item = *(mail_box->mail_item + i);
		if (mail_item == NULL || mail_item->Status != ICON_SEND) {
			continue;
		}
		BoxIndex = 0; //mailbox_name_to_index(mail_item->MailBox);
		if (MailBoxIndex == BoxIndex) {
			return i;
		}
	}
	return -1;
}

/*
 * item_set_body - アイテムに本文を設定
 */
static void item_set_body(MAILITEM *mail_item, char *buf, BOOL download)
{
	char *p, *r;
	int len;
	int header_size;

	p = GetBodyPointa(buf);
	if (p != NULL && *p != '\0') {
		// デコード
		r = MIME_body_decode_transfer(mail_item, p);
		if (r == NULL) {
			return;
		}

		len = tstrlen(r);

		header_size = KeyShowHeader == TRUE ? (p - buf) : 0;

		mem_free(&mail_item->Body);
		mail_item->Body = (char *)mem_alloc(sizeof(char) * (len + header_size + 1));
		if(mail_item->Body != NULL) 
		{
			if( KeyShowHeader == TRUE) 
			{
				str_cpy_n(mail_item->Body, buf, header_size + 1);
			}
		
			tstrcpy( mail_item->Body + header_size, r );
		}

		mem_free(&r);

	} else if ( KeyShowHeader == TRUE) 
	{
		// 本文が存在しない場合はヘッダのみ設定
		mem_free(&mail_item->Body);
		mail_item->Body = alloc_copy(buf);

	} else if (download == TRUE) 
	{
		mem_free(&mail_item->Body);
		mail_item->Body = (char *)mem_alloc(sizeof(char));
		if (mail_item->Body != NULL) 
		{
			*mail_item->Body = '\0';
		}
	}

	if (mail_item->Body != NULL) 
	{
		mail_item->Status = mail_item->MailStatus = ICON_MAIL;
	}
}

/*
 * item_mail_to_item - 
 */
BOOL item_mail_to_item( MAILITEM *mail_item, char *buf, int Size, BOOL download )
{
	TCHAR *msgid1 = NULL, *msgid2 = NULL, *t = NULL;
	char *Content;
#ifdef UNICODE
	char *dcode;
#endif

	if( download == TRUE )
	{
		mem_free( &mail_item->Subject );
		mem_free( &mail_item->From );
		mem_free( &mail_item->To );
		mem_free( &mail_item->Cc );
		mem_free( &mail_item->ReplyTo );
		mem_free( &mail_item->ContentType );
		mem_free( &mail_item->Encoding );
		mem_free( &mail_item->Date );

		mem_free( &mail_item->MessageID );
		mem_free( &mail_item->InReplyTo );
		mem_free( &mail_item->References );
	}

	mail_item->Size = Size; 

	// Subject
	item_get_mime_content(buf, HEAD_SUBJECT, &mail_item->Subject, FALSE);

	// From
	item_get_mime_content(buf, HEAD_FROM, &mail_item->From, FALSE);

	// To
	item_get_mime_content(buf, HEAD_TO, &mail_item->To, TRUE);

	// Cc
	item_get_mime_content(buf, HEAD_CC, &mail_item->Cc, TRUE);

	// Reply-To
	item_get_mime_content(buf, HEAD_REPLYTO, &mail_item->ReplyTo, FALSE);

	// Content-Type
	item_get_mime_content(buf, HEAD_CONTENTTYPE, &mail_item->ContentType, FALSE);

	if( mail_item->ContentType != NULL 
		&& str_cmp_ni_t( mail_item->ContentType, 
		TEXT( "multipart" ), 
		lstrlen( TEXT( "multipart" ) ) ) == 0 )
	{
		mail_item->Multipart = TRUE;
	} 
	else
	{
		// Content-Transfer-Encoding
#ifdef UNICODE
		item_get_content( buf, HEAD_ENCODING, &Content );

		if( Content != NULL )
		{
			mail_item->Encoding = alloc_char_to_tchar( Content );
			mem_free( &Content );
		}
#else
		item_get_content( buf, HEAD_ENCODING, &mail_item->Encoding );
#endif
	}

	// Date
	item_get_content( buf, HEAD_DATE, &Content ); 

	if( Content != NULL )
	{
#ifdef UNICODE
		dcode = ( CHAR* )mem_alloc( BUF_SIZE );
		if( dcode != NULL )
		{
			DateConv( Content, dcode );
			mail_item->Date = alloc_char_to_tchar( dcode );
			mem_free( &dcode );
		}
#else
		mail_item->Date = ( char * )mem_alloc( BUF_SIZE );
		if( mail_item->Date != NULL )
		{
			DateConv( Content, mail_item->Date );
		}
#endif
		mem_free( &Content );
	}

	// Size
//	if( Size >= 0 )
//	{
//		TCHAR num[ BUF_SIZE ];
//#ifndef _itot
//		wsprintf( num, TEXT( "%d" ), Size );
//#else
//		_itot( Size, num, 10 );
//#endif
//
//		mail_item->Size = alloc_copy_t(num);
//	}

	// Message-Id
#ifdef UNICODE
	Content = item_get_message_id( buf );
	if( Content != NULL )
	{
		mail_item->MessageID = alloc_char_to_tchar( Content );
		mem_free( &Content );
	}
#else
	mail_item->MessageID = item_get_message_id( buf );
#endif

	// In-Reply-To
#ifdef UNICODE
	item_get_content( buf, HEAD_INREPLYTO, &Content );
	
	TrimMessageId( Content );
	
	if( Content != NULL )
	{
		mail_item->InReplyTo = alloc_char_to_tchar( Content );
		mem_free( &Content );
	}
#else
	item_get_content( buf, HEAD_INREPLYTO, &mail_item->InReplyTo );
	TrimMessageId( mail_item->InReplyTo );
#endif

	// References
	item_get_content( buf, HEAD_REFERENCES, &Content );

	if( Content != NULL )
	{
#ifdef UNICODE
		dcode = ( CHAR* )mem_alloc( GetReferencesSize( Content, TRUE ) + 1 );
		if( dcode != NULL )
		{
			ConvReferences( Content, dcode, FALSE );
			msgid1 = alloc_char_to_tchar( dcode );

			ConvReferences( Content, dcode, TRUE );
			
			msgid2 = alloc_char_to_tchar( dcode );
			mem_free( &dcode );
		}
#else
		msgid1 = ( char* )mem_alloc( GetReferencesSize( Content, FALSE ) + 1 );
		if( msgid1 != NULL )
		{
			ConvReferences( Content, msgid1, FALSE );
		}

		msgid2 = (char *)mem_alloc( GetReferencesSize( Content, TRUE ) + 1 );
		if( msgid2 != NULL )
		{
			ConvReferences( Content, msgid2, TRUE );
		}
#endif
		if( msgid1 != NULL && msgid2 != NULL && lstrcmp( msgid1, msgid2 ) == 0 )
		{
			mem_free( &msgid2 );
			msgid2 = NULL;
		}

		mem_free( &Content );
	}

	if( mail_item->InReplyTo == NULL || *mail_item->InReplyTo == TEXT('\0') )
	{
		mem_free( &mail_item->InReplyTo ); 

		mail_item->InReplyTo = alloc_copy_t( msgid1 );
		
		t = msgid2;
	} 
	else
	{
		t = ( msgid1 != NULL && lstrcmp( mail_item->InReplyTo, msgid1 ) != 0) 
			? msgid1 : msgid2;
	}

	mail_item->References = alloc_copy_t( t );
	mem_free( &msgid1 );
	mem_free( &msgid2 );

	// Body
	item_set_body( mail_item, buf, download );
	return TRUE;
}

/*
 * item_header_to_item - 
 */
LRESULT item_header_to_item( MAILITEM **mail_item, char *buf, int Size)
{
	LRESULT ret = ERROR_SUCCESS; 
	MAILITEM *_mail_item = NULL;
	int fret;

	ASSERT( mail_item != NULL ); 

	*mail_item = NULL; 

	fret = item_filter_check(  buf );
	
	if( fret == FALSE )
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	// 
	_mail_item  = ( MAILITEM* )mem_calloc( sizeof( MAILITEM ) );
	
	if( _mail_item  == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	// 
	item_mail_to_item( _mail_item, buf, Size, FALSE );
		
	*mail_item = _mail_item; 

_return:
	if( ret != ERROR_SUCCESS )
	{
		if( _mail_item != NULL )
		{
			item_free( &_mail_item, 1 ); 
		}
	}

	return ret;
}

/*
 * item_string_to_item - 文字列からアイテムを作成する
 */
MAILITEM *item_string_to_item(email_box *mail_box, char *buf)
{
	MAILITEM *mail_item;
	int i;

	mail_item = (MAILITEM *)mem_calloc(sizeof(MAILITEM));
	if (mail_item == NULL) {
		return NULL;
	}
	item_get_content_t(buf, HEAD_SUBJECT, &mail_item->Subject);
	item_get_content_t(buf, HEAD_FROM, &mail_item->From);
	item_get_content_t(buf, HEAD_TO, &mail_item->To);
	item_get_content_t(buf, HEAD_CC, &mail_item->Cc);
	item_get_content_t(buf, HEAD_BCC, &mail_item->Bcc);
	item_get_content_t(buf, HEAD_DATE, &mail_item->Date);
	item_get_content_t(buf, HEAD_SIZE, &mail_item->Size);
	item_get_content_t(buf, HEAD_REPLYTO, &mail_item->ReplyTo);
	item_get_content_t(buf, HEAD_CONTENTTYPE, &mail_item->ContentType);
	item_get_content_t(buf, HEAD_ENCODING, &mail_item->Encoding);
	item_get_content_t(buf, HEAD_MESSAGEID, &mail_item->MessageID);
	item_get_content_t(buf, HEAD_INREPLYTO, &mail_item->InReplyTo);
	item_get_content_t(buf, HEAD_REFERENCES, &mail_item->References);
	item_get_content_t(buf, HEAD_X_UIDL, &mail_item->UIDL);

	item_get_content_t(buf, HEAD_X_MAILBOX, &mail_item->MailBox);
	item_get_content_t(buf, HEAD_X_ATTACH, &mail_item->Attach);
	if (mail_item->Attach == NULL) {
		item_get_content_t(buf, HEAD_X_ATTACH_OLD, &mail_item->Attach);
		if (mail_item->Attach != NULL) {
			TCHAR *p;
			for (p = mail_item->Attach; *p != TEXT('\0'); p++) {
#ifndef UNICODE
				// 2バイトコードの場合は2バイト進める
				if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
					p++;
					continue;
				}
#endif
				if (*p == TEXT(',')) {
					*p = ATTACH_SEP;
				}
			}
		}
	}
	item_get_content_t(buf, HEAD_X_HEADCHARSET, &mail_item->HeadCharset);
	mail_item->HeadEncoding = item_get_content_int(buf, HEAD_X_HEADENCODE, 0);
	item_get_content_t(buf, HEAD_X_BODYCHARSET, &mail_item->BodyCharset);
	mail_item->BodyEncoding = item_get_content_int(buf, HEAD_X_BODYENCODE, 0);

	// No
	mail_item->No = item_get_content_int(buf, HEAD_X_NO, -1);
	if (mail_item->No == -1) {
		mail_item->No = item_get_content_int(buf, HEAD_X_NO_OLD, 0);
	}
	// MailStatus
	mail_item->MailStatus = item_get_content_int(buf, HEAD_X_STATUS, -1);
	if (mail_item->MailStatus == -1) {
		mail_item->MailStatus = item_get_content_int(buf, HEAD_X_STATUS_OLD, 0);
	}
	// MarkStatus
	i = item_get_content_int(buf, HEAD_X_MARK, -1);
	mail_item->Status = (i != -1) ? i : mail_item->MailStatus;
	// Download
	mail_item->Download = item_get_content_int(buf, HEAD_X_DOWNLOAD, -1);
	if (mail_item->Download == -1) {
		mail_item->Download = item_get_content_int(buf, HEAD_X_DOWNLOAD_OLD, 0);
	}
	// Multipart
	if (mail_item->Attach != NULL || (mail_item->ContentType != NULL &&
		str_cmp_ni_t(mail_item->ContentType, TEXT("multipart"), lstrlen(TEXT("multipart"))) == 0)) {
		mail_item->Multipart = TRUE;
	}
	return mail_item;
}

/*
 * item_save_header_size - 保存するヘッダのサイズ
 */
static int item_save_header_size(TCHAR *header, TCHAR *buf)
{
	if (buf == NULL) {
		return 0;
	}
#ifdef UNICODE
	return (tchar_to_char_size(header) + 1 + tchar_to_char_size(buf) + 2 - 2);
#else
	return (lstrlen(header) + 1 + lstrlen(buf) + 2);
#endif
}

/*
 * item_save_header - ヘッダを保存する文字列の作成
 */
static char *item_save_header(TCHAR *header, TCHAR *buf, char *ret)
{
#ifdef UNICODE
	TCHAR *wret;
	int len;

	if (buf == NULL) {
		return ret;
	}
	wret = mem_alloc(sizeof(TCHAR) * (lstrlen(header) + 1 + lstrlen(buf) + 2 + 1));
	if (buf == NULL) {
		return ret;
	}
	str_join_t(wret, header, TEXT(" "), buf, TEXT("\r\n"), (TCHAR *)-1);
	len = tchar_to_char_size(wret);
	tchar_to_char(wret, ret, len);
	mem_free(&wret);
	*(ret + len) = '\0';
	return (ret + len - 1);
#else
	if (buf == NULL) {
		return ret;
	}
	return str_join_t(ret, header, TEXT(" "), buf, TEXT("\r\n"), (TCHAR *)-1);
#endif
}

/*
 * item_to_string_size - メールの保存文字列のサイズ取得
 */
int item_to_string_size(MAILITEM *mail_item, BOOL BodyFlag)
{
	TCHAR X_No[10], X_Mstatus[10], X_Status[10], X_Downflag[10];
	TCHAR X_HeadEncoding[10], X_BodyEncoding[10];
	int len = 0;

#ifndef _itot
	wsprintf(X_No, TEXT("%d"), mail_item->No);
	wsprintf(X_Mstatus, TEXT("%d"), mail_item->MailStatus);
	wsprintf(X_Status, TEXT("%d"), mail_item->Status);
	wsprintf(X_Downflag, TEXT("%d"), mail_item->Download);
	wsprintf(X_HeadEncoding, TEXT("%d"), mail_item->HeadEncoding);
	wsprintf(X_BodyEncoding, TEXT("%d"), mail_item->BodyEncoding);
#else
	_itot(mail_item->No, X_No, 10);
	_itot(mail_item->MailStatus, X_Mstatus, 10);
	_itot(mail_item->Status, X_Status, 10);
	_itot(mail_item->Download, X_Downflag, 10);
	_itot(mail_item->HeadEncoding, X_HeadEncoding, 10);
	_itot(mail_item->BodyEncoding, X_BodyEncoding, 10);
#endif

	len += item_save_header_size(TEXT(HEAD_FROM), mail_item->From);
	len += item_save_header_size(TEXT(HEAD_TO), mail_item->To);
	len += item_save_header_size(TEXT(HEAD_CC), mail_item->Cc);
	len += item_save_header_size(TEXT(HEAD_BCC), mail_item->Bcc);
	len += item_save_header_size(TEXT(HEAD_DATE), mail_item->Date);
	len += item_save_header_size(TEXT(HEAD_SUBJECT), mail_item->Subject);
	len += item_save_header_size(TEXT(HEAD_SIZE), mail_item->Size);
	len += item_save_header_size(TEXT(HEAD_REPLYTO), mail_item->ReplyTo);
	len += item_save_header_size(TEXT(HEAD_CONTENTTYPE), mail_item->ContentType);
	len += item_save_header_size(TEXT(HEAD_ENCODING), mail_item->Encoding);
	len += item_save_header_size(TEXT(HEAD_MESSAGEID), mail_item->MessageID);
	len += item_save_header_size(TEXT(HEAD_INREPLYTO), mail_item->InReplyTo);
	len += item_save_header_size(TEXT(HEAD_REFERENCES), mail_item->References);
	len += item_save_header_size(TEXT(HEAD_X_UIDL), mail_item->UIDL);

	len += item_save_header_size(TEXT(HEAD_X_MAILBOX), mail_item->MailBox);
	len += item_save_header_size(TEXT(HEAD_X_ATTACH), mail_item->Attach);
	if (mail_item->HeadCharset != NULL) {
		len += item_save_header_size(TEXT(HEAD_X_HEADCHARSET), mail_item->HeadCharset);
		len += item_save_header_size(TEXT(HEAD_X_HEADENCODE), X_HeadEncoding);
	}
	if (mail_item->BodyCharset != NULL) {
		len += item_save_header_size(TEXT(HEAD_X_BODYCHARSET), mail_item->BodyCharset);
		len += item_save_header_size(TEXT(HEAD_X_BODYENCODE), X_BodyEncoding);
	}

	len += item_save_header_size(TEXT(HEAD_X_NO), X_No);
	len += item_save_header_size(TEXT(HEAD_X_STATUS), X_Mstatus);
	if (mail_item->MailStatus != mail_item->Status) {
		len += item_save_header_size(TEXT(HEAD_X_MARK), X_Status);
	}
	len += item_save_header_size(TEXT(HEAD_X_DOWNLOAD), X_Downflag);
	len += 2;

	if (BodyFlag == TRUE && mail_item->Body != NULL && *mail_item->Body != '\0') {
		len += tstrlen(mail_item->Body);
	}
	len += 5;
	return len;
}

/*
 * item_to_string - メールの保存文字列の取得
 */
char *item_to_string(char *buf, MAILITEM *mail_item, BOOL BodyFlag)
{
	char *p = buf;
	TCHAR X_No[10], X_Mstatus[10], X_Status[10], X_Downflag[10];
	TCHAR X_HeadEncoding[10], X_BodyEncoding[10];

#ifndef _itot
	wsprintf(X_No, TEXT("%d"), mail_item->No);
	wsprintf(X_Mstatus, TEXT("%d"), mail_item->MailStatus);
	wsprintf(X_Status, TEXT("%d"), mail_item->Status);
	wsprintf(X_Downflag, TEXT("%d"), mail_item->Download);
	wsprintf(X_HeadEncoding, TEXT("%d"), mail_item->HeadEncoding);
	wsprintf(X_BodyEncoding, TEXT("%d"), mail_item->BodyEncoding);
#else
	_itot(mail_item->No, X_No, 10);
	_itot(mail_item->MailStatus, X_Mstatus, 10);
	_itot(mail_item->Status, X_Status, 10);
	_itot(mail_item->Download, X_Downflag, 10);
	_itot(mail_item->HeadEncoding, X_HeadEncoding, 10);
	_itot(mail_item->BodyEncoding, X_BodyEncoding, 10);
#endif

	p = item_save_header(TEXT(HEAD_FROM), mail_item->From, p);
	p = item_save_header(TEXT(HEAD_TO), mail_item->To, p);
	p = item_save_header(TEXT(HEAD_CC), mail_item->Cc, p);
	p = item_save_header(TEXT(HEAD_BCC), mail_item->Bcc, p);
	p = item_save_header(TEXT(HEAD_DATE), mail_item->Date, p);
	p = item_save_header(TEXT(HEAD_SUBJECT), mail_item->Subject, p);
	p = item_save_header(TEXT(HEAD_SIZE), mail_item->Size, p);
	p = item_save_header(TEXT(HEAD_REPLYTO), mail_item->ReplyTo, p);
	p = item_save_header(TEXT(HEAD_CONTENTTYPE), mail_item->ContentType, p);
	p = item_save_header(TEXT(HEAD_ENCODING), mail_item->Encoding, p);
	p = item_save_header(TEXT(HEAD_MESSAGEID), mail_item->MessageID, p);
	p = item_save_header(TEXT(HEAD_INREPLYTO), mail_item->InReplyTo, p);
	p = item_save_header(TEXT(HEAD_REFERENCES), mail_item->References, p);
	p = item_save_header(TEXT(HEAD_X_UIDL), mail_item->UIDL, p);

	p = item_save_header(TEXT(HEAD_X_MAILBOX), mail_item->MailBox, p);
	p = item_save_header(TEXT(HEAD_X_ATTACH), mail_item->Attach, p);
	if (mail_item->HeadCharset != NULL) {
		p = item_save_header(TEXT(HEAD_X_HEADCHARSET), mail_item->HeadCharset, p);
		p = item_save_header(TEXT(HEAD_X_HEADENCODE), X_HeadEncoding, p);
	}
	if (mail_item->BodyCharset != NULL) {
		p = item_save_header(TEXT(HEAD_X_BODYCHARSET), mail_item->BodyCharset, p);
		p = item_save_header(TEXT(HEAD_X_BODYENCODE), X_BodyEncoding, p);
	}

	p = item_save_header(TEXT(HEAD_X_NO), X_No, p);
	p = item_save_header(TEXT(HEAD_X_STATUS), X_Mstatus, p);
	if (mail_item->MailStatus != mail_item->Status) {
		p = item_save_header(TEXT(HEAD_X_MARK), X_Status, p);
	}
	p = item_save_header(TEXT(HEAD_X_DOWNLOAD), X_Downflag, p);
	p = str_cpy(p, "\r\n");

	if (BodyFlag == TRUE && mail_item->Body != NULL && *mail_item->Body != '\0') {
		p = str_cpy(p, mail_item->Body);
	}
	p = str_cpy(p, "\r\n.\r\n");
	return p;
}

/*
 * item_filter_check_content - 
 */
BOOL item_filter_check_content(char *buf, TCHAR *filter_header, TCHAR *filter_content)
{
	TCHAR *Content;
	BOOL ret;
	int len;
	char *cbuf;

	if (filter_content == NULL || *filter_content == TEXT('\0')) {
		return TRUE;
	}
	if( lstrcmpi( filter_header, TEXT( "body" ) ) == 0 )
	{
		char *p;
#ifdef UNICODE
		cbuf = alloc_tchar_to_char( filter_content );
#else
		cbuf = filter_content;
#endif
		p = GetBodyPointa( buf );
		if( p == NULL )
		{
			ret = str_match( cbuf, "" );
		} 
		else 
		{
			// 
			ret = str_match( cbuf, p ); 
		}

#ifdef UNICODE
		mem_free( &cbuf );
#endif
		return ret;
	}

#ifdef UNICODE
	cbuf = alloc_tchar_to_char( filter_header );
	if( cbuf == NULL )
	{
		return FALSE;
	}

	// 
	len = item_get_mime_content( buf, cbuf, &Content, TRUE );
	mem_free( &cbuf ); 

#else
	// 
	len = item_get_mime_content( buf, filter_header, &Content, TRUE );
#endif
	if( Content == NULL ) 
	{
		return str_match_t( filter_content, TEXT( "" ) );
	}
	
	// 
	ret = str_match_t( filter_content, Content ); 
	mem_free( &Content ); 
	return ret;
}

/*
 * item_filter_check - 
 */
static INT32 item_filter_check( CHAR *buf )
{
	INT32 ret = FALSE; 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	ASSERT( buf != NULL ); 
		
	if( *buf == '\0' )
	{
		ASSERT( FALSE ); 
		goto _return; 
	}
	
	log_trace( ( MSG_INFO, "check remote control email content: %s\n", buf ) ); 

	//ret = item_filter_check_content( buf, MAIL_HEADER_SUBJECT, REMOTE_CTRL_SUBJECT ); 
	//if( ret == FALSE ) 
	//{
	//	goto _return; 
	//}

	//ret = item_filter_check_content( buf, MAIL_HEADER_BODY, REMOTE_CTRL_BODY_BEGIN ); 

	//if( ret == FALSE )
	//{
	//	goto _return; 
	//}

_return:
	return ret; 
}

/*
 * item_find_thread - メッセージIDを検索する
 */
int item_find_thread(email_box *mail_box, TCHAR *p, int Index)
{
	MAILITEM *mail_item;
	int j;

	if (p == NULL) {
		return -1;
	}
	for (j = Index - 1; j >= 0; j--) {
		if ((mail_item = (*(mail_box->mail_item + j))) == NULL ||
			mail_item->MessageID == NULL) {
			continue;
		}
		if (lstrcmp(mail_item->MessageID, p) == 0) {
			return j;
		}
	}
	return -1;
}

/*
 * item_create_thread - スレッドを構築する
 */
void item_create_thread(email_box *mail_box)
{
	MAILITEM *mail_item;
	MAILITEM *tpNextMailItem;
	int i, no = 0, n;
	int parent;

	if (mail_box->mail_num == 0 ||
		(*mail_box->mail_item != NULL && (*mail_box->mail_item)->NextNo != 0)) {
		return;
	}
	for (i = 0; i < mail_box->mail_num; i++) {
		if ((mail_item = *(mail_box->mail_item + i)) == NULL) {
			continue;
		}
		mail_item->NextNo = mail_item->PrevNo = mail_item->Indent = 0;
		// 元メールの検索
		parent = item_find_thread(mail_box, mail_item->InReplyTo, i);
		if (parent == -1) {
			parent = item_find_thread(mail_box, mail_item->References, i);
		}
		// 元メールなし
		if (parent == -1) {
			(*(mail_box->mail_item + no))->NextNo = i;
			mail_item->PrevNo = no;
			no = i;
			continue;
		}
		// インデントを設定する
		mail_item->Indent = (*(mail_box->mail_item + parent))->Indent + 1;
		n = (*(mail_box->mail_item + parent))->NextNo;
		while (n != 0) {
			if ((tpNextMailItem = (*(mail_box->mail_item + n))) == NULL) {
				n = 0;
				break;
			}
			// インデントからメールの追加位置を取得する
			if (tpNextMailItem->Indent < mail_item->Indent) {
				mail_item->PrevNo = tpNextMailItem->PrevNo;
				mail_item->NextNo = n;

				(*(mail_box->mail_item + tpNextMailItem->PrevNo))->NextNo = i;
				tpNextMailItem->PrevNo = i;
				break;
			}
			n = tpNextMailItem->NextNo;
		}
		if (n == 0) {
			(*(mail_box->mail_item + no))->NextNo = i;
			mail_item->PrevNo = no;
			no = i;
		}
	}
	// ソート数値を設定する
	no = 0;
	for (i = 0; i < mail_box->mail_num; i++) {
		if (*(mail_box->mail_item + no) == NULL) {
			break;
		}
		(*(mail_box->mail_item + no))->PrevNo = i;
		no = (*(mail_box->mail_item + no))->NextNo;
	}
}
/* End of source */
