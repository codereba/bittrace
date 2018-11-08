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
 
 #ifndef __SYS_MNG_API_H__
#define __SYS_MNG_API_H__

typedef enum __SERVICE_HOOK_FILTER_CLASS
{
	NtCreateKeyFilter, 
	NtQueryValueKeyFilter, 
	NtDeleteKeyFilter, 
	NtDeleteValueKeyFilter, 
	NtRenameKeyFilter, 
	NtReplaceKeyFilter, 
	NtRestoreKeyFilter, 
	NtSetValueKeyFilter, 
	NtCreateFileFilter, 
	NtFsControlFileFilter, 
	NtSetInformationFileFilter, 
	NtWriteFileOrNtWriteFileGatherFilter, 
	NtUnusedFilter , 
	NtCreateProcessFilter, 
	NtCreateProcessExFilter, 
	NtCreateUserProcessFilter, 
	NtCreateThreadOrNtCreateThreadExFilter, 
	NtOpenThreadFilter, 
	NtDeleteFileFilter, 
	NtOpenFileFilter, 
	NtReadVirtualMemoryFilter, 
	NtTerminateProcessFilter, 
	NtQueueApcThreadFilter, 
	NtSetContextThreadFilter, 
	NtSetInformationThreadFilter, 
	NtProtectVirtualMemoryFilter, 
	NtWriteVirtualMemoryFilter, 
	NtAdjustGroupTokenFilter, 
	NtAdjustPrivilegesTokenFilter, 
	NtRequestWaitReplyPortFilter, 
	NtCreateSectionFilter, 
	NtOpenSectionFilter, 
	NtCreateSymbolicLinkObjectFilter, 
	NtOpenSymbolicLinkObjectFilter, 
	NtLoadDriverFilter, 
	NtUnloadDriverFilter, 
	NtQuerySystemInformationFilter, 
	NtSetSystemInformationFilter, 
	NtSetSystemTimeFilter, 
	NtSystemDebugControlFilter, 
	NtUserBuildHwndListFilter, 
	NtUserQueryWindowFilter, 
	NtUserFindWindowExFilter, 
	NtUserWindowFromPointFilter, 
	NtUserMessageCallFilter, 
	NtUserPostMessageFilter, 
	NtUserSetWindowsHookExFilter, 
	NtUserPostThreadMessageFilter, 
	NtOpenProcessFilter, 
	NtDeviceIoControlFileFilter, 
	NtUserSetParentFilter, 
	NtOpenKeyFilter, 
	NtDuplicateObjectFilter, 
	NtResumeThreadFilter, 
	NtUserChildWindowFromPointExFilter, 
	NtUserDestroyWindowFilter, 
	NtUserInternalGetWindowTextFilter, 
	NtUserMoveWindowFilter, 
	NtUserRealChildWindowFromPointFilter, 
	NtUserSetInformationThreadFilter, 
	NtUserSetInternalWindowPosFilter, 
	NtUserSetWindowLongFilter, 
	NtUserSetWindowPlacementFilter, 
	NtUserSetWindowPosFilter, 
	NtUserSetWindowRgnFilter, 
	NtUserShowWindowFilter, 
	NtUserShowWindowAsynFilter, 
	NtQueryAttributesFileFilter, 
	NtUserSendInputFilter, 
	NtAlpcSendWaitReceivePortFilter, 
	ProcessNotifyFilter, 
	NtUnmapViewOfSectionFilter, 
	NtUserSetWinEventHookFilter, 
	NtSetSecurityObjectFilter, 
	NtUserCallHwndParamLockFilter, 
	NtUserRegisterUserApiHookFilter, 
	KeUserModeCallbackFilter, 
	NtUserRegisterWindowMessageFilter, 
	NtUserCallNoParamFilter, 
	NtAllocateVirtualMemoryFilter, 
	NtUserCallOneParamFilter, 
	NtCreateMutantFilter, 
	NtOpenMutantFilter, 
	NtVdmControlFilter, 
	NtGetNextThreadFilter, 
	NtGetNextProcessFilter, 
	NtRequestPortFilter, 
	NtFreeVirtualMemoryFilter, 
	NtApiFilterEnd , 
} SERVICE_HOOK_FILTER_CLASS;

//#include <ntdef.h>


#define REPLACE_360_HOOK 0x01
#define FLAGS_1BYTE_OFFSET 65
#define FLAGS_1BYTE_ABS_OFFSET ( sizeof( IMAGE_DOS_HEADER ) + FLAGS_1BYTE_OFFSET ) 

typedef void* HANDLE; 
typedef struct __HOOK_FILTER_INFO
{
	HANDLE TraceMsgTip;
	ULONG Size;
	ULONG FilterIndexes[ 0 ];
} HOOK_FILTER_INFO, *PHOOK_FILTER_INFO;

#define IOCTL_GET_KRNL_HOOK_DRV_ID CTL_CODE( FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_SET_HOOK_FILTER CTL_CODE( FILE_DEVICE_UNKNOWN, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_RESET_HOOK_FILTER CTL_CODE( FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_START_HOOK_FILTER_MSG CTL_CODE( FILE_DEVICE_UNKNOWN, 0x805, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_END_HOOK_FILTER_MSG CTL_CODE( FILE_DEVICE_UNKNOWN, 0x806, METHOD_BUFFERED, FILE_ANY_ACCESS )

#ifdef DRIVER
#include "common_api.h"
#endif //DRIVER

#endif //__SYS_MNG_API_H__