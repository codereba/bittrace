/*
 *
 * Copyright 2010 JiJie Shi
 *
 * This file is part of bittrace.
 *
 * bittrace is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * bittrace is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bittrace.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
 
 #pragma once
//#include "menu_ui.h"
#include "md5.h"
#include "msg_box.h"
#include "UIUtil.h"
#include "one_instance.h"
#include "service.h"
#include "bitsafe_daemon.h"
#include "ui_ctrl_mgr.h"
#include "install_conf.h"

LRESULT CALLBACK updating_status_notify( updating_info *info, PVOID param ); 
INLINE LRESULT smart_update_bitsafe( HWND wnd )
{
	LRESULT ret = ERROR_SUCCESS; 
	LPCTSTR tmp_text; 
	ULONG prepare_work; 

	ret = uninstall_bitsafe_by_conf_file( &prepare_work ); 

	ret = unistall_bitsafe(); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "uninstall bitsafe drivers error 0x%0.8x\n", ret ) ); 
	}

	if( prepare_work == PREPARE_BY_RESTART )
	{
		if( ret != ERROR_FILE_NOT_FOUND )
		{
			tmp_text = _get_string_by_id( TEXT_UPDATE_OLD_VERSION_HAVE_NOT_UNINSTALL, 
				_T( "旧版本比特安全没有完成全部卸载,重新运行程序(新版本)时,要先执行卸载(如出现安装提示,先不要执行安装),在保证卸载全部完成后,再重新安装并运行程序。" ) ); 

			show_msg( wnd, tmp_text ); 
			//tip_text->SetText( ); 
		}

		dlg_ret_state ret_state; 
		LPCTSTR tmp_text; 

		tmp_text = _get_string_by_id( TEXT_INSTALL_NEED_RESTART, _T( "由于系统环境的原因,比特安全需要重启系统来完成安装,是否立即重启?" ) ); 

#if 0
		ret = show_msg( NULL, tmp_text, &ret_state ); 
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE ); 
		}

		if( ret_state == OK_STATE )
		{
			windows_shutdown( EWX_REBOOT | EWX_FORCE );  
		}
#endif //0
	}
	return ret; 
}

class update_dlg : public CWindowWnd, public INotifyUI
{
public:
    update_dlg( /*wchar_t* url == NULL*/ ) 
	{
		//if( url == NULL )
		//{
		//	update_url = _T( "" ); 
		//}
		//else
		//{
		//	update_url = url; 
		//}

		can_update = FALSE; 
		updating_file_name = _T(""); 
		update_file_count = 0; 
		updated_file_count = 0; 
		update_handle = NULL; 
		update_reponse = NULL; 
		yes_btn = NULL; 
		no_btn = NULL; 
    };

	VOID set_update_url( LPCWSTR url )
	{
		update_url = url; 
	}

	VOID set_target_path( LPCWSTR target )
	{
		INT32 index; 
		ASSERT( target != NULL ); 
		
		//target_path = _T(""); 
		target_path = target; 
		index = target_path.ReverseFind( _T( '\\' ) ); 
		if( index < 0 )
		{
			goto _return; 
		}
	
		ASSERT( index < target_path.GetLength() ); 

		if( index < target_path.GetLength() - 1 )
		{
			target_path.SetAt( index + 1, _T( '\0' ) ); 
		}

_return:
		return;
	}

	VOID on_check_update( LPCWSTR url )
	{
		update_url = url; 
	}

	VOID on_file_download_tip( LPCWSTR file_name, ULONG file_size, ULONG downloaded )
	{
		LPCTSTR tmp_text; 

#define FILE_DOWNLOAD_TIP_FMT _T( "文件 已下载 %d/%d" )
		CStdString download_tip; 

		if( downloaded == 0 )
		{
			file_progress->SetMaxValue( file_size - 1 ); 
			file_progress->SetMinValue( 0 ); 
		}

		tmp_text = _get_string_by_id( TEXT_UPDATE_FILE_DOWNLOAD_TIP_FMT, 
			FILE_DOWNLOAD_TIP_FMT ); 

		download_tip.Format( tmp_text, 
			//file_name, 
			downloaded, 
			file_size ); 
		
		file_tip_text->SetText( download_tip ); 

		file_progress->SetValue( downloaded ); 
	}

	VOID on_update_failed( update_tip_info *tip_info)
	{
		//wait_update_end( update_handle ); 
		
		LPCTSTR tmp_text; 

		ASSERT( tip_info != NULL ); 

		if( tip_info->err_msg == NULL 
			&& *tip_info->err_msg == L'\0' )
		{
			ASSERT( FALSE ); 

			tmp_text = _get_string_by_id( TEXT_UPDATE_UPDATE_FAILED, L"更新失败" ); 
			tip_info->err_msg = tmp_text; 
		}

		if( IsWindowVisible( GetHWND() ) == FALSE )
		{
			Close(); 
			goto _return; 
		}

		m_pCloseBtn->SetEnabled( true ); 
		tip_text->SetText( tip_info->err_msg ); //请检查因特网连接.
_return:
		return; 
	}

