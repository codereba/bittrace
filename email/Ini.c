/*
 * nPOP
 *
 * Ini.c
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#include "General.h"
#include "memory_mng.h"
#include "string_mng.h"
#include "Profile.h"
#include "charset.h"

/* Define */
#define GENERAL				TEXT("GENERAL")

#define INI_BUF_SIZE		1024

/* Global Variables */
OPTION op;

// 外部参照
extern HINSTANCE hInst;
extern TCHAR *g_Pass;
extern int gPassSt;
extern TCHAR *AppDir;
extern TCHAR *DataDir;
extern smtp_user *MailBox;
extern int MailBoxCnt;
extern BOOL first_start;

/* Local Function Prototypes */
static void ini_get_encode_info(void);

#ifdef _HAVE_WND
/*
 * ini_start_auth_check - 
 */
#ifndef _WIN32_WCE
BOOL ini_start_auth_check(void)
{
	TCHAR app_path[BUF_SIZE];
	TCHAR ret[BUF_SIZE];
	TCHAR pass[BUF_SIZE];

	str_join_t(app_path, AppDir, KEY_NAME TEXT(".ini"), (TCHAR *)-1);
	profile_initialize(app_path, TRUE);

	op.StertPass = profile_get_int(GENERAL, TEXT("StertPass"), 0, app_path);
	if (op.StertPass == 1) {
		profile_get_string(GENERAL, TEXT("pw"), TEXT(""), ret, BUF_SIZE - 1, app_path);
		EncodePassword(TEXT("_pw_"), ret, pass, BUF_SIZE - 1, TRUE);
		if (*pass == TEXT('\0')) {
			profile_free();
			return TRUE;
		}
		while (1) {
			// 起動パスワード
			gPassSt = 0;
			if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_INPUTPASS), NULL, InputPassProc,
				(LPARAM)STR_TITLE_STARTPASSWORD) == FALSE) {
				profile_free();
				return FALSE;
			}
			if (g_Pass == NULL || lstrcmp(pass, g_Pass) != 0) {
				//ErrorMessage(NULL, STR_ERR_SOCK_BADPASSWORD);
				continue;
			}
			break;
		}
	}
	profile_free();
	return TRUE;
}
#endif

/*
 * ini_get_encode_info - エンコード情報の取得
 */
static void ini_get_encode_info(void)
{
	mem_free(&op.HeadCharset);
	mem_free(&op.BodyCharset);

	switch (GetACP()) {
	case 932:	// ISO-2022-JP, euc-jp
		op.HeadCharset = alloc_copy_t(TEXT(CHARSET_ISO_2022_JP));
		op.HeadEncoding = 2;
		op.BodyCharset = alloc_copy_t(TEXT(CHARSET_ISO_2022_JP));
		op.BodyEncoding = 0;
		break;

	case 1250:	// ISO-8859-2
	case 1251:	// ISO-8859-5, koi8-r, koi8-ru
	case 1252:	// ISO-8859-1
	case 1257:	// ISO-8859-4
		op.HeadEncoding = 3;
		op.BodyEncoding = 3;
		break;

	case 874:	// tis-620, windows-874
	case 936:	// GB2312, hz-gb-2312
	case 949:	// ISO-2022-KR, euc-kr
	case 950:	// BIG5
	case 1253:	// ISO-8859-7
	case 1254:	// ISO-8859-3, ISO-8859-9
	case 1255:	// ISO-8859-8, ISO-8859-8-i, DOS-862
	case 1256:	// ISO-8859-6, ASMO-708, DOS-720, windows-1256
	case 1258:	// windows-1258
	default:
		op.HeadEncoding = 2;
		op.BodyEncoding = 2;
		break;
	}
	set_default_encode(GetACP(), &op.HeadCharset, &op.BodyCharset);
	if (op.HeadCharset == NULL) {
		op.HeadCharset = alloc_copy_t(TEXT(CHARSET_ISO_8859_1));
	}
	if (op.BodyCharset == NULL) {
		op.BodyCharset = alloc_copy_t(TEXT(CHARSET_ISO_8859_1));
	}
}

/*
 * ini_read_setting - INIファイルから設定情報を読みこむ
 */
BOOL ini_read_setting(HWND hWnd)
{
	FILTER *tpFilter;
	HDC hdc;
	TCHAR app_path[BUF_SIZE];
	TCHAR buf[BUF_SIZE];
	TCHAR key_buf[BUF_SIZE];
	TCHAR conv_buf[INI_BUF_SIZE];
	TCHAR ret[BUF_SIZE];
	TCHAR tmp[BUF_SIZE];
	TCHAR *p, *r;
	UINT char_set;
	int i, j, t, cnt;
	int len;
	int fDef;

	hdc = GetDC(hWnd);
#ifndef _WIN32_WCE
	char_set = GetTextCharset(hdc);
#else
	char_set = STR_DEFAULT_FONTCHARSET;
#endif
	ReleaseDC(hWnd, hdc);

	str_join_t(app_path, AppDir, KEY_NAME TEXT(".ini"), (TCHAR *)-1);
	if (profile_initialize(app_path, TRUE) == FALSE) {
		return FALSE;
	}

	len = profile_get_string(GENERAL, TEXT("DataFileDir"), TEXT(""), op.DataFileDir, BUF_SIZE - 1, app_path);
	if (*op.DataFileDir == TEXT('\0')) {
		DataDir = AppDir;
	} else {
		DataDir = op.DataFileDir;
		for (p = r = DataDir; *p != TEXT('\0'); p++) {
#ifndef UNICODE
			if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
				p++;
				continue;
			}
#endif
			if (*p == TEXT('\\') || *p == TEXT('/')) {
				r = p;
			}
		}
		if (r != (DataDir + lstrlen(DataDir) - 1) || lstrlen(DataDir) == 1) {
			lstrcat(DataDir, TEXT("\\"));
		}
	}

	op.SocLog = profile_get_int(GENERAL, TEXT("SocLog"), 0, app_path);

	op.view_font.name = profile_alloc_string(GENERAL, TEXT("FontName"), STR_DEFAULT_FONT, app_path);
	op.view_font.size = profile_get_int(GENERAL, TEXT("FontSize"), 9, app_path);
	op.view_font.weight = profile_get_int(GENERAL, TEXT("FontWeight"), 0, app_path);
	op.view_font.italic = profile_get_int(GENERAL, TEXT("FontItalic"), 0, app_path);
	op.view_font.charset = profile_get_int(GENERAL, TEXT("FontCharset"), char_set, app_path);

	op.lv_font.name = profile_alloc_string(GENERAL, TEXT("LvFontName"), TEXT(""), app_path);
	op.lv_font.size = profile_get_int(GENERAL, TEXT("LvFontSize"), 9, app_path);
	op.lv_font.weight = profile_get_int(GENERAL, TEXT("LvFontWeight"), 0, app_path);
	op.lv_font.italic = profile_get_int(GENERAL, TEXT("LvFontItalic"), 0, app_path);
	op.lv_font.charset = profile_get_int(GENERAL, TEXT("LvFontCharset"), char_set, app_path);

	op.HeadCharset = profile_alloc_string(GENERAL, TEXT("HeadCharset"), STR_DEFAULT_HEAD_CHARSET, app_path);
	op.HeadEncoding = profile_get_int(GENERAL, TEXT("HeadEncoding"), STR_DEFAULT_HEAD_ENCODE, app_path);
	op.BodyCharset = profile_alloc_string(GENERAL, TEXT("BodyCharset"), STR_DEFAULT_BODY_CHARSET, app_path);
	op.BodyEncoding = profile_get_int(GENERAL, TEXT("BodyEncoding"), STR_DEFAULT_BODY_ENCODE, app_path);
	if (op.HeadCharset == NULL || *op.HeadCharset == TEXT('\0') ||
		op.BodyCharset == NULL || *op.BodyCharset == TEXT('\0')) {
		ini_get_encode_info();
	}

	op.TimeZone = profile_alloc_string(GENERAL, TEXT("TimeZone"), TEXT(""), app_path);
	op.DateFormat = profile_alloc_string(GENERAL, TEXT("DateFormat"), STR_DEFAULT_DATEFORMAT, app_path);
	op.TimeFormat = profile_alloc_string(GENERAL, TEXT("TimeFormat"), STR_DEFAULT_TIMEFORMAT, app_path);

