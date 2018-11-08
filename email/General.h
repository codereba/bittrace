/*
 * ERC
 *
 * General.h
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef _INC_MAIL_GENERAL_H
#define _INC_MAIL_GENERAL_H

#define CHARSET_GB2312 "gb2312"
#define FONT_CHARSET "GB2312"

#ifndef ERROR_ERRORS_ENCOUNTERED
#define ERROR_ERRORS_ENCOUNTERED 0xC0000001 
#endif //ERROR_ERRORS_ENCOUNTERED

#if !defined(_W64)
#if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && _MSC_VER >= 1300
#define _W64 __w64
#else
#define _W64
#endif
#endif

// typedefs
#ifdef  _WIN64
typedef unsigned __int64    size_t;
#else
typedef _W64 unsigned int   size_t;
#endif

//char *  __cdecl strcpy(char *, const char *);
//size_t  __cdecl strlen(const char *);
//int     __cdecl strcmp(const char *, const char *); 
//char *  __cdecl strcat(char *, const char *); 
//
//void *  __cdecl memset(void *, int, size_t); 
//int     __cdecl memcmp(const void *, const void *, size_t); 
//void *  __cdecl memcpy(void *, const void *, size_t); 
//void *  __cdecl memmove(void *, const void *, size_t); 

#ifdef _DEBUG
#include <assert.h>
#define ASSERT( x ) assert( x )
#else
#define ASSERT( x ) 
#endif //_DEBUG

/* Include Files */
#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE
#include <windowsx.h>
#include <commctrl.h>
#ifndef _WIN32_WCE_PPC
#include <commdlg.h>
#endif
#include <winsock.h>
#include <tchar.h>

#ifdef UNICODE
#include <stdlib.h>
#endif

#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
#include "stdafx.h"
#endif
#include <resource.h>
#include <Strtbl.h>
#include "font.h"

#define RECVTIME				5					// タイマーインターバル
#define SMTPTIME				100
#define CHECKTIME				100
#define AUTOCHECKTIME			60000
#define TIMEOUTTIME				60000

#define SMTP_RETRY_TIME 60
#define SMTP_TEST_RETRY_TIME 8
#define RETRY_TIME 10
#define ERC_REST_TIME 60000

/* Define */
#define APP_NAME				TEXT("ERC Ver 1.0.1")
#define WINDOW_TITLE			TEXT("ERC")
#define KEY_NAME				TEXT("ERC")

#define STR_MUTEX				TEXT("_ERC_Mutex_")
#define ATTACH_FILE				TEXT("_attach_")
#define ATTACH_SEP				TEXT('|')

#ifdef _WIN32_WCE
#if _WIN32_WCE < 211
#define _WCE_OLD				1
#endif
#endif

#define MAIN_WND_CLASS			TEXT("ERCMainWndClass")
#define VIEW_WND_CLASS			TEXT("ERCViewWndClass")
#define EDIT_WND_CLASS			TEXT("ERCEditWndClass")

#define ADDRESS_FILE_OLD		TEXT("Address.dat")
#define ADDRESS_FILE			TEXT("Address.lst")
#define SAVEBOX_FILE			TEXT("SaveBox.dat")
#define SENDBOX_FILE			TEXT("SendBox.dat")

#define VIEW_FILE				TEXT("$ERC_view")
#ifdef _WIN32_WCE
#define EDIT_FILE				TEXT("$ERC_edit")
#endif

#define LOG_FILE				TEXT("ERC.log")

#define RAS_WAIT_EVENT			TEXT("RAS_WAIT_EVENT")
#define ID_RASWAIT_TIMER		10

#define BUF_SIZE				256					// バッファサイズ
#define MAXSIZE					32768
#define EDITMAXSIZE				30000

#define SICONSIZE				16					// 小さいアイコンのサイズ
#define TB_ICONSIZE				16					// ツールバーのボタンサイズ

#define TABSTOPLEN				8					// TAB Stop

#define LV_COL_CNT				4					// ListViewのカラム数
#define AD_COL_CNT				2					// ListView Addressbook

#define MAILBOX_SAVE			0					// 固定メールボックス
#define MAILBOX_SEND			1
#define MAILBOX_USER			2

#define IDC_COMBO				400					// コントロールID
#define IDC_LISTVIEW			401
#define IDC_STATUS				402

#ifndef LVS_EX_INFOTIP
#define LVS_EX_INFOTIP			0x400
#endif

#define WM_SOCK_SELECT			(WM_APP + 1)		// ソケットイベント
#define WM_SOCK_RECV			(WM_APP + 2)		// ソケット受信イベント
#define WM_LV_EVENT				(WM_APP + 3)		// リストビューイベント
#define WM_STATUSTEXT			(WM_APP + 4)		// ステータスバーへ文字列設定
#define WM_SHOWLASTWINDOW		(WM_APP + 5)		// ウィンドウ表示
#define WM_SMTP_SENDMAIL		(WM_APP + 6)		// メール送信
#define WM_ENDCLOSE				(WM_APP + 7)		// ウィンドウの終了
#define WM_INITTRAYICON			(WM_APP + 8)		// タスクトレイアイコンの初期化

