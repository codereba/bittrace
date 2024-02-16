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
 
#ifndef __ACTION_SHOW_DLG_H__
#define __ACTION_SHOW_DLG_H__

#include "ring0_2_ring3.h"
#include "ui_config.h"
#include "wnd_state.h"
#include "acl_define.h"
#include "action_type.h"
#include "action_check.h"
#include "action_display.h"
#include "action_analyze_define.h"
#include <winsock2.h>

#define UI_DISPLAY_LAYOUT_NAME L"text_layout"
typedef struct _ACTION_INFO
{
	sys_action_record *action; 
	action_context *ctx; 
	PVOID data;  
	ULONG data_len; 
} ACTION_INFO, *PACTION_INFO; 

#define DATA_NEED_CLONE 0x00000001

class UILIB_API CDynamicShape : public CTextUI
{
public:
	CDynamicShape( ULONG mode )
	{
		action_id = INVALID_ACTION_ID; 
		memset( &action_info, 0, sizeof( action_info ) ); 
		m_uButtonState = 0; 
		work_mode = mode; 
	}

	~CDynamicShape()
	{
		if( work_mode & DATA_NEED_CLONE )
		{
			if( action_info.action != NULL )
			{
				free( action_info.action ); 
				action_info.action = NULL; 
			}

			if( action_info.ctx != NULL )
			{
				free( action_info.ctx ); 
				action_info.ctx = NULL; 
			}

			if( action_info.data != NULL )
			{
				free( action_info.data ); 
				action_info.data = NULL; 
			}

			action_info.data_len = 0; 
		}
	}

	void PaintText(HDC hDC)
	{
		CTextUI::PaintText( hDC ); 
	}

	ULONG get_action_id()
	{
		return action_id; 
	}

	VOID set_action_id( ULONG id )
	{
		action_id = id; 
	}

	LRESULT set_action_info( sys_action_record *action, action_context *ctx, PVOID data, ULONG data_len )
	{
		LRESULT ret = ERROR_SUCCESS; 

		if( work_mode & DATA_NEED_CLONE )
		{
			sys_action_record *_action = NULL; 
			action_context *_ctx;
			PVOID _data; 
			//ULONG _data_Len; 

			_action = ( sys_action_record* )malloc( sizeof( sys_action_record ) ); 
			if( _action != NULL )
			{
				memcpy( _action, action, sizeof( *action ) );
			}
			else
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}

			_ctx = ( action_context* )malloc( sizeof( action_context ) ); 
			if( _ctx != NULL )
			{
				memcpy( _ctx, ctx, sizeof( *ctx ) ); 
			}
			else
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}

			if( data_len == 0 )
			{
				_data = NULL; 
			}
			else
			{
				_data = malloc( data_len ); 
				if( _data != NULL )
				{
					memcpy( _data, data, data_len ); 
				}
				else
				{
					ret = ERROR_NOT_ENOUGH_MEMORY; 
				}
			}

			action_info.action = _action; 
			action_info.ctx = _ctx; 
			action_info.data = _data; 
			action_info.data_len = data_len; 

		}
		else
		{
			action_info.action = action; 
			action_info.ctx = ctx; 
			action_info.data = data; 
			action_info.data_len = data_len; 
		}

		return ret; 
	}
	
	LRESULT get_data( PVOID *data, ULONG *data_len )
	{
		LRESULT ret = ERROR_SUCCESS; 

		do 
		{
			ASSERT( data != NULL ); 
			ASSERT( data_len != NULL ); 

			*data = action_info.data; 
			*data_len = action_info.data_len; 

		}while( FALSE );
		
		return ret; 
	}

	LPCTSTR GetClass() const
	{
		return _T( "CDynamicShape" ); 
	}
//	LPVOID GetInterface(LPCTSTR pstrName);
//	UINT GetControlFlags() const;
//

	bool Activate();
//	void SetEnabled(bool bEnable = true);
	void DoEvent(TEventUI& event);
