/*
 * nPOP
 *
 * Ras.c
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

#include "General.h"
/* Include Files */
#include <windows.h>
#include <Ras.h>

#include "General.h"
#include "memory_mng.h"
#include "string_mng.h"

/* Define */
#ifdef _WIN32_WCE
#define RASDLL						TEXT("COREDLL.dll")

#define NAME_RasEnumEntries			TEXT("RasEnumEntries")
#define NAME_RasEnumConnections		TEXT("RasEnumConnections")
#define NAME_RasGetConnectStatus	TEXT("RasGetConnectStatus")
#define NAME_RasHangUp				TEXT("RasHangUp")
#define NAME_RasGetErrorString		TEXT("RasGetErrorString")
#define NAME_RasDial				TEXT("RasDial")

#else
#define RASDLL						TEXT("rasapi32.dll")
#ifdef UNICODE

#define NAME_RasEnumEntries			"RasEnumEntriesW"
#define NAME_RasEnumConnections		"RasEnumConnectionsW"
#define NAME_RasGetConnectStatus	"RasGetConnectStatusW"
#define NAME_RasHangUp				"RasHangUpW"
#define NAME_RasGetErrorString		"RasGetErrorStringW"
#define NAME_RasDial				"RasDialW"

#else

#define NAME_RasEnumEntries			"RasEnumEntriesA"
#define NAME_RasEnumConnections		"RasEnumConnectionsA"
#define NAME_RasGetConnectStatus	"RasGetConnectStatusA"
#define NAME_RasHangUp				"RasHangUpA"
#define NAME_RasGetErrorString		"RasGetErrorStringA"
#define NAME_RasDial				"RasDialA"

#endif
#endif

#define RAS_EVENT					TEXT("RAS_EVENT")

#define RAS_ENTRY_CNT				100
#define RAS_CONN_CNT				30

/* Global Variables */
BOOL RasLoop = FALSE;

UINT WM_RASEVENT = 0;
HANDLE hEvent;
static HINSTANCE hrasapi;
#ifdef _WIN32_WCE_LAGENDA
static DWORD hRasCon = 0;
#else
static HRASCONN hRasCon = NULL;
#endif
static BOOL g_Ret;
static int gRasIndex = -1;

// 外部参照
//extern OPTION op;

//extern HINSTANCE hInst;  // Local copy of hInstance

//extern SOCKET g_soc;
//extern email_box *MailBox;
BOOL AutoCheckFlag = TRUE;

/* Local Function Prototypes */
static BOOL RasConnectStart(HWND hWnd, int BoxIndex);

/*
 * GetRasInfo - RAS情報のインデックスを取得
 */
int GetRasInfo(TCHAR *Entry)
{
	int i;

	if (Entry == NULL || *Entry == TEXT('\0')) {
		return -1;
	}

	//__asm int 3; 
	//for (i = 0; i < op.RasInfoCnt; i++) {
	//	if (*(op.RasInfo + i) == NULL || (*(op.RasInfo + i))->RasEntry == NULL) {
	//		continue;
	//	}
	//	if (lstrcmp(Entry, (*(op.RasInfo + i))->RasEntry) == 0) {
	//		return i;
	//	}
	//}
	return -1;
}

/*
 * SetRasInfo - RAS情報に追加
 */