#define WM_SHOWDIALOG			(WM_APP + 9)		// メール到着メッセージ表示
#define WM_ENDDIALOG			(WM_APP + 10)		// メール到着メッセージ終了

#define WM_CHANGE_MARK			(WM_APP + 11)

#define EDIT_OPEN				0					// 送信メール編集のタイプ
#define EDIT_NEW				1
#define EDIT_REPLY				2

#define EDIT_NONEDIT			0					// 送信メール編集の戻り値
#define EDIT_INSIDEEDIT			1
#define EDIT_OUTSIDEEDIT		2

#define SELECT_MEM_ERROR		-2					// ソケット処理の戻り値
#define SELECT_SOC_ERROR		-1
#define SELECT_SOC_CLOSE		0
#define SELECT_SOC_SUCCEED		1
#define SELECT_SOC_NODATA		2

#define SORT_NO					100					// ソートフラグ
#define SORT_IOCN				101
#define SORT_THREAD				102

#define POP_ERR					-2					// POP3コマンドフラグ
#define POP_QUIT				-1
#define POP_START				0
#define POP_STARTTLS			1
#define POP_USER				2
#define POP_PASS				3
#define POP_LOGIN				4
#define POP_STAT				5
#define POP_LIST				6
#define POP_UIDL				7
#define POP_UIDL_ALL			8
#define POP_UIDL_CHECK			9
#define POP_TOP					10
#define POP_RETR				11
#define POP_DELE				12

#define SMTP_ERR				POP_ERR				// SMTPコマンドフラグ
#define SMTP_QUIT				POP_QUIT
#define SMTP_START				POP_START
#define SMTP_EHLO				1
#define SMTP_STARTTLS			2
#define SMTP_AUTH_CRAM_MD5		3
#define SMTP_AUTH_LOGIN			4
#define SMTP_AUTH_LOGIN_PASS	5
#define SMTP_HELO				6
#define SMTP_RSET				7
#define SMTP_MAILFROM			8
#define SMTP_RCPTTO				9
#define SMTP_DATA				10
#define SMTP_SENDBODY			11
#define SMTP_NEXTSEND			12
#define SMTP_SENDEND			13

#define ICON_NON				0					// アイコン状態
#define ICON_MAIL				1
#define ICON_READ				2
#define ICON_DOWN				3
#define ICON_DEL				4
#define ICON_SENDMAIL			5
#define ICON_SEND				6
#define ICON_ERROR				7
#define ICON_NEW				8

#define FILTER_UNRECV			1					// フィルタタイプ
#define FILTER_RECV				2
#define FILTER_DOWNLOADMARK		4
#define FILTER_DELETEMARK		8
#define FILTER_READICON			16
#define FILTER_SAVE				32

#define MP_ERROR_FILE			-2					// マルチパート処理の戻り値
#define MP_ERROR_ALLOC			-1
#define MP_NO_ATTACH			0
#define MP_ATTACH				1

#define HEAD_SUBJECT			"Subject:"
#define HEAD_FROM				"From:"
#define HEAD_TO					"To:"
#define HEAD_CC					"Cc:"
#define HEAD_BCC				"Bcc:"
#define HEAD_REPLYTO			"Reply-To:"
#define HEAD_DATE				"Date:"
#define HEAD_SIZE				"Content-Length:"
#define HEAD_MESSAGEID			"Message-Id:"
#define HEAD_INREPLYTO			"In-Reply-To:"
#define HEAD_REFERENCES			"References:"
#define HEAD_MIMEVERSION		"MIME-Version:"
#define HEAD_CONTENTTYPE		"Content-Type:"
#define HEAD_ENCODING			"Content-Transfer-Encoding:"
#define HEAD_DISPOSITION		"Content-Disposition:"
#define HEAD_X_MAILER			"X-Mailer:"
#define HEAD_X_UIDL				"X-UIDL:"
#define HEAD_X_NO				"X-No:"
#define HEAD_X_MARK				"X-Mark:"
#define HEAD_X_STATUS			"X-Status:"
#define HEAD_X_DOWNLOAD			"X-Download:"
#define HEAD_X_MAILBOX			"X-MailBox:"
#define HEAD_X_ATTACH			"X-File:"
#define HEAD_X_HEADCHARSET		"X-Header-Charset:"
#define HEAD_X_HEADENCODE		"X-Header-Encodeing:"
#define HEAD_X_BODYCHARSET		"X-Body-Charset:"
#define HEAD_X_BODYENCODE		"X-Body-Encodeing:"

