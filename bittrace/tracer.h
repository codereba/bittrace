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

#ifndef __TRACER_H__
#define __TRACER_H__

typedef struct _trace_session
{
	TRACEHANDLE session_handle; 
	EVENT_TRACE_PROPERTIES *properties; 
} trace_session, *ptrace_session; 

LRESULT prepare_log_trace( trace_session *session ); 
LRESULT start_log_trace(); 
LRESULT stop_log_trace( trace_session *session ); 
LRESULT _stop_log_trace( trace_session *session ); 

#endif //__TRACER_H__