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
 
 #ifndef __ACTION_POLICY_H__
#define __ACTION_POLICY_H__

#define INITIALIZE_REFERENCE_COUNT 2

NTSTATUS CALLBACK _compare_action_policy( policy_holder *src_pol, 
										PVOID policy ); 

NTSTATUS CALLBACK compare_action_policy( policy_holder *src_pol, 
									   policy_holder *dst_pol ); 

NTSTATUS is_policy_hitted( action_source_info *source, 
						  policy_holder *policy ); 

LRESULT dump_policy_holder( policy_holder *pol_holder ); 

NTSTATUS add_hitted_policy( action_source_info *source, 
						   policy_holder *policy ); 

#endif //__ACTION_POLICY_H__