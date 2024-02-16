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

#include "common_func.h"
#include "sys_mng_tab.h"
#include "trace_log_api.h"

//INT32 user_response_sys_event( PSYS_EVENT sys_event, ULONG length, PVOID param )
//{
//	INT32 ret; 
//	INT32 response; 
//	SYS_EVENT_RESPON event_respon; 
//
//	ASSERT( sys_event != NULL ); 
//	ASSERT( sys_event->Msgs != NULL ); 
//
//	ret = MessageBox( NULL, ( CHAR* )sys_event->Msgs, "����", MB_YESNO | MB_TASKMODAL | MB_TOPMOST ); 
//
//	//__asm int 3; 
//
//	if( ret == IDYES )
//	{
//		response = SYS_ACTION_ALLOW; 
//	}
//	else
//	{
//		response = SYS_ACTION_DENY; 
//	}
//
//	event_respon.action = response; 
//	event_respon.id = sys_event->id; 
//
//	ret = do_ui_ctrl( SYS_MON_UI_ID, SYS_MON_EVENT_RESPONSE, ( PBYTE )&event_respon, sizeof( event_respon ), NULL, 0, NULL ); 
//	
//	return ret; 
//}

//INT32 output_sys_info( PTRACE_INFO_OUTPUT msgs_cap, ULONG length, PVOID param )
//{
//	ULONG Outputed;
//	PTRACE_INFO_OUTPUT msg_cap; 
//	sys_mng_tab *ui_tab; 
//	HWND hEdit;
//
//	ASSERT( param != NULL ); 
//
//	ui_tab = ( sys_mng_tab* )param; 
//
//	hEdit = ui_tab->GetDlgItem( IDC_RICHEDIT_INFO ); 
//	ASSERT( NULL != hEdit );
//	ASSERT( NULL != msgs_cap ); 
//	ASSERT( length > sizeof( PTRACE_INFO_OUTPUT ) + sizeof( CHAR ) ); 
//
//	if( msgs_cap->Size > MAX_TRACE_INFO_LEN )
//	{
//		ASSERT( FALSE );
//		return -1;
//	}
//
//	if( 0 == msgs_cap->Size )
//	{
//		return -1;
//	}
//
//	msgs_cap->Msgs[ msgs_cap->Size ] = '\0';
//
//	Outputed = 0;
//
//	msg_cap = ( PTRACE_INFO_OUTPUT )msgs_cap->Msgs;
//	for( ; ; )
//	{
//		if( msg_cap->Size + Outputed + sizeof( TRACE_INFO_OUTPUT ) >= msgs_cap->Size )
//		{
//			if( msg_cap->Size + Outputed + sizeof( TRACE_INFO_OUTPUT ) > msgs_cap->Size )
//			{
//				ASSERT( FALSE );
//			}
//
//			InsertTailText( hEdit, ( CHAR* )msg_cap->Msgs );
//			break;
//		}
//
//		InsertTailText( hEdit, ( CHAR* )msg_cap->Msgs );
//
//		Outputed += msg_cap->Size + sizeof( TRACE_INFO_OUTPUT ); 
//		msg_cap = ( PTRACE_INFO_OUTPUT )( msg_cap->Msgs + msg_cap->Size );
//	}
//
//	return 0; 
//}

INT32 sys_mng_ui_output( ULONG ui_action_id, 
						 INT32 ret_val, 
								PBYTE data, 
								ULONG length, 
								PVOID param )
{
	INT32 ret; 
	sys_mng_tab *ui_tab; 

	ret = 0; 

	switch( ui_action_id )
	{
	case SYS_MON_MSGS_OUTPUT:
		if( ret_val < 0 )
		{
			break; 
		}

		ui_tab = ( sys_mng_tab* )param; 
		ret = output_sys_info( ( PTRACE_INFO_OUTPUT )data, length, ui_tab ); 
		break; 

	case SYS_MON_EVENT_HAPPEND:
		if( ret_val < 0 )
		{
			break; 
		}

		ui_tab = ( sys_mng_tab* )param; 
		ret = user_response_sys_event( ( PSYS_EVENT )data, length, ui_tab ); 
		break; 

	default:
		break; 
	}

	return ret; 
}
