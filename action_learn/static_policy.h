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

#ifndef __STATIC_POLICY_H__
#define __STATIC_POLICY_H__

NTSTATUS check_thrd_create( action_context *ctx, 
						   thrd_create *do_thrd_create, 
						   action_response_type *resp ); 

NTSTATUS init_static_policy_dispatch(); 


#endif //__STATIC_POLICY_H__