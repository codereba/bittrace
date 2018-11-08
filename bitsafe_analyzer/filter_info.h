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

 
 #ifndef __FILTER_INFO_H__
#define __FILTER_INFO_H__

#define MAX_ACTION_FILTER_COND_COUNT 300
#define DATA_BY_POINTER 0x00000001

#define MAX_FILTER_COND_VALUE_LEN _MAX_REG_PATH_LEN

#define WM_ACTION_EVENT_LOG ( WM_USER + 1001 ) 

typedef enum _action_filter_cond
{
	FLT_PATH,
	FLT_PROCESS_NAME, 
	FLT_COMMAND, 
	FLT_PID, 
	FLT_PARENT_PID,
	FLT_TID, 
	FLT_MAIN_TYPE, 
	FLT_ACTION_TYPE, 
	FLT_READ_WRITE, 
	FLT_DESCRITION, 
	FLT_DESC_DETAIL, 
	FLT_ACTION_DATA, 
	FLT_ACTION_RESULT, 
	FLT_CALL_STACK, 
	FLT_DURATION, 
	FLT_DATE_TIME, 
	FLT_RELATIVE_TIME, 
	FLT_SESSION, 
	FLT_USER, 
	FLT_AUTH_ID, 
	FLT_ACTION_NO,
	FLT_CORPORATION, 
	FLT_VERSION, 
	FLT_VIRTUAL_TECH, 
	FLT_CPU_ARCH, 
	FLT_TYPE_INVALID, 
} action_filter_cond, *paction_filter_cond; 

FORCEINLINE BOOLEAN is_valid_filter_condition( LONG condition_value )
{
	BOOLEAN ret = TRUE; 

	if( condition_value >= FLT_TYPE_INVALID || condition_value < FLT_PATH )
	{
		ret = FALSE; 
	}

	return ret; 
}

typedef enum _action_compare_mode
{
	FLT_CONTAINS, 
	FLT_EXCLUDES, 
	FLT_EQUALS, 
	FLT_NOT_EQUALS, 
	FLT_GREATER_THAN, 
	FLT_LOWER_THAN, 
	FLT_START_WITH, 
	FLT_END_WITH, 
	FLT_INVALID_MODE, 
} action_compare_mode, *paction_compare_mode; 

FORCEINLINE BOOLEAN is_valid_filter_compare_mode( LONG value )
{
	BOOLEAN ret = TRUE; 

	if( value >= FLT_INVALID_MODE || value < FLT_CONTAINS )
	{
		ret = FALSE; 
	}

	return ret; 
}

typedef enum _action_filter_mode
{
	FLT_INCLUDE, 
	FLT_EXCLUDE, 
	FLT_DISPLAY, 
	FLT_INVALID_FILTER_MODE, 
} action_filter_mode, *paction_filter_mode; 

FORCEINLINE BOOLEAN is_valid_filtering_mode( LONG value )
{
	BOOLEAN ret = TRUE; 

	if( value >= FLT_INVALID_FILTER_MODE || value < FLT_INCLUDE )
	{
		ret = FALSE; 
	}

	return ret; 
}

typedef enum _action_filter_value_type
{
	ULONGLONG_VALUE_TYPE, 
	ULONG_VALUE_TYPE, 
	INT32_VALUE_TYPE, 
	TIME_VALUE_TYPE, 
	DURATION_VALUE_TYPE, 
	STRING_VALUE_TYPE, 
	//STRING_POINTER_VALUE_TYPE, 
} action_filter_value_type, *paction_filter_value_type; 

FORCEINLINE BOOLEAN is_valid_filter_value_type( LONG value )
{
	BOOLEAN ret = TRUE; 

	if( value > STRING_VALUE_TYPE || value < ULONGLONG_VALUE_TYPE )
	{
		ret = FALSE; 
	}

	return ret; 
}

#define MAX_FILTER_VALUE_LEN _MAX_REG_PATH_LEN 

#pragma pack( push )
#pragma pack( 4 )

typedef struct _time_region
{
	LARGE_INTEGER begin_time; 
	LARGE_INTEGER end_time; 
} time_region, *ptime_region; 

typedef struct _FILTER_VALUE_DATA
{
	union
	{
		ULONGLONG ulonglong_value; 
		INT32 int32_value;
		ULONG ulong_value; 
		LARGE_INTEGER time_value; 
		time_region duration; 
	};
} FILTER_VALUE_DATA, *PFILTER_VALUE_DATA; 

typedef struct _FILTER_VALUE_TEXT
{
	union
	{
		WCHAR *string_value_ptr; 
		WCHAR *text_mode_value_ptr; 
	};

	union
	{
		WCHAR string_value[ MAX_FILTER_VALUE_LEN ]; 
		WCHAR text_mode_value[ MAX_FILTER_VALUE_LEN ]; 
	}; 

	BOOLEAN text_is_ptr; 
	ULONG cc_string_len; 
} FILTER_VALUE_TEXT, *PFILTER_VALUE_TEXT; 

typedef struct _action_filter_value
{
	//ULONG size; 
	action_filter_value_type type; 

	FILTER_VALUE_TEXT text; 
	FILTER_VALUE_DATA data; 

} action_filter_value, *paction_filter_value; 

#define EVENT_HILIGHT_BK_COLOR 0xffcccccc
#define INVALID_UI_FILTERED_BK_COLOR ( ULONG )( 0 )
typedef struct _event_filter_ui_info
{
	ULONG bk_clr; 
} event_filter_ui_info, *pevent_filter_ui_info; 

typedef struct _action_filter_info
{
	event_filter_ui_info ui_info; 
	action_filter_cond cond; 
	action_compare_mode compare_mode; 
	action_filter_mode filter_mode; 
	action_filter_value value; 
}action_filter_info, *paction_filter_info; 

#pragma pack( pop )

#endif //__FILTER_INFO_H__