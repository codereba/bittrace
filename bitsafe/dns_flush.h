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
 
 #pragma once

#define DNS_FLUSH_FUNC_NAME "DnsFlushResolverCache" 
#define DNS_MOD_NAME "dnsapi.dll"
typedef BOOL ( WINAPI *dns_flush_func )( VOID ); 

LRESULT init_dns_mod(); 
LRESULT release_dns_mod(); 
INT32 flush_dns(); 

