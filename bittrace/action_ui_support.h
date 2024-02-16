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

#ifndef __ACTION_UI_SUPPORT_H__
#define __ACTION_UI_SUPPORT_H__

typedef PVOID UI_PARAM; 
LRESULT WINAPI get_action_main_type_desc( action_main_type type, LPWSTR *type_desc, UI_PARAM *ui_param ); 
LRESULT WINAPI get_string_width_from_font( ); 
LPCWSTR WINAPI get_event_name( sys_action_type type ); 

INLINE LPCWSTR WINAPI get_event_name_ex( sys_action_record *action )
{
	LPCWSTR event_name; 
	switch( action->type )
	{
		case PORT_read: 
			switch( action->do_port_read.port_type )
			{
			case PORT_COM_TYPE:
				event_name = L"Read COM"; //L"��ȡ����"; 
				break; 
			case PORT_MAILSLOT_TYPE:
				event_name = L"Read mailslot"; //L"��ȡ�ʲ�"; 
				break; 
			case PORT_PIPE_TYPE:
				event_name = L"Read PIPE"; //L"��ȡ�ܵ�"; 
				break; 
			default:
				event_name = L"Read port"; //L"��ȡ�˿�"; 
				break; 
			}
			break; 
		case PORT_write: 
			switch( action->do_port_read.port_type )
			{
			case PORT_COM_TYPE:
				event_name = L"Write COM"; //L"д�봮��"; 
				break; 
			case PORT_MAILSLOT_TYPE:
				event_name = L"Write mailslot"; //L"д���ʲ�"; 
				break; 
			case PORT_PIPE_TYPE:
				event_name = L"Write PIPE"; //L"д��ܵ�"; 
				break; 
			default:
				event_name = L"Wirte port"; //L"д��˿�"; 
				break; 
			}
			break; 
		
		default:
			event_name = get_event_name( action->type ); 
			break; 
	}

	return event_name; 
}

#endif //__ACTION_UI_SUPPORT_H__