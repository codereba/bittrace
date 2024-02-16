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
 
#include "_stdafx.h"
#include "common_func.h"
#include "bitsafe_common.h"
#include "action_show_dlg.h"
#include "action_logger.h"
#include "text_msg_box.h"
#include "action_log_db.h"

void CDynamicShape::DoEvent(TEventUI& event)
{
	if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
		if( m_pParent != NULL ) m_pParent->DoEvent(event);
		else CLabelUI::DoEvent(event);
		return;
	}

	if( event.Type == UIEVENT_SETFOCUS ) 
	{
		Invalidate();
	}
	if( event.Type == UIEVENT_KILLFOCUS ) 
	{
		Invalidate();
	}
	if( event.Type == UIEVENT_KEYDOWN )
	{
		if( event.chKey == VK_SPACE || event.chKey == VK_RETURN ) {
			Activate();
			return;
		}
	}
	if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK )
	{
		if( ::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled() ) {
			m_uButtonState |= UISTATE_PUSHED | UISTATE_CAPTURED;
			Invalidate();
		}
		return;
	}
	if( event.Type == UIEVENT_MOUSEMOVE )
	{
		if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
			if( ::PtInRect(&m_rcItem, event.ptMouse) ) m_uButtonState |= UISTATE_PUSHED;
			else m_uButtonState &= ~UISTATE_PUSHED;
			Invalidate();
		}
		return;
	}
	if( event.Type == UIEVENT_BUTTONUP )
	{
		if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
			if( ::PtInRect(&m_rcItem, event.ptMouse) ) Activate();
			m_uButtonState &= ~(UISTATE_PUSHED | UISTATE_CAPTURED);
			Invalidate();
		}
		return;
	}
	if( event.Type == UIEVENT_CONTEXTMENU )
	{
		if( IsContextMenuUsed() ) {
			m_pManager->SendNotify(this, _T("menu"), event.wParam, event.lParam);
		}
		return;
	}
	if( event.Type == UIEVENT_MOUSEENTER )
	{
		if( IsEnabled() ) {
			m_uButtonState |= UISTATE_HOT;
			Invalidate();
		}
		return;
	}
	if( event.Type == UIEVENT_MOUSELEAVE )
	{
		if( IsEnabled() ) {
			m_uButtonState &= ~UISTATE_HOT;
			Invalidate();
		}
		return;
	}
	if( event.Type == UIEVENT_SETCURSOR ) {
		::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));
		return;
	}

	CTextUI::DoEvent(event);
}

bool CDynamicShape::Activate()
{
	if( !CControlUI::Activate() ) return false;
	if( m_pManager != NULL ) m_pManager->SendNotify(this, _T("click"));
	return true;
}

LRESULT action_show_dlg::on_action_shape_click( TNotifyUI& msg )
{
	LRESULT ret = ERROR_SUCCESS; 
	//ULONG action_id; 
	PVOID data; 
	ULONG data_len; 
	register CDynamicShape *shape; 
	WCHAR *data_text = NULL; 
	BOOLEAN data_text_alloc = FALSE; 

	do
	{
		if( 0 != wcscmp( L"CDynamicShape", msg.pSender->GetClass() ) )
		{
			log_trace( ( MSG_INFO, "another control 0x%0.8x see to one dynamic shape\n", msg.pSender ) ); 
			break; 
		}

		shape = ( CDynamicShape* )msg.pSender; 
		//action_id = shape->get_action_id(); 
		shape->get_data( &data, &data_len ); 

		if( data == NULL || data_len == 0 )
		{
			//break; 
			data_text = L"not have data\n"; 
		}
		else
		{
			data_text = ( WCHAR* )malloc( sizeof( WCHAR ) * MAX_DATA_DUMP_LEN ); 

			if( data_text == NULL )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
				break; 
			}

			data_text_alloc = TRUE; 
		
			ret = dump_mem_in_buf_w( data_text, MAX_DATA_DUMP_LEN, data, data_len, 0 ); 
			if( ret != ERROR_SUCCESS )
			{

			}
		}

		ret = show_text_msg( m_pm.GetPaintWindow(), data_text ); 
		if( ret != ERROR_SUCCESS )
		{
		}
	}while( FALSE ); 

	if( data_text_alloc == TRUE )
	{
		if( data_text != NULL )
		{
			free( data_text ); 
		}
		else
		{
			ASSERT( FALSE ); 
		}
	}

	return ret; 
}

