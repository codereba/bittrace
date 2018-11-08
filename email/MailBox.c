/*
 * nPOP
 *
 * MailBox.c
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#include "General.h"
#include "memory_mng.h"
#include "string_mng.h"

/* Define */
#define IDC_CB				2000

#define POP_PORT			110
#define SMTP_PORT			25

/* Global Variables */
//extern OPTION op;

//#ifdef _WIN32_WCE_LAGENDA
//extern HMENU hMainMenu;
//#endif

//extern int MailMenuPos;
//extern HWND hMainToolBar;

//extern smtp_user *MailBox;
//extern int MailBoxCnt;
//smtp_user *AddressBox;

//extern int SelBox;
//extern int LvSortFlag;
//extern BOOL EndThreadSortFlag;
//extern BOOL PPCFlag;

/* Local Function Prototypes */

/*
 * mailbox_init - ���[���{�b�N�X�̏�����
 */
BOOL mailbox_init(void)
{

	//���[���{�b�N�X�̃��X�g�̊m��
	MailBoxCnt = 2;
	MailBox = (smtp_user *)mem_calloc(sizeof(smtp_user) * MailBoxCnt);
	if(MailBox == NULL){
		return FALSE;
	}
	//�A�h���X���̊m��
	//AddressBox = (smtp_user *)mem_calloc(sizeof(smtp_user));
	//if(AddressBox == NULL){
	//	return FALSE;
	//}
	return TRUE;
}

/*
 * mailbox_create - ���[���{�b�N�X�̒ǉ�
 */
int mailbox_create(HWND hWnd, BOOL ShowFlag)
{
	smtp_user *TmpMailBox;
	int cnt, index;

	//���[���{�b�N�X�̃��X�g�ɒǉ�
	index = MailBoxCnt;
	cnt = MailBoxCnt + 1;

	TmpMailBox = (smtp_user *)mem_calloc(sizeof(smtp_user) * cnt);
	if(TmpMailBox == NULL){
		return -1;
	}
	CopyMemory(TmpMailBox, MailBox, sizeof(smtp_user) * MailBoxCnt);

	(TmpMailBox + index)->Port = POP_PORT;
	(TmpMailBox + index)->SmtpPort = SMTP_PORT;

	(TmpMailBox + index)->PopSSLInfo.Verify = 1;
	(TmpMailBox + index)->PopSSLInfo.Depth = -1;
	(TmpMailBox + index)->SmtpSSLInfo.Verify = 1;
	(TmpMailBox + index)->SmtpSSLInfo.Depth = -1;

	mem_free(&MailBox);
	MailBox = TmpMailBox;
	MailBoxCnt++;

	if(ShowFlag == TRUE){
		int i;

		//�R���{�{�b�N�X�Ƀ��[���{�b�N�X��ǉ����đI������
		i = SendDlgItemMessage(hWnd, IDC_COMBO, CB_ADDSTRING, 0, (LPARAM)STR_MAILBOX_NONAME);
		if(i == CB_ERR){
			return -1;
		}
		SendDlgItemMessage(hWnd, IDC_COMBO, CB_SETCURSEL, i, 0);
		return i;
	}
	//�������̊m�ۂ̂�
	return index;
}

/*
 * mailbox_delete - ���[���{�b�N�X�̍폜
 */
int mailbox_delete(HWND hWnd, int DelIndex)
{
	smtp_user *TmpMailBox;
	int cnt;
	int i, j;

	//���[���{�b�N�X�̃��X�g����폜
	cnt = MailBoxCnt - 1;
	TmpMailBox = (smtp_user *)mem_calloc(sizeof(smtp_user) * cnt);
	if(TmpMailBox == NULL){
		return -1;
	}
	j = 0;
	for(i = 0; i < MailBoxCnt; i++){
		if(i == DelIndex){
			mailbox_free(MailBox + i);
			continue;
		}
		CopyMemory((TmpMailBox + j), (MailBox + i), sizeof(smtp_user));
		j++;
	}
	mem_free(&MailBox);
	MailBox = TmpMailBox;
	MailBoxCnt = cnt;

	//�R���{�{�b�N�X����폜���āA��O�̃��[���{�b�N�X��I������
	SendDlgItemMessage(hWnd, IDC_COMBO, CB_DELETESTRING, DelIndex, 0);
	SendDlgItemMessage(hWnd, IDC_COMBO, CB_SETCURSEL, DelIndex - 1, 0);
	return DelIndex - 1;
}

/*
 * mailbox_read - �Œ胁�[���{�b�N�X�̓ǂݍ���
 */