#define HEAD_X_NO_OLD			"X-MailNo:"
#define HEAD_X_MARK_OLD			"X-MarkStatus:"
#define HEAD_X_STATUS_OLD		"X-MailStatus:"
#define HEAD_X_DOWNLOAD_OLD		"X-MailDownload:"
#define HEAD_X_ATTACH_OLD		"X-Attach:"

#define CHARSET_US_ASCII		"US-ASCII"
#define CHARSET_ISO_8859_1		"ISO-8859-1"
#define CHARSET_ISO_2022_JP		"ISO-2022-JP"
#define CHARSET_UTF_7			"UTF-7"
#define CHARSET_UTF_8			"UTF-8"

#define ENCODE_7BIT				"7bit"
#define ENCODE_8BIT				"8bit"
#define ENCODE_BASE64			"base64"
#define ENCODE_Q_PRINT			"quoted-printable"

#define URL_HTTP				TEXT("http://")
#define URL_HTTPS				TEXT("https://")
#define URL_FTP					TEXT("ftp://")
#define URL_MAILTO				TEXT("mailto:")

#define ABS(n)					((n < 0) ? (n * -1) : n)				// 絶対値

#define MAX_EMAIL_NAME_LEN MAX_PATH

__inline LPCWSTR get_smtp_action_desc( ULONG action )
{
	LPCWSTR desc = L"UNKNOW_MAIL_SEND_STATE"; 

	switch( action )
	{
	case SMTP_ERR: 
		desc = L"SMTP_ERR"; 
		break; 
	case SMTP_QUIT:
		desc = L"SMTP_QUIT"; 
		break; 
	case SMTP_START:
		desc = L"SMTP_START"; 
		break; 
	case SMTP_EHLO:
		desc = L"SMTP_EHLO"; 
		break; 
	case SMTP_STARTTLS:
		desc = L"SMTP_STARTTLS"; 
		break; 
	case SMTP_AUTH_CRAM_MD5:
		desc = L"SMTP_AUTH_CRAM_MD5"; 
		break; 
	case SMTP_AUTH_LOGIN:
		desc = L"SMTP_AUTH_LOGIN"; 
		break; 
	case SMTP_AUTH_LOGIN_PASS:
		desc = L"SMTP_AUTH_LOGIN_PASS"; 
		break; 
	case SMTP_HELO:
		desc = L"SMTP_HELO"; 
		break; 
	case SMTP_RSET:
		desc = L"SMTP_RSET"; 
		break; 
	case SMTP_MAILFROM:
		desc = L"SMTP_MAILFROM"; 
		break; 
	case SMTP_RCPTTO:
		desc = L"SMTP_RCPTTO"; 
		break; 
	case SMTP_DATA:
		desc = L"SMTP_DATA"; 
		break; 
	case SMTP_SENDBODY:
		desc = L"SMTP_SENDBODY"; 
		break; 
	case SMTP_NEXTSEND:
		desc = L"SMTP_NEXTSEND"; 
		break; 
	case SMTP_SENDEND:
		desc = L"SMTP_SENDEND"; 
		break; 
	default:
		ASSERT( FALSE && "invalid mail send state" ); 
	}

	return desc; 
}

__inline LPCWSTR get_pop3_action_desc( ULONG action )
{
	LPCWSTR desc = L"UNKNOW_POP3_ACTION"; 
	switch( action )
	{
	case POP_ERR:
		desc = L"POP_ERR"; 
		break; 
	case POP_QUIT:
		desc = L"POP_QUIT"; 
		break; 
	case POP_START:
		desc = L"POP_START"; 
		break; 
	case POP_STARTTLS:
		desc = L"POP_STARTTLS"; 
		break; 
	case POP_USER:
		desc = L"POP_USER"; 
		break; 
	case POP_PASS:
		desc = L"POP_PASS"; 
		break; 
	case POP_LOGIN:
		desc = L"POP_LOGIN"; 
		break; 
	case POP_STAT:
		desc = L"POP_STAT"; 
		break; 
	case POP_LIST:
		desc = L"POP_LIST"; 
		break; 
	case POP_UIDL:
		desc = L"POP_UIDL"; 
		break; 
	case POP_UIDL_ALL:
		desc = L"POP_UIDL_ALL"; 
		break; 
	case POP_UIDL_CHECK:
		desc = L"POP_UIDL_CHECK"; 
		break; 
	case POP_TOP:
		desc = L"POP_TOP"; 
		break; 
	case POP_RETR:
		desc = L"POP_RETR"; 
		break; 
	case POP_DELE:
		desc = L"POP_DELE"; 
		break; 
	default:
		ASSERT( FALSE && "invalid pop3 action" ); 
		break; 
	}

	return desc; 
}
/* Struct */