#ifndef _WIN32_WCE
	op.MainRect.left = profile_get_int(GENERAL, TEXT("left"), 0, app_path);
	op.MainRect.top = profile_get_int(GENERAL, TEXT("top"), 0, app_path);
	op.MainRect.right = profile_get_int(GENERAL, TEXT("right"), 440, app_path);
	op.MainRect.bottom = profile_get_int(GENERAL, TEXT("bottom"), 320, app_path);
#endif

	op.ShowTrayIcon = profile_get_int(GENERAL, TEXT("ShowTrayIcon"), 1, app_path);
	op.StartHide = profile_get_int(GENERAL, TEXT("StartHide"), 0, app_path);
	op.MinsizeHide = profile_get_int(GENERAL, TEXT("MinsizeHide"), 0, app_path);
	op.CloseHide = profile_get_int(GENERAL, TEXT("CloseHide"), 0, app_path);
	op.TrayIconToggle = profile_get_int(GENERAL, TEXT("TrayIconToggle"), 0, app_path);
	op.StartInit = profile_get_int(GENERAL, TEXT("StartInit"), 0, app_path);

	op.LvDefSelectPos = profile_get_int(GENERAL, TEXT("LvDefSelectPos"), 1, app_path);
	op.LvAutoSort = profile_get_int(GENERAL, TEXT("LvAutoSort"), 1, app_path);
	op.LvSortItem = profile_get_int(GENERAL, TEXT("LvSortItem"), 3, app_path);
	op.LvThreadView = profile_get_int(GENERAL, TEXT("LvThreadView"), 0, app_path);
	op.LvStyle = profile_get_int(GENERAL, TEXT("LvStyle"), LVS_SHOWSELALWAYS | LVS_REPORT, app_path);
	op.LvStyleEx = profile_get_int(GENERAL, TEXT("LvStyleEx"), LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP, app_path);
	op.MoveAllMailBox = profile_get_int(GENERAL, TEXT("MoveAllMailBox"), 1, app_path);
	op.RecvScroll = profile_get_int(GENERAL, TEXT("RecvScroll"), 1, app_path);
	op.SaveMsg = profile_get_int(GENERAL, TEXT("SaveMsg"), 1, app_path);
	op.AutoSave = profile_get_int(GENERAL, TEXT("AutoSave"), 1, app_path);

	op.StertPass = profile_get_int(GENERAL, TEXT("StertPass"), 0, app_path);
	op.ShowPass = profile_get_int(GENERAL, TEXT("ShowPass"), 0, app_path);
	profile_get_string(GENERAL, TEXT("pw"), TEXT(""), ret, BUF_SIZE - 1, app_path);
	EncodePassword(TEXT("_pw_"), ret, tmp, BUF_SIZE - 1, TRUE);
	op.Password = alloc_copy_t(tmp);

	op.LvColSize[0] = profile_get_int(GENERAL, TEXT("LvColSize-0"), 150, app_path);
	op.LvColSize[1] = profile_get_int(GENERAL, TEXT("LvColSize-1"), 100, app_path);
	op.LvColSize[2] = profile_get_int(GENERAL, TEXT("LvColSize-2"), 110, app_path);
	op.LvColSize[3] = profile_get_int(GENERAL, TEXT("LvColSize-3"), 50, app_path);

#ifdef _WIN32_WCE
	op.AddColSize[0] = profile_get_int(GENERAL, TEXT("AddColSize-0"), 100, app_path);
	op.AddColSize[1] = profile_get_int(GENERAL, TEXT("AddColSize-1"), 100, app_path);
#else
	op.AddColSize[0] = profile_get_int(GENERAL, TEXT("AddColSize-0"), 250, app_path);
	op.AddColSize[1] = profile_get_int(GENERAL, TEXT("AddColSize-1"), 190, app_path);
#endif

#ifndef _WIN32_WCE
	op.ViewRect.left = profile_get_int(GENERAL, TEXT("viewleft"), 0, app_path);
	op.ViewRect.top = profile_get_int(GENERAL, TEXT("viewtop"), 0, app_path);
	op.ViewRect.right = profile_get_int(GENERAL, TEXT("viewright"), 450, app_path);
	op.ViewRect.bottom = profile_get_int(GENERAL, TEXT("viewbottom"), 400, app_path);

	op.EditRect.left = profile_get_int(GENERAL, TEXT("editleft"), 0, app_path);
	op.EditRect.top = profile_get_int(GENERAL, TEXT("edittop"), 0, app_path);
	op.EditRect.right = profile_get_int(GENERAL, TEXT("editright"), 450, app_path);
	op.EditRect.bottom = profile_get_int(GENERAL, TEXT("editbottom"), 400, app_path);
