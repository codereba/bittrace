/*
 *
 * Copyright 2010 JiJie Shi(weixin:AIChangeLife)
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
				event_name = L"Read COM"; //L"读取串口"; 
				break; 
			case PORT_MAILSLOT_TYPE:
				event_name = L"Read mailslot"; //L"读取邮槽"; 
				break; 
			case PORT_PIPE_TYPE:
				event_name = L"Read PIPE"; //L"读取管道"; 
				break; 
			default:
				event_name = L"Read port"; //L"读取端口"; 
				break; 
			}
			break; 
		case PORT_write: 
			switch( action->do_port_read.port_type )
			{
			case PORT_COM_TYPE:
				event_name = L"Write COM"; //L"写入串口"; 
				break; 
			case PORT_MAILSLOT_TYPE:
				event_name = L"Write mailslot"; //L"写入邮槽"; 
				break; 
			case PORT_PIPE_TYPE:
				event_name = L"Write PIPE"; //L"写入管道"; 
				break; 
			default:
				event_name = L"Wirte port"; //L"写入端口"; 
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