typedef struct _OPTION {
	int StertPass;
	int ShowPass;
	TCHAR *Password;

	TCHAR DataFileDir[BUF_SIZE];

	FONT_INFO view_font;
	FONT_INFO lv_font;

	#ifndef _WIN32_WCE
	RECT MainRect;
	RECT ViewRect;
	RECT EditRect;
	#endif

	int ShowTrayIcon;
	int StartHide;
	int MinsizeHide;
	int CloseHide;
	int TrayIconToggle;
	int StartInit;
	int SocLog;

	int LvDefSelectPos;
	int LvAutoSort;
	int LvSortItem;
	int LvThreadView;
	int LvStyle;
	int LvStyleEx;
	int MoveAllMailBox;
	int RecvScroll;
	int SaveMsg;
	int AutoSave;
	int LvColSize[LV_COL_CNT];
	int AddColSize[AD_COL_CNT];

	int ListGetLine;
	int ListDownload;
	int ShowHeader;
	int ListSaveMode;
	int WordBreakFlag;
	int EditWordBreakFlag;
	int ViewShowDate;
	int MstchCase;
	int AllFind;
	int SubjectFind;

	int ESMTP;
	TCHAR *SendHelo;
	int SendMessageId;
	int SendDate;
	int SelectSendBox;
	int PopBeforeSmtpIsLoginOnly;
	int PopBeforeSmtpWait;

	// SSL
	TCHAR *CAFile;

	TCHAR *HeadCharset;
	int HeadEncoding;
	TCHAR *BodyCharset;
	int BodyEncoding;

	int AutoQuotation;
	TCHAR *QuotationChar;
	int WordBreakSize;
	int QuotationBreak;
	TCHAR *ReSubject;
	TCHAR *ReHeader;
	TCHAR *Bura;
	TCHAR *Oida;
	TCHAR *sBura;
	TCHAR *sOida;

	int IPCache;
	int EncodeType;
	TCHAR *TimeZone;
	TCHAR *DateFormat;
	TCHAR *TimeFormat;

	int ShowNewMailMessgae;
	int ShowNoMailMessage;
	int ActiveNewMailMessgae;

	int NewMailSound;
	TCHAR *NewMailSoundFile;
	int ExecEndSound;
	TCHAR *ExecEndSoundFile;

	int AutoCheck;
	int AutoCheckTime;
	int StartCheck;
	int CheckAfterUpdate;
	int SocIgnoreError;
	int SendIgnoreError;
	int CheckEndExec;
	int CheckEndExecNoDelMsg;
	int TimeoutInterval;

	int ViewClose;
	TCHAR *ViewApp;
	TCHAR *ViewAppCmdLine;
	TCHAR *ViewFileSuffix;
	TCHAR *ViewFileHeader;
	int ViewAppClose;
	int DefViewApp;
	TCHAR *EditApp;
	TCHAR *EditAppCmdLine;
	TCHAR *EditFileSuffix;
	int DefEditApp;
	TCHAR *AttachPath;
	int AttachWarning;
	int AttachDelete;

	TCHAR *URLApp;

	int EnableLAN;

	int RasCon;
	int RasCheckEndDisCon;
	int RasEndDisCon;
	int RasNoCheck;
	int RasWaitSec;
	struct _RASINFO **RasInfo;
	int RasInfoCnt;
} OPTION;

typedef struct _SSL_INFO {
	int Type;
	int Verify;
	int Depth;
	TCHAR *Cert;
	TCHAR *Pkey;
	TCHAR *Pass;
} SSL_INFO;

typedef struct _MAILBOX {
	TCHAR *Name;

	// POP
	TCHAR *Server;
	int Port;
	TCHAR *User;
	TCHAR *Pass;
	TCHAR *TmpPass;
	int APOP;
	int PopSSL;
	SSL_INFO PopSSLInfo;
	int NoRETR;
	int NoUIDL;
	unsigned long PopIP;

	int mail_num;
	unsigned int MailSize;

	char *LastMessageId;
	int LastNo;

	// SMTP
	TCHAR *SmtpServer;
	int SmtpPort;
	TCHAR *UserName;
	TCHAR *MailAddress;
	TCHAR *Signature;
	TCHAR *ReplyTo;
	int MyAddr2Bcc;
	TCHAR *BccAddr;
	int PopBeforeSmtp;
	int SmtpAuth;
	int SmtpAuthType;
	int AuthUserPass;
	TCHAR *SmtpUser;
	TCHAR *SmtpPass;
	TCHAR *SmtpTmpPass;
	int SmtpSSL;
	SSL_INFO SmtpSSLInfo;
	unsigned long SmtpIP;

	// Check
	BOOL NewMail;
	BOOL NoRead;
	int CyclicFlag;

	// Filter
	int FilterEnable;
	struct _FILTER **tpFilter;
	int FilterCnt;

	// Ras
	int RasMode;
	TCHAR *RasEntry;
	int RasReCon;

	// MailItem
	struct _MAILITEM **tpMailItem;
	int MailItemCnt;
	int AllocCnt;
} MAILBOX;