#endif

	op.ShowHeader = profile_get_int(GENERAL, TEXT("ShowHeader"), 0, app_path);
	op.ListGetLine = profile_get_int(GENERAL, TEXT("ListGetLine"), 100, app_path);
	op.ListDownload = profile_get_int(GENERAL, TEXT("ListDownload"), 0, app_path);
	op.ListSaveMode = profile_get_int(GENERAL, TEXT("ListSaveMode"), 2, app_path);
	op.WordBreakFlag = profile_get_int(GENERAL, TEXT("WordBreakFlag"), 1, app_path);
	op.EditWordBreakFlag = profile_get_int(GENERAL, TEXT("EditWordBreakFlag"), 1, app_path);
	op.ViewShowDate = profile_get_int(GENERAL, TEXT("ViewShowDate"), 0, app_path);
	op.MstchCase = profile_get_int(GENERAL, TEXT("MstchCase"), 0, app_path);
	op.AllFind = profile_get_int(GENERAL, TEXT("AllFind"), 1, app_path);
	op.SubjectFind = profile_get_int(GENERAL, TEXT("SubjectFind"), 0, app_path);

	op.ESMTP = profile_get_int(GENERAL, TEXT("ESMTP"), 0, app_path);
	op.SendHelo = profile_alloc_string(GENERAL, TEXT("SendHelo"), TEXT(""), app_path);
	op.SendMessageId = profile_get_int(GENERAL, TEXT("SendMessageId"), 1, app_path);
	op.SendDate = profile_get_int(GENERAL, TEXT("SendDate"), 1, app_path);
	op.SelectSendBox = profile_get_int(GENERAL, TEXT("SelectSendBox"), 1, app_path);
	op.PopBeforeSmtpIsLoginOnly = profile_get_int(GENERAL, TEXT("PopBeforeSmtpIsLoginOnly"), 1, app_path);
	op.PopBeforeSmtpWait = profile_get_int(GENERAL, TEXT("PopBeforeSmtpWait"), 300, app_path);

	op.AutoQuotation = profile_get_int(GENERAL, TEXT("AutoQuotation"), 1, app_path);
	op.QuotationChar = profile_alloc_string(GENERAL, TEXT("QuotationChar"), TEXT(">"), app_path);
	op.WordBreakSize = profile_get_int(GENERAL, TEXT("WordBreakSize"), 70, app_path);
	op.QuotationBreak = profile_get_int(GENERAL, TEXT("QuotationBreak"), 1, app_path);
	op.ReSubject = profile_alloc_string(GENERAL, TEXT("ReSubject"), TEXT("Re: "), app_path);
	len = profile_get_string(GENERAL, TEXT("ReHeader"), TEXT("\\n%f wrote:\\n(%d)\\n"), conv_buf, INI_BUF_SIZE - 1, app_path);
	op.ReHeader = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
	if (op.ReHeader != NULL) {
		DecodeCtrlChar(conv_buf, op.ReHeader);
	}

	op.Bura = profile_alloc_string(GENERAL, TEXT("Bura"), STR_DEFAULT_BURA, app_path);
	op.Oida = profile_alloc_string(GENERAL, TEXT("Oida"), STR_DEFAULT_OIDA, app_path);
	op.sBura = profile_alloc_string(GENERAL, TEXT("sBura"), TEXT(",.?!%:;)]}?"), app_path);
	op.sOida = profile_alloc_string(GENERAL, TEXT("sOida"), TEXT("\\$([{?" ), app_path);

	op.CAFile = profile_alloc_string(GENERAL, TEXT("CAFile"), TEXT("ca.pem"), app_path);
	op.IPCache = profile_get_int(GENERAL, TEXT("IPCache"), 1, app_path);
	op.EncodeType = profile_get_int(GENERAL, TEXT("EncodeType"), 0, app_path);

	op.ShowNewMailMessgae = profile_get_int(GENERAL, TEXT("ShowNewMailMessgae"), 1, app_path);
	op.ShowNoMailMessage = profile_get_int(GENERAL, TEXT("ShowNoMailMessage"), 0, app_path);
#ifdef _WIN32_WCE
	op.ActiveNewMailMessgae = profile_get_int(GENERAL, TEXT("ActiveNewMailMessgae"), 1, app_path);
#else
	op.ActiveNewMailMessgae = profile_get_int(GENERAL, TEXT("ActiveNewMailMessgae"), 0, app_path);
#endif

	op.NewMailSound = profile_get_int(GENERAL, TEXT("NewMailSound"), 1, app_path);
	op.NewMailSoundFile = profile_alloc_string(GENERAL, TEXT("NewMailSoundFile"), TEXT(""), app_path);
	op.ExecEndSound = profile_get_int(GENERAL, TEXT("ExecEndSound"), 0, app_path);
	op.ExecEndSoundFile = profile_alloc_string(GENERAL, TEXT("ExecEndSoundFile"), TEXT(""), app_path);

	op.AutoCheck = profile_get_int(GENERAL, TEXT("AutoCheck"), 0, app_path);
	op.AutoCheckTime = profile_get_int(GENERAL, TEXT("AutoCheckTime"), 10, app_path);
	op.StartCheck = profile_get_int(GENERAL, TEXT("StartCheck"), 0, app_path);
	op.CheckAfterUpdate = profile_get_int(GENERAL, TEXT("CheckAfterUpdate"), 0, app_path);
	op.SocIgnoreError = profile_get_int(GENERAL, TEXT("SocIgnoreError"), 0, app_path);
	op.SendIgnoreError = profile_get_int(GENERAL, TEXT("SendIgnoreError"), 0, app_path);
	op.CheckEndExec = profile_get_int(GENERAL, TEXT("CheckEndExec"), 0, app_path);
	op.CheckEndExecNoDelMsg = profile_get_int(GENERAL, TEXT("CheckEndExecNoDelMsg"), 1, app_path);
	op.TimeoutInterval = profile_get_int(GENERAL, TEXT("TimeoutInterval"), 3, app_path);
	if (op.TimeoutInterval <= 0) op.TimeoutInterval = 1;

	op.ViewClose = profile_get_int(GENERAL, TEXT("ViewClose"), 1, app_path);
	op.ViewApp = profile_alloc_string(GENERAL, TEXT("ViewApp"), TEXT(""), app_path);
	op.ViewAppCmdLine = profile_alloc_string(GENERAL, TEXT("ViewAppCmdLine"), TEXT(""), app_path);
	op.ViewFileSuffix = profile_alloc_string(GENERAL, TEXT("ViewFileSuffix"), TEXT("txt"), app_path);
	len = profile_get_string(GENERAL, TEXT("ViewFileHeader"),
		TEXT("From: %f\\nTo: %t\\nCc: %c\\nSubject: %s\\nDate: %d\\n\\n"), conv_buf, INI_BUF_SIZE - 1, app_path);
	op.ViewFileHeader = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
	if (op.ViewFileHeader != NULL) {
		DecodeCtrlChar(conv_buf, op.ViewFileHeader);
	}
	op.ViewAppClose = profile_get_int(GENERAL, TEXT("ViewAppClose"), 0, app_path);
	op.DefViewApp = profile_get_int(GENERAL, TEXT("DefViewApp"), 0, app_path);
	op.EditApp = profile_alloc_string(GENERAL, TEXT("EditApp"), TEXT(""), app_path);
	op.EditAppCmdLine = profile_alloc_string(GENERAL, TEXT("EditAppCmdLine"), TEXT(""), app_path);
	op.EditFileSuffix = profile_alloc_string(GENERAL, TEXT("EditFileSuffix"), TEXT("txt"), app_path);
	op.DefEditApp = profile_get_int(GENERAL, TEXT("DefEditApp"), 0, app_path);
	op.AttachPath = profile_alloc_string(GENERAL, TEXT("AttachPath"), TEXT("attach"), app_path);
	op.AttachWarning = profile_get_int(GENERAL, TEXT("AttachWarning"), 1, app_path);
	op.AttachDelete = profile_get_int(GENERAL, TEXT("AttachDelete"), 1, app_path);

#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_LAGENDA
	op.URLApp = profile_alloc_string(GENERAL, TEXT("URLApp"), TEXT("internet.exe"), app_path);
#else	// _WIN32_WCE_LAGENDA
	op.URLApp = profile_alloc_string(GENERAL, TEXT("URLApp"), TEXT("iexplore.exe"), app_path);
#endif	// _WIN32_WCE_LAGENDA
#else	// _WIN32_WCE
	op.URLApp = profile_alloc_string(GENERAL, TEXT("URLApp"), TEXT(""), app_path);
