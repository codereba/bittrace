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

#pragma once

#include "sys_mng_api.h"

#define MAX_HOOK_INFO_SIZE ( sizeof( HOOK_FILTER_INFO ) + sizeof( ULONG ) * NtApiFilterEnd )
#define MAX_TRACE_INFO_LEN ( ULONG )( 1024 * 10 )

//typedef struct __FILTER_INDEX_TABLE
//{
//	ULONG *FilterIndexes;
//	ULONG IndexNumber;
//} FILTER_INDEX_TABLE, *PFILTER_INDEX_TABLE;

INT32 sys_mng_ui_output( ULONG ui_action_id, 
						 INT32 ret_val, 
								PBYTE data, 
								ULONG length, 
								PVOID param ); 

//typedef struct _CHARRANGE
//{
//	LONG	cpMin;
//	LONG	cpMax;
//} CHARRANGE;

#define EM_EXSETSEL				(WM_USER + 55)

INLINE INT32 InsertTailText( HWND hEdit, CHAR *Text )
{
	CHARRANGE cr;
	ASSERT( NULL != hEdit );
	ASSERT( NULL != Text );

	cr.cpMin = -1;
	cr.cpMax = -1;
	SendMessageA( hEdit, EM_EXSETSEL, 0, (LPARAM)&cr);
	SendMessageA( hEdit, EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)Text);
	SendMessageA( hEdit, EM_EXSETSEL, 0, (LPARAM)&cr);
	SendMessageA( hEdit, EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)"\r\n" );

	PostMessageA( hEdit, WM_VSCROLL, SB_BOTTOM,0 ); 

	return 0; 
}

class sys_mng_tab :
	public CDialogImpl<sys_mng_tab, ui_base_tab>,
	public CWinDataExchange<sys_mng_tab>
{
public:
	enum { IDD = IDD_SYS_MNG_TAB };

	sys_mng_tab() : sys_act_info( NULL )
	{
		Managing = FALSE; 
	}

	BEGIN_MSG_MAP( sys_mng_tab )
		MESSAGE_HANDLER(WM_INITDIALOG, on_init_dlg)
		MESSAGE_HANDLER( WM_DESTROY, on_destroy_dlg )
		MESSAGE_HANDLER(WM_SHOWWINDOW, on_show_wnd)
		COMMAND_HANDLER(IDOK, BN_CLICKED, on_ok_bn )
		//COMMAND_HANDLER(IDC_BUTTON_MENU_DELETE_, BN_CLICKED, OnDelBtnClicked)
		//MESSAGE_HANDLER(WM_PAINT, on_paint);
		//MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColor )
		//MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColor )
		//COMMAND_HANDLER(IDC_STATIC_IMG_MENU, STN_CLICKED, OnNorImgClicked)
		//COMMAND_HANDLER(IDC_LIST_ALL_MENUITEM, LBN_SELCHANGE, OnListSelChange)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	BEGIN_DDX_MAP( sys_mng_tab ) 
	END_DDX_MAP()

	LRESULT on_show_wnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 0;
	}

	//HBRUSH hBakBr;
	
	LRESULT on_init_dlg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		INT32 ret; 
		//if (1 == GetPropertySheet().GetActiveIndex())
		//	GetPropertySheet().CenterWindow();

		bHandled = TRUE; 
		ret = init_flt_info();
		if( 0 > ret )
		{
			ret = -1; 
			goto err_return; 
		}

		ret = do_ui_ctrl( SEVENFW_UI_ID, SYS_MON_INIT, NULL, 0, NULL, 0, this ); 
		//hBakBr = ::CreateSolidBrush( XCONFIG_DLG_BK_CLR ); 
		if( 0 > ret )
		{
			//::MessageBox( NULL, "sys mon init failed,destroy window\n", NULL, 0 ); 
			DestroyWindow(); 
		}

