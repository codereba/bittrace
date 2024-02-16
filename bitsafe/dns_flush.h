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

#pragma once

#define DNS_FLUSH_FUNC_NAME "DnsFlushResolverCache" 
#define DNS_MOD_NAME "dnsapi.dll"
typedef BOOL ( WINAPI *dns_flush_func )( VOID ); 

LRESULT init_dns_mod(); 
LRESULT release_dns_mod(); 
INT32 flush_dns(); 