#define UPDATE_DRV_EVENT_NAME _T( "bitsafe_drv_update" )
	LRESULT update_drv_files()
	{
		LRESULT ret = ERROR_SUCCESS; 
		CStdString exe_file_name; 
		HANDLE update_event = NULL; 
		ULONG wait_ret; 

		exe_file_name = _T( "\"" ); 
	
		exe_file_name.Append( target_path ); 
		
		if( exe_file_name.GetAt( exe_file_name.GetLength() - 1 ) != _T( '\\' ) )
		{
			exe_file_name.Append( _T( "\\" ) ); 
		}

		exe_file_name.Append( _T( "bitsafev2.exe\" " ) _T( "/updated" ) ); 

		update_event = CreateEvent( NULL, TRUE, FALSE, UPDATE_DRV_EVENT_NAME ); 
		if( update_event == NULL )
		{
			ret = GetLastError(); 
			goto _return; 
		}

		ret = run_proc_sync( exe_file_name ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		wait_ret = WaitForSingleObject( update_event, INFINITE ); 
		if( wait_ret != WAIT_OBJECT_0 )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

_return:
		if( update_event != NULL )
		{
			CloseHandle( update_event ); 
		}

		return ret; 
	}

	VOID on_update_successful()
	{
		LRESULT ret; 
		LPCTSTR tmp_text; 

		//wait_update_end( update_handle ); 
		m_pCloseBtn->SetEnabled( true ); 
		
		tmp_text = _get_string_by_id( TEXT_UPDATE_UPDATE_SUCCESSFUL, _T( "更新成功" ) ); 

		tip_text->SetText( tmp_text ); 

		ret = update_drv_files(); 

		if( ret != ERROR_SUCCESS )
		{
			ret = init_all_ui_ctrls(); 
			if( ret != ERROR_SUCCESS )
			{
				ASSERT( FALSE ); 

				tmp_text = _get_string_by_id( TEXT_UPDATE_OLD_VERSION_HAVE_NOT_UNINSTALL, 
					_T( "旧版本比特安全没有完成全部卸载,重新运行程序(新版本)时,要先执行卸载(如出现安装提示,先不要执行安装),在保证卸载全部完成后,再重新安装并运行程序。" ) ); 
				
				show_msg( GetHWND(), tmp_text ); 
				goto _return; 
			}

			smart_update_bitsafe( GetHWND() ); 
		}

_return:
		return; 
	}

	LRESULT on_file_updated( LPCWSTR file_name )
	{
		LRESULT ret = ERROR_SUCCESS;
		LPCTSTR tmp_text; 

		CStdString _file_name; 
		updating_file_name = file_name; 
		file_name = wcsrchr( file_name, L'\\' ); 
		if( file_name == NULL )
		{
			file_name = L""; 
		}

		file_name += 1; 
		
		tmp_text = _get_string_by_id( TEXT_UPDATE_UPDATING_FILE_NO_TIP, _T( "正在更新第%d个文件%s\n" ) ); 

		_file_name.Format( tmp_text, updated_file_count + 1, file_name ); 
		tip_text->SetText( _file_name ); 

		whole_progress->SetValue( updated_file_count ++ );
	
		return ret; 
	}
	
	ULONG get_update_file_num()
	{
		return update_file_count; 
	}

	LRESULT on_no_need_update( update_tip_info *info )
	{
		LRESULT ret = ERROR_SUCCESS; 
		CStdString tip; 
		LPWSTR version = NULL; 
		LPCTSTR tmp_text; 

		ASSERT( tip_text != NULL ); 

		version = StringConvertor::Utf8ToWide( info->version ); 
		
		//DBG_BP(); 

#ifdef _DEBUG
		tmp_text = _get_string_by_id( TEXT_UPDATE_ALREADY_NEWEST_VERSION_TIP, 
			_T( "已经是最新的版本 %s, 不需要进行更新" ) ); 
		
		tip.Format( tmp_text, version );

		tip_text->SetText( tip ); 

		ShowWindow(); 
#else
		PostQuitMessage( 0 ); 
		//Close(); 
#endif //_DEBUG
_return:
		if( version != NULL )
		{
			StringConvertor::StringFree( version ); 
		}

		return ret; 
	}

	LRESULT on_update_starting( update_tip_info *info )
	{
		LRESULT ret = ERROR_SUCCESS; 
		LPWSTR version = NULL; 
		LPWSTR msg = NULL; 
		CStdString tip; 
		LPCTSTR tmp_text; 
	
		ASSERT( info != NULL ); 

		update_file_count = info->count; 

		version = StringConvertor::Utf8ToWide( info->version ); 
		if( version == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; 
		}

		msg = StringConvertor::Utf8ToWide( info->msg ); 
		if( msg == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; 
		}

		tmp_text = _get_string_by_id( TEXT_UPDATE_NEWER_VERSION_CHECKED, 
			_T( "检测到更新的版本 %s (%s), 是否进行更新?" ) ); 
		
		tip.Format( tmp_text, version, msg );

		ShowWindow(); 

		tip_text->SetText( tip ); 

		ASSERT( yes_btn != NULL ); 
		ASSERT( no_btn != NULL ); 

		yes_btn->SetEnabled( true ); 
		no_btn->SetEnabled( true ); 

		WaitForSingleObject( update_reponse, INFINITE ); 

		yes_btn->SetEnabled( false ); 
		no_btn->SetEnabled( false ); 


		if( can_update == TRUE )
		{
			ret = stop_service( SERVICE_NAME ); ;
			if( ret != ERROR_SUCCESS )
			{
				log_trace( ( MSG_INFO, "service stop failed \n" ) ); 
				goto _return; 
			}

			ret = kill_app_instance( FALSE ); 
			if( ret != ERROR_SUCCESS 
				&& ret != ERROR_MAIN_WND_PROC_NOT_FOUND  )
			{
				goto _return; 
			}

			ret = ERROR_SUCCESS; 

			whole_progress->SetMinValue( 0 ); 
			whole_progress->SetMaxValue( update_file_count - 1 ); 

			tmp_text = _get_string_by_id( TEXT_UPDATE_ALL_FILE_COUNT_TIP, _T( "开始更新,共有%d个文件" ) ); 

			tip.Format( tmp_text, update_file_count ); 
			tip_text->SetText(  tip ); 
		}
		else
		{
			PostQuitMessage( 0 ); 
		}

_return:
		if( version != NULL )
		{
			StringConvertor::StringFree( version ); 
		}

		if( msg != NULL )
		{
			StringConvertor::StringFree( msg ); 
		}

		return ret; 
	}

	VOID inc_updated_file_num()
	{
		updated_file_count ++; 
	}

	LPCWSTR get_update_url()
	{
		return ( LPCWSTR )update_url.GetData(); 
	}

    LPCTSTR GetWindowClassName() const 
    { 
        return _T("UpdateDlg"); 
    };

    UINT GetClassStyle() const
    { 
        return CS_DBLCLKS; 
    };

    void OnFinalMessage(HWND /*hWnd*/) 
    { 
    };

    void Init() 
    {
		LRESULT ret = ERROR_SUCCESS; 
		CHAR *_update_url = NULL;
		CHAR *_target_path = NULL; 

        m_pCloseBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("close_btn")));
		ASSERT( m_pCloseBtn != NULL ); 
		m_pCloseBtn->SetEnabled( false ); 

		yes_btn = static_cast<CButtonUI*>(m_pm.FindControl(_T("yes_btn")));
		ASSERT( m_pCloseBtn != NULL ); 
		yes_btn->SetEnabled( false ); 

		no_btn = static_cast<CButtonUI*>(m_pm.FindControl(_T("no_btn")));
		ASSERT( m_pCloseBtn != NULL ); 
		no_btn->SetEnabled( false ); 

		tip_text = static_cast<CControlUI*>(m_pm.FindControl(_T("text_tip")));
		ASSERT( tip_text != NULL ); 

		file_tip_text = static_cast<CControlUI*>(m_pm.FindControl(_T("file_text_tip")));
		ASSERT( file_tip_text != NULL ); 
		
		whole_progress = static_cast<CProgressUI*>(m_pm.FindControl(_T("whole_progress")));
		ASSERT( whole_progress != NULL ); 
		
		file_progress = static_cast<CProgressUI*>(m_pm.FindControl(_T("file_progress")));
		ASSERT( file_progress != NULL ); 

		if( update_url.IsEmpty() )
		{
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}

		_update_url = StringConvertor::WideToAnsi( update_url.GetData() ); 
		if( _update_url == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; 
		}

		_target_path = StringConvertor::WideToAnsi( target_path.GetData() ); 
		if( _target_path == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; 
		}

		update_handle = start_auto_update( _update_url, _target_path, updating_status_notify, ( PVOID )this ); 
		if( update_handle == NULL )
		{
			log_trace( ( MSG_ERROR, "start update thread failed \n" ) ); 

			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		update_reponse = CreateEvent( NULL, FALSE, FALSE, NULL ); 

		if( update_reponse == NULL )
		{
			log_trace( ( MSG_ERROR, "create response event failed 0x%0.8x \n", GetLastError() ) ); 

			ret = GetLastError(); 
			goto _return; 
		}

_return:
		if( _update_url != NULL )
		{
			StringConvertor::StringFree( _update_url ); 
		}

		if( _target_path != NULL )
		{
			StringConvertor::StringFree( _target_path ); 
		}

		if( ret != ERROR_SUCCESS )
		{

			Close(); 
		}

		return; 
	}

    void OnPrepare(TNotifyUI& msg) 
    {

    }

    void Notify(TNotifyUI& msg)
    {
        if( msg.sType == _T("windowinit") ) 
            OnPrepare(msg);
        else if( msg.sType == _T("click") ) 
        {
            if( msg.pSender == m_pCloseBtn ) 
            {
				Close(); 
                return; 
            }
			else
			{
				CStdString name = msg.pSender->GetName(); 

				if( name == _T( "yes_btn" ) )
				{
					can_update = TRUE; 
					ASSERT( update_reponse != NULL ); 
					SetEvent( update_reponse ); 
				}
				else if( name == _T( "no_btn" ) )
				{
					can_update = FALSE; 
					Close(); 
				}
			}
        }
        else if(msg.sType==_T("setfocus"))
        {
        }
        else if( msg.sType == _T("itemclick") ) 
        {
        }
        else if( msg.sType == _T("itemactivate") ) 
        {

        }
        else if(msg.sType == _T("menu")) 
        {

        }
        else if( msg.sType == _T("menu_Delete") ) 
		{
        }
    }

    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
		set_app_flag( GetHWND(), TRUE ); 

        LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
        styleValue &= ~WS_CAPTION;
        ::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
        m_pm.Init(m_hWnd);
        //m_pm.SetTransparent(100);
        CDialogBuilder builder;
        CControlUI* pRoot = builder.Create(_T("update.xml"), (UINT)0, NULL, &m_pm);
        ASSERT(pRoot && "Failed to parse XML");
        m_pm.AttachDialog(pRoot);
        m_pm.AddNotifier(this); 
		set_ui_style( &m_pm ); 

        Init();
        return 0;
    }

    LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        bHandled = FALSE;

		if( update_handle != NULL )
		{
			CloseHandle( update_handle ); 
			update_handle = NULL; 
		}

		if( update_reponse != NULL )
		{
			CloseHandle( update_reponse ); 
			update_reponse = NULL; 
		}

        return 0;
    }

    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        bHandled = FALSE; 

		CStdString file_name; 

		log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 
		if( !CPaintManagerUI::GetResourceZip().IsEmpty() )
		{
			BOOL ret; 
			file_name = CPaintManagerUI::GetResourcePath(); 
			file_name += CPaintManagerUI::GetResourceZip(); 
			ret = DeleteFile( file_name.GetData() );
			
			log_trace( ( MSG_INFO, "delete zip file in update dialog 0x%0.8x\n", ret ) ); 
		}
		remove_app_flag( GetHWND(), FALSE ); 

		log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

		PostQuitMessage( 0 ); 
        return 0;
    }

    LRESULT OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        if( ::IsIconic(*this) ) bHandled = FALSE;
        return (wParam == 0) ? TRUE : FALSE;
    }

    LRESULT OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        return 0;
    }

    LRESULT OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        return 0;
    }

    LRESULT OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        POINT pt; pt.x = GET_X_LPARAM(lParam); pt.y = GET_Y_LPARAM(lParam);
        ::ScreenToClient(*this, &pt);

        RECT rcClient;
        ::GetClientRect(*this, &rcClient);

        if( !::IsZoomed(*this) ) {
            RECT rcSizeBox = m_pm.GetSizeBox();
            if( pt.y < rcClient.top + rcSizeBox.top ) {
                if( pt.x < rcClient.left + rcSizeBox.left ) return HTTOPLEFT;
                if( pt.x > rcClient.right - rcSizeBox.right ) return HTTOPRIGHT;
                return HTTOP;
            }
            else if( pt.y > rcClient.bottom - rcSizeBox.bottom ) {
                if( pt.x < rcClient.left + rcSizeBox.left ) return HTBOTTOMLEFT;
                if( pt.x > rcClient.right - rcSizeBox.right ) return HTBOTTOMRIGHT;
                return HTBOTTOM;
            }
            if( pt.x < rcClient.left + rcSizeBox.left ) return HTLEFT;
            if( pt.x > rcClient.right - rcSizeBox.right ) return HTRIGHT;
        }

        RECT rcCaption = m_pm.GetCaptionRect();
        if( pt.x >= rcClient.left + rcCaption.left && pt.x < rcClient.right - rcCaption.right \
            && pt.y >= rcCaption.top && pt.y < rcCaption.bottom ) {
                CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(pt));
                if( pControl && _tcscmp(pControl->GetClass(), _T("ButtonUI")) != 0 && 
                    _tcscmp(pControl->GetClass(), _T("OptionUI")) != 0 &&
                    _tcscmp(pControl->GetClass(), _T("TextUI")) != 0 )
                    return HTCAPTION;
        }

        return HTCLIENT;
    }

    LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        SIZE szRoundCorner = m_pm.GetRoundCorner();
        if( !::IsIconic(*this) && (szRoundCorner.cx != 0 || szRoundCorner.cy != 0) ) {
            CRect rcWnd;
            ::GetWindowRect(*this, &rcWnd);
            rcWnd.Offset(-rcWnd.left, -rcWnd.top);
            rcWnd.right++; rcWnd.bottom++;
            RECT rc = { rcWnd.left, rcWnd.top + szRoundCorner.cx, rcWnd.right, rcWnd.bottom };
            HRGN hRgn1 = ::CreateRectRgnIndirect( &rc );
            HRGN hRgn2 = ::CreateRoundRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom - szRoundCorner.cx, szRoundCorner.cx, szRoundCorner.cy);
            ::CombineRgn( hRgn1, hRgn1, hRgn2, RGN_OR );
            ::SetWindowRgn(*this, hRgn1, TRUE);
            ::DeleteObject(hRgn1);
            ::DeleteObject(hRgn2);
        }

        bHandled = FALSE;
        return 0;
    }

    LRESULT OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        MONITORINFO oMonitor = {};
        oMonitor.cbSize = sizeof(oMonitor);
        ::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTOPRIMARY), &oMonitor);
        CRect rcWork = oMonitor.rcWork;
        rcWork.Offset(-rcWork.left, -rcWork.top);

        LPMINMAXINFO lpMMI = (LPMINMAXINFO) lParam;
        lpMMI->ptMaxPosition.x	= rcWork.left;
        lpMMI->ptMaxPosition.y	= rcWork.top;
        lpMMI->ptMaxSize.x		= rcWork.right;
        lpMMI->ptMaxSize.y		= rcWork.bottom;

        bHandled = FALSE;
        return 0;
    }

    LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        // 有时会在收到WM_NCDESTROY后收到wParam为SC_CLOSE的WM_SYSCOMMAND
        if( wParam == SC_CLOSE ) {
            ::PostQuitMessage(0L);
            bHandled = TRUE;
            return 0;
        }
        BOOL bZoomed = ::IsZoomed(*this);
        LRESULT lRes = CWindowWnd::HandleMessage(uMsg, wParam, lParam);
        if( ::IsZoomed(*this) != bZoomed ) {
            if( !bZoomed ) {
                CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("max_btn")));
                if( pControl ) pControl->SetVisible(false);
                pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("nor_btn")));
                if( pControl ) pControl->SetVisible(true);
            }
            else {
                CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("max_btn")));
                if( pControl ) pControl->SetVisible(true);
                pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("nor_btn")));
                if( pControl ) pControl->SetVisible(false);
            }
        }
        return lRes;
    }

    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        LRESULT lRes = 0;
        BOOL bHandled = TRUE;
        switch( uMsg ) {
            case WM_CREATE:        lRes = OnCreate(uMsg, wParam, lParam, bHandled); break;
            case WM_CLOSE:         lRes = OnClose(uMsg, wParam, lParam, bHandled); break;
            case WM_DESTROY:       lRes = OnDestroy(uMsg, wParam, lParam, bHandled); break;
            case WM_NCACTIVATE:    lRes = OnNcActivate(uMsg, wParam, lParam, bHandled); break;
            case WM_NCCALCSIZE:    lRes = OnNcCalcSize(uMsg, wParam, lParam, bHandled); break;
            case WM_NCPAINT:       lRes = OnNcPaint(uMsg, wParam, lParam, bHandled); break;
            case WM_NCHITTEST:     lRes = OnNcHitTest(uMsg, wParam, lParam, bHandled); break;
            case WM_SIZE:          lRes = OnSize(uMsg, wParam, lParam, bHandled); break;
            case WM_GETMINMAXINFO: lRes = OnGetMinMaxInfo(uMsg, wParam, lParam, bHandled); break;
            case WM_SYSCOMMAND:    lRes = OnSysCommand(uMsg, wParam, lParam, bHandled); break;
            default:
                bHandled = FALSE;
        }
        if( bHandled ) return lRes;
        if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;
        return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
    }
public:
    CPaintManagerUI m_pm;

private:
    CButtonUI* m_pCloseBtn;
	CControlUI *tip_text; 
	CControlUI *file_tip_text; 
	CControlUI *yes_btn; 
	CControlUI *no_btn; 
	CProgressUI *whole_progress; 
	CProgressUI *file_progress; 
	CStdString update_url; 
	CStdString target_path; 
	CStdString updating_file_name; 
	ULONG update_file_count; 
	ULONG updated_file_count; 
	PVOID update_handle; 
	HANDLE update_reponse; 
	INT32 can_update; 
    //...
};