BOOL SetRasInfo(TCHAR *Entry, TCHAR *User, TCHAR *Pass)
{
	RASINFO **TmpRasInfo;
	int i;

	if (*Entry == TEXT('\0')) {
		return TRUE;
	}
	//if ((i = GetRasInfo(Entry)) != -1) {
	//	mem_free(&(*(op.RasInfo + i))->RasUser);
	//	mem_free(&(*(op.RasInfo + i))->RasPass);
	//	(*(op.RasInfo + i))->RasUser = alloc_copy_t(User);
	//	(*(op.RasInfo + i))->RasPass = alloc_copy_t(Pass);
	//	return TRUE;
	//}

	//TmpRasInfo = (RASINFO **)mem_alloc(sizeof(RASINFO *) * (op.RasInfoCnt + 1));
	//if (TmpRasInfo == NULL) {
	//	return FALSE;
	//}
	//if (op.RasInfo != NULL) {
	//	CopyMemory(TmpRasInfo, op.RasInfo,
	//		sizeof(RASINFO *) * op.RasInfoCnt);
	//}
	//*(TmpRasInfo + op.RasInfoCnt) = (RASINFO *)mem_calloc(sizeof(RASINFO));
	//if (*(TmpRasInfo + op.RasInfoCnt) == NULL) {
	//	return FALSE;
	//}
	//(*(TmpRasInfo + op.RasInfoCnt))->RasEntry = alloc_copy_t(Entry);
	//(*(TmpRasInfo + op.RasInfoCnt))->RasUser = alloc_copy_t(User);
	//(*(TmpRasInfo + op.RasInfoCnt))->RasPass = alloc_copy_t(Pass);

	//mem_free((void **)&op.RasInfo);
	//op.RasInfo = TmpRasInfo;
	//op.RasInfoCnt++;
	return TRUE;
}

/*
 * FreeRasInfo - RAS情報の解放
 */
void FreeRasInfo(void)
{
	int i;

	// RAS
	//for (i = 0; i < op.RasInfoCnt; i++) {
	//	if (*(op.RasInfo + i) == NULL) {
	//		continue;
	//	}
	//	mem_free(&(*(op.RasInfo + i))->RasEntry);
	//	mem_free(&(*(op.RasInfo + i))->RasUser);
	//	mem_free(&(*(op.RasInfo + i))->RasPass);

	//	mem_free(&*(op.RasInfo + i));
	//}
	//mem_free((void **)&op.RasInfo);
	//op.RasInfo = NULL;
}

/*
 * initRas - RASの初期化
 */
void initRas(void)
{
#ifndef _WIN32_WCE_LAGENDA
	hrasapi = LoadLibrary(RASDLL);

#ifdef _WIN32_WCE
	WM_RASEVENT = WM_RASDIALEVENT;
#else
	WM_RASEVENT = RegisterWindowMessage(TEXT(RASDIALEVENT));
	if (WM_RASEVENT == 0) {
		WM_RASEVENT = WM_RASDIALEVENT;
	}
#endif
#endif
}

/*
 * FreeRas - RASの解放
 */
void FreeRas(void)
{
#ifndef _WIN32_WCE_LAGENDA
	if (hrasapi != NULL) {
		FreeLibrary(hrasapi);
	}
#endif
}

/*
 * GetRasEntries - ComboBoxにRASのエントリーを設定する
 */
BOOL GetRasEntries(HWND hCmboWnd)
{
#ifndef _WIN32_WCE_LAGENDA
	FARPROC wRasEnumEntries;
	RASENTRYNAME rent[RAS_ENTRY_CNT];
	int ret;
	int sz;
	int i;

	if (hrasapi == NULL) {
		return FALSE;
	}
	rent[0].dwSize = sizeof(rent[0]);
	sz = sizeof(rent);
	wRasEnumEntries = GetProcAddress((HMODULE)hrasapi, NAME_RasEnumEntries);
	if (wRasEnumEntries == NULL) {
		return FALSE;
	}
	if (wRasEnumEntries(NULL, NULL, rent, (LPDWORD)&sz, (LPDWORD)&ret) == 0) {
		for (i = 0; i < ret; i++) {
			ComboBox_AddString(hCmboWnd, rent[i].szEntryName);
		}
	}
	return TRUE;
#else
	COMMSETINFO csi[RAS_ENTRY_CNT];
	DWORD size;
	DWORD cnt;
	DWORD i;

	size = sizeof(csi);
	cnt = 0;
	if (COMMSet_UIDList(csi, &size, &cnt) != 0) {
		return FALSE;
	}

	for (i = 0; i < cnt; i++) {
		ComboBox_AddString(hCmboWnd, csi[i].EntryName);
	}
	return TRUE;
#endif
}

/*
 * GetRasStatus - RASの状態を取得する
 */