BOOL mailbox_read(void)
{
	//�ۑ���
	(MailBox + MAILBOX_SAVE)->Name = alloc_copy_t(STR_SAVEBOX_NAME);
	if(file_read_mailbox(SAVEBOX_FILE, (MailBox + MAILBOX_SAVE)) == FALSE){
		return FALSE;
	}

	//���M��
	(MailBox + MAILBOX_SEND)->Name = alloc_copy_t(STR_SENDBOX_NAME);
	if(file_read_mailbox(SENDBOX_FILE, (MailBox + MAILBOX_SEND)) == FALSE){
		return FALSE;
	}

	//�A�h���X��
	//if(file_read_address_book(ADDRESS_FILE, AddressBox) == 0){
	//	if(file_read_mailbox(ADDRESS_FILE_OLD, AddressBox) == FALSE){
	//		return FALSE;
	//	}
	//}
	return TRUE;
}

/*
 * mailbox_move_up - ���[���{�b�N�X�̈ʒu����Ɉړ�����
 */
void mailbox_move_up(HWND hWnd)
{
	smtp_user *TmpMailBox;
	int i;

	if(SelBox <= MAILBOX_USER || MailBoxCnt <= MAILBOX_USER + 1){
		return;
	}

	//�������̈ʒu���ړ�
	TmpMailBox = (smtp_user *)mem_calloc(sizeof(smtp_user) * MailBoxCnt);
	if(TmpMailBox == NULL){
		return;
	}

	//for(i = 0; i < MailBoxCnt; i++){
	//	if(SelBox == i + 1){
	//		CopyMemory((TmpMailBox + i), (MailBox + i + 1), sizeof(smtp_user));
	//		CopyMemory((TmpMailBox + i + 1), (MailBox + i), sizeof(smtp_user));
	//		i++;
	//	}else{
	//		CopyMemory((TmpMailBox + i), (MailBox + i), sizeof(smtp_user));
	//	}
	//}
	//mem_free(&MailBox);
	//MailBox = TmpMailBox;

	//
//	SendDlgItemMessage(hWnd, IDC_COMBO, CB_DELETESTRING, SelBox, 0);
//	SelBox--;
//	SendDlgItemMessage(hWnd, IDC_COMBO, CB_INSERTSTRING, SelBox,
//		(LPARAM)(((MailBox + SelBox)->Name == NULL || *(MailBox + SelBox)->Name == TEXT('\0'))
//		? STR_MAILBOX_NONAME : (MailBox + SelBox)->Name));
//	SendDlgItemMessage(hWnd, IDC_COMBO, CB_SETCURSEL, SelBox, 0);
}

/*
 * mailbox_move_down - ���[���{�b�N�X�̈ʒu�����Ɉړ�����
 */
void mailbox_move_down(HWND hWnd)
{
	smtp_user *TmpMailBox;
	int i;

	if(SelBox < MAILBOX_USER || SelBox >= MailBoxCnt - 1 || MailBoxCnt <= MAILBOX_USER + 1){
		return;
	}

	//�������̈ʒu���ړ�
	TmpMailBox = (smtp_user *)mem_calloc(sizeof(smtp_user) * MailBoxCnt);
	if(TmpMailBox == NULL){
		return;
	}

	for(i = 0; i < MailBoxCnt; i++){
		if(SelBox == i){
			CopyMemory((TmpMailBox + i), (MailBox + i + 1), sizeof(smtp_user));
			CopyMemory((TmpMailBox + i + 1), (MailBox + i), sizeof(smtp_user));
			i++;
		}else{
			CopyMemory((TmpMailBox + i), (MailBox + i), sizeof(smtp_user));
		}
	}
	mem_free(&MailBox);
	MailBox = TmpMailBox;

	//�R���{�{�b�N�X�ɕ\������Ă���ʒu���ړ�
	SendDlgItemMessage(hWnd, IDC_COMBO, CB_DELETESTRING, SelBox, 0);
	SelBox++;
	SendDlgItemMessage(hWnd, IDC_COMBO, CB_INSERTSTRING, SelBox,
		(LPARAM)(((MailBox + SelBox)->Name == NULL || *(MailBox + SelBox)->Name == TEXT('\0'))
		? STR_MAILBOX_NONAME : (MailBox + SelBox)->Name));
	SendDlgItemMessage(hWnd, IDC_COMBO, CB_SETCURSEL, SelBox, 0);
}

/*
 * mailbox_unread_check - ���[���{�b�N�X�ɖ��J�����[�������݂��邩���ׂ�
 */
BOOL mailbox_unread_check(int index, BOOL NewFlag)
{
	int i;

	for(i = (MailBox + index)->MailItemCnt - 1; i >= 0 ; i--){
		if(*((MailBox + index)->mail_item + i) == NULL){
			continue;
		}
		if((NewFlag == TRUE || (*((MailBox + index)->mail_item + i))->New == TRUE) &&
			(*((MailBox + index)->mail_item + i))->MailStatus == ICON_MAIL){
			return TRUE;
		}
	}
	return FALSE;
}