NTSTATUS CALLBACK show_event_log_to_ui( sys_action_record *action, 
										action_context *ctx, 
										PVOID data, 
										ULONG data_len, 
										PVOID param, 
										ULONG *state_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	action_show_dlg *action_show_ui; 
	LRESULT ret; 
	ULONG state; 

	do 
	{
		if( state_out != NULL )
		{
			*state_out = 0; 
		}

		if( action ==  NULL )
		{
			ASSERT( FALSE && "input null action" ); 
			ntstatus = STATUS_INVALID_PARAMETER; 
			break; 
		}

		//if( data == NULL )
		//{
		//	ntstatus = STATUS_INVALID_PARAMETER; 
		//	break; 
		//}

		//if( data_len == 0 )
		//{
		//	ntstatus = STATUS_INVALID_PARAMETER; 
		//	break; 
		//}

		if( param == NULL )
		{
			ASSERT( FALSE ); 
			ntstatus = STATUS_INVALID_PARAMETER; 
			break; 
		}

		action_show_ui = ( action_show_dlg* )param; 

		ret = action_show_ui->show_one_action( action, ctx, data, data_len, &state ); 
		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_IMPORTANT, "show action content error %u\n", ret ) ); 
		}
		
		if( state_out != NULL )
		{
			*state_out = state; 
		}

	}while( FALSE );

	return ntstatus; 
}

LRESULT action_show_dlg::release_all_shapes()
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	INT32 i; 
	CDynamicShape *shape; 

	do 
	{
		for( i = 0; i < all_shapes.GetSize(); i ++ )
		{
			shape = ( CDynamicShape* )all_shapes.GetAt( i ); 
			_ret = remove_sub_ctrl( shape, UI_DISPLAY_LAYOUT_NAME ); 
			if( _ret != ERROR_SUCCESS )
			{

			}
		}

		all_shapes.Empty(); 
		//all_shapes.Remove( 0 ); 
	} while ( FALSE );

	return ret; 
}

LRESULT action_show_dlg::analyze_actions_from_db()
{
	LRESULT ret = ERROR_SUCCESS; 
	NTSTATUS ntstatus; 
	sys_action_record action; 
	action_context ctx = { 0 }; 
	param_sel_mode all_conds[ MAX_PARAMS_COUNT ]; 
	param_info all_params[ MAX_PARAMS_COUNT ]; 
	ULONG param_count; 
	INT32 i; 
	ULONG display_count; 

	do 
	{
		
		//ntstatus = get_params_from_action( &action, 
		//	all_params, 
		//	ARRAY_SIZE( all_params ), 
		//	&param_count ); 

		//if( ntstatus != STATUS_SUCCESS )
		//{
		//	break; 
		//}

		memset( &all_params, 0, sizeof( all_params ) ); 
		param_count = 0; 

		if( param_count + 1 > ARRAY_SIZE( all_params ) )
		{
			break; 
		}

		//all_params[ param_count ].type = DATA_BLOB_TYPE; 
		//all_params[ param_count ].data.uint64_val = 0; 

		//param_count += 1; 

		for( i = 0; ( ULONG )i < param_count; i ++ )
		{
			all_conds[ i ] = SELECT_OR; 
		}

		display_count = get_ui_show_action_count(); 
		
		ret = output_action_log_from_db( &ctx, 
			SYS_ACTION_NONE, 
			all_conds, 
			all_params, 
			param_count, 
			action_show_count, 
			display_count, 
			( PVOID )this, 
			show_event_log_to_ui ); 

		if( ret != 0 )
		{
			log_trace( ( MSG_FATAL_ERROR, "show action from db error %u\n", ret ) ); 
		}

		action_show_count += display_count; 

#define MAX_SHAPE_CACHED_SIZE 50
		if( all_shapes.GetSize() > MAX_SHAPE_CACHED_SIZE )
		{
			release_all_shapes(); 
		}
	}while( FALSE ); 

	return ret; 
}

VOID action_show_dlg::Notify(TNotifyUI& msg)
{
	LRESULT ret; 
	if( msg.sType == _T("windowinit") ) 
		OnPrepare(msg);
	else if( msg.sType == _T("click") ) 
	{
		//if( msg.pSender == m_pCloseBtn ) 
		//{
		//	ret_status = CANCEL_STATE; 
		//	Close(); 
		//	return; 
		//}
		//else
		{
			CStdString name = msg.pSender->GetName(); 

			if( name == _T( "close_btn" ) )
			{
				Close(); 
			}
			else if( name == _T( "yes_btn" ) )
			{
				ret = analyze_actions_from_db(); 
			}
			else if( name == _T( "no_btn" ) )
			{
				ret_status = CANCEL_STATE; 
				ret = release_all_shapes(); 
				//Close(); 
			}
			else if( name == _T( "browser_btn" ) )
			{
				open_file_dlg( TRUE ); 
			}
			else
			{
				on_action_shape_click( msg ); 
			}
		}
	}
	else if(msg.sType==_T("setfocus"))
	{
	}
}