BOOL GetRasStatus(void)
{
#ifndef _WIN32_WCE_LAGENDA
	FARPROC wRasEnumConnections;
	FARPROC wRasGetConnectStatus;
	RASCONNSTATUS rasconsts;
	RASCONN rcon[RAS_CONN_CNT];
	int i;
	int ccnt;
	int sz;

	if (hrasapi == NULL) {
		return TRUE;
	}
	rcon[0].dwSize = sizeof(rcon[0]);
	sz = sizeof(rcon);
	wRasEnumConnections = GetProcAddress((HMODULE)hrasapi, NAME_RasEnumConnections);
	if (wRasEnumConnections == NULL) {
		return TRUE;
	}
	if (wRasEnumConnections((LPRASCONN)rcon, (LPDWORD)&sz, (LPDWORD)&ccnt) != 0) {
		return TRUE;
	}
	if (ccnt <= 0) {
		gRasIndex = -1;
		hRasCon = NULL;
		return FALSE;
	}
	ccnt = (ccnt > RAS_CONN_CNT) ? RAS_CONN_CNT : ccnt;
	wRasGetConnectStatus = GetProcAddress((HMODULE)hrasapi, NAME_RasGetConnectStatus);
	if (wRasGetConnectStatus == NULL) {
		return TRUE;
	}
	rasconsts.dwSize = sizeof(rasconsts);
	for (i = 0; i < ccnt; i++) {
		if (wRasGetConnectStatus(rcon[i].hrasconn, (LPRASCONNSTATUS)&rasconsts) != 0) {
			continue;
		}
		if (rasconsts.rasconnstate == RASCS_Connected) {
			return TRUE;
		}
	}
	gRasIndex = -1;
	hRasCon = NULL;
	return FALSE;
#else
	COMMSETINFO csi[RAS_ENTRY_CNT];
	DWORD size;
	DWORD cnt;

	size = sizeof(csi);
	cnt = 0;
	if (DIALUP_ConnectUID_Get(csi, &size, &cnt) != 0) {
		gRasIndex = -1;
		hRasCon = 0;
		return FALSE;
	}
	if (cnt > 0) {
		return TRUE;
	}
	gRasIndex = -1;
	hRasCon = 0;
	return FALSE;
#endif
}

/*
 * RasDisconnect - RASの切断処理を行う
 */
void RasDisconnect(void)
{
#ifndef _WIN32_WCE_LAGENDA
	FARPROC wRasHangUp;

	gRasIndex = -1;
	RasLoop = FALSE;

	if (hrasapi == NULL || hRasCon == NULL) {
		return;
	}
	wRasHangUp = GetProcAddress((HMODULE)hrasapi, NAME_RasHangUp);
	if (wRasHangUp == NULL) {
		return;
	}
	wRasHangUp(hRasCon);
	hRasCon = NULL;

	g_Ret = FALSE;
	if (hEvent != NULL) {
		SetEvent(hEvent);
	}
#else
	gRasIndex = -1;
	RasLoop = FALSE;

	if (hRasCon == 0) {
		return;
	}
	DIALUP_Disconnect(&hRasCon, DISCONNECT_NODROP);
	hRasCon = 0;
	g_Ret = FALSE;
#endif
}

/*
 * RasErr - RASのエラー文字列を取得する
 */
static void RasErr(int ErrorCode, TCHAR *ErrStr)
{
#ifndef _WIN32_WCE_LAGENDA
	FARPROC wRasGetErrorString;

	if (hrasapi == NULL) {
		lstrcpy(ErrStr, STR_ERR_RAS_CONNECT);
	} else {
		wRasGetErrorString = GetProcAddress((HMODULE)hrasapi, NAME_RasGetErrorString);
		if (wRasGetErrorString == NULL) {
			lstrcpy(ErrStr,STR_ERR_RAS_DISCONNECT);
		} else {
			wRasGetErrorString((unsigned int)ErrorCode, ErrStr, BUF_SIZE - 1);
		}
	}
	RasDisconnect();
#else
	lstrcpy(ErrStr, STR_ERR_RAS_CONNECT);
	RasDisconnect();
#endif
}

/*
 * RasConnect - RASの接続を行う
 */