/*
 * mailbox_next_unread - ���J�����[�������݂��郁�[���{�b�N�X�̃C���f�b�N�X���擾
 */
int mailbox_next_unread(int index, int endindex)
{
	int j;

	if(index < MAILBOX_USER){
		return -1;
	}
	for(j = index; j < endindex; j++){
		if(mailbox_unread_check(j, TRUE) == TRUE){
			return j;
		}
	}
	return -1;
}

#ifdef _HAVE_WND
/*
 * mailbox_select - ���[���{�b�N�X�̑I��
 */
void mailbox_select(HWND hWnd, int Sel)
{
	HMENU hMenu;
	LV_COLUMN lvc;
	TCHAR *p;

	if(Sel == -1){
		return;
	}
	SelBox = Sel;

	p = ((MailBox + SelBox)->Name == NULL || *(MailBox + SelBox)->Name == TEXT('\0'))
		? STR_MAILBOX_NONAME : (MailBox + SelBox)->Name;
	(MailBox + SelBox)->NoRead = FALSE;
	//�V����������� "*" ���t���Ă���ꍇ�̓R���{�{�b�N�X�̃��X�g��ݒ肵����
	SendDlgItemMessage(hWnd, IDC_COMBO, CB_DELETESTRING, SelBox, 0);
	SendDlgItemMessage(hWnd, IDC_COMBO, CB_INSERTSTRING, SelBox, (LPARAM)p);
	SendDlgItemMessage(hWnd, IDC_COMBO, CB_SETCURSEL, SelBox, 0);

	//���j���[�̎擾
#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
	hMenu = SHGetSubMenu(hMainToolBar, ID_MENUITEM_MAIL);
#elif defined(_WIN32_WCE_LAGENDA)
	hMenu = GetSubMenu(hMainMenu, MailMenuPos);
#else
	hMenu = GetSubMenu(CommandBar_GetMenu(GetDlgItem(hWnd, IDC_CB), 0), MailMenuPos);
#endif
#else
	hMenu = GetSubMenu(GetMenu(hWnd), MailMenuPos);
#endif

	//�ԐM�A��M�p�Ƀ}�[�N �̍��ڂ��폜����
	DeleteMenu(hMenu, ID_MENUITEM_REMESSEGE, MF_BYCOMMAND);
	DeleteMenu(hMenu, ID_MENUITEM_DOWNMARK, MF_BYCOMMAND);
	DeleteMenu(hMenu, ID_MENUITE_SAVECOPY, MF_BYCOMMAND);

	if(SelBox == MAILBOX_SEND){
		//���M��
#ifdef _WIN32_WCE
		InsertMenu(hMenu, 1, MF_BYPOSITION | MF_STRING,
			ID_MENUITEM_REMESSEGE,
			(PPCFlag == TRUE) ? STR_LIST_PPCMENU_SENDINFO : STR_LIST_MENU_SENDINFO);
		InsertMenu(hMenu, ID_MENUITEM_DELMARK, MF_STRING,
			ID_MENUITEM_DOWNMARK,
			(PPCFlag == TRUE) ? STR_LIST_PPCMENU_SENDMARK : STR_LIST_MENU_SENDMARK);
		InsertMenu(hMenu, ID_MENUITEM_DELETE, MF_STRING,
			ID_MENUITE_SAVECOPY,
			(PPCFlag == TRUE) ? STR_LIST_PPCMENU_CREATECOPY : STR_LIST_MENU_CREATECOPY);
#else
		InsertMenu(hMenu, 1, MF_BYPOSITION | MF_STRING,
			ID_MENUITEM_REMESSEGE, STR_LIST_MENU_SENDINFO);
		InsertMenu(hMenu, ID_MENUITEM_DELMARK, MF_STRING,
			ID_MENUITEM_DOWNMARK, STR_LIST_MENU_SENDMARK);
		InsertMenu(hMenu, ID_MENUITEM_DELETE, MF_STRING,
			ID_MENUITE_SAVECOPY, STR_LIST_MENU_CREATECOPY);
#endif
		lvc.pszText = STR_LIST_LVHEAD_TO;
	}else{
		//��M��
#ifdef _WIN32_WCE
		InsertMenu(hMenu, 1, MF_BYPOSITION | MF_STRING,
			ID_MENUITEM_REMESSEGE,
			(PPCFlag == TRUE) ? STR_LIST_PPCMENU_REPLY : STR_LIST_MENU_REPLY);
		InsertMenu(hMenu, ID_MENUITEM_DELMARK, MF_STRING,
			ID_MENUITEM_DOWNMARK,
			(PPCFlag == TRUE) ? STR_LIST_PPCMENU_RECVMARK : STR_LIST_MENU_RECVMARK);
		InsertMenu(hMenu, ID_MENUITEM_DELETE, MF_STRING,
			ID_MENUITE_SAVECOPY,
			(PPCFlag == TRUE) ? STR_LIST_PPCMENU_SAVEBOXCOPY : STR_LIST_MENU_SAVEBOXCOPY);
#else
		InsertMenu(hMenu, 1, MF_BYPOSITION | MF_STRING,
			ID_MENUITEM_REMESSEGE, STR_LIST_MENU_REPLY);
		InsertMenu(hMenu, ID_MENUITEM_DELMARK, MF_STRING,
			ID_MENUITEM_DOWNMARK, STR_LIST_MENU_RECVMARK);
		InsertMenu(hMenu, ID_MENUITEM_DELETE, MF_STRING,
			ID_MENUITE_SAVECOPY, STR_LIST_MENU_SAVEBOXCOPY);
#endif
		lvc.pszText = STR_LIST_LVHEAD_FROM;
	}

	//���X�g�r���[�w�b�_�̐ݒ�
	lvc.mask = LVCF_TEXT;
	lvc.cchTextMax = BUF_SIZE;
	ListView_SetColumn(GetDlgItem(hWnd, IDC_LISTVIEW), 1, &lvc);

	LvSortFlag = SORT_NO + 1;
	EndThreadSortFlag = FALSE;

	//���X�g�r���[�ɃA�C�e����ݒ肷��
	SwitchCursor(FALSE);
	ListView_ShowItem(GetDlgItem(hWnd, IDC_LISTVIEW), (MailBox + SelBox));
	SwitchCursor(TRUE);

	SetMailMenu(hWnd);
	SetItemCntStatusText(hWnd, NULL);
	SetNoReadCntTitle(hWnd);
}
#endif //_HAVE_WND