#endif	// _WIN32_WCE

	op.EnableLAN = profile_get_int(GENERAL, TEXT("EnableLAN"), 0, app_path);

	op.RasCon = profile_get_int(GENERAL, TEXT("RasCon"), 1, app_path);
	op.RasCheckEndDisCon = profile_get_int(GENERAL, TEXT("RasCheckEndDisCon"), 1, app_path);
	op.RasEndDisCon = profile_get_int(GENERAL, TEXT("RasEndDisCon"), 1, app_path);
	op.RasNoCheck = profile_get_int(GENERAL, TEXT("RasNoCheck"), 1, app_path);
	op.RasWaitSec = profile_get_int(GENERAL, TEXT("RasWaitSec"), 5, app_path);

	op.RasInfoCnt = profile_get_int(GENERAL, TEXT("RasInfoCnt"), 0, app_path);
	op.RasInfo = (RASINFO **)mem_calloc(sizeof(RASINFO *) * op.RasInfoCnt);
	if (op.RasInfo == NULL) {
		op.RasInfoCnt = 0;
	}
	for (j = 0; j < op.RasInfoCnt; j++) {
		*(op.RasInfo + j) = (RASINFO *)mem_calloc(sizeof(RASINFO));
		if (*(op.RasInfo + j) == NULL) {
			continue;
		}
		wsprintf(key_buf, TEXT("RASINFO-%d_%s"), j, TEXT("RasEntry"));
		(*(op.RasInfo + j))->RasEntry = profile_alloc_string(TEXT("RASINFO"), key_buf, TEXT(""), app_path);

		wsprintf(key_buf, TEXT("RASINFO-%d_%s"), j, TEXT("RasUser"));
		(*(op.RasInfo + j))->RasUser = profile_alloc_string(TEXT("RASINFO"), key_buf, TEXT(""), app_path);

		wsprintf(key_buf, TEXT("RASINFO-%d_%s"), j, TEXT("RasPass"));
		len = profile_get_string(TEXT("RASINFO"), key_buf, TEXT(""), ret, BUF_SIZE - 1, app_path);
		EncodePassword((*(op.RasInfo + j))->RasUser, ret, tmp, BUF_SIZE - 1, TRUE);
		(*(op.RasInfo + j))->RasPass = alloc_copy_t(tmp);
	}

	i = profile_get_int(GENERAL, TEXT("MailBoxCnt"), 0, app_path);
	if (i == 0) {
		mailbox_create(hWnd, FALSE);
		first_start = TRUE;
		profile_free();
		return TRUE;
	}
	for (j = 0; j < i; j++) {
		if ((cnt = mailbox_create(hWnd, FALSE)) == -1) {
			continue;
		}
		wsprintf(buf, TEXT("MAILBOX-%d"), j);

		// Name
		(MailBox + cnt)->Name = profile_alloc_string(buf, TEXT("Name"), TEXT(""), app_path);
		// Server
		(MailBox + cnt)->Server = profile_alloc_string(buf, TEXT("Server"), TEXT(""), app_path);
		// Port
		(MailBox + cnt)->Port = profile_get_int(buf, TEXT("Port"), 110, app_path);
		// User
		(MailBox + cnt)->User = profile_alloc_string(buf, TEXT("User"), TEXT(""), app_path);
		// Pass
		profile_get_string(buf, TEXT("Pass"), TEXT(""), ret, BUF_SIZE - 1, app_path);
		EncodePassword((MailBox + cnt)->User, ret, tmp, BUF_SIZE - 1, TRUE);
		(MailBox + cnt)->Pass = alloc_copy_t(tmp);
		// APOP
		(MailBox + cnt)->APOP = profile_get_int(buf, TEXT("APOP"), 0, app_path);
		// POP SSL
		(MailBox + cnt)->PopSSL = profile_get_int(buf, TEXT("PopSSL"), 0, app_path);
		// POP SSL Option
		(MailBox + cnt)->PopSSLInfo.Type = profile_get_int(buf, TEXT("PopSSLType"), 0, app_path);
		(MailBox + cnt)->PopSSLInfo.Verify = profile_get_int(buf, TEXT("PopSSLVerify"), 1, app_path);
		(MailBox + cnt)->PopSSLInfo.Depth = profile_get_int(buf, TEXT("PopSSLDepth"), -1, app_path);
		(MailBox + cnt)->PopSSLInfo.Cert = profile_alloc_string(buf, TEXT("PopSSLCert"), TEXT(""), app_path);
		(MailBox + cnt)->PopSSLInfo.Pkey = profile_alloc_string(buf, TEXT("PopSSLPkey"), TEXT(""), app_path);
		(MailBox + cnt)->PopSSLInfo.Pass = profile_alloc_string(buf, TEXT("PopSSLPass"), TEXT(""), app_path);
		// Disable RETR
		(MailBox + cnt)->no_retr = profile_get_int(buf, TEXT("NoRETR"), 0, app_path);
		// Disable UIDL
		(MailBox + cnt)->NoUIDL = profile_get_int(buf, TEXT("NoUIDL"), 0, app_path);

		// MailCnt
		(MailBox + cnt)->mail_num = profile_get_int(buf, TEXT("MailCnt"), 0, app_path);
		// MailSize
		(MailBox + cnt)->MailSize = profile_get_int(buf, TEXT("MailSize"), 0, app_path);

		if (op.StartInit == 0) {
			// LastMessageId
			profile_get_string(buf, TEXT("LastMessageId"), TEXT(""), ret, BUF_SIZE - 1, app_path);
			(MailBox + cnt)->LastMessageId = alloc_tchar_to_char(ret);
			// LastNo
			(MailBox + cnt)->LastNo = profile_get_int(buf, TEXT("LastNo"), 0, app_path);
		} else {
			// 起動時に新着位置の初期化
			(MailBox + cnt)->LastNo = -1;
		}

		// CyclicFlag
		(MailBox + cnt)->CyclicFlag = profile_get_int(buf, TEXT("CyclicFlag"), 0, app_path);

		// SmtpServer
		(MailBox + cnt)->SmtpServer = profile_alloc_string(buf, TEXT("SmtpServer"), TEXT(""), app_path);
		// SmtpPort
		(MailBox + cnt)->SmtpPort = profile_get_int(buf, TEXT("SmtpPort"), 25, app_path);
		// UserName
		(MailBox + cnt)->UserName = profile_alloc_string(buf, TEXT("UserName"), TEXT(""), app_path);
		// MailAddress
		(MailBox + cnt)->MailAddress = profile_alloc_string(buf, TEXT("MailAddress"), TEXT(""), app_path);
		// Signature
		p = (TCHAR *)mem_alloc(sizeof(TCHAR) * MAXSIZE);
		if (p != NULL) {
			len = profile_get_string(buf, TEXT("Signature"), TEXT(""), p, MAXSIZE - 1, app_path);
			(MailBox + cnt)->Signature = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
			if ((MailBox + cnt)->Signature != NULL) {
				DecodeCtrlChar(p, (MailBox + cnt)->Signature);
			}
			mem_free(&p);
		}
		// ReplyTo
		(MailBox + cnt)->ReplyTo = profile_alloc_string(buf, TEXT("ReplyTo"), TEXT(""), app_path);
		// MyAddr2Bcc
		(MailBox + cnt)->MyAddr2Bcc = profile_get_int(buf, TEXT("MyAddr2Bcc"), 0, app_path);
		// BccAddr
		(MailBox + cnt)->BccAddr = profile_alloc_string(buf, TEXT("BccAddr"), TEXT(""), app_path);

		// POP before SMTP
		(MailBox + cnt)->PopBeforeSmtp = profile_get_int(buf, TEXT("PopBeforeSmtp"), 0, app_path);
		// SMTP Authentication
		(MailBox + cnt)->SmtpAuth = profile_get_int(buf, TEXT("SmtpAuth"), 0, app_path);
		// SMTP Authentication type
		(MailBox + cnt)->SmtpAuthType = profile_get_int(buf, TEXT("SmtpAuthType"), 0, app_path);
		// SMTP Authentication User & Pass mode
		(MailBox + cnt)->AuthUserPass = profile_get_int(buf, TEXT("AuthUserPass"), 0, app_path);
		// SMTP Authentication User
		(MailBox + cnt)->SmtpUser = profile_alloc_string(buf, TEXT("SmtpUser"), TEXT(""), app_path);
		// SMTP Authentication Pass
		profile_get_string(buf, TEXT("SmtpPass"), TEXT(""), ret, BUF_SIZE - 1, app_path);
		EncodePassword((MailBox + cnt)->SmtpUser, ret, tmp, BUF_SIZE - 1, TRUE);
		(MailBox + cnt)->SmtpPass = alloc_copy_t(tmp);
		// SMTP SSL
		(MailBox + cnt)->SmtpSSL = profile_get_int(buf, TEXT("SmtpSSL"), 0, app_path);
		// SMTP SSL Option
		(MailBox + cnt)->SmtpSSLInfo.Type = profile_get_int(buf, TEXT("SmtpSSLType"), 0, app_path);
		(MailBox + cnt)->SmtpSSLInfo.Verify = profile_get_int(buf, TEXT("SmtpSSLVerify"), 1, app_path);
		(MailBox + cnt)->SmtpSSLInfo.Depth = profile_get_int(buf, TEXT("SmtpSSLDepth"), -1, app_path);
		(MailBox + cnt)->SmtpSSLInfo.Cert = profile_alloc_string(buf, TEXT("SmtpSSLCert"), TEXT(""), app_path);
		(MailBox + cnt)->SmtpSSLInfo.Pkey = profile_alloc_string(buf, TEXT("SmtpSSLPkey"), TEXT(""), app_path);
		(MailBox + cnt)->SmtpSSLInfo.Pass = profile_alloc_string(buf, TEXT("SmtpSSLPass"), TEXT(""), app_path);

		// Filter
		(MailBox + cnt)->FilterEnable = profile_get_int(buf, TEXT("FilterEnable"), 0, app_path);
		(MailBox + cnt)->FilterCnt = profile_get_int(buf, TEXT("FilterCnt"), 0, app_path);
		fDef = profile_get_int(buf, TEXT("FilterFlag"), -1, app_path);

		(MailBox + cnt)->tpFilter = (FILTER **)mem_calloc(sizeof(FILTER *) * (MailBox + cnt)->FilterCnt);
		if ((MailBox + cnt)->tpFilter == NULL) {
			(MailBox + cnt)->FilterCnt = 0;
		}
		for (t = 0; t < (MailBox + cnt)->FilterCnt; t++) {
			tpFilter = *((MailBox + cnt)->tpFilter + t) = (FILTER *)mem_calloc(sizeof(FILTER));
			if (tpFilter == NULL) {
				continue;
			}

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Enable"));
			tpFilter->Enable = profile_get_int(buf, key_buf, 0, app_path);

			if (fDef == -1) {
				wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Action"));
				tpFilter->Action = profile_get_int(buf, key_buf, 0, app_path);
			} else {
				tpFilter->Action = fDef;
			}

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Header1"));
			tpFilter->Header1 = profile_alloc_string(buf, key_buf, TEXT(""), app_path);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Content1"));
			tpFilter->Content1 = profile_alloc_string(buf, key_buf, TEXT(""), app_path);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Header2"));
			tpFilter->Header2 = profile_alloc_string(buf, key_buf, TEXT(""), app_path);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Content2"));
			tpFilter->Content2 = profile_alloc_string(buf, key_buf, TEXT(""), app_path);
		}

		// RAS
		(MailBox + cnt)->RasMode = profile_get_int(buf, TEXT("RasMode"), 0, app_path);
		(MailBox + cnt)->RasEntry = profile_alloc_string(buf, TEXT("RasEntry"), TEXT(""), app_path);
		(MailBox + cnt)->RasReCon = profile_get_int(buf, TEXT("RasReCon"), 0, app_path);

		// メールアイテム
		wsprintf(buf, TEXT("MailBox%d.dat"), j);
		if (file_read_mailbox(buf, (MailBox + cnt)) == FALSE) {
			profile_free();
			return FALSE;
		}
	}
	profile_free();
	return TRUE;
}

