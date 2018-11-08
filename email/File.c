/*
 * ERC
 *
 * File.c
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#include "General.h"
#include "memory_mng.h"
#include "string_mng.h"
#ifdef _WIN32_WCE_PPC
#include "SelectFile.h"
#endif

/* Define */

/* Global Variables */
//extern OPTION op;

//extern HINSTANCE hInst;  // Local copy of hInstance
TCHAR *AppDir = _T( "c;\\" );
TCHAR *DataDir = _T( "c:\\" );
email_box *MailBox = NULL;

/* Local Function Prototypes */
static int file_get_mail_count(char *buf, long Size);
static BOOL file_save_address_item(HANDLE hFile, MAILITEM *mail_item);

/*
 * log_init - ログの区切りの保存
 */
BOOL log_init(TCHAR *fpath, TCHAR *fname, TCHAR *buf)
{
#define LOG_SEP				TEXT("\r\n-------------------------------- ")
	TCHAR fDay[BUF_SIZE];
	TCHAR fTime[BUF_SIZE];
	TCHAR *p;
	BOOL ret;

	if (GetDateFormat(0, 0, NULL, NULL, fDay, BUF_SIZE - 1) == 0) {
		return FALSE;
	}
	if (GetTimeFormat(0, 0, NULL, NULL, fTime, BUF_SIZE - 1) == 0) {
		return FALSE;
	}
	p = mem_alloc(sizeof(TCHAR) * (lstrlen(LOG_SEP) + lstrlen(fDay) + 1 + lstrlen(fTime) + 2 + lstrlen(buf) + 2));
	if (p == NULL) {
		return FALSE;
	}
	str_join_t(p, LOG_SEP, fDay, TEXT(" "), fTime, TEXT(" ("), buf, TEXT(")"), (TCHAR *)-1);
	ret = log_save(fpath, fname, p);
	mem_free(&p);
	return ret;
}

/*
 * log_save - ログの保存
 */
BOOL log_save(TCHAR *fpath, TCHAR *fname, TCHAR *buf)
{
	HANDLE hFile;
	TCHAR path[BUF_SIZE];
	DWORD ret;

	// ファイルに保存
	wsprintf(path, TEXT("%s%s"), fpath, fname);

	// 保存するファイルを開く
	hFile = CreateFile(path, GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL || hFile == (HANDLE)-1) {
		return FALSE;
	}
	SetFilePointer(hFile, 0, NULL, FILE_END);

	if (file_write_ascii(hFile, buf, lstrlen(buf)) == FALSE) {
		CloseHandle(hFile);
		return FALSE;
	}
	if (*(buf + lstrlen(buf) - 1) != TEXT('\n')) {
		if (WriteFile(hFile, "\r\n", 2, &ret, NULL) == FALSE) {
			CloseHandle(hFile);
			return FALSE;
		}
	}
	CloseHandle(hFile);
	return TRUE;
}

/*
 * log_clear - ログのクリア
 */
BOOL log_clear(TCHAR *fpath, TCHAR *fname)
{
	TCHAR path[BUF_SIZE];

	wsprintf(path, TEXT("%s%s"), fpath, fname);
	return DeleteFile(path);
}

/*
 * dir_check - ディレクトリかどうかチェック
 */
BOOL dir_check(TCHAR *path)
{
	WIN32_FIND_DATA FindData;
	HANDLE hFindFile;

	if ((hFindFile = FindFirstFile(path, &FindData)) == INVALID_HANDLE_VALUE) {
		return FALSE;
	}
	FindClose(hFindFile);

	if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		return TRUE;
	}
	return FALSE;
}

/*
 * dir_delete - ディレクトリ内のファイルを削除
 */
