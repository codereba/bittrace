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
 
#ifndef __TDIFW_API_H__
#define __TDIFW_API_H__

#define MAX_DATA_DUMP_LEN 256

#define SYSTEM_IDLE_PROCESS_ID 0
#define SYSTEM_SYSTEM_PROCESS_ID 4

#define FILE_DEVICE_TDI_FW		0x8e86

#define IOCTL_CMD_GETREQUEST	CTL_CODE(FILE_DEVICE_TDI_FW, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CMD_CLEARCHAIN	CTL_CODE(FILE_DEVICE_TDI_FW, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CMD_APPENDRULE	CTL_CODE(FILE_DEVICE_TDI_FW, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CMD_SETCHAINPNAME	CTL_CODE(FILE_DEVICE_TDI_FW, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CMD_SETPNAME		CTL_CODE(FILE_DEVICE_TDI_FW, 0x805, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CMD_ACTIVATECHAIN	CTL_CODE(FILE_DEVICE_TDI_FW, 0x806, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CMD_SET_SIDS		CTL_CODE(FILE_DEVICE_TDI_FW, 0x807, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FILE_DEVICE_TDI_FW_NFO	0x8e87

#define IOCTL_CMD_ENUM_LISTEN	CTL_CODE(FILE_DEVICE_TDI_FW_NFO, 0x901, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CMD_ENUM_TCP_CONN	CTL_CODE(FILE_DEVICE_TDI_FW_NFO, 0x902, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CMD_GET_COUNTERS	CTL_CODE(FILE_DEVICE_TDI_FW_NFO, 0x903, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SELF_PROTECT_NOTIFY CTL_CODE( FILE_DEVICE_UNKNOWN, 0x805, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_GET_KRNL_HOOK_IF CTL_CODE( FILE_DEVICE_UNKNOWN, 0x801,METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_GET_DRIVER_VERSION CTL_CODE( FILE_DEVICE_UNKNOWN, 0x100, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_GET_ALL_PROCESS_TRAFFIC CTL_CODE( FILE_DEVICE_UNKNOWN, 0x101, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_START_FILTER CTL_CODE( FILE_DEVICE_UNKNOWN, 0x102, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_STOP_FILTER CTL_CODE( FILE_DEVICE_UNKNOWN, 0x103, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_CHECK_FILTER_STATE CTL_CODE( FILE_DEVICE_UNKNOWN, 0x104, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_GET_ALL_TRAFFIC CTL_CODE( FILE_DEVICE_UNKNOWN, 0x105, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_TERMINATE_CONN CTL_CODE( FILE_DEVICE_UNKNOWN, 0x106, METHOD_BUFFERED, FILE_ANY_ACCESS )

#define IOCTL_CTRL_PROCESS_TRAFFIC CTL_CODE( FILE_DEVICE_UNKNOWN, 0x201, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_GET_ALL_PROCESS_TRAFFIC_CTRL CTL_CODE( FILE_DEVICE_UNKNOWN, 0x202, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_STOP_ALL_PROCESS_TRAFFIC_CTRL CTL_CODE( FILE_DEVICE_UNKNOWN, 0x203, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_SET_DEBUG_TRACE_FLAG CTL_CODE( FILE_DEVICE_UNKNOWN, 0x204, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_SET_DBG_BP_FLAG CTL_CODE( FILE_DEVICE_UNKNOWN, 0x205, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_TRACE_DATA_FLOW CTL_CODE( FILE_DEVICE_UNKNOWN, 0x208, METHOD_BUFFERED, FILE_ANY_ACCESS )

#include "common_api.h"