#endif //_HAVE_WND

/*
 * ini_save_setting - INIファイルへ設定情報を書き出す
 */
BOOL ini_save_setting(HWND hWnd, BOOL SaveMailFlag)
{
	FILTER *tpFilter;
	TCHAR app_path[BUF_SIZE];
	TCHAR buf[BUF_SIZE];
	TCHAR key_buf[BUF_SIZE];
	TCHAR conv_buf[INI_BUF_SIZE];
	TCHAR tmp[BUF_SIZE];
#ifdef UNICODE
	TCHAR ret[BUF_SIZE];
#endif
	TCHAR *p;
	int j, t;
	BOOL rc = TRUE;

	str_join_t(app_path, AppDir, KEY_NAME TEXT(".ini"), (TCHAR *)-1);
	profile_initialize(app_path, TRUE);

	profile_write_string(GENERAL, TEXT("DataFileDir"), op.DataFileDir, app_path);
	profile_write_int(GENERAL, TEXT("SocLog"), op.SocLog, app_path);

	profile_write_string(GENERAL, TEXT("FontName"), op.view_font.name, app_path);
	profile_write_int(GENERAL, TEXT("FontSize"), op.view_font.size, app_path);
	profile_write_int(GENERAL, TEXT("FontWeight"), op.view_font.weight, app_path);
	profile_write_int(GENERAL, TEXT("FontItalic"), op.view_font.italic, app_path);
	profile_write_int(GENERAL, TEXT("FontCharset"), op.view_font.charset, app_path);
	
	profile_write_string(GENERAL, TEXT("LvFontName"), op.lv_font.name, app_path);
	profile_write_int(GENERAL, TEXT("LvFontSize"), op.lv_font.size, app_path);
	profile_write_int(GENERAL, TEXT("LvFontWeight"), op.lv_font.weight, app_path);
	profile_write_int(GENERAL, TEXT("LvFontItalic"), op.lv_font.italic, app_path);
	profile_write_int(GENERAL, TEXT("LvFontCharset"), op.lv_font.charset, app_path);

	profile_write_string(GENERAL, TEXT("HeadCharset"), op.HeadCharset, app_path);
	profile_write_int(GENERAL, TEXT("HeadEncoding"), op.HeadEncoding, app_path);
	profile_write_string(GENERAL, TEXT("BodyCharset"), op.BodyCharset, app_path);
	profile_write_int(GENERAL, TEXT("BodyEncoding"), op.BodyEncoding, app_path);
	profile_write_string(GENERAL, TEXT("TimeZone"), op.TimeZone, app_path);
	profile_write_string(GENERAL, TEXT("DateFormat"), op.DateFormat, app_path);
	profile_write_string(GENERAL, TEXT("TimeFormat"), op.TimeFormat, app_path);

#ifndef _WIN32_WCE
	profile_write_int(GENERAL, TEXT("left"), op.MainRect.left, app_path);
	profile_write_int(GENERAL, TEXT("top"), op.MainRect.top, app_path);
	profile_write_int(GENERAL, TEXT("right"), op.MainRect.right, app_path);
	profile_write_int(GENERAL, TEXT("bottom"), op.MainRect.bottom, app_path);
#endif

	profile_write_int(GENERAL, TEXT("ShowTrayIcon"), op.ShowTrayIcon, app_path);
	profile_write_int(GENERAL, TEXT("StartHide"), op.StartHide, app_path);
	profile_write_int(GENERAL, TEXT("MinsizeHide"), op.MinsizeHide, app_path);
	profile_write_int(GENERAL, TEXT("CloseHide"), op.CloseHide, app_path);
	profile_write_int(GENERAL, TEXT("TrayIconToggle"), op.TrayIconToggle, app_path);
	profile_write_int(GENERAL, TEXT("StartInit"), op.StartInit, app_path);

	profile_write_int(GENERAL, TEXT("LvDefSelectPos"), op.LvDefSelectPos, app_path);
	profile_write_int(GENERAL, TEXT("LvAutoSort"), op.LvAutoSort, app_path);
	profile_write_int(GENERAL, TEXT("LvSortItem"), op.LvSortItem, app_path);
	profile_write_int(GENERAL, TEXT("LvThreadView"), op.LvThreadView, app_path);
	profile_write_int(GENERAL, TEXT("LvStyle"), op.LvStyle, app_path);
	profile_write_int(GENERAL, TEXT("LvStyleEx"), op.LvStyleEx, app_path);
	profile_write_int(GENERAL, TEXT("MoveAllMailBox"), op.MoveAllMailBox, app_path);
	profile_write_int(GENERAL, TEXT("RecvScroll"), op.RecvScroll, app_path);
	profile_write_int(GENERAL, TEXT("SaveMsg"), op.SaveMsg, app_path);
	profile_write_int(GENERAL, TEXT("AutoSave"), op.AutoSave, app_path);
	profile_write_int(GENERAL, TEXT("StertPass"), op.StertPass, app_path);
	profile_write_int(GENERAL, TEXT("ShowPass"), op.ShowPass, app_path);
	EncodePassword(TEXT("_pw_"), op.Password, tmp, BUF_SIZE - 1, FALSE);
	profile_write_string(GENERAL, TEXT("pw"), tmp, app_path);

	profile_write_int(GENERAL, TEXT("LvColSize-0"), op.LvColSize[0], app_path);
	profile_write_int(GENERAL, TEXT("LvColSize-1"), op.LvColSize[1], app_path);
	profile_write_int(GENERAL, TEXT("LvColSize-2"), op.LvColSize[2], app_path);
	profile_write_int(GENERAL, TEXT("LvColSize-3"), op.LvColSize[3], app_path);

	profile_write_int(GENERAL, TEXT("AddColSize-0"), op.AddColSize[0], app_path);
	profile_write_int(GENERAL, TEXT("AddColSize-1"), op.AddColSize[1], app_path);

#ifndef _WIN32_WCE
	profile_write_int(GENERAL, TEXT("viewleft"), op.ViewRect.left, app_path);
	profile_write_int(GENERAL, TEXT("viewtop"), op.ViewRect.top, app_path);
	profile_write_int(GENERAL, TEXT("viewright"), op.ViewRect.right, app_path);
	profile_write_int(GENERAL, TEXT("viewbottom"), op.ViewRect.bottom, app_path);

	profile_write_int(GENERAL, TEXT("editleft"), op.EditRect.left, app_path);
	profile_write_int(GENERAL, TEXT("edittop"), op.EditRect.top, app_path);
	profile_write_int(GENERAL, TEXT("editright"), op.EditRect.right, app_path);
	profile_write_int(GENERAL, TEXT("editbottom"), op.EditRect.bottom, app_path);
#endif

	profile_write_int(GENERAL, TEXT("ShowHeader"), op.ShowHeader, app_path);
	profile_write_int(GENERAL, TEXT("ListGetLine"), op.ListGetLine, app_path);
	profile_write_int(GENERAL, TEXT("ListDownload"), op.ListDownload, app_path);
	profile_write_int(GENERAL, TEXT("ListSaveMode"), op.ListSaveMode, app_path);
	profile_write_int(GENERAL, TEXT("WordBreakFlag"), op.WordBreakFlag, app_path);
	profile_write_int(GENERAL, TEXT("EditWordBreakFlag"), op.EditWordBreakFlag, app_path);
	profile_write_int(GENERAL, TEXT("ViewShowDate"), op.ViewShowDate, app_path);
	profile_write_int(GENERAL, TEXT("MstchCase"), op.MstchCase, app_path);
	profile_write_int(GENERAL, TEXT("AllFind"), op.AllFind, app_path);
	profile_write_int(GENERAL, TEXT("SubjectFind"), op.SubjectFind, app_path);

	profile_write_int(GENERAL, TEXT("ESMTP"), op.ESMTP, app_path);
	profile_write_string(GENERAL, TEXT("SendHelo"), op.SendHelo, app_path);
	profile_write_int(GENERAL, TEXT("SendMessageId"), op.SendMessageId, app_path);
	profile_write_int(GENERAL, TEXT("SendDate"), op.SendDate, app_path);
	profile_write_int(GENERAL, TEXT("SelectSendBox"), op.SelectSendBox, app_path);
	profile_write_int(GENERAL, TEXT("PopBeforeSmtpIsLoginOnly"), op.PopBeforeSmtpIsLoginOnly, app_path);
	profile_write_int(GENERAL, TEXT("PopBeforeSmtpWait"), op.PopBeforeSmtpWait, app_path);

	profile_write_int(GENERAL, TEXT("AutoQuotation"), op.AutoQuotation, app_path);
	profile_write_string(GENERAL, TEXT("QuotationChar"), op.QuotationChar, app_path);
	profile_write_int(GENERAL, TEXT("WordBreakSize"), op.WordBreakSize, app_path);
	profile_write_int(GENERAL, TEXT("QuotationBreak"), op.QuotationBreak, app_path);
	profile_write_string(GENERAL, TEXT("ReSubject"), op.ReSubject, app_path);
	EncodeCtrlChar(op.ReHeader, conv_buf);
	profile_write_string(GENERAL, TEXT("ReHeader"), conv_buf, app_path);

	profile_write_string(GENERAL, TEXT("Bura"), op.Bura, app_path);
	profile_write_string(GENERAL, TEXT("Oida"), op.Oida, app_path);

	profile_write_string(GENERAL, TEXT("sBura"), op.sBura, app_path);
	profile_write_string(GENERAL, TEXT("sOida"), op.sOida, app_path);

	profile_write_string(GENERAL, TEXT("CAFile"), op.CAFile, app_path);

	profile_write_int(GENERAL, TEXT("IPCache"), op.IPCache, app_path);
	profile_write_int(GENERAL, TEXT("EncodeType"), op.EncodeType, app_path);

	profile_write_int(GENERAL, TEXT("ShowNewMailMessgae"), op.ShowNewMailMessgae, app_path);
	profile_write_int(GENERAL, TEXT("ShowNoMailMessage"), op.ShowNoMailMessage, app_path);
	profile_write_int(GENERAL, TEXT("ActiveNewMailMessgae"), op.ActiveNewMailMessgae, app_path);

	profile_write_int(GENERAL, TEXT("NewMailSound"), op.NewMailSound, app_path);
	profile_write_string(GENERAL, TEXT("NewMailSoundFile"), op.NewMailSoundFile, app_path);
	profile_write_int(GENERAL, TEXT("ExecEndSound"), op.ExecEndSound, app_path);
	profile_write_string(GENERAL, TEXT("ExecEndSoundFile"), op.ExecEndSoundFile, app_path);

	profile_write_int(GENERAL, TEXT("AutoCheck"), op.AutoCheck, app_path);
	profile_write_int(GENERAL, TEXT("AutoCheckTime"), op.AutoCheckTime, app_path);
	profile_write_int(GENERAL, TEXT("StartCheck"), op.StartCheck, app_path);
	profile_write_int(GENERAL, TEXT("CheckAfterUpdate"), op.CheckAfterUpdate, app_path);
	profile_write_int(GENERAL, TEXT("SocIgnoreError"), op.SocIgnoreError, app_path);
	profile_write_int(GENERAL, TEXT("SendIgnoreError"), op.SendIgnoreError, app_path);
	profile_write_int(GENERAL, TEXT("CheckEndExec"), op.CheckEndExec, app_path);
	profile_write_int(GENERAL, TEXT("CheckEndExecNoDelMsg"), op.CheckEndExecNoDelMsg, app_path);
	profile_write_int(GENERAL, TEXT("TimeoutInterval"), op.TimeoutInterval, app_path);

	profile_write_int(GENERAL, TEXT("ViewClose"), op.ViewClose, app_path);
	profile_write_string(GENERAL, TEXT("ViewApp"), op.ViewApp, app_path);
	profile_write_string(GENERAL, TEXT("ViewAppCmdLine"), op.ViewAppCmdLine, app_path);
	profile_write_string(GENERAL, TEXT("ViewFileSuffix"), op.ViewFileSuffix, app_path);
	EncodeCtrlChar(op.ViewFileHeader, conv_buf);
	profile_write_string(GENERAL, TEXT("ViewFileHeader"), conv_buf, app_path);
	profile_write_int(GENERAL, TEXT("ViewAppClose"), op.ViewAppClose, app_path);
	profile_write_int(GENERAL, TEXT("DefViewApp"), op.DefViewApp, app_path);
	profile_write_string(GENERAL, TEXT("EditApp"), op.EditApp, app_path);
	profile_write_string(GENERAL, TEXT("EditAppCmdLine"), op.EditAppCmdLine, app_path);
	profile_write_string(GENERAL, TEXT("EditFileSuffix"), op.EditFileSuffix, app_path);
	profile_write_int(GENERAL, TEXT("DefEditApp"), op.DefEditApp, app_path);
	profile_write_string(GENERAL, TEXT("AttachPath"), op.AttachPath, app_path);
	profile_write_int(GENERAL, TEXT("AttachWarning"), op.AttachWarning, app_path);
	profile_write_int(GENERAL, TEXT("AttachDelete"), op.AttachDelete, app_path);

	profile_write_string(GENERAL, TEXT("URLApp"), op.URLApp, app_path);

	profile_write_int(GENERAL, TEXT("EnableLAN"), op.EnableLAN, app_path);

	profile_write_int(GENERAL, TEXT("RasCon"), op.RasCon, app_path);
	profile_write_int(GENERAL, TEXT("RasCheckEndDisCon"), op.RasCheckEndDisCon, app_path);
	profile_write_int(GENERAL, TEXT("RasEndDisCon"), op.RasEndDisCon, app_path);
	profile_write_int(GENERAL, TEXT("RasNoCheck"), op.RasNoCheck, app_path);
	profile_write_int(GENERAL, TEXT("RasWaitSec"), op.RasWaitSec, app_path);

	for (t = 0, j = 0; j < op.RasInfoCnt; j++) {
		if (*(op.RasInfo + j) == NULL ||
			(*(op.RasInfo + j))->RasEntry == NULL || *(*(op.RasInfo + j))->RasEntry == TEXT('\0')) {
			continue;
		}
		wsprintf(key_buf, TEXT("RASINFO-%d_%s"), t, TEXT("RasEntry"));
		profile_write_string(TEXT("RASINFO"), key_buf, (*(op.RasInfo + j))->RasEntry, app_path);

		wsprintf(key_buf, TEXT("RASINFO-%d_%s"), t, TEXT("RasUser"));
		profile_write_string(TEXT("RASINFO"), key_buf, (*(op.RasInfo + j))->RasUser, app_path);

		wsprintf(key_buf, TEXT("RASINFO-%d_%s"), t, TEXT("RasPass"));
		EncodePassword((*(op.RasInfo + j))->RasUser, (*(op.RasInfo + j))->RasPass, tmp, BUF_SIZE - 1, FALSE);
		profile_write_string(TEXT("RASINFO"), key_buf, tmp, app_path);
		t++;
	}
	profile_write_int(GENERAL, TEXT("RasInfoCnt"), t, app_path);

	// メールボックスの設定の保存
	profile_write_int(GENERAL, TEXT("MailBoxCnt"), MailBoxCnt - MAILBOX_USER, app_path);

	for (j = MAILBOX_USER; j < MailBoxCnt; j++) {
		if ((MailBox + j) == NULL) {
			continue;
		}
		wsprintf(buf, TEXT("MAILBOX-%d"), j - MAILBOX_USER);

		// Name
		profile_write_string(buf, TEXT("Name"), (MailBox + j)->Name, app_path);
		// Server
		profile_write_string(buf, TEXT("Server"), (MailBox + j)->Server, app_path);
		// Port
		profile_write_int(buf, TEXT("Port"), (MailBox + j)->Port, app_path);
		// User
		profile_write_string(buf, TEXT("User"), (MailBox + j)->User, app_path);
		// Pass
		EncodePassword((MailBox + j)->User, (MailBox + j)->Pass, tmp, BUF_SIZE - 1, FALSE);
		profile_write_string(buf, TEXT("Pass"), tmp, app_path);
		// APOP
		profile_write_int(buf, TEXT("APOP"), (MailBox + j)->APOP, app_path);
		// POP SSL
		profile_write_int(buf, TEXT("PopSSL"), (MailBox + j)->PopSSL, app_path);
		// POP SSL Option
		profile_write_int(buf, TEXT("PopSSLType"), (MailBox + j)->PopSSLInfo.Type, app_path);
		profile_write_int(buf, TEXT("PopSSLVerify"), (MailBox + j)->PopSSLInfo.Verify, app_path);
		profile_write_int(buf, TEXT("PopSSLDepth"), (MailBox + j)->PopSSLInfo.Depth, app_path);
		profile_write_string(buf, TEXT("PopSSLCert"), (MailBox + j)->PopSSLInfo.Cert, app_path);
		profile_write_string(buf, TEXT("PopSSLPkey"), (MailBox + j)->PopSSLInfo.Pkey, app_path);
		profile_write_string(buf, TEXT("PopSSLPass"), (MailBox + j)->PopSSLInfo.Pass, app_path);
		// Disable RETR
		profile_write_int(buf, TEXT("NoRETR"), (MailBox + j)->no_retr, app_path);
		// Disable UIDL
		profile_write_int(buf, TEXT("NoUIDL"), (MailBox + j)->NoUIDL, app_path);

		// MailCnt
		profile_write_int(buf, TEXT("MailCnt"), (MailBox + j)->mail_num, app_path);
		// MailSize
		profile_write_int(buf, TEXT("MailSize"), (MailBox + j)->MailSize, app_path);

		if (SaveMailFlag == TRUE) {
			// LastMessageId
			if ((MailBox + j)->LastMessageId != NULL) {
#ifdef UNICODE
				char_to_tchar((MailBox + j)->LastMessageId, ret, BUF_SIZE - 1);
				*(ret + BUF_SIZE - 1) = TEXT('\0');
				profile_write_string(buf, TEXT("LastMessageId"), ret, app_path);
#else
				profile_write_string(buf, TEXT("LastMessageId"), (MailBox + j)->LastMessageId, app_path);
#endif
			}
			// LastNo
			profile_write_int(buf, TEXT("LastNo"), (MailBox + j)->LastNo, app_path);
		}

		// CyclicFlag
		profile_write_int(buf, TEXT("CyclicFlag"), (MailBox + j)->CyclicFlag, app_path);

		// SmtpServer
		profile_write_string(buf, TEXT("SmtpServer"), (MailBox + j)->SmtpServer, app_path);
		// SmtpPort
		profile_write_int(buf, TEXT("SmtpPort"), (MailBox + j)->SmtpPort, app_path);
		// UserName
		profile_write_string(buf, TEXT("UserName"), (MailBox + j)->UserName, app_path);
		// MailAddress
		profile_write_string(buf, TEXT("MailAddress"), (MailBox + j)->MailAddress, app_path);
		// Signature
		if ((MailBox + j)->Signature != NULL) {
			p = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen((MailBox + j)->Signature) * 2 + 1));
			if (p != NULL) {
				EncodeCtrlChar((MailBox + j)->Signature, p);
				profile_write_string(buf, TEXT("Signature"), p, app_path);
				mem_free(&p);
			}
		}
		// ReplyTo
		profile_write_string(buf, TEXT("ReplyTo"), (MailBox + j)->ReplyTo, app_path);
		// MyAddr2Bcc
		profile_write_int(buf, TEXT("MyAddr2Bcc"), (MailBox + j)->MyAddr2Bcc, app_path);
		// BccAddr
		profile_write_string(buf, TEXT("BccAddr"), (MailBox + j)->BccAddr, app_path);

		// POP before SMTP
		profile_write_int(buf, TEXT("PopBeforeSmtp"), (MailBox + j)->PopBeforeSmtp, app_path);
		// SMTP Authentication
		profile_write_int(buf, TEXT("SmtpAuth"), (MailBox + j)->SmtpAuth, app_path);
		// SMTP Authentication type
		profile_write_int(buf, TEXT("SmtpAuthType"), (MailBox + j)->SmtpAuthType, app_path);
		// SMTP Authentication User & Pass mode
		profile_write_int(buf, TEXT("AuthUserPass"), (MailBox + j)->AuthUserPass, app_path);
		// SMTP Authentication User
		profile_write_string(buf, TEXT("SmtpUser"), (MailBox + j)->SmtpUser, app_path);
		// SMTP Authentication Pass
		EncodePassword((MailBox + j)->SmtpUser, (MailBox + j)->SmtpPass, tmp, BUF_SIZE - 1, FALSE);
		profile_write_string(buf, TEXT("SmtpPass"), tmp, app_path);
		// SMTP SSL
		profile_write_int(buf, TEXT("SmtpSSL"), (MailBox + j)->SmtpSSL, app_path);
		// SMTP SSL Option
		profile_write_int(buf, TEXT("SmtpSSLType"), (MailBox + j)->SmtpSSLInfo.Type, app_path);
		profile_write_int(buf, TEXT("SmtpSSLVerify"), (MailBox + j)->SmtpSSLInfo.Verify, app_path);
		profile_write_int(buf, TEXT("SmtpSSLDepth"), (MailBox + j)->SmtpSSLInfo.Depth, app_path);
		profile_write_string(buf, TEXT("SmtpSSLCert"), (MailBox + j)->SmtpSSLInfo.Cert, app_path);
		profile_write_string(buf, TEXT("SmtpSSLPkey"), (MailBox + j)->SmtpSSLInfo.Pkey, app_path);
		profile_write_string(buf, TEXT("SmtpSSLPass"), (MailBox + j)->SmtpSSLInfo.Pass, app_path);

		// Filter
		profile_write_int(buf, TEXT("FilterEnable"), (MailBox + j)->FilterEnable, app_path);
		profile_write_int(buf, TEXT("FilterCnt"), (MailBox + j)->FilterCnt, app_path);

		for (t = 0; (MailBox + j)->tpFilter != NULL && t < (MailBox + j)->FilterCnt; t++) {
			tpFilter = *((MailBox + j)->tpFilter + t);
			if (tpFilter == NULL) {
				continue;
			}

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Enable"));
			profile_write_int(buf, key_buf, tpFilter->Enable, app_path);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Action"));
			profile_write_int(buf, key_buf, tpFilter->Action, app_path);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Header1"));
			profile_write_string(buf, key_buf, tpFilter->Header1, app_path);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Content1"));
			profile_write_string(buf, key_buf, tpFilter->Content1, app_path);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Header2"));
			profile_write_string(buf, key_buf, tpFilter->Header2, app_path);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Content2"));
			profile_write_string(buf, key_buf, tpFilter->Content2, app_path);
		}

		// RAS
		profile_write_int(buf, TEXT("RasMode"), (MailBox + j)->RasMode, app_path);
		profile_write_string(buf, TEXT("RasEntry"), (MailBox + j)->RasEntry, app_path);
		profile_write_int(buf, TEXT("RasReCon"), (MailBox + j)->RasReCon, app_path);
	}
	if (profile_flush(app_path) == FALSE) {
		rc = FALSE;
	}
	profile_free();

	if (SaveMailFlag == FALSE) {
		// 設定保存のみ
		return rc;
	}

	// メールボックス内のメールの保存
	for (j = MAILBOX_USER; j < MailBoxCnt; j++) {
		if ((MailBox + j) == NULL) {
			continue;
		}
		// メールアイテム
		wsprintf(buf, TEXT("MailBox%d.dat"), j - MAILBOX_USER);
		if (file_save_mailbox(buf, MailBox + j, op.ListSaveMode) == FALSE) {
			rc = FALSE;
		}
	}
	return rc;
}

/*
 * ini_free - 設定情報を解放する
 */
void ini_free(void)
{
	mem_free(&op.view_font.name);
	mem_free(&op.lv_font.name);
	mem_free(&op.SendHelo);
	mem_free(&op.CAFile);
	mem_free(&op.QuotationChar);
	mem_free(&op.ReSubject);
	mem_free(&op.ReHeader);
	mem_free(&op.Bura);
	mem_free(&op.Oida);
	mem_free(&op.sBura);
	mem_free(&op.sOida);
	mem_free(&op.HeadCharset);
	mem_free(&op.BodyCharset);
	mem_free(&op.TimeZone);
	mem_free(&op.DateFormat);
	mem_free(&op.TimeFormat);
	mem_free(&op.NewMailSoundFile);
	mem_free(&op.ExecEndSoundFile);
	mem_free(&op.ViewApp);
	mem_free(&op.ViewAppCmdLine);
	mem_free(&op.ViewFileSuffix);
	mem_free(&op.ViewFileHeader);
	mem_free(&op.EditApp);
	mem_free(&op.EditAppCmdLine);
	mem_free(&op.EditFileSuffix);
	mem_free(&op.URLApp);
	mem_free(&op.AttachPath);
	mem_free(&op.Password);
}
/* End of source */
