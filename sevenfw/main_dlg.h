/*
 * Copyright 2010 JiJie Shi
 *
 * This file is part of NetMonitor.
 *
 * NetMonitor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * NetMonitor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with NetMonitor.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 *  2007.6.10	Ji Jie Shi modified it for this ui lib. 
 */

 
 #ifndef __MAIN_DLG_H__
#define __MAIN_DLG_H__

#include "sys_mng_api.h"
#include "seven_fw_api.h"

#include "http_url_flt_tab.h"
#include "net_mon_tab.h"
#include "sys_mng_tab.h"
#include "ui_base_tab.h"
#include "anti_arp_tab.h"
#include "http_txt_flt_tab.h"

#define XCONFIG_DLG_BK_CLR RGB( 240, 240, 240 )
#define IDM_ABOUTBOX 0x0010
#define IDM_INSTALL 0x0020
#define IDM_UNINSTALL 0x0030

#define WM_CTLCOLOR                             0x0019
class tab_ctrl_wnd : public CWindowImpl<tab_ctrl_wnd, CTabCtrlT< ui_base_tab > >
{
public:
	tab_ctrl_wnd()
	{
	}

	~tab_ctrl_wnd()
	{
	}

	BEGIN_MSG_MAP(tab_ctrl_wnd)
		//XLOG( ( 0,  __t( "Current tab_ctrl_wnd recved msg is %u\r\n" ), uMsg ) );
		//MESSAGE_HANDLER(WM_CTLCOLOR, OnCtlColor)
		//MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkGnd)
		//MESSAGE_HANDLER(WM_PAINT, on_paint)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	
	LRESULT on_paint( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
	{
		bHandled = FALSE;
		CPaintDC DrawDC( m_hWnd );
		DrawDC.FillRect( &CRect( 0, 0, 100, 100 ), RGB( 100, 100, 100 ) );
		DWORD dwStyle;
		dwStyle = GetWindowLong( GWL_STYLE );
		ATLTRACE( __t( "tab_ctrl_wnd style is %u now in self paint\r\n" ), dwStyle );
		return 0;
	}

	LRESULT OnEraseBkGnd( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
	{
		bHandled = FALSE;
		RECT rcTab;
		CBrush BkBr;
		CDC DC;
		BkBr.CreateSolidBrush( XCONFIG_DLG_BK_CLR );
		GetWindowRect( &rcTab );
		DC = GetDC();
		DC.FillRect( &rcTab, BkBr );
		bHandled = TRUE;
		return 0;
	}
};

class about_dlg: public CDialogImpl< about_dlg, CWindow >, 
	public COwnerDraw< about_dlg >, 
	public CWinDataExchange< about_dlg >
{
public:
	about_dlg()
	{
	}

	virtual ~about_dlg()
	{
	}

	enum { IDD = IDD_ABOUTBOX };

	BEGIN_MSG_MAP(about_dlg)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, on_bn_cancel)
	END_MSG_MAP()

	BEGIN_DDX_MAP(about_dlg)
	END_DDX_MAP()

	LRESULT on_bn_cancel( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
	{
		bHandled = FALSE;
		EndDialog( FALSE );
		bHandled = TRUE;
		return 0;
	}
}; 

class main_dlg : public CDialogImpl< main_dlg, CWindow >, 
	public COwnerDraw< main_dlg >, 
	public CWinDataExchange< main_dlg >
{

public:
	main_dlg()
	{
	}

	virtual ~main_dlg()
	{
	}

// 对话框数据
	enum { IDD = IDD_MAIN_DLG };

	BEGIN_MSG_MAP(main_dlg)
		//XLOG( ( 0, __t( "Current main_dlg recved msg is %u\r\n" ), uMsg ) );
		MESSAGE_HANDLER(WM_INITDIALOG, on_init_dlg)
		MESSAGE_HANDLER( WM_QUERYDRAGICON, on_query_drag_icon )
		MESSAGE_HANDLER( WM_DESTROY, on_destroy )
		MESSAGE_HANDLER( WM_SYSCOMMAND, on_sys_cmd )
		//MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColor)
		MESSAGE_HANDLER(WM_PAINT, on_paint)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, on_bn_cancel)
		MESSAGE_HANDLER( WM_CHAR, on_key_down ) 
		NOTIFY_HANDLER(IDC_7FW_FUNCS_TAB, TCN_SELCHANGE, on_tab_sel_change)
		//if( uMsg == WM_DRAWITEM )
		//{
		//	HANDLE hWndChild;
		//	if(wParam)	// not from a menu
		//		hWndChild = ((LPDRAWITEMSTRUCT)lParam)->hwndItem;
		//	XLOG( ( 0, __t( "draw item window handle is %u\r\n" ), hWndChild ) );
		//}
		//REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	BEGIN_DDX_MAP(main_dlg)
	END_DDX_MAP()

	LRESULT on_key_down(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		BOOL tmp; 
		bHandled = FALSE; 

		::MessageBox( NULL, "key down\n", NULL, 0 ); 
		switch( wParam )
		{
		case VK_RETURN:
			//on_ok_bn( NULL, IDOK, NULL, tmp ); 
			break; 
		}

		return 0; 
	}

	LRESULT on_paint( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
	{
		bHandled = FALSE;
		if (IsIconic())
		{
			CPaintDC dc(m_hWnd); // 用于绘制的设备上下文

			SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.m_hDC), 0);

			// 使图标在工作矩形中居中
			int cxIcon = GetSystemMetrics(SM_CXICON);
			int cyIcon = GetSystemMetrics(SM_CYICON);
			CRect rect;
			GetClientRect(&rect);
			int x = (rect.Width() - cxIcon + 1) / 2;
			int y = (rect.Height() - cyIcon + 1) / 2;

			// 绘制图标
			dc.DrawIcon(x, y, m_hIcon);
			bHandled = TRUE;
		}
		return 0;
	}

#define HTTP_URL_FLT_TAB_INDEX  0
//#define HTTP_TXT_FLT_TAB_INDEX  1
#define ANTI_ARP_TAB_INDEX  1
#define SYS_ACT_MNG_TAB_INDEX  2
#define NET_MON_TAB_INDEX  3