INLINE LPCWSTR get_ioctl_code_desc( ULONG ctl_code )
{
	LPCWSTR desc = L"UNKNOW_IOCTL_CODE"; 

	switch( ctl_code )
	{
	case IOCTL_CMD_GETREQUEST:
		desc = L"IOCTL_CMD_GETREQUEST"; 
		break; 
	case IOCTL_CMD_CLEARCHAIN:
		desc = L"IOCTL_CMD_CLEARCHAIN"; 
		break; 
	case IOCTL_CMD_APPENDRULE:
		desc = L"IOCTL_CMD_APPENDRULE"; 
		break; 
	case IOCTL_CMD_SETCHAINPNAME:
		desc = L"IOCTL_CMD_SETCHAINPNAME"; 
		break; 
	case IOCTL_CMD_SETPNAME:
		desc = L"IOCTL_CMD_SETPNAME"; 
		break; 
	case IOCTL_CMD_ACTIVATECHAIN:
		desc = L"IOCTL_CMD_ACTIVATECHAIN"; 
		break; 
	case IOCTL_CMD_SET_SIDS:
		desc = L"IOCTL_CMD_SET_SIDS"; 
		break; 
	case IOCTL_CMD_ENUM_LISTEN:
		desc = L"IOCTL_CMD_ENUM_LISTEN"; 
		break; 
	case IOCTL_CMD_ENUM_TCP_CONN:
		desc = L"IOCTL_CMD_ENUM_TCP_CONN"; 
		break; 
	case IOCTL_CMD_GET_COUNTERS:
		desc = L"IOCTL_CMD_GET_COUNTERS"; 
		break; 
	case IOCTL_TERMINATE_CONN:
		desc = L"IOCTL_TERMINATE_CONN"; 
		break; 
	case IOCTL_SELF_PROTECT_NOTIFY:
		desc = L"IOCTL_SELF_PROTECT_NOTIFY"; 
		break; 
	case IOCTL_GET_KRNL_HOOK_IF:
		desc = L"IOCTL_GET_KRNL_HOOK_IF"; 
		break; 
	case IOCTL_GET_DRIVER_VERSION:
		desc = L"IOCTL_GET_DRIVER_VERSION"; 
		break; 
	case IOCTL_GET_ALL_PROCESS_TRAFFIC:
		desc = L"IOCTL_GET_ALL_PROCESS_TRAFFIC"; 
		break; 
	case IOCTL_START_FILTER:
		desc = L"IOCTL_START_FILTER"; 
		break; 
	case IOCTL_STOP_FILTER:
		desc = L"IOCTL_STOP_FILTER"; 
		break; 
	case IOCTL_CHECK_FILTER_STATE:
		desc = L"IOCTL_CHECK_FILTER_STATE"; 
		break; 
	case IOCTL_GET_ALL_TRAFFIC:
		desc = L"IOCTL_GET_ALL_TRAFFIC"; 
		break; 
	case IOCTL_CTRL_PROCESS_TRAFFIC:
		desc = L"IOCTL_CTRL_PROCESS_TRAFFIC"; 
		break; 
	case IOCTL_GET_ALL_PROCESS_TRAFFIC_CTRL:
		desc = L"IOCTL_GET_ALL_PROCESS_TRAFFIC_CTRL"; 
		break; 
	case IOCTL_STOP_ALL_PROCESS_TRAFFIC_CTRL:
		desc = L"IOCTL_STOP_ALL_PROCESS_TRAFFIC_CTRL"; 
		break; 
	case IOCTL_SET_DEBUG_TRACE_FLAG:
		desc = L"IOCTL_SET_DEBUG_TRACE_FLAG"; 
		break; 
	case IOCTL_SET_DBG_BP_FLAG:
		desc = L"IOCTL_SET_DBG_BP_FLAG"; 
		break; 
	case IOCTL_TRACE_DATA_FLOW:
		desc = L"IOCTL_TRACE_DATA_FLOW"; 
		break; 
	default:
		break; 
	}

	return desc; 
}

#ifdef DRIVER
#define NETMON_DEVICE_DOS_NAME L"\\DosDevices\\BCTdiFW"
#define NETMON_DEVICE_NAME L"\\Device\\BCTdiFW"
#endif //DRIVER

#define MAX_PATH 260