typedef struct _MAILITEM {
	int No;
	int Status;
	int MailStatus;
	int PrevNo;
	int NextNo;
	int Indent;
	BOOL New;
	BOOL Download;
	BOOL Multipart;
	HWND hEditWnd;
	HANDLE hProcess;

	TCHAR *From;
	TCHAR *To;
	TCHAR *Cc;
	TCHAR *Bcc;
	TCHAR *Date;
	ULONG Size;
	TCHAR *Subject;
	TCHAR *ReplyTo;
	TCHAR *ContentType;
	TCHAR *Encoding;
	TCHAR *MessageID;
	TCHAR *UIDL;
	TCHAR *InReplyTo;
	TCHAR *References;
	char *Body;

	TCHAR *MailBox;
	TCHAR *Attach;
	TCHAR *HeadCharset;
	int HeadEncoding;
	TCHAR *BodyCharset;
	int BodyEncoding;
} MAILITEM;

typedef struct _FILTER {
	int Enable;
	int Action;

	TCHAR *Header1;
	TCHAR *Content1;

	TCHAR *Header2;
	TCHAR *Content2;
} FILTER;

typedef struct _ATTACHINFO {
	TCHAR *from;
	TCHAR *fname;
	TCHAR *mime;
	int size;
} ATTACHINFO;

typedef struct _RASINFO {
	TCHAR *RasEntry;
	TCHAR *RasUser;
	TCHAR *RasPass;
} RASINFO;

typedef struct _email_box
{
	TCHAR name[ MAX_EMAIL_NAME_LEN ]; 
	TCHAR email[ MAX_EMAIL_NAME_LEN ]; 
	TCHAR pwd[ MAX_EMAIL_NAME_LEN ]; 
	TCHAR server[ MAX_EMAIL_NAME_LEN ];
	TCHAR pop3_server[ MAX_EMAIL_NAME_LEN ]; 
	USHORT port; 
	USHORT pop3_port; 
	BOOLEAN erc_on; 
	BOOLEAN need_auth; 
	BOOLEAN use_ssl; 
	BOOLEAN use_apop; 
	BOOLEAN pop_use_ssl; 
	BOOLEAN no_uidl; 
	SSL_INFO ssl_info; 
	SSL_INFO pop_ssl_info; 
	ULONG smtp_auth_type; 
	MAILITEM **mail_item; 
	ULONG max_mail_count; 
	ULONG cur_mail_count; 
	BOOLEAN no_retr; 
	//BOOLEAN need_update; 
	ULONG mail_index; 
	//TCHAR *last_message_id; 
	CHAR *last_message_id; 
	ULONG mail_num; 
	ULONG mail_size; 
} email_box, *pemail_box; 

typedef BOOL ( *command_process_func )( struct _email_handler*, SOCKET, char*, int, TCHAR*, BOOL);

typedef LRESULT ( *check_remote_command_func )( struct _email_handler* handler, LPCSTR data, ULONG data_len, PVOID *context ); 
typedef LRESULT ( *execute_remote_command_func )( struct _email_handler* handler, PVOID context ); 

typedef struct _email_handler
{
	TCHAR *ca_file; 
	command_process_func command_proc; 
	check_remote_command_func command_check; 
	execute_remote_command_func command_handler; 
	PVOID notifier; 
	//INT32 have_ui; 
	MAILITEM *send_mail_item; 
	email_box *cur_email; 
	ULONG command_status; 
	ULONG ssl_type; 
	ULONG auth_type; 
	BOOLEAN auth_flag; 
	BOOLEAN starttls_flag; 
	BOOLEAN is_esmtp; 
	BOOLEAN send_date; 
	BOOLEAN send_msg_id; 
	BOOLEAN pop_start_tls; 
	BOOLEAN pop_before_smtp; 
	//BOOLEAN need_update; 
	ULONG send_end_cmd; 

	TCHAR *body_charset; 
	CHAR *send_body; 
	PTCHAR send_pt; 
	ULONG send_len; 

	ULONG new_mail_count; 
	PVOID work_context; 
	PVOID erc_cmd_record; 
} email_handler, *pemail_handler; 

/* Function Prototypes */

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

	VOID get_recv_buf( CHAR **buf, ULONG *buf_len ); 
	VOID get_old_recv_buf( CHAR **buf, ULONG *buf_len ); 

// Ras
BOOL GetRasInfo(TCHAR *Entry);
BOOL SetRasInfo(TCHAR *Entry, TCHAR *User, TCHAR *Pass);
void FreeRasInfo(void);
void initRas(void);
void FreeRas(void);
BOOL GetRasEntries(HWND hCmboWnd);
BOOL GetRasStatus(void);
void RasDisconnect(void);
BOOL RasStatusProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL RasMailBoxStart(HWND hWnd, int BoxIndex);

