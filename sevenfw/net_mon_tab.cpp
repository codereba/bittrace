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
#include "ui_ctrl.h"
#include "net_mon_tab.h"

INT32 net_mon_ui_output( ULONG ui_action_id, 
						 INT32 ret_val, 
								PBYTE data, 
								ULONG length, 
								PVOID param )
{
	INT32 ret; 
	net_mon_tab *ui_tab; 

	ret = 0; 

	switch( ui_action_id )
	{
	//case NET_MON_CTRL_PROC_TRAFFIC:
	//	break; 
	case NET_MON_OUTPUT_PROC_TRAFFIC:
		if( ret_val < 0 )
		{
			break; 
		}

		ui_tab = ( net_mon_tab* )param; 
		ret = ui_tab->update_all_proc_traffic( data, length, NULL ); 
		break; 
	case NET_MON_OUTPUT_TRAFFIC_COUNT:

		if( ret_val < 0 )
		{
			break; 
		}

		ui_tab = ( net_mon_tab* )param; 
		ret = ui_tab->update_traffic_sum( data, length, NULL ); 
		break; 
	default:
		break; 
	}

	return ret; 
}