err_return:
		return ret;  // Let the system set the focus
	}

	LRESULT on_destroy_dlg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		INT32 ret; 
		//if (1 == GetPropertySheet().GetActiveIndex())
		//	GetPropertySheet().CenterWindow();

		ret = do_ui_ctrl( SEVENFW_UI_ID, SYS_MON_UNINIT, NULL, 0, NULL, 0, this ); 

		if( NULL != sys_act_info )
		{
			if( NULL != sys_act_info->TraceMsgTip )
			{
				CloseHandle( sys_act_info->TraceMsgTip );
			}

			free( sys_act_info );
			sys_act_info = NULL;
		}
		//hBakBr = ::CreateSolidBrush( XCONFIG_DLG_BK_CLR ); 
		return ret;  // Let the system set the focus
	}
	
	LRESULT on_paint( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
	{
		bHandled = FALSE;
		//bHandled = TRUE;
		return 0;
	}

BOOL init_flt_info()
{
	INT32 ret;
	DWORD ret_length;

	if( sys_act_info != NULL )
	{
		return 0;
	}

	sys_act_info = ( PHOOK_FILTER_INFO )malloc( MAX_HOOK_INFO_SIZE );

	if( NULL == sys_act_info )
	{
		ret = -1;
		goto _ERROR;
	}

	sys_act_info->Size = 0;
	sys_act_info->TraceMsgTip = CreateEvent( NULL, FALSE, FALSE, NULL ); 

	if( sys_act_info->TraceMsgTip == NULL )
	{
		ret = -1;
		goto _ERROR;
	}

	//ret = DeviceIoControl( SysMgrDevice, 
	//	IOCTL_SET_HOOK_FILTER, 
	//	sys_act_info,
	//	sizeof( HOOK_FILTER_INFO ), 
	//	( PVOID )NULL, 
	//	0, 
	//	&ret_length, 
	//	NULL );

	//if( FALSE == ret )
	//{
	//	goto _ERROR;
	//}

	return 0;

_ERROR:
	if( NULL != sys_act_info )
	{
		if( NULL != sys_act_info->TraceMsgTip )
		{
			CloseHandle( sys_act_info->TraceMsgTip );
		}

		free( sys_act_info );
		sys_act_info = NULL;
	}
	return ret;
}

BOOL SetFiltering( HWND hMainDlg )
{
	HWND CheckBox;
	INT32 ret;
	INT32 CheckRet; 
	DWORD ret_length; 
	ULONG Index;

	ULONG RegMngFilterIndex[] = {
		NtCreateKeyFilter, 
			NtQueryValueKeyFilter, 
			NtDeleteKeyFilter, 
			NtDeleteValueKeyFilter, 
			NtSetValueKeyFilter, 
			NtRestoreKeyFilter, 
			NtOpenKeyFilter
	};

	ULONG ProcessMngFilterIndex[] = {
		NtCreateProcessFilter, 
			NtCreateProcessExFilter, 
			NtCreateUserProcessFilter, 
			NtCreateThreadOrNtCreateThreadExFilter, 
			NtOpenThreadFilter, 
			NtTerminateProcessFilter, 
	};

	ULONG FileMngFilterIndex[] = {
		NtDeleteFileFilter, 
			NtOpenFileFilter, 
			NtCreateFileFilter
	};

	ULONG DrvMngFilterIndex[] = {
		NtLoadDriverFilter 
	};

	FILTER_INDEX_TABLE AllFilterIndexes[] = {
		{ DrvMngFilterIndex, ARRAY_SIZE( DrvMngFilterIndex ) }, 
		{ FileMngFilterIndex, ARRAY_SIZE( FileMngFilterIndex ) }, 
		{ RegMngFilterIndex, ARRAY_SIZE( RegMngFilterIndex ) }, 
		{ ProcessMngFilterIndex, ARRAY_SIZE( ProcessMngFilterIndex ) } 
	};

	ASSERT( NULL != hMainDlg );

	if( NULL == sys_act_info )
	{
		return FALSE;
	}

#define FILTER_CHECK_BOX_ID_BEGIN IDC_CHECK_DRV_MNG
#define FILTER_CHECK_BOX_ID_END IDC_CHECK_PROC_MNG

	ASSERT( FILTER_CHECK_BOX_ID_END - FILTER_CHECK_BOX_ID_BEGIN <= NtApiFilterEnd );

	sys_act_info->Size = 0;
	for( Index = FILTER_CHECK_BOX_ID_BEGIN; Index <= FILTER_CHECK_BOX_ID_END; Index ++ )
	{
		CheckBox = GetDlgItem( Index );

		ASSERT( NULL != CheckBox );
		CheckRet = ( INT32 )SendMessage( CheckBox, BM_GETCHECK, 0, 0 );

		if( CheckRet == BST_CHECKED )
		{
			INT32 i; 
			CHAR Output[ 1024 ];
			for( i = 0; i < AllFilterIndexes[ Index - FILTER_CHECK_BOX_ID_BEGIN ].IndexNumber; i ++ )
			{
				sprintf( Output, "Add filter index: table %d, index %d, value %d \n", Index - FILTER_CHECK_BOX_ID_BEGIN, 
					i, 
					AllFilterIndexes[ Index - FILTER_CHECK_BOX_ID_BEGIN ].FilterIndexes[ i ] ); 
				OutputDebugStringA( Output ); 

				sys_act_info->FilterIndexes[ sys_act_info->Size ] = AllFilterIndexes[ Index - FILTER_CHECK_BOX_ID_BEGIN ].FilterIndexes[ i ];
				sys_act_info->Size += 1;
			}
		}
	}

	if( sys_act_info->Size == 0 )
	{
		return TRUE;
	}

	ret = do_ui_ctrl( SYS_MON_UI_ID, SYS_MON_START, ( PBYTE )sys_act_info, /*sizeof( HOOK_FILTER_INFO ) + sizeof( ULONG ) * sys_act_info->Size */MAX_HOOK_INFO_SIZE, NULL, 0, this ); 
	//ASSERT( TRUE == ret ); 
	return ret;
}

	LRESULT on_ok_bn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		// TODO: �ڴ����ӿؼ�֪ͨ�����������
		INT32 ret; 
		HWND hWnd; 
		hWnd = GetDlgItem( IDOK ); 
		ASSERT( NULL != hWnd ); 

		if( Managing == FALSE )
		{
			INT32 Index; 
			HWND CheckBox; 

			for( Index = FILTER_CHECK_BOX_ID_BEGIN; Index <= FILTER_CHECK_BOX_ID_END; Index ++ )
			{
				CheckBox = GetDlgItem( Index );
				::EnableWindow( CheckBox, FALSE ); 
			}

			SetFiltering( m_hWnd );
			::SetWindowText( hWnd, "Stop" );
			Managing = TRUE;
		}
		else
		{
			INT32 Index; 
			HWND CheckBox; 
			for( Index = FILTER_CHECK_BOX_ID_BEGIN; Index <= FILTER_CHECK_BOX_ID_END; Index ++ )
			{
				CheckBox = GetDlgItem( Index );
				::EnableWindow( CheckBox, TRUE ); 
			}

			ret = do_ui_ctrl( SYS_MON_UI_ID, SYS_MON_STOP, NULL, 0, NULL, 0, this ); 
			//ASSERT( TRUE == ret ); 
			Managing = FALSE; 
			::SetWindowText( hWnd, "Start" );
		}
		return 0; 
	}

protected:
	//CXmlNode* m_pMainMenu;
	//CXmlNode* m_pRootNode;
	//CXml* m_pXmlDoc;
	//CImgListBoxImpl m_ListBox;
	////CBmpStaticImpl m_MenuStc;
	//CButtonXP m_BtnOK;
	//CButtonXP m_BtnCancel;
	//_bstr_t m_strUrl;
	//_bstr_t m_strDesc;
	//_bstr_t m_strCaption;
	////_bstr_t m_strImage;
	//_bstr_t m_strFilter;
	PHOOK_FILTER_INFO sys_act_info;
	INT32 Managing; 
};