typedef LRESULT ( *net_cmd_process_callback )( email_handler *handler, SOCKET soc, char *buf, int len, TCHAR *err_msg, BOOL show_flag ); 

// WinSock
LRESULT get_host_by_name( email_handler* handler, TCHAR *server, ULONG *ip_out, TCHAR *ErrStr); 
LRESULT connect_server( email_handler *handler, ULONG ip_addr, USHORT port, const ULONG ssl_tp, const SSL_INFO *si, SOCKET *socket_out, TCHAR *ErrStr ); 
LRESULT recv_proc(email_handler *handler, SOCKET soc, net_cmd_process_callback cmd_handler ); 
#ifndef WSAASYNC
LRESULT recv_select( email_handler *handler, SOCKET soc, net_cmd_process_callback cmd_handler );
#endif	//WSAASYNC
int send_data( email_handler *handler, SOCKET soc, char *wbuf, int len);
LRESULT send_buf( email_handler *hnadler, SOCKET soc, char *buf);
#ifdef UNICODE
int send_buf_t( email_handler *handler, SOCKET soc, TCHAR *wbuf);
#else	//UNICODE
#define send_buf_t	send_buf
#endif	//UNICODE
void socket_close(email_handler *handler, SOCKET soc);
LRESULT init_ssl( const email_handler *handler, 
				 const SOCKET soc, 
				 TCHAR *ErrStr ); 

void free_ssl(void);

BOOL item_filter_check_content(char *buf, TCHAR *filter_header, TCHAR *filter_content); 

// Pop3
LRESULT pop3_list_proc( email_handler *handler, SOCKET soc, char *buf, int len, TCHAR *ErrStr, BOOL ShowFlag);
LRESULT pop3_test_login_proc( email_handler *handler, SOCKET soc, char *buf, int len, TCHAR *ErrStr, BOOL ShowFlag); 
LRESULT pop3_exec_proc( email_handler *handler, SOCKET soc, char *buf, int len, TCHAR *ErrStr, BOOL ShowFlag);
void pop3_free(void);

// Smtp
void HMAC_MD5(unsigned char *input, int len, unsigned char *key, int keylen, unsigned char *digest);
#ifdef WSAASYNC
BOOL smtp_send_proc(HWND hWnd, SOCKET soc, TCHAR *ErrStr, email_box *mail_box);
#endif
LRESULT smtp_proc( email_handler *handler, 
			   SOCKET soc, 
			   char *buf, 
			   int len, 
			   TCHAR *ErrStr, 
			   BOOL ShowFlag ); 

LRESULT smtp_send_mail( email_handler *handler, email_box *mail_box, MAILITEM *tpMailItem, int end_cmd, TCHAR *ErrStr); 
LRESULT smtp_test_login( email_handler *handler, email_box *mail_box, MAILITEM *mail_item, int end_cmd, TCHAR *err_msg); 
void smtp_set_error(email_handler *handler);
void smtp_free( email_handler *handler);

// File
BOOL log_init(TCHAR *fpath, TCHAR *fname, TCHAR *buf);
BOOL log_save(TCHAR *fpath, TCHAR *fname, TCHAR *buf);
BOOL log_clear(TCHAR *fpath, TCHAR *fname);
BOOL dir_check(TCHAR *path);
BOOL dir_delete(TCHAR *Path, TCHAR *file);
void filename_conv(TCHAR *buf);
BOOL filename_select(HWND hWnd, TCHAR *ret, TCHAR *DefExt, TCHAR *filter, BOOL OpenSave);
long file_get_size(TCHAR *FileName);
char *file_read(TCHAR *path, long FileSize);
BOOL file_read_select(HWND hWnd, TCHAR **buf);
BOOL file_read_mailbox(TCHAR *FileName, email_box *mail_box);
int file_read_address_book(TCHAR *FileName, email_box *mail_box);
BOOL file_write(HANDLE hFile, char *buf, int len);
BOOL file_write_ascii(HANDLE hFile, TCHAR *buf, int len);
BOOL file_save(HWND hWnd, TCHAR *FileName, TCHAR *Ext, char *buf, int len);
BOOL file_save_exec(HWND hWnd, TCHAR *FileName, char *buf, int len);
BOOL file_save_mailbox(TCHAR *FileName, email_box *mail_box, int SaveFlag);
BOOL file_save_address_book(TCHAR *FileName, email_box *mail_box);

// Ini
#ifndef _WIN32_WCE
BOOL ini_start_auth_check(void);
#endif
BOOL ini_read_setting(HWND hWnd);
BOOL ini_save_setting(HWND hWnd, BOOL SaveMailFlag);
void ini_free(void);

