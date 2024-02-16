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