//
//	HANDLE GetNormalImage();
//	void SetNormalImage(HANDLE Image);
//	HANDLE GetHotImage();
//	void SetHotImage(HANDLE Image);
//	HANDLE GetPushedImage();
//	void SetPushedImage(HANDLE Image);
//	HANDLE GetFocusedImage();
//	void SetFocusedImage(HANDLE Image);
//	HANDLE GetDisabledImage();
//	void SetDisabledImage(HANDLE Image);
//	DWORD GetFocusedBkColor(); 
//	DWORD GetNormalBkColor(); 
//	DWORD GetPushedBkColor(); 
//	DWORD GetHotBkColor(); 
//	DWORD GetBkFade(); 
//	DWORD GetUIStyle(); 
//	void SetUIStyle( DWORD ui_style ); 
//	//DWORD SetNormalBkColor( DWORD nor_clr ); 
//	void SetPushedBkColor( DWORD push_clr ); 
//	void SetHotBkColor( DWORD hot_clr ); 
//	void SetBkFade( DWORD bk_fade ); 
//	void SetFocusedBkColor( DWORD focusd_clr ); 
//	void SetDisabledBkColor( DWORD disabled_clr ); 
//
//	void SetHotTextColor(DWORD dwColor);
//	DWORD GetHotTextColor() const;
//	void SetPushedTextColor(DWORD dwColor);
//	DWORD GetPushedTextColor() const;
//	void SetFocusedTextColor(DWORD dwColor);
//	DWORD GetFocusedTextColor() const;
//	SIZE EstimateSize(SIZE szAvailable);
//	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
//
//	void PaintText(HDC hDC);
//	void PaintStatusImage(HDC hDC);
//	//void OnPainted( HDC hDC ); 
//	void _PaintStatusImage(HDC hDC);
//
//#define is_valid_bk_color( clr ) ( ( clr & 0xff000000 ) == 0x0f000000 )
//
//	VOID PaintStatusHolRound( HDC hDC ); 
//	VOID PaintStatusBkColor( HDC hDC ); 
//
protected:
	ACTION_INFO action_info; 
	ULONG action_id; 
	ULONG m_uButtonState; 
	ULONG work_mode; 
};

#define RECT_WIDTH( __rect ) ( ( __rect ).right - ( __rect ).left )
#define RECT_HEIGHT( __rect ) ( ( __rect ).bottom - ( __rect ).top )

#define ACTION_SHAPE_HORIZONTAL_SPAN 10
#define ACTION_SHAPE_HEIGHT 50
#define ACTION_SHAPE_WIDTH 200

#define ACTION_SHAPE_BORDER_ROUND { 6, 6 }

class action_show_dlg : public CWindowWnd, public wnd_state, public INotifyUI
{
public:
	action_show_dlg() 
	{
		action_show_count = 0; 
	};

	LPCTSTR GetWindowClassName() const 
	{ 
		return _T("TActionsShow"); 
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
		//      m_pCloseBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("close_btn")));
		//ASSERT( m_pCloseBtn != NULL ); 
		ret_status = CANCEL_STATE; 

		set_ctrl_text( &m_pm, _T( "yes_btn" ), TEXT_COMMON_BUTTON_YES ); 
		set_ctrl_text( &m_pm, _T( "no_btn" ), TEXT_COMMON_BUTTON_NO ); 
	}

	void OnPrepare(TNotifyUI& msg) 
	{

	}


	LRESULT on_action_shape_click( TNotifyUI& msg ); 

	void Notify(TNotifyUI& msg); 

	LRESULT analyze_actions_from_db(); 

	typedef struct _action_ui
	{
		ULONG color; 
		ULONG level; 
		PVOID data; 
		ULONG data_len; 
		PVOID context; 
		
	} action_ui, *paction_ui; 

	ULONG MapActionColor( sys_action_record *action )
	{
		ULONG color = 0; 
		do 
		{
			switch( action->type )
			{
			case PROC_exec:
#define PROC_EXEC_COLOR 0xff302817
				color = PROC_EXEC_COLOR; 		
				break; 
			default:
				ASSERT( FALSE ); 
				break; 
			}
		} while ( FALSE );

		return color; 
	}

	LRESULT draw_action_unit( sys_action_record *action )
	{
		LRESULT ret = ERROR_SUCCESS; 

		do 
		{
			ASSERT( action != NULL ); 

		}while( FALSE );
	
		return ret; 
	}
 