// Item
BOOL item_is_mailbox(email_box *mail_box, MAILITEM *tpMailItem);
BOOL item_set_count(email_box *mail_box, int i);
BOOL item_add(email_box *mail_box, MAILITEM *tpNewMailItem);
void item_copy(MAILITEM *tpFromMailItem, MAILITEM *tpToMailItem);
MAILITEM *item_to_mailbox(email_box *mail_box, MAILITEM *tpNewMailItem, TCHAR *MailBoxName, BOOL SendClear);
BOOL item_resize_mailbox(email_box *mail_box);
void item_free(MAILITEM **tpMailItem, int cnt);
char *item_get_message_id(char *buf);
int item_get_number_to_index(email_box *mail_box, int No);
int item_get_next_download_mark(email_box *mail_box, int Index, int *No);
int item_get_next_delete_mark(email_box *mail_box, int Index, int *No);
int item_get_next_send_mark(email_box *mail_box, int Index, int *MailBoxIndex);
int item_get_next_send_mark_mailbox(email_box *mail_box, int Index, int MailBoxIndex);
BOOL item_mail_to_item(email_box *tpMailItem, char *buf, int Size, BOOL download);
LRESULT item_header_to_item( MAILITEM **mail_item, char *buf, int Size); 
MAILITEM *item_string_to_item(email_box *mail_box, char *buf);
int item_to_string_size(MAILITEM *tpMailItem, BOOL BodyFlag);
char *item_to_string(char *buf, MAILITEM *tpMailItem, BOOL BodyFlag);
int item_find_thread(email_box *mail_box, TCHAR *p, int Index);
void item_create_thread(email_box *mail_box);

// MailBox
BOOL mailbox_init(void);
int mailbox_create(HWND hWnd, BOOL ShowFlag);
int mailbox_delete(HWND hWnd, int DelIndex);
BOOL mailbox_read(void);
void mailbox_move_up(HWND hWnd);
void mailbox_move_down(HWND hWnd);
BOOL mailbox_unread_check(int index, BOOL NewFlag);
int mailbox_next_unread(int index, int endindex);
void mailbox_select(HWND hWnd, int Sel);
int mailbox_name_to_index(TCHAR *Name);
void filer_free(email_box *mail_box);
void mailbox_free(email_box *mail_box);

// util
TCHAR *GetHeaderStringPointT(TCHAR *buf, TCHAR *str);
#ifdef UNICODE
char *GetHeaderStringPoint(char *buf, char *str);
#else
#define GetHeaderStringPoint GetHeaderStringPointT
#endif
int GetHeaderStringSizeT(TCHAR *buf, BOOL CrLfFlag);
#ifdef UNICODE
int GetHeaderStringSize(char *buf, BOOL CrLfFlag);
#else
#define GetHeaderStringSize GetHeaderStringSizeT
#endif
BOOL GetHeaderStringT(TCHAR *buf, TCHAR *ret, BOOL CrLfFlag);
#ifdef UNICODE
BOOL GetHeaderString(char *buf, char *ret, BOOL CrLfFlag);
#else
#define GetHeaderString GetHeaderStringT
#endif

TCHAR *GetBodyPointaT(TCHAR *buf);
#ifdef UNICODE
char *GetBodyPointa(char *buf);
#else
#define GetBodyPointa GetBodyPointaT
#endif

void TrimMessageId(char *buf);
int GetReferencesSize(char *p, BOOL Flag);
BOOL ConvReferences(char *p, char *r, BOOL Flag);

void DateAdd(SYSTEMTIME *sTime, char *tz);
int DateConv(char *buf, char *ret);
int SortDateConv(char *buf, char *ret);
void GetTimeString(TCHAR *buf);
void EncodePassword(TCHAR *Key, TCHAR *Word, TCHAR *ret, int retsize, BOOL decode);
void EncodeCtrlChar(TCHAR *buf, TCHAR *ret);
void DecodeCtrlChar(TCHAR *buf, TCHAR *ret);
TCHAR *CreateMessageId(long id, TCHAR *MailAddress);
int CreateHeaderStringSize(TCHAR *buf, MAILITEM *tpMailItem);
TCHAR *CreateHeaderString(TCHAR *buf, TCHAR *ret, MAILITEM *tpMailItem);
int GetReplyBodySize(TCHAR *body, TCHAR *ReStr);
TCHAR *SetReplyBody(TCHAR *body, TCHAR *ret, TCHAR *ReStr);
int SetDotSize(TCHAR *buf);
void SetDot(TCHAR *buf, TCHAR *ret);
void DelDot(TCHAR *buf, TCHAR *ret);
int WordBreakStringSize(TCHAR *buf, TCHAR *str, int BreakCnt, BOOL BreakFlag);
void WordBreakString(TCHAR *buf, TCHAR *ret, TCHAR *str, int BreakCnt, BOOL BreakFlag);
BOOL URLToMailItem(TCHAR *buf, MAILITEM *tpMailItem);
TCHAR *GetMailAddress(TCHAR *buf, TCHAR *ret, BOOL quote);
TCHAR *GetMailString(TCHAR *buf, TCHAR *ret);
void SetUserName(TCHAR *buf, TCHAR *ret);
int SetCcAddressSize(TCHAR *To);
TCHAR *SetCcAddress(TCHAR *Type, TCHAR *To, TCHAR *r);
TCHAR *GetFileNameString(TCHAR *p);
int SetAttachListSize(TCHAR *buf);
TCHAR *SetAttachList(TCHAR *buf, TCHAR *ret);
TCHAR *GetMIME2Extension(TCHAR *MIMEStr, TCHAR *Filename);
TCHAR *CreateCommandLine(TCHAR *buf, TCHAR *filename, BOOL spFlag);