#define BP_ON_GET_ALL_PROCESS_IO 0x01
#define BP_ON_GET_ALL_PROCESS_CONTROL 0x02
#define BP_ON_ADD_PROCESS_CONTROL 0x04

#ifdef DRIVER
#ifndef NETFW_WIN7

//typedef enum _PROCESS_IMFORMATION_CLASS {
//	ProcessBasicInformation,
//	ProcessQuotaLimits,
//	ProcessIoCounters,
//	ProcessVmCounters,
//	ProcessTimes,
//	ProcessBasePriority,
//	ProcessRaisePriority,
//	ProcessDebugPort,
//	ProcessExceptionPort,
//	ProcessAccessToken,
//	ProcessLdtInformation,
//	ProcessLdtSize,
//	ProcessDeaultHardErrorMode,
//	ProcessIoPortHandlers,
//	ProcessPooledUsageAndLimits,
//	ProcessWorkingSetWatch,
//	ProcessUserModeIOPL,
//	ProcessEnableAlignmentFaultFixup,
//	ProcessPriorityClass,
//	ProcessWx86Information,
//	ProcessHandleCount,
//	ProcessAffinityMask,
//	ProcessPriorityBoost,
//	ProcessDeviceMap,
//	ProcessSessionInformation,
//	ProcessForegroundInformation,
//	ProcessWow64Information
//} PROCESS_INFORMATION_CLASS;

//typedef enum _KPROFILE_SOURCE {
//	ProfileTime,
//	ProfileAlignmentFixup,
//	ProfileTotalIssues,
//	ProfilePipelineDry,
//	ProfileLoadInstructions,
//	ProfilePipelineFrozen,
//	ProfileBranchInstructions,
//	ProfileTotalNonissues,
//	ProfileDcacheMisses,
//	ProfileIcacheMisses,
//	ProfileCacheMisses,
//	ProfileBranchMispredictions,
//	ProfileStoreInstructions,
//	ProfileFpInstructions,
//	ProfileIntegerInstructions,
//	Profile2Issue,
//	Profile3Issue,
//	Profile4Issue,
//	ProfileSpecialInstructions,
//	ProfileTotalCycles,
//	ProfileIcacheIssues,
//	ProfileDcacheAccesses,
//	ProfileMemoryBarrierCycles,
//	ProfileLoadLinkedIssues,
//	ProfileMaximum
//} KPROFILE_SOURCE, *PKPROFILE_SOURCE;

//typedef enum __SYSTEM_INFORMATION_CLASS {
//	SystemBasicInformation,
//	SystemProcessorInformation,
//	SystemPerformanceInformation,
//	SystemTimeOfDayInformation,
//	SystemPathInformation,
//	SystemProcessInformation,
//	SystemCallCountInformation,
//	SystemDeviceInformation,
//	SystemProcessorPerformanceInformation,
//	SystemFlagsInformation,
//	SystemCallTimeInformation,
//	SystemModuleInformation,
//	SystemLocksInformation,
//	SystemStackTraceInformation,
//	SystemPagedPoolInformation,
//	SystemNonPagedPoolInformation,
//	SystemHandleInformation,
//	SystemObjectInformation,
//	SystemPageFileInformation,
//	SystemVdmInstemulInformation,
//	SystemVdmBopInformation,
//	SystemFileCacheInformation,
//	SystemPoolTagInformation,
//	SystemInterruptInformation,
//	SystemDpcBehaviorInformation,
//	SystemFullMemoryInformation,
//	SystemLoadGdiDriverInformation,
//	SystemUnloadGdiDriverInformation,
//	SystemTimeAdjustmentInformation,
//	SystemSummaryMemoryInformation,
//	SystemNextEventIdInformation,
//	SystemEventIdsInformation,
//	SystemCrashDumpInformation,
//	SystemExceptionInformation,
//	SystemCrashDumpStateInformation,
//	SystemKernelDebuggerInformation,
//	SystemContextSwitchInformation,
//	SystemRegistryQuotaInformation,
//	SystemExtendServiceTableInformation,
//	SystemPrioritySeperation,
//	SystemPlugPlayBusInformation,
//	SystemDockInformation,
//	SystemPowersInformation,
//	SystemProcessorSpeedInformation,
//	SystemCurrentTimeZoneInformation,
//	SystemLookasideInformation
//} SYSTEM_INFORMATION_CLASS, *PSYSTEM_INFORMATION_CLASS;