BOOL dir_delete(TCHAR *Path, TCHAR *file)
{
	WIN32_FIND_DATA FindData;
	HANDLE hFindFile;
	TCHAR sPath[BUF_SIZE];
	TCHAR buf[BUF_SIZE];

	if (Path == NULL || *Path == TEXT('\0')) {
		return FALSE;
	}
	wsprintf(sPath, TEXT("%s\\%s"), Path, file);

	if ((hFindFile = FindFirstFile(sPath, &FindData)) == INVALID_HANDLE_VALUE) {
		return FALSE;
	}
	do{
		if (!(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			wsprintf(buf, TEXT("%s\\%s"), Path, FindData.cFileName);
			DeleteFile(buf);
		}
	} while (FindNextFile(hFindFile, &FindData) == TRUE);
	FindClose(hFindFile);
	return TRUE;
}

/*
 * filename_conv - ファイル名にできない文字を _ に変換する
 */
void filename_conv(TCHAR *buf)
{
	TCHAR *p = buf, *r;

	while (*p != TEXT('\0')) {
#ifndef UNICODE
		if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
			// 2バイトコード
			p += 2;
			continue;
		}
#endif
		// 空白の除去
		if (*p == TEXT(' ') || *p == TEXT('\t')) {
			r = p + 1;
			for (; *r == TEXT(' ') || *r == TEXT('\t'); r++);
			if (p + 1 != r) {
				lstrcpy(p + 1, r);
			}
		}
		// ファイル名にできない文字は指定の文字に変換
		switch (*p) {
		case TEXT('\\'): case TEXT('/'): case TEXT(':'):
		case TEXT(','): case TEXT(';'): case TEXT('*'):
		case TEXT('?'): case TEXT('\"'): case TEXT('<'):
		case TEXT('>'): case TEXT('|'):
			*p = TEXT('_');
			break;
		}
		p++;
	}
}

/*
 * filename_select - 
 */
BOOL filename_select(HWND hWnd, TCHAR *ret, TCHAR *DefExt, TCHAR *filter, BOOL OpenSave)
{
#ifdef _WIN32_WCE_PPC
	TCHAR path[BUF_SIZE];

	lstrcpy( path, ret );
	return SelectFile(hWnd, NULL, OpenSave, path, ret);
#else	// _WIN32_WCE_PPC
	OPENFILENAME of;
	TCHAR path[BUF_SIZE];
#ifdef _WIN32_WCE
	TCHAR *ph;
#endif	// _WIN32_WCE

	// 
	lstrcpy(path, ret);
	ZeroMemory(&of, sizeof(OPENFILENAME));
	of.lStructSize = sizeof(OPENFILENAME);
	of.hInstance = NULL;
	of.hwndOwner = hWnd;
	if (filter == NULL) {
		of.lpstrFilter = STR_FILE_FILTER;
	} else {
		of.lpstrFilter = filter;
	}
	of.nFilterIndex = 1;
	if (OpenSave == TRUE) {
		of.lpstrTitle = STR_TITLE_OPEN;
	} else {
		of.lpstrTitle = STR_TITLE_SAVE;
	}
	of.lpstrFile = path;
	of.nMaxFile = BUF_SIZE - 1;
	of.lpstrDefExt = DefExt;
	of.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

	// 
	if (OpenSave == TRUE) {
		of.Flags |= OFN_FILEMUSTEXIST;
		if (GetOpenFileName((LPOPENFILENAME)&of) == FALSE) {
			return FALSE;
		}
	} else {
		if (GetSaveFileName((LPOPENFILENAME)&of) == FALSE) {
			return FALSE;
		}
	}
#ifdef _WIN32_WCE
	ph = path;
	if (*ph == TEXT('\\') && *(ph + 1) == TEXT('\\')) {
		ph++;
	}
	if (*(ph + lstrlen(ph)) == TEXT('.')) {
		*(ph + lstrlen(ph)) = TEXT('\0');
	}
	lstrcpy(ret, ph);
#else	// _WIN32_WCE
	lstrcpy(ret, path);
#endif	// _WIN32_WCE
	return TRUE;
#endif	// _WIN32_WCE_PPC
}

/*
 * file_get_size - ファイルのサイズを取得する
 */