// View
void SetWordBreakMenu(HWND hWnd, HMENU hEditMenu, int Flag);
#ifdef _WIN32_WCE_LAGENDA
int SetWordBreak(HWND hWnd, HMENU hMenu);
#else
int SetWordBreak(HWND hWnd);
#endif
void View_FindMail(HWND hWnd, BOOL FindSet);
BOOL View_InitApplication(HINSTANCE hInstance);
BOOL View_InitInstance(HINSTANCE hInstance, LPVOID lpParam, BOOL NoAppFlag);

// Edit
#ifndef _WIN32_WCE
BOOL CALLBACK enum_windows_proc(const HWND hWnd, const LPARAM lParam);
#endif
BOOL Edit_InitApplication(HINSTANCE hInstance);
int Edit_MailToSet(HINSTANCE hInstance, HWND hWnd, TCHAR *mail_addr, int rebox);
int Edit_InitInstance(HINSTANCE hInstance, HWND hWnd, int rebox,
					   MAILITEM *tpReMailItem, int OpenFlag, int ReplyFag);

// Option
void AllocGetText(HWND hEdit, TCHAR **buf);
BOOL SetMailBoxOption(HWND hWnd);
BOOL CALLBACK SetEncodeProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void SetOption(HWND hWnd);
BOOL CALLBACK InputPassProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK InitMailBoxProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK SetAttachProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CheckDependence(HWND hWnd, int Ctl);
BOOL CALLBACK SetSendProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK MailPropProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK AddressListProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK SetFindProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK NewMailMessageProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK AttachNoticeProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

// ListView
void ListView_AddColumn(HWND hListView, int fmt, int cx, TCHAR *buf, int iSubItem);
HWND CreateListView(HWND hWnd, int Top, int bottom);
void ListView_SetRedraw(HWND hListView, BOOL DrawFlag);
int ListView_InsertItemEx(HWND hListView, TCHAR *buf, int len, int Img, long lp, int iItem);
void ListView_MoveItem(HWND hListView, int SelectItem, int Move, int ColCnt);
TCHAR *ListView_GetSelStringList(HWND hListView);
long ListView_GetlParam(HWND hListView, int i);
int ListView_GetMemToItem(HWND hListView, MAILITEM *tpMemMailItem);
int ListView_GetNextDeleteItem(HWND hListView, int Index);
int ListView_GetNextMailItem(HWND hListView, int Index);
int ListView_GetPrevMailItem(HWND hListView, int Index);
int ListView_GetNextNoReadItem(HWND hListView, int Index, int endindex);
int ListView_GetNewItem(HWND hListView, email_box *mail_box);
BOOL ListView_ShowItem(HWND hListView, email_box *mail_box);
int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
LRESULT ListView_NotifyProc(HWND hWnd, LPARAM lParam);

// main
LRESULT SwitchCursor(const BOOL Flag);
#ifdef _WIN32_WCE
#define _SetForegroundWindow		SetForegroundWindow
#else
BOOL _SetForegroundWindow(const HWND hWnd);
#endif
__inline void SetStatusTextT(HWND hWnd, TCHAR *buf, int Part)
{

}

void set_email_process_state( PVOID context, TCHAR *buf );
#ifdef UNICODE
void SetSocStatusText(HWND hWnd, char *buf);
#else
#define SetSocStatusText(hWnd, buf)	set_email_process_state(hWnd, buf)
#endif
void SetItemCntStatusText(HWND hWnd, email_box *tpViewMailBox);
void SetStatusRecvLen(HWND hWnd, int len, TCHAR *msg);
//void ErrorMessage(HWND hWnd, TCHAR *buf);
void SocketErrorMessage(HWND hWnd, TCHAR *buf, int BoxIndex);
void ErrorSocketEnd(HWND hWnd, int BoxIndex);
int ShowMenu(HWND hWnd, HMENU hMenu, int mpos, int PosFlag, BOOL ReturnFlag);
__inline void SetMailMenu(HWND hWnd)
{

}
void SetNoReadCntTitle(HWND hWnd);
//BOOL MessageFunc(HWND hWnd, MSG *msg);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
/* End of source */