extern NTSTATUS NTAPI ZwQuerySystemInformation( 
	SYSTEM_INFORMATION_CLASS SystemInformationClass, 
	PVOID SystemInformation, 
	ULONG SystemInformationLength, 
	PULONG ReturnLength
	);

typedef NTSTATUS 
( *ClientEventReceive )(
						PVOID  TdiEventContext,
						CONNECTION_CONTEXT  ConnectionContext,
						ULONG  ReceiveFlags,
						ULONG  BytesIndicated,
						ULONG  BytesAvailable,
						ULONG  *BytesTaken,
						PVOID  Tsdu,
						PIRP  *IoRequestPacket
						);

typedef NTSTATUS ( *ClientEventChainedReceive )(
	IN PVOID  TdiEventContext,
	IN CONNECTION_CONTEXT  ConnectionContext,
	IN ULONG  ReceiveFlags,
	IN ULONG  ReceiveLength,
	IN ULONG  StartingOffset,
	IN PMDL  Tsdu,
	IN PVOID  TsduDescriptor
	);

typedef NTSTATUS ( *ClientEventReceiveDatagram )(
	IN PVOID  TdiEventContext,
	IN LONG  SourceAddressLength,
	IN PVOID  SourceAddress,
	IN LONG  OptionsLength,
	IN PVOID  Options,
	IN ULONG  ReceiveDatagramFlags,
	IN ULONG  BytesIndicated,
	IN ULONG  BytesAvailable,
	OUT ULONG  *BytesTaken,
	IN PVOID  Tsdu,
	OUT PIRP  *IoRequestPacket
	);

NTKERNELAPI NTSTATUS 
ZwQueryDirectoryFile(
					 HANDLE  FileHandle,
					 HANDLE  Event,
					 PIO_APC_ROUTINE  ApcRoutine,
					 PVOID  ApcContext,
					 PIO_STATUS_BLOCK  IoStatusBlock,
					 PVOID  FileInformation,
					 ULONG  Length,
					 FILE_INFORMATION_CLASS  FileInformationClass,
					 BOOLEAN  ReturnSingleEntry,
					 PUNICODE_STRING  FileName,
					 BOOLEAN  RestartScan
					 );

NTKERNELAPI PEPROCESS IoThreadToProcess(
										PETHREAD Thread
										);

NTKERNELAPI BOOLEAN
IoIsOperationSynchronous(
						 PIRP  irp
						 );

NTKERNELAPI HANDLE
PsGetProcessId(
			   PEPROCESS  Process
			   );

NTKERNELAPI NTSTATUS
PsLookupProcessByProcessId(
						   HANDLE ProcessId,
						   PEPROCESS *Process
						   );

#endif //NETFW_WIN7
#endif //DRIVER

#include <PshPack1.h>

#define MAX_IMAGE_FILE_PATH 300

//#define MAX_PROCESS_IMAGE_PATH_LEN 512
typedef struct __PROCESS_TRAFFIC_CTRL
{
	WCHAR native_image_file[ MAX_IMAGE_FILE_PATH ];
	WCHAR image_file[ MAX_PATH ];
	BOOL remove_op;
	BOOL stop_send;
	BOOL stop_recv;
	LARGE_INTEGER send_speed_ctrl;
} PROCESS_TRAFFIC_CTRL, *PPROCESS_TRAFFIC_CTRL;