	LRESULT on_query_drag_icon(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = TRUE; 
		return reinterpret_cast< LRESULT >( m_hIcon ); 
	}

	LRESULT on_init_dlg( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
	{
		INT32 i; 
		CMenuHandle pSysMenu = GetSystemMenu(FALSE);
		ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
		ASSERT(IDM_ABOUTBOX < 0xF000);

		m_hIcon = AtlLoadIcon( IDI_MAIN_ICON ); 
		ASSERT( m_hIcon != NULL ); 

		if (pSysMenu != NULL)
		{
			CString strAboutMenu;
			strAboutMenu.LoadString(IDS_ABOUTBOX);
			if (!strAboutMenu.IsEmpty())
			{
				pSysMenu.AppendMenu( MF_SEPARATOR, ( UINT_PTR )0, ( LPCTSTR ) NULL );
				pSysMenu.AppendMenu( MF_STRING, ( UINT_PTR )IDM_INSTALL, ( LPCTSTR )"安装7层防火墙(&I)" );
				pSysMenu.AppendMenu( MF_STRING, ( UINT_PTR )IDM_UNINSTALL, ( LPCTSTR )"卸载7层防火墙(&U)" );

				pSysMenu.AppendMenu( MF_SEPARATOR, ( UINT_PTR )0, ( LPCTSTR )NULL );
				pSysMenu.AppendMenu( MF_STRING, ( UINT_PTR )IDM_ABOUTBOX, "About..." );
			}
		}

		SetIcon(m_hIcon, TRUE);
		SetIcon(m_hIcon, FALSE);

		pchar_t all_tab_text[] = { 
			str_http_url_flt, 
			//str_http_txt_flt, 
			str_anti_arp_status, 
			str_sys_act_mng, 
			str_net_mon }; 

		bHandled = FALSE;
		TCITEM tab_item;
		sub_tabs.SubclassWindow( GetDlgItem( IDC_7FW_FUNCS_TAB ) );
		ATLASSERT( NULL != sub_tabs.m_pfnSuperWindowProc  );
		tab_item.mask = TCIF_TEXT | TCIF_IMAGE;  
		tab_item.iImage = -1; 

		for( i = 0; i < ARRAY_SIZE( all_tab_text ); i ++ )
		{
			tab_item.pszText = get_lang_string( all_tab_text[ i ] );  
			sub_tabs.AddItem( &tab_item );
		}

		sub_tabs.GetWindowRect( &sub_tab_rc );
		ScreenToClient( &sub_tab_rc );
		sub_tab_rc.top += SM_CYVSCROLL + SM_CYBORDER;
		sub_tab_rc.left += SM_CXBORDER;
		sub_tab_rc.right -= SM_CXBORDER;
		sub_tab_rc.bottom -= SM_CYBORDER; 

		//BOOL bHandled;
		on_tab_sel_change( 0, 0, bHandled );

		bHandled = FALSE;
		return TRUE;
	}

#define DEBUG_SINGLE_TAB HTTP_URL_FLT_TAB_INDEX
	LRESULT on_tab_sel_change(INT32 wParam, LPNMHDR lpNMHdr, BOOL &bHandled)
	{ 
		bHandled = FALSE;
		INT32 tab_sel = sub_tabs.GetCurSel();
		
		if( NULL != sel_tab )
		{
			::ShowWindow( sel_tab, SW_HIDE );
			::EnableWindow( sel_tab, FALSE );
		}

		// Create the new child dialog box. 
		switch( tab_sel )
		{
//		case HTTP_TXT_FLT_TAB_INDEX:
//#if DEBUG_SINGLE_TAB == HTTP_TXT_FLT_TAB_INDEX
//			if( NULL == http_txt_flt.m_hWnd )
//			{
//				if( NULL != http_txt_flt.Create( m_hWnd ) )
//				{
//					http_txt_flt.MoveWindow( &sub_tab_rc, FALSE );
//					http_txt_flt.ShowWindow( SW_SHOW );
//					Invalidate( FALSE ); 
//				}
//
//			}
//			else
//			{
//				http_txt_flt.ShowWindow( SW_SHOW );
//				http_txt_flt.EnableWindow( TRUE );
//			}
//
//			sel_tab = http_txt_flt.m_hWnd; 
//#endif
//			break;
		case HTTP_URL_FLT_TAB_INDEX:
#if DEBUG_SINGLE_TAB == HTTP_URL_FLT_TAB_INDEX
			if( NULL == http_url_flt.m_hWnd )
			{
				if( NULL != http_url_flt.Create( m_hWnd ) )
				{
					http_url_flt.MoveWindow( &sub_tab_rc, FALSE );
					http_url_flt.ShowWindow( SW_SHOW );
					Invalidate( FALSE ); 
				}
			}
			else
			{
				http_url_flt.ShowWindow( SW_SHOW );
				http_url_flt.EnableWindow( TRUE );
			}

			sel_tab = http_url_flt.m_hWnd; 
#endif
			break;
		case ANTI_ARP_TAB_INDEX:
//#if DEBUG_SINGLE_TAB == ANTI_ARP_TAB_INDEX
			if( NULL == anti_arp.m_hWnd )
			{
				if( NULL != anti_arp.Create( m_hWnd ) )
				{
					anti_arp.MoveWindow( &sub_tab_rc, FALSE );
					anti_arp.ShowWindow( SW_SHOW );
					Invalidate( FALSE ); 
				}
			}
			else
			{
				anti_arp.ShowWindow( SW_SHOW );
				anti_arp.EnableWindow( TRUE );
			}

			sel_tab = anti_arp.m_hWnd; 
//#endif
			break;
		case NET_MON_TAB_INDEX:
//#if DEBUG_SINGLE_TAB == NET_MON_TAB_INDEX
			if( NULL == net_mon.m_hWnd )
			{
				if( NULL != net_mon.Create( m_hWnd ) )
				{
					net_mon.MoveWindow( &sub_tab_rc, FALSE );
					net_mon.ShowWindow( SW_SHOW );
					Invalidate( FALSE ); 
				}
			}
			else
			{
				net_mon.ShowWindow( SW_SHOW );
				net_mon.EnableWindow( TRUE );
			}

			sel_tab = net_mon.m_hWnd; 
//#endif
			break;
		case SYS_ACT_MNG_TAB_INDEX:
//#if DEBUG_SINGLE_TAB == SYS_ACT_MNG_TAB_INDEX
			if( NULL == sys_mng.m_hWnd )
			{
				if( NULL != sys_mng.Create( m_hWnd ) )
				{
					sys_mng.MoveWindow( &sub_tab_rc, FALSE );
					sys_mng.ShowWindow( SW_SHOW );
					Invalidate( FALSE ); 
				}
			}
			else
			{
				sys_mng.ShowWindow( SW_SHOW );
				sys_mng.EnableWindow( TRUE );
			}

			sel_tab = sys_mng.m_hWnd; 
//#endif
			break;
		default:
			break;
		}

		bHandled = TRUE;
		return TRUE;
	} 

	LRESULT on_bn_cancel( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
	{
		bHandled = FALSE;
		EndDialog( FALSE );
		bHandled = TRUE;
		return 0;
	}

public:

	LRESULT on_sys_cmd( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
	{
		INT32 ret = 0; 
		bHandled = TRUE; 

		//CString output;
		//output.Format( " function %s id %d", __FUNCTION__, LOWORD( wParam) ); 
		//MessageBox(output ); 

		switch( LOWORD( wParam ) )
		{
		case IDM_ABOUTBOX:
			{
				about_dlg about_box; 
				about_box.DoModal(); 
			}
			break; 
		case IDM_UNINSTALL:
			{
				ret = do_ui_ctrl( HTTP_URL_FLT_UI_ID, 
					HTTP_URL_FLT_UNINSTALL_7FW, 
					NULL, 
					0, 
					NULL, 
					0, 
					this ); 
				if( ret < 0 )
				{
					dbg_print( 0, DBG_CONSOLE_OUT, "uninstall 7fw failed\n" ); 					
				}

				ret = do_ui_ctrl( SYS_MON_UI_ID, SYS_MON_UNINIT, NULL, 0, NULL, 0, this ); 
				
				DestroyWindow();
				PostQuitMessage( 0 );
				break; 
			}
		case IDM_INSTALL:
			{
				ret = do_ui_ctrl( HTTP_URL_FLT_UI_ID, 
					HTTP_URL_FLT_INSTALL_7FW, 
					NULL, 
					0, 
					NULL, 
					0, 
					this ); 
				break; 
			} 
		default:
			bHandled = FALSE; 
			break; 
		}
		
		return 0; 
	}

	LRESULT on_destroy( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
	{
		return 0;
	}


protected:
	tab_ctrl_wnd sub_tabs; 
	HWND sel_tab; 
	RECT sub_tab_rc; 
	http_txt_flt_tab http_txt_flt; 
	http_url_flt_tab http_url_flt; 
	anti_arp_tab anti_arp; 
	net_mon_tab net_mon; 
	sys_mng_tab sys_mng; 
	HICON m_hIcon; 
};

//class CHelloDlg : public winx::ModelDialog<CHelloDlg, IDD_MAIN_DLG>
//{
//
//
//	//winx::DimEditCtrl*	m_decHint;
//	//winx::DimEditCtrl*	m_decPassword;
//	//winx::DimEditCtrl*	m_decUsername;
//	
//public:
//	BOOL on_init_dlg(HWND hDlg, HWND hWndDefaultFocus)
//	{
//		//winx::SubclassDlgItem(&m_decUsername, hDlg, IDC_EDIT1);
//		//winx::SubclassDlgItem(&m_decPassword, hDlg, IDC_EDIT2);
//		//winx::SubclassDlgItem(&m_decHint, hDlg, IDC_EDIT3);
//
//		//
//		//	Set Text Limits...
//		//
//		//m_decUsername->LimitText( 32 );		
//		//m_decPassword->LimitText( 32 );
//		//m_decHint->LimitText( 128 );
//		//
//		////
//		////	Set Dim Text...
//		////
//		//m_decUsername->SetDimText( __t( "Username Required (Max 32 Characters)" ) );
//		//m_decPassword->SetDimText( __t( "Password Required (Max 32 Characters)" ) );
//		//m_decHint->SetDimText( __t( "Hint Optional (Max 128 Characters)" ) );
//		//
//		////
//		////	Override Dim Colors For Two Controos...
//		////
//		//m_decUsername->SetDimColor( RGB( 0xFF, 0x80, 0x80 ) );
//		//m_decPassword->SetDimColor( RGB( 0xFF, 0x80, 0x80 ) );
//		
//		return TRUE;
//	}
//};

#endif //__MAIN_DLG_H__