/*
 * mailbox_name_to_index - 
 */
int mailbox_name_to_index(TCHAR *Name)
{
	int i;

	if(Name == NULL){
		return -1;
	}
	for(i = 0; i < MailBoxCnt; i++){
		if(lstrcmpi((MailBox + i)->Name, Name) == 0){
			return i;
		}
	}
	return -1;
}

/*
 * filer_free - �t�B���^���̉��
 */
void filer_free(smtp_user *mail_box)
{
	int i;

	//�t�B���^���̉��
	for(i = 0; i < mail_box->FilterCnt; i++){
		if(*(mail_box->tpFilter + i) == NULL){
			continue;
		}
		mem_free(&(*(mail_box->tpFilter + i))->Header1);
		mem_free(&(*(mail_box->tpFilter + i))->Content1);

		mem_free(&(*(mail_box->tpFilter + i))->Header2);
		mem_free(&(*(mail_box->tpFilter + i))->Content2);

		mem_free(&*(mail_box->tpFilter + i));
	}
	mem_free((void **)&mail_box->tpFilter);
	mail_box->tpFilter = NULL;
}

/*
 * mailbox_free - ���[���{�b�N�X�̉��
 */
void mailbox_free(smtp_user *mail_box)
{
	if(mail_box == NULL){
		return;
	}
	//�A�J�E���g���̉��
	mem_free(&mail_box->Name);
	mem_free(&mail_box->Server);
	mem_free(&mail_box->User);
	mem_free(&mail_box->Pass);
	mem_free(&mail_box->TmpPass);
	mem_free(&mail_box->PopSSLInfo.Cert);
	mem_free(&mail_box->PopSSLInfo.Pkey);
	mem_free(&mail_box->PopSSLInfo.Pass);

	mem_free(&mail_box->LastMessageId);
	mem_free(&mail_box->SmtpServer);
	mem_free(&mail_box->UserName);
	mem_free(&mail_box->MailAddress);
	mem_free(&mail_box->Signature);
	mem_free(&mail_box->ReplyTo);
	mem_free(&mail_box->BccAddr);
	mem_free(&mail_box->SmtpUser);
	mem_free(&mail_box->SmtpPass);
	mem_free(&mail_box->SmtpTmpPass);
	mem_free(&mail_box->SmtpSSLInfo.Cert);
	mem_free(&mail_box->SmtpSSLInfo.Pkey);
	mem_free(&mail_box->SmtpSSLInfo.Pass);

	mem_free(&mail_box->RasEntry);

	//�t�B���^���̉��
	filer_free(mail_box);

	//���[�����̉��
	if(mail_box->mail_item != NULL){
		item_free(mail_box->mail_item, mail_box->MailItemCnt);
		mem_free((void **)&mail_box->mail_item);
	}
	mail_box->mail_item = NULL;
	mail_box->AllocCnt = mail_box->MailItemCnt = 0;
}
/* End of source */
