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