typedef struct _PROCESS_TRAFFIC_OUTPUT
{
	DWORD proc_id;
	LARGE_INTEGER all_sended_data;
	LARGE_INTEGER all_recved_data;
	BOOL stop_send;
	BOOL stop_recv;
	LARGE_INTEGER send_speed_ctrl;
	LARGE_INTEGER send_speed;
	LARGE_INTEGER recv_speed;
} PROCESS_TRAFFIC_OUTPUT, *PPROCESS_TRAFFIC_OUTPUT;

typedef struct _PROCESS_EXIT_HOOK_PARAM
{
	DWORD parent_id;
	DWORD proc_id;
	BOOL bCreate;
} PROCESS_EXIT_HOOK_PARAM, *PPROCESS_EXIT_HOOK_PARAM;

#ifndef __WIN32
typedef struct _IMAGE_FIXUP_ENTRY {
	USHORT        Offset:12;
	USHORT        Type:4;
} IMAGE_FIXUP_ENTRY, *PIMAGE_FIXUP_ENTRY;

#endif

#include <PopPack.h>

/*
 * direction type for filter
 * for quick filter:
 *  if proto == IPPROTO_TCP (DIRECTION_IN - accept connections; DIRECTION_OUT - connect)
 *  if proto == IPPROTO_UDP (DIRECTION_IN - receive datagram; DIRECTION_OUT - send datagram)
 */
#define DIRECTION_IN	0
#define DIRECTION_OUT	1
#define DIRECTION_ANY	-1

/* filter result */
//enum {
//	FILTER_ALLOW = 1,
//	FILTER_DENY,
//	FILTER_PACKET_LOG,
//	FILTER_PACKET_BAD,
//	FILTER_DISCONNECT
//};

#pragma pack(1)

// I think 128 is a good number :-) (better than 256 :))
#define MAX_CHAINS_COUNT	128

// how many users can be assigned per rule? (MUST: MAX_SIDS_COUNT % 8 == 0 !!!)

#define RULE_LOG_NOLOG			0
#define RULE_LOG_LOG			1
#define RULE_LOG_COUNT			2

#define IPPROTO_ANY		-1

/*
 * Entry for listen info
 */
typedef struct _listen_nfo {
	int				ipproto;
	ULONG			addr;
	USHORT			port;
	ULONG			proc_id;
} listen_nfo, *plisten_nfo; 

typedef struct _socket_ctrl
{
	ULONG lipv4_addr; 
} socket_ctrl, *psocket_ctrl; 

/*
 * TCP states
 */
typedef enum _tcp_socket_state{
	TCP_STATE_NONE,
	TCP_STATE_SYN_SENT,
	TCP_STATE_SYN_RCVD,
	TCP_STATE_ESTABLISHED_IN,
	TCP_STATE_ESTABLISHED_OUT,
	TCP_STATE_FIN_WAIT1,
	TCP_STATE_FIN_WAIT2,
	TCP_STATE_TIME_WAIT,
	TCP_STATE_CLOSE_WAIT,
	TCP_STATE_LAST_ACK,
	TCP_STATE_CLOSED,
	
	TCP_STATE_MAX
}tcp_socket_state, *ptcp_socket_state; 

#define is_valid_tcp_state( state_val ) ( state_val >= TCP_STATE_NONE && state_val < TCP_STATE_MAX )  

/*
 * Entry for connection info
 */
typedef struct _tcp_conn_nfo {
	int				state;
	ULONG			laddr;
	USHORT			lport;
	ULONG			raddr;
	USHORT			rport;
	ULONG			proc_id;
	ULONG			bytes_in;
	ULONG			bytes_out;
} tcp_conn_nfo, *ptcp_conn_nfo; 


#pragma pack()

NTSTATUS process_request(ULONG code, char *buf, ULONG *buf_len, ULONG buf_size);  

/*
 * traffic counters for IOCTL_CMD_GET_COUNTERS
 */
enum
{
	TRAFFIC_TOTAL_IN,
	TRAFFIC_TOTAL_OUT,
	TRAFFIC_COUNTED_IN,
	TRAFFIC_COUNTED_OUT,
	
	TRAFFIC_MAX
}; 

#endif //__TDIFW_API_H__