long file_get_size(TCHAR *FileName)
{
	WIN32_FIND_DATA FindData;
	HANDLE hFindFile;

	if ((hFindFile = FindFirstFile(FileName, &FindData)) == INVALID_HANDLE_VALUE) {
		return -1;
	}
	FindClose(hFindFile);

	if ((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
		// ディレクトリではない場合はサイズを返す
		return (long)FindData.nFileSizeLow;
	}
	return -1;
}

/*
 * file_get_mail_count - メール一覧の文字列からメールの数を取得
 */
static int file_get_mail_count(char *buf, long Size)
{
	char *p, *r, *t;
	int ret = 0;

	p = buf;
	while (Size > p - buf && *p != '\0') {
		for (t = r = p; Size > r - buf && *r != '\0'; r++) {
			if (*r == '\r' && *(r + 1) == '\n') {
				if (*t == '.' && (r - t) == 1) {
					break;
				}
				t = r + 2;
			}
		}
		p = r;
		if (Size > p - buf && *p != '\0') {
			p += 2;
		}
		ret++;
	}
	return ret;
}

/*
 * file_read - ファイルを読み込む
 */
char *file_read(TCHAR *path, long FileSize)
{
	HANDLE hFile;
	DWORD ret;
	char *cBuf;

	// ファイルを開く
	hFile = CreateFile(path, GENERIC_READ, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL || hFile == (HANDLE)-1) {
		return NULL;
	}
	cBuf = (char *)mem_calloc(FileSize + 1);
	if (cBuf == NULL) {
		CloseHandle(hFile);
		return NULL;
	}

	if (ReadFile(hFile, cBuf, FileSize, &ret, NULL) == FALSE) {
		mem_free(&cBuf);
		CloseHandle(hFile);
		return NULL;
	}
	CloseHandle(hFile);
	return cBuf;
}

LRESULT SwitchCursor(const BOOL Flag)
{
	return ERROR_SUCCESS; 
}

/*
 * file_read_select - ファイルを選択して読み込む
 */
BOOL file_read_select(HWND hWnd, TCHAR **buf)
{
	TCHAR path[BUF_SIZE];
	char *cBuf;
	long FileSize;

	*buf = NULL;

	// ファイル名取得
	lstrcpy(path, TEXT("*.txt"));
	if (filename_select(hWnd, path, TEXT("txt"), STR_TEXT_FILTER, TRUE) == FALSE) {
		return TRUE;
	}

	// ファイルのサイズを取得
	FileSize = file_get_size(path);
	if (FileSize <= 0) {
		return TRUE;
	}

	// ファイルを読み込む
	SwitchCursor(FALSE);
	cBuf = file_read(path, FileSize);
	if (cBuf == NULL) {
		SwitchCursor(TRUE);
		return FALSE;
	}

#ifdef UNICODE
	// UNICODEに変換
	*buf = alloc_char_to_tchar(cBuf);
	if (*buf == NULL) {
		mem_free(&cBuf);
		SwitchCursor(TRUE);
		return FALSE;
	}
	mem_free(&cBuf);
#else
	*buf = cBuf;
#endif
	SwitchCursor(TRUE);
	return TRUE;
}

/*
 * file_read_mailbox - ファイルからメールアイテムの作成
 */
BOOL file_read_mailbox(TCHAR *FileName, email_box *mail_box)
{
	MAILITEM *mail_item;
#ifndef _NOFILEMAP
	HANDLE hFile;
	HANDLE hMapFile;
#endif
	TCHAR path[BUF_SIZE];
	char *p, *r, *s, *t;
	char *FileBuf;
	long FileSize;
	int i;

#ifndef _WIN32_WCE
	SetCurrentDirectory(AppDir);
#endif

	str_join_t(path, DataDir, FileName, (TCHAR *)-1);

	FileSize = file_get_size(path);
	if (FileSize <= 0) {
		return TRUE;
	}
#ifdef _NOFILEMAP
	FileBuf = file_read(path, FileSize);
	if (FileBuf == NULL) {
		return FALSE;
	}
#else	// _NOFILEMAP
	// ファイルを開く
#ifdef _WIN32_WCE
	hFile = CreateFileForMapping(path, GENERIC_READ, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
#else	// _WIN32_WCE
	hFile = CreateFile(path, GENERIC_READ, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
#endif	// _WIN32_WCE
	if (hFile == NULL) {
		return FALSE;
	}

	hMapFile = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
	if (hMapFile == NULL) {
#ifndef _WCE_OLD
		CloseHandle(hFile);
#endif	// _WCE_OLD
		return FALSE;
	}
	FileBuf = MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0);
	if (FileBuf == NULL) {
		CloseHandle(hMapFile);
#ifndef _WCE_OLD
		CloseHandle(hFile);
#endif	// _WCE_OLD
		return FALSE;
	}
#endif	// _NOFILEMAP

	// メール数分のメモリを確保
	mail_box->max_mail_count = mail_box->mail_num = file_get_mail_count(FileBuf, FileSize);
	mail_box->mail_item = (MAILITEM **)mem_calloc(sizeof(MAILITEM *) * mail_box->mail_num);
	if (mail_box->mail_item == NULL) {
		mail_box->max_mail_count = mail_box->mail_num = 0;
#ifdef _NOFILEMAP
		mem_free(&FileBuf);
#else	// _NOFILEMAP
		UnmapViewOfFile(FileBuf);
		CloseHandle(hMapFile);
#ifndef _WCE_OLD
		CloseHandle(hFile);
#endif	// _WCE_OLD
#endif	// _NOFILEMAP
		return FALSE;
	}

	i = 0;
	p = FileBuf;
	while (FileSize > p - FileBuf && *p != '\0') {
		// ヘッダからメールアイテムを作成
		mail_item = *(mail_box->mail_item + i) = item_string_to_item(mail_box, p);

		// Body位置の取得
		p = GetBodyPointa(p);
		if (p == NULL) {
			break;
		}
		// メールの終わりの位置を取得
		for (t = r = p; *r != '\0'; r++) {
			if (*r == '\r' && *(r + 1) == '\n') {
				if (*t == '.' && (r - t) == 1) {
					t -= 2;
					for (; FileSize > r - FileBuf && (*r == '\r' || *r == '\n'); r++);
					break;
				}
				t = r + 2;
			}
		}
		if (mail_item != NULL) {
			// Bodyをコピー
			if ((t - p) > 0) {
				mail_item->Body = (char *)mem_alloc(sizeof(char) * (t - p + 1));
				if (mail_item->Body != NULL) {
					for (s = mail_item->Body; p < t; p++, s++) {
						*s = *p;
					}
					*s = '\0';
				}
			}
			if (mail_item->Body == NULL && mail_item->MailStatus < ICON_SENDMAIL) {
				if (mail_item->Download == TRUE) {
					mail_item->Body = (char *)mem_alloc(sizeof(char));
					if (mail_item->Body != NULL) {
						*mail_item->Body = '\0';
					}
				} else {
					mail_item->MailStatus = ICON_NON;
					if (mail_item->Status != ICON_DOWN && mail_item->Status != ICON_DEL && mail_item->Status != ICON_SEND) {
						mail_item->Status = ICON_NON;
					}
					mail_item->Download = FALSE;
				}
			}
		}
		p = r;
		i++;
	}

#ifdef _NOFILEMAP
	mem_free(&FileBuf);
#else	// _NOFILEMAP
	UnmapViewOfFile(FileBuf);
	CloseHandle(hMapFile);
#ifndef _WCE_OLD
	CloseHandle(hFile);
#endif	// _WCE_OLD
#endif	// _NOFILEMAP
	return TRUE;
}

/*
 * file_read_address_book - ファイルからアドレス帳を読み込む
 */
int file_read_address_book(TCHAR *FileName, email_box *mail_box)
{
	MAILITEM *mail_item;
	TCHAR path[BUF_SIZE];
	TCHAR *MemFile, *AllocBuf = NULL;
	TCHAR *p, *r, *s;
	char *FileBuf;
	long FileSize;
#ifdef UNICODE
	long Len;
#endif
	int LineCnt = 0;
	int i;

#ifndef _WIN32_WCE
	SetCurrentDirectory(AppDir);
#endif

	str_join_t(path, DataDir, FileName, (TCHAR *)-1);

	FileSize = file_get_size(path);
	if (FileSize < 0) {
		return 0;
	}
	if (FileSize == 0) {
		return 1;
	}
	FileBuf = file_read(path, FileSize);
	if (FileBuf == NULL) {
		return -1;
	}

#ifdef UNICODE
	// UNICODEに変換
	Len = char_to_tchar_size(FileBuf);
	MemFile = AllocBuf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (Len + 1));
	if (MemFile == NULL) {
		mem_free(&FileBuf);
		return -1;
	}
	char_to_tchar(FileBuf, MemFile, Len);
	FileSize = Len;
#else	// UNICODE
	MemFile = (TCHAR *)FileBuf;
#endif	// UNICODE

	// 行数のカウント
	for (LineCnt = 0, p = MemFile; *p != TEXT('\0'); p++) {
		if (*p == TEXT('\n')) {
			LineCnt++;
		}
	}
	mail_box->max_mail_count = mail_box->mail_num = LineCnt;
	mail_box->mail_item = (MAILITEM **)mem_calloc(sizeof(MAILITEM *) * mail_box->mail_num);
	if (mail_box->mail_item == NULL) {
		mail_box->max_mail_count = mail_box->mail_num = 0;
		mem_free(&FileBuf);
		mem_free(&AllocBuf);
		return -1;
	}
	i = 0;
	p = MemFile;
	while (FileSize > p - MemFile && *p != TEXT('\0')) {
		mail_item = *(mail_box->mail_item + i) = (MAILITEM *)mem_calloc(sizeof(MAILITEM));

		// メールアドレス
		for (r = p; *r != TEXT('\0') && *r != TEXT('\t') && *r != TEXT('\r') && *r != TEXT('\n'); r++);
		if (mail_item != NULL) {
			mail_item->To = (TCHAR *)mem_alloc(sizeof(TCHAR) * (r - p + 1));
			if (mail_item->To != NULL) {
				for (s = mail_item->To; p < r; p++, s++) {
					*s = *p;
				}
				*s = '\0';
			}
		}
		if (*r == TEXT('\t')) r++;

		// コメント
		for (p = r; *r != TEXT('\0') && *r != TEXT('\t') && *r != TEXT('\r') && *r != TEXT('\n'); r++);
		if (mail_item != NULL) {
			mail_item->Subject = (TCHAR *)mem_alloc(sizeof(TCHAR) * (r - p + 1));
			if (mail_item->Subject != NULL) {
				for (s = mail_item->Subject; p < r; p++, s++) {
					*s = *p;
				}
				*s = '\0';
			}
		}
		for (; *p != TEXT('\0') && *p != TEXT('\r') && *p != TEXT('\n'); p++);
		for (; *p == TEXT('\r') || *p == TEXT('\n'); p++);
		i++;
	}
	mem_free(&FileBuf);
	mem_free(&AllocBuf);
	return 1;
}

/*
 * file_write - マルチバイトに変換して保存
 */
BOOL file_write(HANDLE hFile, char *buf, int len)
{
	DWORD ret;

	if (len == 0) return TRUE;
	return WriteFile(hFile, buf, len, &ret, NULL);
}

/*
 * file_write_ascii - マルチバイトに変換して保存
 */
BOOL file_write_ascii(HANDLE hFile, TCHAR *buf, int len)
{
	DWORD ret;
#ifdef UNICODE
	char *str;
	int clen;

	if (len == 0) return TRUE;

	clen = tchar_to_char_size(buf);
	str = (char *)mem_alloc(clen + 1);
	if (str == NULL) {
		return FALSE;
	}
	tchar_to_char(buf, str, clen);
	if (WriteFile(hFile, str, clen - 1, &ret, NULL) == FALSE) {
		mem_free(&str);
		return FALSE;
	}
	mem_free(&str);
	return TRUE;

#else
	if (len == 0) return TRUE;
	return WriteFile(hFile, buf, len, &ret, NULL);
#endif
}

/*
 * file_save - ファイルの保存
 */
BOOL file_save(HWND hWnd, TCHAR *FileName, TCHAR *Ext, char *buf, int len)
{
	HANDLE hFile;
	TCHAR path[BUF_SIZE];
	DWORD ret;

	// ファイルに保存
	if (FileName == NULL) {
		*path = TEXT('\0');
	} else {
		lstrcpy(path, FileName);
	}
	if (filename_select(hWnd, path, Ext, NULL, FALSE) == FALSE) {
		return TRUE;
	}

	// 保存するファイルを開く
	hFile = CreateFile(path, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL || hFile == (HANDLE)-1) {
		return FALSE;
	}
	SwitchCursor(FALSE);
	if (WriteFile(hFile, buf, len, &ret, NULL) == FALSE) {
		CloseHandle(hFile);
		SwitchCursor(TRUE);
		return FALSE;
	}
	SwitchCursor(TRUE);
	CloseHandle(hFile);
	return TRUE;
}

/*
 * file_save_exec - 
 */
BOOL file_save_exec(HWND hWnd, TCHAR *FileName, char *buf, int len)
{
	WIN32_FIND_DATA FindData;
	SHELLEXECUTEINFO sei;
	HANDLE hFindFile;
	HANDLE hFile;
	TCHAR path[BUF_SIZE];
	TCHAR tmp_path[BUF_SIZE];
	DWORD ret;

	if (FileName == NULL) {
		return FALSE;
	}
	SwitchCursor(FALSE);

	// 
	wsprintf(path, TEXT("%s%s"), DataDir, TEXT( "data" ) /*op.AttachPath*/);
	hFindFile = FindFirstFile(path, &FindData);
	FindClose(hFindFile);
	if (hFindFile == INVALID_HANDLE_VALUE || !(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
		CreateDirectory(path, NULL);
	}

	wsprintf(path, TEXT("%s%s\\%s"), DataDir, TEXT( "data" ), FileName);
	
#if 0
	if( op.AttachDelete == 0 )
	{
		// 
		wsprintf( tmp_path, TEXT( "%s_%lX.tmp" ), TEXT( "data" ), ( long )buf ); 

		hFile = CreateFile( tmp_path, 
			GENERIC_READ | GENERIC_WRITE, 
			0, 
			0, 
			CREATE_ALWAYS, 
			FILE_ATTRIBUTE_NORMAL, 
			NULL );

		if( /*hFile == NULL || */hFile == INVALID_HANDLE_VALUE ) 
		{
			SwitchCursor( TRUE ); 
			return FALSE;
		}

		if( WriteFile( hFile, buf, len, &ret, NULL ) == FALSE )
		{
			CloseHandle( hFile );
			DeleteFile( tmp_path );
			SwitchCursor( TRUE );
			return FALSE;
		}

		FlushFileBuffers( hFile );
		CloseHandle( hFile );

		// 
		if( CopyFile( tmp_path, path, FALSE ) == FALSE )
		{
			DeleteFile(tmp_path);
			SwitchCursor(TRUE);
			return FALSE;
		}

		DeleteFile(tmp_path);
	} 
	else 
#endif //0

	{
		// 
		hFile = CreateFile( path, GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL ); 

		if( /*hFile == NULL || */hFile == INVALID_HANDLE_VALUE )
		{
			SwitchCursor(TRUE);
			return FALSE;
		}
		
		if( WriteFile( hFile, buf, len, &ret, NULL ) == FALSE )
		{
			CloseHandle( hFile ); 
			SwitchCursor( TRUE );
			
			return FALSE;
		}
		
		FlushFileBuffers( hFile );
		CloseHandle( hFile );
	}

	SwitchCursor( TRUE );

	ZeroMemory( &sei, sizeof( SHELLEXECUTEINFO ) );
	
	sei.cbSize = sizeof( sei );
	sei.fMask = 0;
	sei.hwnd = NULL;
	sei.lpVerb = NULL;
	sei.lpFile = path;
	sei.lpParameters = NULL;
	sei.lpDirectory = NULL;
	sei.nShow = SW_SHOWNORMAL;
	sei.hInstApp = NULL;
	ShellExecuteEx( &sei );
	
	return TRUE;
}

/*
 * file_save_mailbox - メールボックス内のメールを保存
 */
BOOL file_save_mailbox(TCHAR *FileName, email_box *mail_box, int SaveFlag)
{
	HANDLE hFile;
	TCHAR path[BUF_SIZE];
	char *tmp, *p;
	int len = 0;
	int i;

#ifndef _WIN32_WCE
	SetCurrentDirectory(AppDir);
#endif	// _WIN32_WCE
	str_join_t(path, DataDir, FileName, (TCHAR *)-1);

	if (SaveFlag == 0) {
		// 保存しない場合は削除
		DeleteFile(path);
		return TRUE;
	}

#ifndef DIV_SAVE
	// メール文字列作成
	for (i = 0; i < mail_box->mail_num; i++) {
		if (*(mail_box->mail_item + i) == NULL) {
			continue;
		}
		len += item_to_string_size(*(mail_box->mail_item + i), (SaveFlag == 1) ? FALSE : TRUE);
	}
	p = tmp = (char *)mem_alloc(sizeof(char) * (len + 1));
	if (tmp == NULL) {
		return FALSE;
	}
	*p = '\0';
	for (i = 0; i < mail_box->mail_num; i++) {
		if (*(mail_box->mail_item + i) == NULL) {
			continue;
		}
		p = item_to_string(p, *(mail_box->mail_item + i), (SaveFlag == 1) ? FALSE : TRUE);
	}
#endif	// DIV_SAVE

	// 保存
	hFile = CreateFile(path, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL || hFile == (HANDLE)-1) {
#ifndef DIV_SAVE
		mem_free(&tmp);
#endif	// DIV_SAVE
		return FALSE;
	}
#ifndef DIV_SAVE
	if (file_write(hFile, tmp, len) == FALSE) {
		mem_free(&tmp);
		return FALSE;
	}
	mem_free(&tmp);
#else	// DIV_SAVE
	for (i = 0; i < mail_box->mail_num; i++) {
		if (*(mail_box->mail_item + i) == NULL) {
			continue;
		}
		len = item_to_string_size(*(mail_box->mail_item + i), (SaveFlag == 1) ? FALSE : TRUE);

		p = tmp = (char *)mem_alloc(sizeof(char) * (len + 1));
		if (tmp == NULL) {
			CloseHandle(hFile);
			return FALSE;
		}
		item_to_string(tmp, *(mail_box->mail_item + i), (SaveFlag == 1) ? FALSE : TRUE);
		if (file_write(hFile, tmp, len) == FALSE) {
			mem_free(&tmp);
			CloseHandle(hFile);
			return FALSE;
		}
		mem_free(&tmp);
	}
#endif	// DIV_SAVE
	CloseHandle(hFile);
	return TRUE;
}

/*
 * file_save_address_item - アドレス帳を1件保存
 */
static BOOL file_save_address_item(HANDLE hFile, MAILITEM *mail_item)
{
	TCHAR *tmp;
	int len = 0;

	if (mail_item->To != NULL) {
		len += lstrlen(mail_item->To);
	}
	if (mail_item->Subject != NULL && *mail_item->Subject != TEXT('\0')) {
		len += 1;	// TAB
		len += lstrlen(mail_item->Subject);
	}
	len += 2;		// CRLF

	tmp = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
	if (tmp == NULL) {
		return FALSE;
	}
	if (mail_item->Subject != NULL && *mail_item->Subject != TEXT('\0')) {
		str_join_t(tmp, mail_item->To, TEXT("\t"), mail_item->Subject, TEXT("\r\n"), (TCHAR *)-1);
	} else {
		str_join_t(tmp, mail_item->To, TEXT("\r\n"), (TCHAR *)-1);
	}

	if (file_write_ascii(hFile, tmp, len) == FALSE) {
		mem_free(&tmp);
		return FALSE;
	}
	mem_free(&tmp);
	return TRUE;
}

/*
 * file_save_address_book - アドレス帳をファイルに保存
 */
BOOL file_save_address_book(TCHAR *FileName, email_box *mail_box)
{
	HANDLE hFile;
	TCHAR path[BUF_SIZE];
	int i;

#ifndef _WIN32_WCE
	SetCurrentDirectory(AppDir);
#endif

	str_join_t(path, DataDir, FileName, (TCHAR *)-1);

	// 保存するファイルを開く
	hFile = CreateFile(path, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL || hFile == (HANDLE)-1) {
		return FALSE;
	}

	// 保存する文字列をコピー
	for (i = 0; i < mail_box->mail_num; i++) {
		if (*(mail_box->mail_item + i) == NULL) {
			continue;
		}
		if (file_save_address_item(hFile, *(mail_box->mail_item + i)) == FALSE) {
			CloseHandle(hFile);
			return FALSE;
		}
	}
	CloseHandle(hFile);
	return TRUE;
}
/* End of source */