static BOOL RasConnect(HWND hWnd, TCHAR *strEntry, TCHAR *strUser, TCHAR *strPass,
					   TCHAR *ErrStr)
{
#ifndef _WIN32_WCE_LAGENDA
	FARPROC wRasDial;
	RASDIALPARAMS rdp;
	DWORD ret;

	if (hrasapi == NULL) {
		lstrcpy(ErrStr, STR_ERR_RAS_CONNECT);
		return FALSE;
	}
	rdp.dwSize = sizeof(rdp);
	lstrcpy(rdp.szEntryName,strEntry);
	lstrcpy(rdp.szUserName,strUser);
	lstrcpy(rdp.szPassword,strPass);
	*rdp.szPhoneNumber = TEXT('\0');
	*rdp.szCallbackNumber = TEXT('\0');
	*rdp.szDomain = TEXT('\0');

	wRasDial = GetProcAddress((HMODULE)hrasapi, NAME_RasDial);
	if (wRasDial == NULL) {
		return FALSE;
	}
	hRasCon = NULL;
	ret = wRasDial(NULL, NULL, &rdp, 0xFFFFFFFF, (LPVOID)hWnd, (LPHRASCONN)&hRasCon);
	if (ret != 0) {
		RasErr(ret, ErrStr);
		return FALSE;
	}
	return TRUE;
#else
	COMMSETINFO csi[RAS_ENTRY_CNT];
	DWORD size;
	DWORD cnt;
	DWORD i;

	size = sizeof(csi);
	cnt = 0;
	if (COMMSet_UIDList(csi, &size, &cnt) != 0) {
		return FALSE;
	}
	for (i = 0; i < cnt; i++) {
		if (lstrcmp(strEntry, csi[i].EntryName) == 0) {
			break;
		}
	}
	if (i >= cnt) {
		RasErr(0, ErrStr);
		return FALSE;
	}
	hRasCon = csi[i].CommSetUID;
	if (DIALUP_Connect(&hRasCon, NO_CONNECT_DSP) != 0) {
		hRasCon = 0;
		RasErr(0, ErrStr);
		return FALSE;
	}
	return TRUE;
#endif
}

/*
 * RasStatusProc - RASの状況表示ダイアログのプロシージャ
 */
BOOL RasStatusProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#ifndef _WIN32_WCE_LAGENDA
	TCHAR ErrStr[BUF_SIZE];
	BOOL ShowError;

	if (hRasCon == NULL) {
		gRasIndex = -1;
		return TRUE;
	}

#if 0 
	if (lParam != 0) {
		ShowError = RasLoop;
		RasErr(lParam, ErrStr);
		SetStatusTextT(hWnd, ErrStr, 1);
		if (op.SocIgnoreError != 1 && ShowError == TRUE) {
			//ErrorMessage(hWnd, ErrStr);
		}
		return TRUE;
	}
	switch (wParam) {
	case RASCS_OpenPort:
		SetStatusTextT(hWnd, STR_STATUS_RAS_PORTOPEN, 1);
		break;

	case RASCS_ConnectDevice:
		SetStatusTextT(hWnd, STR_STATUS_RAS_DEVICE, 1);
		break;

	case RASCS_Authenticate:
		SetStatusTextT(hWnd, STR_STATUS_RAS_AUTH, 1);
		break;

	case RASCS_Connected:
		SetStatusTextT(hWnd, STR_STATUS_RAS_CONNECT, 1);

		g_Ret = TRUE;
		if (hEvent != NULL) {
			SetEvent(hEvent);
		}
		break;

	case RASCS_Disconnected:
		ShowError = RasLoop;
		RasDisconnect();
		SetStatusTextT(hWnd, STR_STATUS_RAS_DISCONNECT, 1);
		if (op.SocIgnoreError != 1 && ShowError == TRUE) {
			//ErrorMessage(hWnd, STR_STATUS_RAS_DISCONNECT);
		}
		break;
	}

#endif //0
#endif
	return TRUE;
}

/*
 * RasConnectStart - RASの接続を開始する
 */