	LRESULT config_ui_unit( CDynamicShape *Shape, RECT rc )
	{
		LRESULT ret = ERROR_SUCCESS; 

		do 
		{
			if( Shape == NULL )
			{
				ASSERT( FALSE ); 

				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			Shape->SetPos( rc ); 

		}while( FALSE );
	
		return ret; 
	}

	LRESULT clone_temp_control( CDynamicShape *shape )
	{
		LRESULT ret = ERROR_SUCCESS; 
		CButtonUI *temp_ctrl;

		do 
		{
			if( shape == NULL )
			{
				ASSERT( FALSE ); 
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			temp_ctrl = ( CButtonUI* )m_pm.FindControl( _T( "yes_btn" ) ); 
			ASSERT( temp_ctrl != NULL ); 

			*( CLabelUI* )shape = *( CLabelUI* )temp_ctrl; 

		}while( FALSE );
	
		return ret; 
	}

	LRESULT get_action_shape_name( sys_action_record *action, LPWSTR shape_name, ULONG ccb_buf_len )
	{
		LRESULT ret = ERROR_SUCCESS; 
		INT32 _ret; 

#define ACTION_NAME_PREFIX L"action_"
#define MAX_ACTION_NAME_NUM_POSTFIX_LEN 4
#define MAX_ACTION_NAME_LEN ( CONST_STR_LEN( ACTION_NAME_PREFIX ) + MAX_ACTION_NAME_NUM_POSTFIX_LEN + 1 )
		do
		{
			if( action == NULL )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			if( shape_name == NULL )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			if( ccb_buf_len == 0 )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}
			
			if( ccb_buf_len < CONST_STR_LEN( ACTION_NAME_PREFIX ) + MAX_ACTION_NAME_NUM_POSTFIX_LEN + 1 )
			{
				ASSERT( FALSE ); 
				ret = ERROR_BUFFER_TOO_SMALL; 
				break; 
			}

			memcpy( shape_name, ACTION_NAME_PREFIX, sizeof( ACTION_NAME_PREFIX ) - sizeof( ACTION_NAME_PREFIX[ 0 ] ) ); 

			_ret = _snwprintf( shape_name + CONST_STR_LEN( ACTION_NAME_PREFIX ), ccb_buf_len - CONST_STR_LEN( ACTION_NAME_PREFIX ), L"%u", all_shapes.GetSize() );  
		
			if( _ret == -1 
				|| _ret >= ( INT32 )ccb_buf_len - CONST_STR_LEN( ACTION_NAME_PREFIX ) )
			{
				ret = ERROR_BUFFER_TOO_SMALL; 
				break; 
			}
		}while( FALSE ); 

		return ret; 
	}

	LRESULT get_action_shape_attrs( sys_action_record *action, ULONG *bk_color, ULONG *text_color )
	{
		LRESULT ret = ERROR_SUCCESS; 

		do 
		{
			if( action == NULL )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			if( bk_color == NULL )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			if( text_color == NULL )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			if( FALSE == is_valid_action_type( action->type ) )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			//adding time tip
			switch( action->type )
			{
#define NET_SEND_SHAPE_BK_COLOR 0xFFFFFB00
#define NET_SEND_SHAPE_TEXT_COLOR 0x080808

			case NET_send:
				*bk_color = NET_SEND_SHAPE_BK_COLOR; 
				*text_color = NET_SEND_SHAPE_TEXT_COLOR; 
				break; 
			default:
				break; 
			}
		}while( FALSE );
	
		return ret; 
	}

	ULONG get_ui_show_action_count()
	{
		ULONG action_count; 
		//SIZE wnd_size; 
		ULONG canvas_height; 
		ULONG canvas_width; 
		CControlUI *dialog_layout; 

		do 
		{
			dialog_layout = ( CControlUI* )m_pm.FindControl( UI_DISPLAY_LAYOUT_NAME ); 

			if( dialog_layout == NULL )
			{
				ASSERT( FALSE ); 
				action_count = 0; 
				break; 
			}

			canvas_height = dialog_layout->GetHeight(); 
			canvas_width = dialog_layout->GetWidth(); 

			action_count = canvas_height / ACTION_SHAPE_HEIGHT; 
			action_count = action_count * ( canvas_width / ACTION_SHAPE_WIDTH ); 

			if( action_count < 5 )
			{
				action_count = 5; 
			}

		}while( FALSE );

		return action_count; 
	}

	inline LRESULT show_one_action( sys_action_record *action, 
		action_context *ctx, 
		PVOID data, 
		ULONG data_len, 
		ULONG *state_out )
	{
		LRESULT ret = ERROR_SUCCESS; 
		ULONG action_count; 

		do 
		{
			ASSERT( state_out != NULL ); 

			*state_out = 0; 

			if( action == NULL )
			{
				ASSERT( FALSE ); 
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			//		memset( &action, 0, sizeof( action ) ); 
			//
			//		action.type = NET_send; 
			//		action.do_net_send.data_len = 100; 
			//		action.do_net_send.ip_addr = inet_addr( "192.168.6.25" ); 
			//		action.do_net_send.port = 1001; 
			//		action.do_net_send.protocol = PROT_TCP; 
			//		action.do_net_send.result = 0; 
			//		action.do_net_send.local_ip_addr = inet_addr( "192.168.6.99" ); 
			//		action.do_net_send.local_port = 10202; 
			//
			//#define TEST_DATA "182834918234JKLZSDFAKJLJZNXCVJAHF98Q17349087UAOI;SDFJHOPAISDJFOASJDFL;KASJDFPOQ7324098UA;SDFJ34S65DF710-92348M.,XCNVBMXMC ZHHF"
			//#define TEST_EXE_FILE_NAME L"C:\\QQ.exe"
			//		memcpy( ctx.proc_name, TEST_EXE_FILE_NAME, sizeof( TEST_EXE_FILE_NAME ) ); 
			//		ctx.proc_name_len = CONST_STR_LEN( TEST_EXE_FILE_NAME ); 

			action_count = get_ui_show_action_count(); 

			if( action_count <= all_shapes.GetSize() )
			{
				ASSERT( FALSE ); 
				*state_out = ACTION_COUNT_IS_REACH_MAX; 

				ret = ERROR_BUFFER_OVERFLOW; 
				break; 
			}

			ret = add_action_unit( action, ctx, data, data_len, DATA_NEED_CLONE ); 
		}while( FALSE );

		return ret;  
	}

	LRESULT add_action_unit( sys_action_record *action, action_context *ctx, PVOID data, ULONG data_len, ULONG flags )
	{
		LRESULT ret = ERROR_SUCCESS; 
		CDynamicShape *shape = NULL; 
		RECT shape_pos; 
		static ULONG ctrl_top = 0; 
		//LPCWSTR action_desc; 
		WCHAR action_name[ MAX_ACTION_NAME_LEN ]; 
		WCHAR *action_tip = NULL; 
		ULONG bk_color; 
		ULONG text_color; 
		SIZE border_round = ACTION_SHAPE_BORDER_ROUND; 
		ULONG all_shapes_height; 
		ULONG canvas_height; 
		ULONG canvas_width; 

		do 
		{
			shape = new CDynamicShape( flags ); 
			if( shape == NULL )
			{
				break; 
			}

			ret = get_action_shape_name( action, action_name, ARRAY_SIZE( action_name ) ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			ret = get_action_shape_attrs( action, &bk_color, &text_color ); 

			action_tip = ( WCHAR* )malloc( sizeof( WCHAR ) * MAX_ACTION_TIP_LEN ); 
			if( action_tip == NULL )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
				break; 
			}

			ret = get_action_tip( action, ctx, action_tip, MAX_ACTION_TIP_LEN, 0 ); 
			if( ret != ERROR_SUCCESS )
			{
				DBG_BP(); 
				break; 
			}

			ret = clone_temp_control( shape ); 
			if( ret != 0 )
			{
				break; 
			}

			CControlUI *dialog_layout; 

			dialog_layout = ( CControlUI* )m_pm.FindControl( UI_DISPLAY_LAYOUT_NAME ); 

			if( dialog_layout == NULL )
			{
				ASSERT( FALSE ); 
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			canvas_height = dialog_layout->GetHeight(); 
			canvas_width = dialog_layout->GetWidth(); 

			canvas_height = canvas_height - ( canvas_height % ACTION_SHAPE_HEIGHT ); 

			all_shapes_height = all_shapes.GetSize() * ACTION_SHAPE_HEIGHT; 

			shape_pos.left = ACTION_SHAPE_HORIZONTAL_SPAN 
				+ ( ACTION_SHAPE_WIDTH * ( all_shapes_height / canvas_height ) ); 
			
			shape_pos.top = ( all_shapes_height % canvas_height ); 
			shape_pos.right = shape_pos.left + ACTION_SHAPE_WIDTH; 
			shape_pos.bottom = shape_pos.top + ACTION_SHAPE_HEIGHT; 

			shape->SetFloat( true ); 

			shape->SetBkColor( bk_color + all_shapes.GetSize() ); 
			shape->SetTextColor( text_color + all_shapes.GetSize() ); 

			shape->SetPos( shape_pos ); 
			shape->SetFixedWidth( RECT_WIDTH( shape_pos ) ); 
			shape->SetFixedHeight( RECT_HEIGHT( shape_pos ) ); 
			shape->SetVisible( true ); 
			shape->SetName( action_name ); 
			shape->SetUserData( ( LPCTSTR )action ); 
			shape->SetBorderRound( border_round ); 

			shape->set_action_id( all_shapes.GetSize() ); 
			shape->set_action_info( action, ctx, data, data_len ); 
			//
			//UINT32 text_style; 

			//text_style = shape->GetTextStyle(); 

			shape->SetTextStyle( DT_VCENTER | DT_CENTER | DT_WORDBREAK ); 


			//( LPCTSTR )all_shapes.GetSize()  
			//shape->SetBkImage( _T( "button_over.png" ) ); 
			
			shape->SetText( action_tip ); 
 
			ret = create_sub_ctrl( shape, UI_DISPLAY_LAYOUT_NAME ); 
			if( ret == ERROR_SUCCESS )
			{
				all_shapes.Add( shape ); 
				shape = NULL; 
			}

		}while( FALSE );

		if( action_tip != NULL )
		{
			free( action_tip ); 
		}

		if( shape != NULL )
		{
			free( shape ); 
		}

		return ret; 
	}

	LRESULT release_all_shapes(); 
	LRESULT create_sub_ctrl_from_xml( LPCWSTR xml )
	{
		LRESULT ret = ERROR_SUCCESS; 
		CDialogBuilder builder; 

		do 
		{
			if( xml == NULL )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

		}while( FALSE );
	
		return ret; 
	}

	LRESULT create_sub_ctrl( CControlUI *ctrl, LPCWSTR container_name )
	{
		LRESULT ret = ERROR_SUCCESS; 
		IItemManager *container; 
		//CControlUI *root; 
		LPCWSTR ctrl_class; 
		LPCTSTR pDefaultAttributes; 
		CControlUI *dialog_layout; 
		TCHAR szValue[500] = { 0 };
		SIZE_T cchLen = ARRAY_SIZE( szValue ) - 1;
		UINT uMode = 0;


		do 
		{
			if( ctrl == NULL )
			{
				ASSERT( FALSE ); 
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			if( container_name == NULL )
			{
				ASSERT( FALSE ); 
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			dialog_layout = ( CControlUI* )m_pm.FindControl( container_name ); 

			if( dialog_layout == NULL )
			{
				ASSERT( FALSE ); 
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			container = ( IItemManager* )dialog_layout->GetInterface( ITEM_MANAGER_INTERFACE_NAME ); 
			
			if( container == NULL )
			{
				ASSERT( FALSE ); 
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}
			// Attach to parent
			// ��ΪĳЩ���Ժ͸�������أ�����selected��������Add������

			if( !container->AddItem( ctrl ) )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				//delete control;
				continue;
			}

			// Init default attributes

			ctrl->SetManager( &m_pm, NULL, false );
			ctrl_class = ctrl->GetClass(); 

			//pDefaultAttributes = m_pm.GetDefaultAttributeList();

			//if( pDefaultAttributes )
			//{
			//	ctrl->ApplyAttributeList( pDefaultAttributes );
			//}

			// Process attributes

			//if( _tcsstr(szValue, _T("move_x")) != NULL ) uMode |= UISTRETCH_MOVE_X;
			//if( _tcsstr(szValue, _T("move_y")) != NULL ) uMode |= UISTRETCH_MOVE_Y;
			//if( _tcsstr(szValue, _T("move_xy")) != NULL ) uMode |= UISTRETCH_MOVE_X | UISTRETCH_MOVE_Y;
			//if( _tcsstr(szValue, _T("size_x")) != NULL ) uMode |= UISTRETCH_SIZE_X;
			//if( _tcsstr(szValue, _T("size_y")) != NULL ) uMode |= UISTRETCH_SIZE_Y;
			//if( _tcsstr(szValue, _T("size_xy")) != NULL ) uMode |= UISTRETCH_SIZE_X | UISTRETCH_SIZE_Y;
			//if( _tcsstr(szValue, _T("group")) != NULL ) uMode |= UISTRETCH_NEWGROUP;
			//if( _tcsstr(szValue, _T("line")) != NULL ) uMode |= UISTRETCH_NEWLINE;
			//dialog_layout->SetStretchMode( ctrl, uMode );

			
			ctrl->SetManager( &m_pm, NULL, false ); 

		}while( FALSE );
	
		return ret; 
	}

	LRESULT remove_sub_ctrl( CControlUI *ctrl, LPCWSTR container_name )
	{
		LRESULT ret = ERROR_SUCCESS; 
		bool _ret; 
		IItemManager *container; 
		LPCWSTR ctrl_class; 
		CControlUI *dialog_layout; 

		do 
		{
			ASSERT( ctrl != NULL ); 

			if( container_name == NULL )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			dialog_layout = ( CControlUI* )m_pm.FindControl( container_name ); 

			if( dialog_layout == NULL )
			{
				ASSERT( FALSE ); 
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			container = ( IItemManager* )dialog_layout->GetInterface( ITEM_MANAGER_INTERFACE_NAME ); 

			if( container == NULL )
			{
				ASSERT( FALSE ); 
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			_ret = container->RemoveItem( ctrl ); 
			if( _ret == false )
			{
				ASSERT( FALSE ); 
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

		}while( FALSE );

		return ret; 
	}


	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
		styleValue &= ~WS_CAPTION;
		::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
		m_pm.Init(m_hWnd);
		//m_pm.SetTransparent(100);
		CDialogBuilder builder;
		CControlUI* pRoot = builder.Create(_T("action_show_dlg.xml"), (UINT)0, NULL, &m_pm);
		ASSERT(pRoot && "Failed to parse XML");
		m_pm.AttachDialog(pRoot);
		m_pm.AddNotifier(this);

		set_ui_style( &m_pm, NO_ALPHA ); 
		Init();
		return 0;
	}

	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = FALSE;
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
		// ��ʱ�����յ�WM_NCDESTROY���յ�wParamΪSC_CLOSE��WM_SYSCOMMAND
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
	CStdPtrArray all_shapes; 
	ULONG action_show_count; 
	//CControlUI *m_pCloseBtn; 

};

INLINE LRESULT show_action_show_dlg( HWND parent, LPCTSTR title, UINT32 main_icon = NULL )
{
	LRESULT ret = ERROR_SUCCESS; 
	action_show_dlg dialog; 

	if( title == NULL )
	{
		title = _T( "" ); 
	}

	if( NULL == dialog.Create( parent, title, UI_WNDSTYLE_DIALOG | WS_EX_TOPMOST, 0L, 0, 0, 1024, 738) )
	{
		goto _return; 
	}

	if( main_icon != NULL )
	{
		dialog.SetIcon( main_icon ); 
	}

	dialog.CenterWindow();
	BringWindowToTop( dialog.GetHWND() ); 

	dialog.ShowModal(); 

_return:
	return ret; 
} 

#endif //__ACTION_SHOW_DLG_H__