#if 0
static BOOL RasConnectStart(HWND hWnd, int BoxIndex)
{
#ifndef _WIN32_WCE_LAGENDA
	MSG msg;
	TCHAR ErrStr[BUF_SIZE];
	int i;
	BOOL Stats;

	//if (hrasapi == NULL || (MailBox + BoxIndex)->RasMode == 0 ||
	//	((Stats = GetRasStatus()) == TRUE &&
	//	(hRasCon == NULL || (MailBox + BoxIndex)->RasReCon == 0))) {
	//	return TRUE;
	//}
	//if ((MailBox + BoxIndex)->RasEntry == NULL || *(MailBox + BoxIndex)->RasEntry == TEXT('\0')) {
	//	if (AutoCheckFlag == TRUE && op.RasNoCheck == 1 && Stats == FALSE) {
	//		return FALSE;
	//	} else {
	//		return TRUE;
	//	}
	//}
	// RAS情報のIndexを取得する
	//if ((i = GetRasInfo((MailBox + BoxIndex)->RasEntry)) == -1) {
	//	if (op.SocIgnoreError != 1) {
	//		ErrorMessage(hWnd, STR_ERR_RAS_NOSET);
	//	}
	//	return FALSE;
	//}
	// 既に同一のエントリで接続している場合はTRUEを返す
	if (Stats == TRUE && i == gRasIndex) {
		return TRUE;
	}
	// 自動チェックで未接続時はチェックしない設定の場合
	if (AutoCheckFlag == TRUE && op.RasNoCheck == 1) {
		return FALSE;
	}
	if (hRasCon != NULL) {
		// ダイヤルアップ切断
		RasDisconnect();

		// 次のダイヤルまでにラグを付ける
		SetTimer(hWnd, ID_RASWAIT_TIMER, op.RasWaitSec * 1000, NULL);

		hEvent = CreateEvent(NULL, TRUE, FALSE, RAS_WAIT_EVENT);
		if (hEvent == NULL) {
			if (op.SocIgnoreError != 1) {
				//ErrorMessage(hWnd, STR_ERR_SOCK_EVENT);
			}
			return FALSE;
		}
		RasLoop = TRUE;
		ResetEvent(hEvent);
		while (WaitForSingleObject(hEvent, 0) == WAIT_TIMEOUT) {
			if (GetMessage(&msg, NULL, 0, 0) == FALSE) {
				break;
			}
			//MessageFunc(hWnd, &msg);
			if (RasLoop == FALSE) {
				break;
			}
		}
		CloseHandle(hEvent);
		hEvent = NULL;
		if (RasLoop == FALSE) {
			return FALSE;
		}
		RasLoop = FALSE;
	}

	// ダイヤルアップ接続開始
	SetStatusTextT(hWnd, STR_STATUS_RAS_START, 1);

	if (RasConnect(hWnd, (*(op.RasInfo + i))->RasEntry,
		(*(op.RasInfo + i))->RasUser, (*(op.RasInfo + i))->RasPass, ErrStr) == FALSE) {
		SetStatusTextT(hWnd, ErrStr, 1);
		if (op.SocIgnoreError != 1) {
			//ErrorMessage(hWnd, ErrStr);
		}
		return FALSE;
	}

	// RASの接続処理
	hEvent = CreateEvent(NULL, TRUE, FALSE, RAS_EVENT);
	if (hEvent == NULL) {
		if (op.SocIgnoreError != 1) {
			//ErrorMessage(hWnd, STR_ERR_SOCK_EVENT);
		}
		return FALSE;
	}
	RasLoop = TRUE;
	ResetEvent(hEvent);
	g_Ret = FALSE;
	while (WaitForSingleObject(hEvent, 0) == WAIT_TIMEOUT) {
		if (GetMessage(&msg, NULL, 0, 0) == FALSE) {
			break;
		}
		//MessageFunc(hWnd, &msg);
		if (RasLoop == FALSE) {
			break;
		}
	}
	CloseHandle(hEvent);
	hEvent = NULL;
	if (RasLoop == FALSE) {
		return FALSE;
	}
	RasLoop = FALSE;
	if (g_Ret == TRUE) {
		gRasIndex = i;
	}
	return g_Ret;
#else
	MSG msg;
	TCHAR ErrStr[BUF_SIZE];
	DWORD ConnStatus;
	int i, j;
	BOOL Stats;

	//if ((MailBox + BoxIndex)->RasMode == 0 ||
	//	((Stats = GetRasStatus()) == TRUE &&
	//	(hRasCon == 0 || (MailBox + BoxIndex)->RasReCon == 0))) {
	//	return TRUE;
	//}
	//if ((MailBox + BoxIndex)->RasEntry == NULL || *(MailBox + BoxIndex)->RasEntry == TEXT('\0')) {
	//	if (AutoCheckFlag == TRUE && op.RasNoCheck == 1 && Stats == FALSE) {
	//		return FALSE;
	//	} else {
	//		return TRUE;
	//	}
	//}
	//// RAS情報のIndexを取得する
	//if ((i = GetRasInfo((MailBox + BoxIndex)->RasEntry)) == -1) {
	//	if (op.SocIgnoreError != 1) {
	//		ErrorMessage(hWnd, STR_ERR_RAS_NOSET);
	//	}
	//	return FALSE;
	//}
	// 既に同一のエントリで接続している場合はTRUEを返す
	if (Stats == TRUE && i == gRasIndex) {
		return TRUE;
	}
	// 自動チェックで未接続時はチェックしない設定の場合
	if (AutoCheckFlag == TRUE && op.RasNoCheck == 1) {
		return FALSE;
	}
	if (hRasCon != 0) {
		// ダイヤルアップ切断
		RasDisconnect();

		// 次のダイヤルまでにラグを付ける
		SetTimer(hWnd, ID_RASWAIT_TIMER, op.RasWaitSec * 1000, NULL);

		hEvent = CreateEvent(NULL, TRUE, FALSE, RAS_WAIT_EVENT);
		if (hEvent == NULL) {
			if (op.SocIgnoreError != 1) {
				//ErrorMessage(hWnd, STR_ERR_SOCK_EVENT);
			}
			return FALSE;
		}
		RasLoop = TRUE;
		ResetEvent(hEvent);
		while (WaitForSingleObject(hEvent, 0) == WAIT_TIMEOUT) {
			if (GetMessage(&msg, NULL, 0, 0) == FALSE) {
				break;
			}
			//MessageFunc(hWnd, &msg);
			if (RasLoop == FALSE) {
				break;
			}
		}
		CloseHandle(hEvent);
		hEvent = NULL;
		if (RasLoop == FALSE) {
			return FALSE;
		}
		RasLoop = FALSE;
	}

	// ダイヤルアップ接続開始
	//if (RasConnect(hWnd, (*(op.RasInfo + i))->RasEntry,
	//	(*(op.RasInfo + i))->RasUser, (*(op.RasInfo + i))->RasPass, ErrStr) == FALSE) {
	//	SetStatusTextT(hWnd, ErrStr, 1);
	//	if (op.SocIgnoreError != 1) {
	//		ErrorMessage(hWnd, ErrStr);
	//	}
	//	return FALSE;
	//}

	RasLoop = TRUE;
	for (j = 0; j < 10; j++) {
		if (DIALUP_ConnectState_Get(&hRasCon, &ConnStatus) == 0) {
			break;
		}
		Sleep(100);
	}
	while (1) {
		if (DIALUP_ConnectState_Get(&hRasCon, &ConnStatus) != 0) {
			return FALSE;
		}
		if (ConnStatus == CMS_DIALUP_CONNECT) {
			break;
		}

		if (GetMessage(&msg, NULL, 0, 0) == FALSE) {
			break;
		}
		//MessageFunc(hWnd, &msg);
		if (RasLoop == FALSE) {
			return FALSE;
		}
	}
	RasLoop = FALSE;
	gRasIndex = i;
	return TRUE;
#endif
}

#endif //0

#if 0
/*
 * RasMailBoxStart - アカウント毎のRASの接続を開始する
 */
BOOL RasMailBoxStart(HWND hWnd, int BoxIndex)
{
	BOOL ret;

	if (op.EnableLAN == 1) {
		// LAN接続オプションが有効
		return TRUE;
	}

	//g_soc = 0;
	SetMailMenu(hWnd);

	ret = RasConnectStart(hWnd, BoxIndex);

	//g_soc = -1;
	SetMailMenu(hWnd);
	return ret;
}
#endif //0 
/* End of source */
