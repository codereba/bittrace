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

#ifndef __RING0_2_RING3_H__
#define __RING0_2_RING3_H__

#include <stdio.h>

#ifndef __out 
#define __out 
#endif //__out 

#ifndef __in 
#define __in 
#endif //__in 

#ifndef __inout
#define __inout
#endif //__inout

#ifndef __possibly_notnullterminated
#define __possibly_notnullterminated
#endif //__possibly_notnullterminated

#ifndef __in_range
#define __in_range( _x_, _y_ )
#endif //__in_range

#ifndef __deref_out_range
#define __deref_out_range( _x_, _y_ )
#endif //__deref_out_range

#ifndef __in_xcount
#define __in_xcount( _x_ )
#endif //__in_xcount

#ifndef __deref_in_opt_out
#define __deref_in_opt_out
#endif //__deref_in_opt_out

#ifndef __struct_bcount
#define __struct_bcount( x ) 
#endif //__struct_bcount

#ifndef __volatile 
#define __volatile volatile
#endif //__volatile 

#ifndef __drv_aliasesMem 
#define __drv_aliasesMem 
#endif //__drv_aliasesMem 

#ifdef _DEBUG
#define DBG 1
#endif //_DEBUG

#define init_sp_lock( lock ) KeInitializeSpinLock( &lock ) 
#define DbgPrint 
#define NANO100_TO_MILI_SEC_TIME ( 10 * 1000 )
#define NANO100_TO_MILI_SEC( time ) ( ( time ) / NANO100_TO_MILI_SEC_TIME )
#define PAGED_CODE() 
#define KeDelayExecutionThread( WaitMode, Alertable, Interval ) Sleep( ( ULONG )NANO100_TO_MILI_SEC( ( Interval )->QuadPart ) ) 
#define get_proc_name_from_eproc( proc, name ) 
#define KeGetCurrentIrql() 0
#define SFW_BUG_CHECK( status ) ASSERT( FALSE && "bug check" )
#define PsTerminateSystemThread( status_code ) 


#if (NTDDI_VERSION >= NTDDI_WIN2K)
INLINE VOID
NTAPI
KeBugCheck( __in ULONG BugCheckCode )
{
	ASSERT( FALSE && __FUNCTION__ ); 
}

#endif //(NTDDI_VERSION >= NTDDI_WIN2K)

#define allocate_mem( size ) malloc( size )
#define free_mem( buf ) free( buf )

#ifndef STATUS_INVALID_PARAMETER
#define STATUS_INVALID_PARAMETER         ((NTSTATUS)0xC000000DL)
#endif //STATUS_INVALID_PARAMETER

#ifndef CONTAINING_RECORD

#define CONTAINING_RECORD(address, type, field) ((type *)( \
	(PCHAR)(address) - \
	(ULONG_PTR)(&((type *)0)->field)))

#endif //CONTAINING_RECORD

INT32 __declspec( selectany ) test_weak = 0;

#ifdef _DEBUG
#include <assert.h>
#define ASSERT( x ) assert( x )
#else
#define ASSERT( x ) 
#endif //_DEBUG

#define NT_SUCCESS( x ) ( x >= 0 )

#ifndef ARRAY_SIZE
#define ARRAY_SIZE( arr ) ( sizeof( arr ) / sizeof( arr[ 0 ] ) )
#endif //ARRAY_SIZE

#define KdPrint( _x_ ) printf _x_

#define KIRQL BYTE 

typedef KSPIN_LOCK NDIS_RW_LOCK; 
typedef BYTE LOCK_STATE; 

//typedef char BOOLEAN; 
typedef ULONG ERESOURCE; 
typedef PVOID PNDIS_PACKET; 
typedef PVOID PNDIS_BUFFER; 
#define HTTP_GET_PACK 0x00000001 

#define NdisGetNextBuffer(CurrentBuffer, NextBuffer)                        \
{                                                                           \
	(NextBuffer);                                  \
}

typedef enum _MM_PAGE_PRIORITY {
	LowPagePriority,
	NormalPagePriority = 16,
	HighPagePriority = 32
} MM_PAGE_PRIORITY;

#define ExDeleteResourceLite( resource ) STATUS_SUCCESS
#define ExInitializeResourceLite( resource ) STATUS_SUCCESS
#define ExAcquireResourceSharedLite( resource, wait ) STATUS_SUCCESS
#define ExReleaseResourceLite( resource ) 
#define ExAcquireResourceExclusiveLite( resource, wait ) STATUS_SUCCESS
#define KeEnterCriticalRegion() 
#define KeLeaveCriticalRegion() 

typedef void* PKEVENT; 
typedef CCHAR KPROCESSOR_MODE;
typedef ULONG KEVENT; 

typedef enum _EVENT_TYPE {
	NotificationEvent,
	SynchronizationEvent
} EVENT_TYPE;

typedef struct _spin_lock
{
	KIRQL old_irql; 
	KSPIN_LOCK lock; 
} spin_lock, *pspin_lock; 

#define release_pool( mem ) free( mem )
#define alloc_pool( type, size ) malloc( size )
#define KeSetEvent( event, alertable, reason ) 
#define KeInitializeEvent( event, type, manual ) 
#define KeWaitForSingleObject( a, b, c, d, e ) STATUS_SUCCESS

typedef
NTSTATUS
DRIVER_DISPATCH (
				 __in struct _DEVICE_OBJECT *DeviceObject,
				 __inout struct _IRP *Irp
				 );

typedef DRIVER_DISPATCH *PDRIVER_DISPATCH; 

typedef 
NTSTATUS
DRIVER_INITIALIZE (
				   __in struct _DRIVER_OBJECT *DriverObject,
				   __in PUNICODE_STRING RegistryPath
				   );

typedef DRIVER_INITIALIZE *PDRIVER_INITIALIZE;

typedef
NTSTATUS
DRIVER_ADD_DEVICE (
				   __in struct _DRIVER_OBJECT *DriverObject,
				   __in struct _DEVICE_OBJECT *PhysicalDeviceObject
				   );

typedef DRIVER_ADD_DEVICE *PDRIVER_ADD_DEVICE;

typedef
VOID
DRIVER_STARTIO (
				__inout struct _DEVICE_OBJECT *DeviceObject,
				__inout struct _IRP *Irp
				);

typedef DRIVER_STARTIO *PDRIVER_STARTIO;

typedef struct _IO_REMOVE_LOCK {
	ULONG /*IO_REMOVE_LOCK_COMMON_BLOCK */Common;
#if DBG
	ULONG /*IO_REMOVE_LOCK_DBG_BLOCK */Dbg;
#endif
} IO_REMOVE_LOCK, *PIO_REMOVE_LOCK;

typedef struct _DRIVER_EXTENSION {

	struct _DRIVER_OBJECT *DriverObject;

	//
	// The AddDevice entry point is called by the Plug & Play manager
	// to inform the driver when a new device instance arrives that this
	// driver must control.
	//

	PDRIVER_ADD_DEVICE AddDevice;

	//
	// The count field is used to count the number of times the driver has
	// had its registered reinitialization routine invoked.
	//

	ULONG Count;

	//
	// The service name field is used by the pnp manager to determine
	// where the driver related info is stored in the registry.
	//

	UNICODE_STRING ServiceKeyName;

	//
	// Note: any new shared fields get added here.
	//
} DRIVER_EXTENSION, *PDRIVER_EXTENSION;

typedef PVOID PFAST_IO_DISPATCH; 
typedef short CSHORT; 
typedef PVOID PIO_TIMER; 

typedef struct _EPROCESS
{
	ULONG reserved; 
} EPROCESS, *PEPROCESS; 

#ifndef UNDEFINE_MDL
typedef __struct_bcount(Size) struct _MDL {
	struct _MDL *Next;
	CSHORT Size;
	CSHORT MdlFlags;
	struct _EPROCESS *Process;
	PVOID MappedSystemVa;
	PVOID StartVa;
	ULONG ByteCount;
	ULONG ByteOffset;
} MDL, *PMDL;
#else
typedef PVOID PMDL; 
#endif //UNDEFINE_MDL

#define DUMMYUNIONNAME

typedef struct _IO_STATUS_BLOCK {
	union {
		NTSTATUS Status;
		PVOID Pointer;
	} DUMMYUNIONNAME;

	ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

typedef
VOID
DRIVER_CANCEL (
			   __inout struct _DEVICE_OBJECT *DeviceObject,
			   __inout struct _IRP *Irp
			   );

typedef DRIVER_CANCEL *PDRIVER_CANCEL;

typedef
VOID
(NTAPI *PIO_APC_ROUTINE) (
						  __in PVOID ApcContext,
						  __in PIO_STATUS_BLOCK IoStatusBlock,
						  __in ULONG Reserved
						  );

#define PIO_APC_ROUTINE_DEFINED

typedef PVOID PETHREAD; 
typedef struct _KDEVICE_QUEUE_ENTRY {
	LIST_ENTRY DeviceListEntry;
	ULONG SortKey;
	BOOLEAN Inserted;
} KDEVICE_QUEUE_ENTRY, *PKDEVICE_QUEUE_ENTRY, *PRKDEVICE_QUEUE_ENTRY;

typedef struct _KTHREAD
{
	ULONG reserved; 
} KTHREAD, *PKTHREAD; 

#define __drv_functionClass( routine )
#define __drv_maxIRQL( irql )
#define __drv_minIRQL( irql )
#define __drv_requiresIRQL( irql )
#define __drv_sameIRQL

#ifndef __in_opt
#define __in_opt
#endif //__in_opt

#ifndef __deref_inout_opt
#define __deref_inout_opt 
#endif //__deref_inout_opt

typedef
__drv_functionClass(KNORMAL_ROUTINE)
__drv_maxIRQL(PASSIVE_LEVEL)
__drv_minIRQL(PASSIVE_LEVEL)
__drv_requiresIRQL(PASSIVE_LEVEL)
__drv_sameIRQL
VOID
KNORMAL_ROUTINE (
				 __in_opt PVOID NormalContext,
				 __in_opt PVOID SystemArgument1,
				 __in_opt PVOID SystemArgument2
				 );
typedef KNORMAL_ROUTINE *PKNORMAL_ROUTINE;

typedef
__drv_functionClass(KKERNEL_ROUTINE)
__drv_maxIRQL(APC_LEVEL)
__drv_minIRQL(APC_LEVEL)
__drv_requiresIRQL(APC_LEVEL)
__drv_sameIRQL
VOID
KKERNEL_ROUTINE (
				 __in struct _KAPC *Apc,
				 __deref_inout_opt PKNORMAL_ROUTINE *NormalRoutine,
				 __deref_inout_opt PVOID *NormalContext,
				 __deref_inout_opt PVOID *SystemArgument1,
				 __deref_inout_opt PVOID *SystemArgument2
				 );
typedef KKERNEL_ROUTINE *PKKERNEL_ROUTINE;

typedef
__drv_functionClass(KRUNDOWN_ROUTINE)
__drv_maxIRQL(PASSIVE_LEVEL)
__drv_minIRQL(PASSIVE_LEVEL)
__drv_requiresIRQL(PASSIVE_LEVEL)
__drv_sameIRQL
VOID
KRUNDOWN_ROUTINE (
				  __in struct _KAPC *Apc
				  );
typedef KRUNDOWN_ROUTINE *PKRUNDOWN_ROUTINE;

#define ASSERT_DEVICE_QUEUE(E) NT_ASSERT((E)->Type == DeviceQueueObject)

typedef struct _KDEVICE_QUEUE {
	CSHORT Type;
	CSHORT Size;
	LIST_ENTRY DeviceListHead;
	KSPIN_LOCK Lock;

#if defined(_AMD64_)

	union {
		BOOLEAN Busy;
		struct {
			LONG64 Reserved : 8;
			LONG64 Hint : 56;
		};
	};

#else

	BOOLEAN Busy;

#endif

} KDEVICE_QUEUE, *PKDEVICE_QUEUE, *PRKDEVICE_QUEUE;

typedef struct _KAPC {
	UCHAR Type;
	UCHAR SpareByte0;
	UCHAR Size;
	UCHAR SpareByte1;
	ULONG SpareLong0;
	struct _KTHREAD *Thread;
	LIST_ENTRY ApcListEntry;
	PKKERNEL_ROUTINE KernelRoutine;
	PKRUNDOWN_ROUTINE RundownRoutine;
	PKNORMAL_ROUTINE NormalRoutine;
	PVOID NormalContext;

	//
	// N.B. The following two members MUST be together.
	//

	PVOID SystemArgument1;
	PVOID SystemArgument2;
	CCHAR ApcStateIndex;
	KPROCESSOR_MODE ApcMode;
	BOOLEAN Inserted;
} KAPC, *PKAPC, *PRKAPC;

#ifndef __in_xcount_opt
#define __in_xcount_opt( x ) 
#endif //__in_xcount_opt

typedef struct _DEVICE_OBJECT *PDEVICE_OBJECT; 

#ifndef UNDEFINE_IRP
typedef struct _IRP *PIRP; 
#else
typedef PVOID PIRP; 
#endif //UNDEFINE_IRP

typedef PVOID PACCESS_STATE; 

typedef struct _IO_SECURITY_CONTEXT {
	PSECURITY_QUALITY_OF_SERVICE SecurityQos;
	PACCESS_STATE AccessState;
	ACCESS_MASK DesiredAccess;
	ULONG FullCreateOptions;
} IO_SECURITY_CONTEXT, *PIO_SECURITY_CONTEXT;

__drv_functionClass(IO_COMPLETION_ROUTINE)
__drv_sameIRQL
typedef
NTSTATUS
IO_COMPLETION_ROUTINE (
					   __in PDEVICE_OBJECT DeviceObject,
					   __in PIRP Irp,
					   __in_xcount_opt("varies") PVOID Context
					   );

typedef IO_COMPLETION_ROUTINE *PIO_COMPLETION_ROUTINE;

#define POINTER_ALIGNMENT 

#ifndef FILE_INFORMATION_CLASS_DEFINED
#define FILE_INFORMATION_CLASS_DEFINED 1
typedef enum _FILE_INFORMATION_CLASS {
	FileDirectoryInformation         = 1,
	FileFullDirectoryInformation,   // 2
	FileBothDirectoryInformation,   // 3
	FileBasicInformation,           // 4
	FileStandardInformation,        // 5
	FileInternalInformation,        // 6
	FileEaInformation,              // 7
	FileAccessInformation,          // 8
	FileNameInformation,            // 9
	FileRenameInformation,          // 10
	FileLinkInformation,            // 11
	FileNamesInformation,           // 12
	FileDispositionInformation,     // 13
	FilePositionInformation,        // 14
	FileFullEaInformation,          // 15
	FileModeInformation,            // 16
	FileAlignmentInformation,       // 17
	FileAllInformation,             // 18
	FileAllocationInformation,      // 19
	FileEndOfFileInformation,       // 20
	FileAlternateNameInformation,   // 21
	FileStreamInformation,          // 22
	FilePipeInformation,            // 23
	FilePipeLocalInformation,       // 24
	FilePipeRemoteInformation,      // 25
	FileMailslotQueryInformation,   // 26
	FileMailslotSetInformation,     // 27
	FileCompressionInformation,     // 28
	FileObjectIdInformation,        // 29
	FileCompletionInformation,      // 30
	FileMoveClusterInformation,     // 31
	FileQuotaInformation,           // 32
	FileReparsePointInformation,    // 33
	FileNetworkOpenInformation,     // 34
	FileAttributeTagInformation,    // 35
	FileTrackingInformation,        // 36
	FileIdBothDirectoryInformation, // 37
	FileIdFullDirectoryInformation, // 38
	FileValidDataLengthInformation, // 39
	FileShortNameInformation,       // 40
	FileIoCompletionNotificationInformation, // 41
	FileIoStatusBlockRangeInformation,       // 42
	FileIoPriorityHintInformation,           // 43
	FileSfioReserveInformation,              // 44
	FileSfioVolumeInformation,               // 45
	FileHardLinkInformation,                 // 46
	FileProcessIdsUsingFileInformation,      // 47
	FileNormalizedNameInformation,           // 48
	FileNetworkPhysicalNameInformation,      // 49
	FileIdGlobalTxDirectoryInformation,      // 50
	FileIsRemoteDeviceInformation,           // 51
	FileAttributeCacheInformation,           // 52
	FileNumaNodeInformation,                 // 53
	FileStandardLinkInformation,             // 54
	FileRemoteProtocolInformation,           // 55
	FileMaximumInformation
} FILE_INFORMATION_CLASS, *PFILE_INFORMATION_CLASS; 
#endif //FILE_INFORMATION_CLASS_DEFINED

typedef struct _FILE_DIRECTORY_INFORMATION {
	ULONG NextEntryOffset;
	ULONG FileIndex;
	LARGE_INTEGER CreationTime;
	LARGE_INTEGER LastAccessTime;
	LARGE_INTEGER LastWriteTime;
	LARGE_INTEGER ChangeTime;
	LARGE_INTEGER EndOfFile;
	LARGE_INTEGER AllocationSize;
	ULONG FileAttributes;
	ULONG FileNameLength;
	WCHAR FileName[1];
} FILE_DIRECTORY_INFORMATION, *PFILE_DIRECTORY_INFORMATION;

typedef struct _FILE_FULL_DIR_INFORMATION {
	ULONG NextEntryOffset;
	ULONG FileIndex;
	LARGE_INTEGER CreationTime;
	LARGE_INTEGER LastAccessTime;
	LARGE_INTEGER LastWriteTime;
	LARGE_INTEGER ChangeTime;
	LARGE_INTEGER EndOfFile;
	LARGE_INTEGER AllocationSize;
	ULONG FileAttributes;
	ULONG FileNameLength;
	ULONG EaSize;
	WCHAR FileName[1];
} FILE_FULL_DIR_INFORMATION, *PFILE_FULL_DIR_INFORMATION;

typedef struct _FILE_ID_FULL_DIR_INFORMATION {
	ULONG NextEntryOffset;
	ULONG FileIndex;
	LARGE_INTEGER CreationTime;
	LARGE_INTEGER LastAccessTime;
	LARGE_INTEGER LastWriteTime;
	LARGE_INTEGER ChangeTime;
	LARGE_INTEGER EndOfFile;
	LARGE_INTEGER AllocationSize;
	ULONG FileAttributes;
	ULONG FileNameLength;
	ULONG EaSize;
	LARGE_INTEGER FileId;
	WCHAR FileName[1];
} FILE_ID_FULL_DIR_INFORMATION, *PFILE_ID_FULL_DIR_INFORMATION;

typedef struct _FILE_BOTH_DIR_INFORMATION {
	ULONG NextEntryOffset;
	ULONG FileIndex;
	LARGE_INTEGER CreationTime;
	LARGE_INTEGER LastAccessTime;
	LARGE_INTEGER LastWriteTime;
	LARGE_INTEGER ChangeTime;
	LARGE_INTEGER EndOfFile;
	LARGE_INTEGER AllocationSize;
	ULONG FileAttributes;
	ULONG FileNameLength;
	ULONG EaSize;
	CCHAR ShortNameLength;
	WCHAR ShortName[12];
	WCHAR FileName[1];
} FILE_BOTH_DIR_INFORMATION, *PFILE_BOTH_DIR_INFORMATION;

typedef struct _FILE_ID_BOTH_DIR_INFORMATION {
	ULONG NextEntryOffset;
	ULONG FileIndex;
	LARGE_INTEGER CreationTime;
	LARGE_INTEGER LastAccessTime;
	LARGE_INTEGER LastWriteTime;
	LARGE_INTEGER ChangeTime;
	LARGE_INTEGER EndOfFile;
	LARGE_INTEGER AllocationSize;
	ULONG FileAttributes;
	ULONG FileNameLength;
	ULONG EaSize;
	CCHAR ShortNameLength;
	WCHAR ShortName[12];
	LARGE_INTEGER FileId;
	WCHAR FileName[1];
} FILE_ID_BOTH_DIR_INFORMATION, *PFILE_ID_BOTH_DIR_INFORMATION;

typedef struct _FILE_NAMES_INFORMATION {
	ULONG NextEntryOffset;
	ULONG FileIndex;
	ULONG FileNameLength;
	WCHAR FileName[1];
} FILE_NAMES_INFORMATION, *PFILE_NAMES_INFORMATION;

typedef struct _FILE_ID_GLOBAL_TX_DIR_INFORMATION {
	ULONG NextEntryOffset;
	ULONG FileIndex;
	LARGE_INTEGER CreationTime;
	LARGE_INTEGER LastAccessTime;
	LARGE_INTEGER LastWriteTime;
	LARGE_INTEGER ChangeTime;
	LARGE_INTEGER EndOfFile;
	LARGE_INTEGER AllocationSize;
	ULONG FileAttributes;
	ULONG FileNameLength;
	LARGE_INTEGER FileId;
	GUID LockingTransactionId;
	ULONG TxInfoFlags;
	WCHAR FileName[1];
} FILE_ID_GLOBAL_TX_DIR_INFORMATION, *PFILE_ID_GLOBAL_TX_DIR_INFORMATION;

#define FILE_ID_GLOBAL_TX_DIR_INFO_FLAG_WRITELOCKED         0x00000001
#define FILE_ID_GLOBAL_TX_DIR_INFO_FLAG_VISIBLE_TO_TX       0x00000002
#define FILE_ID_GLOBAL_TX_DIR_INFO_FLAG_VISIBLE_OUTSIDE_TX  0x00000004

typedef struct _FILE_OBJECTID_INFORMATION {
	LONGLONG FileReference;
	UCHAR ObjectId[16];
	union {
		struct {
			UCHAR BirthVolumeId[16];
			UCHAR BirthObjectId[16];
			UCHAR DomainId[16];
		} DUMMYSTRUCTNAME;
		UCHAR ExtendedInfo[48];
	} DUMMYUNIONNAME;
} FILE_OBJECTID_INFORMATION, *PFILE_OBJECTID_INFORMATION;

//
//  The following constants provide addition meta characters to fully
//  support the more obscure aspects of DOS wild card processing.
//

#define ANSI_DOS_STAR   ('<')
#define ANSI_DOS_QM     ('>')
#define ANSI_DOS_DOT    ('"')

#define DOS_STAR        (L'<')
#define DOS_QM          (L'>')
#define DOS_DOT         (L'"')

typedef struct _FILE_INTERNAL_INFORMATION {
	LARGE_INTEGER IndexNumber;
} FILE_INTERNAL_INFORMATION, *PFILE_INTERNAL_INFORMATION;

typedef struct _FILE_EA_INFORMATION {
	ULONG EaSize;
} FILE_EA_INFORMATION, *PFILE_EA_INFORMATION;

typedef struct _FILE_ACCESS_INFORMATION {
	ACCESS_MASK AccessFlags;
} FILE_ACCESS_INFORMATION, *PFILE_ACCESS_INFORMATION;

typedef struct _FILE_MODE_INFORMATION {
	ULONG Mode;
} FILE_MODE_INFORMATION, *PFILE_MODE_INFORMATION;

typedef struct _FILE_BASIC_INFORMATION {
	LARGE_INTEGER CreationTime;
	LARGE_INTEGER LastAccessTime;
	LARGE_INTEGER LastWriteTime;
	LARGE_INTEGER ChangeTime;
	ULONG FileAttributes;
} FILE_BASIC_INFORMATION, *PFILE_BASIC_INFORMATION;

typedef struct _FILE_STANDARD_INFORMATION {
	LARGE_INTEGER AllocationSize;
	LARGE_INTEGER EndOfFile;
	ULONG NumberOfLinks;
	BOOLEAN DeletePending;
	BOOLEAN Directory;
} FILE_STANDARD_INFORMATION, *PFILE_STANDARD_INFORMATION;


typedef struct _FILE_POSITION_INFORMATION {
	LARGE_INTEGER CurrentByteOffset;
} FILE_POSITION_INFORMATION, *PFILE_POSITION_INFORMATION;


typedef struct _FILE_NETWORK_OPEN_INFORMATION {
	LARGE_INTEGER CreationTime;
	LARGE_INTEGER LastAccessTime;
	LARGE_INTEGER LastWriteTime;
	LARGE_INTEGER ChangeTime;
	LARGE_INTEGER AllocationSize;
	LARGE_INTEGER EndOfFile;
	ULONG FileAttributes;
} FILE_NETWORK_OPEN_INFORMATION, *PFILE_NETWORK_OPEN_INFORMATION;

typedef struct _FILE_FULL_EA_INFORMATION {
	ULONG NextEntryOffset;
	UCHAR Flags;
	UCHAR EaNameLength;
	USHORT EaValueLength;
	CHAR EaName[1];
} FILE_FULL_EA_INFORMATION, *PFILE_FULL_EA_INFORMATION;


//
// Support to reserve bandwidth for a file handle.
//

typedef struct _FILE_SFIO_RESERVE_INFORMATION {
	ULONG RequestsPerPeriod;
	ULONG Period;
	BOOLEAN RetryFailures;
	BOOLEAN Discardable;
	ULONG RequestSize;
	ULONG NumOutstandingRequests;
} FILE_SFIO_RESERVE_INFORMATION, *PFILE_SFIO_RESERVE_INFORMATION;

//
// Support to query bandwidth properties of a volume.
//

typedef struct _FILE_SFIO_VOLUME_INFORMATION {
	ULONG MaximumRequestsPerPeriod;
	ULONG MinimumPeriod;
	ULONG MinimumTransferSize;
} FILE_SFIO_VOLUME_INFORMATION, *PFILE_SFIO_VOLUME_INFORMATION;

//
// Support to set priority hints on a filehandle.
//

typedef enum _IO_PRIORITY_HINT {
	IoPriorityVeryLow = 0,          // Defragging, content indexing and other background I/Os
	IoPriorityLow,                  // Prefetching for applications.
	IoPriorityNormal,               // Normal I/Os
	IoPriorityHigh,                 // Used by filesystems for checkpoint I/O
	IoPriorityCritical,             // Used by memory manager. Not available for applications.
	MaxIoPriorityTypes
} IO_PRIORITY_HINT;

typedef struct _FILE_IO_PRIORITY_HINT_INFORMATION {
	IO_PRIORITY_HINT   PriorityHint;
} FILE_IO_PRIORITY_HINT_INFORMATION, *PFILE_IO_PRIORITY_HINT_INFORMATION;

//
// Don't queue an entry to an associated completion port if returning success
// synchronously.
//
#define FILE_SKIP_COMPLETION_PORT_ON_SUCCESS    0x1

//
// Don't set the file handle event on IO completion.
//
#define FILE_SKIP_SET_EVENT_ON_HANDLE           0x2

//
// Don't set user supplied event on successful fast-path IO completion.
//
#define FILE_SKIP_SET_USER_EVENT_ON_FAST_IO     0x4

typedef  struct _FILE_IO_COMPLETION_NOTIFICATION_INFORMATION {
	ULONG Flags;
} FILE_IO_COMPLETION_NOTIFICATION_INFORMATION, *PFILE_IO_COMPLETION_NOTIFICATION_INFORMATION;

typedef  struct _FILE_PROCESS_IDS_USING_FILE_INFORMATION {
	ULONG NumberOfProcessIdsInList;
	ULONG_PTR ProcessIdList[1];
} FILE_PROCESS_IDS_USING_FILE_INFORMATION, *PFILE_PROCESS_IDS_USING_FILE_INFORMATION;

typedef struct _FILE_IS_REMOTE_DEVICE_INFORMATION {
	BOOLEAN IsRemote;
} FILE_IS_REMOTE_DEVICE_INFORMATION, *PFILE_IS_REMOTE_DEVICE_INFORMATION;

typedef struct _FILE_NUMA_NODE_INFORMATION {
	USHORT NodeNumber;
} FILE_NUMA_NODE_INFORMATION, *PFILE_NUMA_NODE_INFORMATION;

//
// Set an range of IOSBs on a file handle.
//

typedef struct _FILE_IOSTATUSBLOCK_RANGE_INFORMATION {
	PUCHAR       IoStatusBlockRange;
	ULONG        Length;
} FILE_IOSTATUSBLOCK_RANGE_INFORMATION, *PFILE_IOSTATUSBLOCK_RANGE_INFORMATION;

//
// flags specified here will be propagated up and down a device stack
// after FDO and all filter devices are added, but before the device
// stack is started
//

#define FILE_CHARACTERISTICS_PROPAGATED (   FILE_REMOVABLE_MEDIA   | \
	FILE_READ_ONLY_DEVICE  | \
	FILE_FLOPPY_DISKETTE   | \
	FILE_WRITE_ONCE_MEDIA  | \
	FILE_DEVICE_SECURE_OPEN  )


typedef struct _FILE_ALIGNMENT_INFORMATION {
	ULONG AlignmentRequirement;
} FILE_ALIGNMENT_INFORMATION, *PFILE_ALIGNMENT_INFORMATION;

typedef struct _FILE_BOTH_DIRECTORY_INFORMATION{
	ULONG NextEntryOffset; 
	ULONG Unknown; 
	LARGE_INTEGER CreationTime; 
	LARGE_INTEGER LastAccessTime; 
	LARGE_INTEGER LastWriteTime; 
	LARGE_INTEGER ChangeTime; 
	LARGE_INTEGER EndOfFile; 
	LARGE_INTEGER AllocationSize; 
	ULONG FileAttributes; 
	ULONG FileNameLength; 
	ULONG EaInformationLength; 
	UCHAR AlternateNameLength; 
	WCHAR AlternateName[12]; 
	WCHAR FileName[1]; 
} FILE_BOTH_DIRECTORY_INFORMATION, *PFILE_BOTH_DIRECTORY_INFORMATION;   

typedef enum _PRIORITY_HINT {
	IoPriorityHintVeryLow = 0,
	IoPriorityHintLow,
	IoPriorityHintNormal,
	MaximumIoPriorityHintType
} PRIORITY_HINT;

typedef struct _FILE_IO_PRIORITY_HINT_INFO {
	PRIORITY_HINT PriorityHint;
} FILE_IO_PRIORITY_HINT_INFO, *PFILE_IO_PRIORITY_HINT_INFO;

typedef struct _FILE_VALID_DATA_LENGTH_INFORMATION {
	LARGE_INTEGER ValidDataLength;
} FILE_VALID_DATA_LENGTH_INFORMATION, *PFILE_VALID_DATA_LENGTH_INFORMATION;

typedef struct _FILE_ATTRIBUTE_TAG_INFORMATION {
	ULONG FileAttributes;
	ULONG ReparseTag;
} FILE_ATTRIBUTE_TAG_INFORMATION, *PFILE_ATTRIBUTE_TAG_INFORMATION;

typedef struct _FILE_END_OF_FILE_INFORMATION {
	LARGE_INTEGER EndOfFile;
} FILE_END_OF_FILE_INFORMATION, *PFILE_END_OF_FILE_INFORMATION; 

typedef struct _FILE_DISPOSITION_INFORMATION {
	BOOLEAN DeleteFile;
} FILE_DISPOSITION_INFORMATION, *PFILE_DISPOSITION_INFORMATION;

//
// This is also used for FileNormalizedNameInformation
//

typedef struct _FILE_NAME_INFORMATION {
	ULONG FileNameLength;
	WCHAR FileName[1];
} FILE_NAME_INFORMATION, *PFILE_NAME_INFORMATION;

typedef struct _FILE_ALL_INFORMATION {
	FILE_BASIC_INFORMATION BasicInformation;
	FILE_STANDARD_INFORMATION StandardInformation;
	FILE_INTERNAL_INFORMATION InternalInformation;
	FILE_EA_INFORMATION EaInformation;
	FILE_ACCESS_INFORMATION AccessInformation;
	FILE_POSITION_INFORMATION PositionInformation;
	FILE_MODE_INFORMATION ModeInformation;
	FILE_ALIGNMENT_INFORMATION AlignmentInformation;
	FILE_NAME_INFORMATION NameInformation;
} FILE_ALL_INFORMATION, *PFILE_ALL_INFORMATION;


typedef struct _FILE_ALLOCATION_INFORMATION {
	LARGE_INTEGER AllocationSize;
} FILE_ALLOCATION_INFORMATION, *PFILE_ALLOCATION_INFORMATION;


typedef struct _FILE_COMPRESSION_INFORMATION {
	LARGE_INTEGER CompressedFileSize;
	USHORT CompressionFormat;
	UCHAR CompressionUnitShift;
	UCHAR ChunkShift;
	UCHAR ClusterShift;
	UCHAR Reserved[3];
} FILE_COMPRESSION_INFORMATION, *PFILE_COMPRESSION_INFORMATION;


#ifdef _MAC
#pragma warning( disable : 4121)
#endif

typedef struct _FILE_LINK_INFORMATION {
	BOOLEAN ReplaceIfExists;
	HANDLE RootDirectory;
	ULONG FileNameLength;
	WCHAR FileName[1];
} FILE_LINK_INFORMATION, *PFILE_LINK_INFORMATION;


#ifdef _MAC
#pragma warning( default : 4121 )
#endif

typedef struct _FILE_MOVE_CLUSTER_INFORMATION {
	ULONG ClusterCount;
	HANDLE RootDirectory;
	ULONG FileNameLength;
	WCHAR FileName[1];
} FILE_MOVE_CLUSTER_INFORMATION, *PFILE_MOVE_CLUSTER_INFORMATION;

#ifdef _MAC
#pragma warning( disable : 4121)
#endif

typedef struct _FILE_RENAME_INFORMATION {
	BOOLEAN ReplaceIfExists;
	HANDLE RootDirectory;
	ULONG FileNameLength;
	WCHAR FileName[1];
} FILE_RENAME_INFORMATION, *PFILE_RENAME_INFORMATION;

#ifdef _MAC
#pragma warning( default : 4121 )
#endif

typedef struct _FILE_STREAM_INFORMATION {
	ULONG NextEntryOffset;
	ULONG StreamNameLength;
	LARGE_INTEGER StreamSize;
	LARGE_INTEGER StreamAllocationSize;
	WCHAR StreamName[1];
} FILE_STREAM_INFORMATION, *PFILE_STREAM_INFORMATION;

typedef struct _FILE_TRACKING_INFORMATION {
	HANDLE DestinationFile;
	ULONG ObjectInformationLength;
	CHAR ObjectInformation[1];
} FILE_TRACKING_INFORMATION, *PFILE_TRACKING_INFORMATION;

typedef struct _FILE_COMPLETION_INFORMATION {
	HANDLE Port;
	PVOID Key;
} FILE_COMPLETION_INFORMATION, *PFILE_COMPLETION_INFORMATION;

typedef struct _FILE_PIPE_INFORMATION {
	ULONG ReadMode;
	ULONG CompletionMode;
} FILE_PIPE_INFORMATION, *PFILE_PIPE_INFORMATION;

typedef struct _FILE_PIPE_LOCAL_INFORMATION {
	ULONG NamedPipeType;
	ULONG NamedPipeConfiguration;
	ULONG MaximumInstances;
	ULONG CurrentInstances;
	ULONG InboundQuota;
	ULONG ReadDataAvailable;
	ULONG OutboundQuota;
	ULONG WriteQuotaAvailable;
	ULONG NamedPipeState;
	ULONG NamedPipeEnd;
} FILE_PIPE_LOCAL_INFORMATION, *PFILE_PIPE_LOCAL_INFORMATION;

typedef struct _FILE_PIPE_REMOTE_INFORMATION {
	LARGE_INTEGER CollectDataTime;
	ULONG MaximumCollectionCount;
} FILE_PIPE_REMOTE_INFORMATION, *PFILE_PIPE_REMOTE_INFORMATION;


typedef struct _FILE_MAILSLOT_QUERY_INFORMATION {
	ULONG MaximumMessageSize;
	ULONG MailslotQuota;
	ULONG NextMessageSize;
	ULONG MessagesAvailable;
	LARGE_INTEGER ReadTimeout;
} FILE_MAILSLOT_QUERY_INFORMATION, *PFILE_MAILSLOT_QUERY_INFORMATION;

typedef struct _FILE_MAILSLOT_SET_INFORMATION {
	PLARGE_INTEGER ReadTimeout;
} FILE_MAILSLOT_SET_INFORMATION, *PFILE_MAILSLOT_SET_INFORMATION;

typedef struct _FILE_REPARSE_POINT_INFORMATION {
	LONGLONG FileReference;
	ULONG Tag;
} FILE_REPARSE_POINT_INFORMATION, *PFILE_REPARSE_POINT_INFORMATION;

typedef struct _FILE_LINK_ENTRY_INFORMATION {
	ULONG NextEntryOffset;
	LONGLONG ParentFileId;
	ULONG FileNameLength;
	WCHAR FileName[1];
} FILE_LINK_ENTRY_INFORMATION, *PFILE_LINK_ENTRY_INFORMATION;

typedef struct _FILE_LINKS_INFORMATION {
	ULONG BytesNeeded;
	ULONG EntriesReturned;
	FILE_LINK_ENTRY_INFORMATION Entry;
} FILE_LINKS_INFORMATION, *PFILE_LINKS_INFORMATION;

typedef struct _FILE_NETWORK_PHYSICAL_NAME_INFORMATION {
	ULONG FileNameLength;
	WCHAR FileName[1];
} FILE_NETWORK_PHYSICAL_NAME_INFORMATION, *PFILE_NETWORK_PHYSICAL_NAME_INFORMATION;

typedef struct _FILE_STANDARD_LINK_INFORMATION {
	ULONG NumberOfAccessibleLinks;
	ULONG TotalNumberOfLinks;
	BOOLEAN DeletePending;
	BOOLEAN Directory;
} FILE_STANDARD_LINK_INFORMATION, *PFILE_STANDARD_LINK_INFORMATION;

//
// NtQuery(Set)EaFile
//
// The offset for the start of EaValue is EaName[EaNameLength + 1]
//
// begin_wdm


typedef struct _FILE_GET_EA_INFORMATION {
	ULONG NextEntryOffset;
	UCHAR EaNameLength;
	CHAR EaName[1];
} FILE_GET_EA_INFORMATION, *PFILE_GET_EA_INFORMATION;


//
// File Remote protocol information (FileRemoteProtocolInformation)
//

#define REMOTE_PROTOCOL_FLAG_LOOPBACK       0x00000001
#define REMOTE_PROTOCOL_FLAG_OFFLINE        0x00000002

typedef struct _FILE_REMOTE_PROTOCOL_INFORMATION
{
	// Structure Version
	USHORT StructureVersion;     // 1
	USHORT StructureSize;        // sizeof(FILE_REMOTE_PROTOCOL_INFORMATION)

	ULONG  Protocol;             // Protocol (WNNC_NET_*) defined in winnetwk.h or ntifs.h.

	// Protocol Version & Type
	USHORT ProtocolMajorVersion;
	USHORT ProtocolMinorVersion;
	USHORT ProtocolRevision;

	USHORT Reserved;

	// Protocol-Generic Information
	ULONG  Flags;

	struct {
		ULONG Reserved[8];
	} GenericReserved;

	// Protocol specific information

	struct {
		ULONG Reserved[16];
	} ProtocolSpecificReserved;

} FILE_REMOTE_PROTOCOL_INFORMATION, *PFILE_REMOTE_PROTOCOL_INFORMATION;

//
// NtQuery(Set)QuotaInformationFile
//

typedef struct _FILE_GET_QUOTA_INFORMATION {
	ULONG NextEntryOffset;
	ULONG SidLength;
	SID Sid;
} FILE_GET_QUOTA_INFORMATION, *PFILE_GET_QUOTA_INFORMATION;

typedef struct _FILE_QUOTA_INFORMATION {
	ULONG NextEntryOffset;
	ULONG SidLength;
	LARGE_INTEGER ChangeTime;
	LARGE_INTEGER QuotaUsed;
	LARGE_INTEGER QuotaThreshold;
	LARGE_INTEGER QuotaLimit;
	SID Sid;
} FILE_QUOTA_INFORMATION, *PFILE_QUOTA_INFORMATION;

typedef enum _FSINFOCLASS {
	FileFsVolumeInformation       = 1,
	FileFsLabelInformation,      // 2
	FileFsSizeInformation,       // 3
	FileFsDeviceInformation,     // 4
	FileFsAttributeInformation,  // 5
	FileFsControlInformation,    // 6
	FileFsFullSizeInformation,   // 7
	FileFsObjectIdInformation,   // 8
	FileFsDriverPathInformation, // 9
	FileFsVolumeFlagsInformation,// 10
	FileFsMaximumInformation
} FS_INFORMATION_CLASS, *PFS_INFORMATION_CLASS;

//
// Volume Parameter Block (VPB)
//

#define MAXIMUM_VOLUME_LABEL_LENGTH  (32 * sizeof(WCHAR)) // 32 characters

typedef struct _VPB {
	CSHORT Type;
	CSHORT Size;
	USHORT Flags;
	USHORT VolumeLabelLength; // in bytes
	struct _DEVICE_OBJECT *DeviceObject;
	struct _DEVICE_OBJECT *RealDevice;
	ULONG SerialNumber;
	ULONG ReferenceCount;
	WCHAR VolumeLabel[MAXIMUM_VOLUME_LABEL_LENGTH / sizeof(WCHAR)];
} VPB, *PVPB;

//
// Define PNP/POWER types required by IRP_MJ_PNP/IRP_MJ_POWER.
//

typedef enum _DEVICE_RELATION_TYPE {
	BusRelations,
	EjectionRelations,
	PowerRelations,
	RemovalRelations,
	TargetDeviceRelation,
	SingleBusRelations,
	TransportRelations
} DEVICE_RELATION_TYPE, *PDEVICE_RELATION_TYPE;

//
// Define Interface reference/dereference routines for
//  Interfaces exported by IRP_MN_QUERY_INTERFACE
//

typedef VOID (*PINTERFACE_REFERENCE)(PVOID Context);
typedef VOID (*PINTERFACE_DEREFERENCE)(PVOID Context);

// workaround overloaded definition (rpc generated headers all define INTERFACE
// to match the class name).
#undef INTERFACE

typedef struct _INTERFACE {
	USHORT Size;
	USHORT Version;
	PVOID Context;
	PINTERFACE_REFERENCE InterfaceReference;
	PINTERFACE_DEREFERENCE InterfaceDereference;
	// interface specific entries go here
} INTERFACE, *PINTERFACE;

typedef __struct_bcount(Size) struct _DEVICE_CAPABILITIES {
	USHORT Size;
	USHORT Version;  // the version documented here is version 1
	ULONG DeviceD1:1;
	ULONG DeviceD2:1;
	ULONG LockSupported:1;
	ULONG EjectSupported:1; // Ejectable in S0
	ULONG Removable:1;
	ULONG DockDevice:1;
	ULONG UniqueID:1;
	ULONG SilentInstall:1;
	ULONG RawDeviceOK:1;
	ULONG SurpriseRemovalOK:1;
	ULONG WakeFromD0:1;
	ULONG WakeFromD1:1;
	ULONG WakeFromD2:1;
	ULONG WakeFromD3:1;
	ULONG HardwareDisabled:1;
	ULONG NonDynamic:1;
	ULONG WarmEjectSupported:1;
	ULONG NoDisplayInUI:1;
	ULONG Reserved1:1;
	ULONG Reserved:13;

	ULONG Address;
	ULONG UINumber;
} DEVICE_CAPABILITIES, *PDEVICE_CAPABILITIES;

//
// Define the I/O bus interface types.
//

typedef enum _INTERFACE_TYPE {
	InterfaceTypeUndefined = -1,
	Internal,
	Isa,
	Eisa,
	MicroChannel,
	TurboChannel,
	PCIBus,
	VMEBus,
	NuBus,
	PCMCIABus,
	CBus,
	MPIBus,
	MPSABus,
	ProcessorInternal,
	InternalPowerBus,
	PNPISABus,
	PNPBus,
	Vmcs,
	MaximumInterfaceType
}INTERFACE_TYPE, *PINTERFACE_TYPE;

typedef ULONG IO_RESOURCE_DESCRIPTOR; 

typedef struct _IO_RESOURCE_LIST {
	USHORT Version;
	USHORT Revision;

	ULONG Count;
	IO_RESOURCE_DESCRIPTOR Descriptors[1];
} IO_RESOURCE_LIST, *PIO_RESOURCE_LIST;

typedef struct _IO_RESOURCE_REQUIREMENTS_LIST {
	ULONG ListSize;
	INTERFACE_TYPE InterfaceType; // unused for WDM
	ULONG BusNumber; // unused for WDM
	ULONG SlotNumber;
	ULONG Reserved[3];
	ULONG AlternativeLists;
	IO_RESOURCE_LIST  List[1];
} IO_RESOURCE_REQUIREMENTS_LIST, *PIO_RESOURCE_REQUIREMENTS_LIST;

typedef enum {
	BusQueryDeviceID = 0,       // <Enumerator>\<Enumerator-specific device id>
	BusQueryHardwareIDs = 1,    // Hardware ids
	BusQueryCompatibleIDs = 2,  // compatible device ids
	BusQueryInstanceID = 3,     // persistent id for this instance of the device
	BusQueryDeviceSerialNumber = 4,   // serial number for this device
	BusQueryContainerID = 5     // unique id of the device's physical container
} BUS_QUERY_ID_TYPE, *PBUS_QUERY_ID_TYPE;

typedef enum {
	DeviceTextDescription = 0,            // DeviceDesc property
	DeviceTextLocationInformation = 1     // DeviceLocation property
} DEVICE_TEXT_TYPE, *PDEVICE_TEXT_TYPE;

typedef enum _DEVICE_USAGE_NOTIFICATION_TYPE {
	DeviceUsageTypeUndefined,
	DeviceUsageTypePaging,
	DeviceUsageTypeHibernation,
	DeviceUsageTypeDumpFile
} DEVICE_USAGE_NOTIFICATION_TYPE;

typedef struct _POWER_SEQUENCE {
	ULONG SequenceD1;
	ULONG SequenceD2;
	ULONG SequenceD3;
} POWER_SEQUENCE, *PPOWER_SEQUENCE;

#if (NTDDI_VERSION >= NTDDI_VISTA)
typedef struct _SYSTEM_POWER_STATE_CONTEXT {
	union {
		struct {
			ULONG   Reserved1             : 8;
			ULONG   TargetSystemState     : 4;
			ULONG   EffectiveSystemState  : 4;
			ULONG   CurrentSystemState    : 4;
			ULONG   IgnoreHibernationPath : 1;
			ULONG   PseudoTransition      : 1;
			ULONG   Reserved2             : 10;
		} DUMMYSTRUCTNAME;

		ULONG ContextAsUlong;
	} DUMMYUNIONNAME;
} SYSTEM_POWER_STATE_CONTEXT, *PSYSTEM_POWER_STATE_CONTEXT;
#endif // (NTDDI_VERSION >= NTDDI_VISTA)

typedef union _POWER_STATE {
	SYSTEM_POWER_STATE SystemState;
	DEVICE_POWER_STATE DeviceState;
} POWER_STATE, *PPOWER_STATE;

typedef enum _POWER_STATE_TYPE {
	SystemPowerState = 0,
	DevicePowerState
} POWER_STATE_TYPE, *PPOWER_STATE_TYPE;

typedef ULONG CM_FULL_RESOURCE_DESCRIPTOR; 

typedef struct _CM_RESOURCE_LIST {
	ULONG Count;
	CM_FULL_RESOURCE_DESCRIPTOR List[1];
} CM_RESOURCE_LIST, *PCM_RESOURCE_LIST;

//
// The following structure is pointed to by the SectionObject pointer field
// of a file object, and is allocated by the various NT file systems.
//

typedef struct _SECTION_OBJECT_POINTERS {
	PVOID DataSectionObject;
	PVOID SharedCacheMap;
	PVOID ImageSectionObject;
} SECTION_OBJECT_POINTERS;
typedef SECTION_OBJECT_POINTERS *PSECTION_OBJECT_POINTERS;

//
// Define File Object (FO) flags
//

#define FO_FILE_OPEN                    0x00000001
#define FO_SYNCHRONOUS_IO               0x00000002
#define FO_ALERTABLE_IO                 0x00000004
#define FO_NO_INTERMEDIATE_BUFFERING    0x00000008
#define FO_WRITE_THROUGH                0x00000010
#define FO_SEQUENTIAL_ONLY              0x00000020
#define FO_CACHE_SUPPORTED              0x00000040
#define FO_NAMED_PIPE                   0x00000080
#define FO_STREAM_FILE                  0x00000100
#define FO_MAILSLOT                     0x00000200
#define FO_GENERATE_AUDIT_ON_CLOSE      0x00000400
#define FO_QUEUE_IRP_TO_THREAD          FO_GENERATE_AUDIT_ON_CLOSE
#define FO_DIRECT_DEVICE_OPEN           0x00000800
#define FO_FILE_MODIFIED                0x00001000
#define FO_FILE_SIZE_CHANGED            0x00002000
#define FO_CLEANUP_COMPLETE             0x00004000
#define FO_TEMPORARY_FILE               0x00008000
#define FO_DELETE_ON_CLOSE              0x00010000
#define FO_OPENED_CASE_SENSITIVE        0x00020000
#define FO_HANDLE_CREATED               0x00040000
#define FO_FILE_FAST_IO_READ            0x00080000
#define FO_RANDOM_ACCESS                0x00100000
#define FO_FILE_OPEN_CANCELLED          0x00200000
#define FO_VOLUME_OPEN                  0x00400000
#define FO_REMOTE_ORIGIN                0x01000000
#define FO_DISALLOW_EXCLUSIVE           0x02000000
#define FO_SKIP_COMPLETION_PORT         FO_DISALLOW_EXCLUSIVE
#define FO_SKIP_SET_EVENT               0x04000000
#define FO_SKIP_SET_FAST_IO             0x08000000

//
// This mask allows us to re-use flags that are not present during a create
// operation for uses that are only valid for the duration of the create.
//
#define FO_FLAGS_VALID_ONLY_DURING_CREATE FO_DISALLOW_EXCLUSIVE

typedef struct _IO_COMPLETION_CONTEXT {
	PVOID Port;
	PVOID Key;
} IO_COMPLETION_CONTEXT, *PIO_COMPLETION_CONTEXT;

typedef struct _FILE_OBJECT {
	CSHORT Type;
	CSHORT Size;
	PDEVICE_OBJECT DeviceObject;
	PVPB Vpb;
	PVOID FsContext;
	PVOID FsContext2;
	PSECTION_OBJECT_POINTERS SectionObjectPointer;
	PVOID PrivateCacheMap;
	NTSTATUS FinalStatus;
	struct _FILE_OBJECT *RelatedFileObject;
	BOOLEAN LockOperation;
	BOOLEAN DeletePending;
	BOOLEAN ReadAccess;
	BOOLEAN WriteAccess;
	BOOLEAN DeleteAccess;
	BOOLEAN SharedRead;
	BOOLEAN SharedWrite;
	BOOLEAN SharedDelete;
	ULONG Flags;
	UNICODE_STRING FileName;
	LARGE_INTEGER CurrentByteOffset;
	__volatile ULONG Waiters;
	__volatile ULONG Busy;
	PVOID LastLock;
	KEVENT Lock;
	KEVENT Event;
	__volatile PIO_COMPLETION_CONTEXT CompletionContext;
	KSPIN_LOCK IrpListLock;
	LIST_ENTRY IrpList;
	__volatile PVOID FileObjectExtension;
} FILE_OBJECT;
typedef struct _FILE_OBJECT *PFILE_OBJECT; 

typedef struct _IO_STACK_LOCATION {
	UCHAR MajorFunction;
	UCHAR MinorFunction;
	UCHAR Flags;
	UCHAR Control;

	//
	// The following user parameters are based on the service that is being
	// invoked.  Drivers and file systems can determine which set to use based
	// on the above major and minor function codes.
	//

	union {

		//
		// System service parameters for:  NtCreateFile
		//

		struct {
			PIO_SECURITY_CONTEXT SecurityContext;
			ULONG Options;
			USHORT POINTER_ALIGNMENT FileAttributes;
			USHORT ShareAccess;
			ULONG POINTER_ALIGNMENT EaLength;
		} Create;


		//
		// System service parameters for:  NtReadFile
		//

		struct {
			ULONG Length;
			ULONG POINTER_ALIGNMENT Key;
			LARGE_INTEGER ByteOffset;
		} Read;

		//
		// System service parameters for:  NtWriteFile
		//

		struct {
			ULONG Length;
			ULONG POINTER_ALIGNMENT Key;
			LARGE_INTEGER ByteOffset;
		} Write;

		//
		// System service parameters for:  NtQueryDirectoryFile
		//

		struct {
			ULONG Length;
			PUNICODE_STRING FileName;
			FILE_INFORMATION_CLASS FileInformationClass;
			ULONG POINTER_ALIGNMENT FileIndex;
		} QueryDirectory;

		//
		// System service parameters for:  NtNotifyChangeDirectoryFile
		//

		struct {
			ULONG Length;
			ULONG POINTER_ALIGNMENT CompletionFilter;
		} NotifyDirectory;

		//
		// System service parameters for:  NtQueryInformationFile
		//

		struct {
			ULONG Length;
			FILE_INFORMATION_CLASS POINTER_ALIGNMENT FileInformationClass;
		} QueryFile;

		//
		// System service parameters for:  NtSetInformationFile
		//

		struct {
			ULONG Length;
			FILE_INFORMATION_CLASS POINTER_ALIGNMENT FileInformationClass;
			PFILE_OBJECT FileObject;
			union {
				struct {
					BOOLEAN ReplaceIfExists;
					BOOLEAN AdvanceOnly;
				};
				ULONG ClusterCount;
				HANDLE DeleteHandle;
			};
		} SetFile;



		//
		// System service parameters for:  NtQueryEaFile
		//

		struct {
			ULONG Length;
			PVOID EaList;
			ULONG EaListLength;
			ULONG POINTER_ALIGNMENT EaIndex;
		} QueryEa;

		//
		// System service parameters for:  NtSetEaFile
		//

		struct {
			ULONG Length;
		} SetEa;



		//
		// System service parameters for:  NtQueryVolumeInformationFile
		//

		struct {
			ULONG Length;
			FS_INFORMATION_CLASS POINTER_ALIGNMENT FsInformationClass;
		} QueryVolume;



		//
		// System service parameters for:  NtSetVolumeInformationFile
		//

		struct {
			ULONG Length;
			FS_INFORMATION_CLASS POINTER_ALIGNMENT FsInformationClass;
		} SetVolume;

		//
		// System service parameters for:  NtFsControlFile
		//
		// Note that the user's output buffer is stored in the UserBuffer field
		// and the user's input buffer is stored in the SystemBuffer field.
		//

		struct {
			ULONG OutputBufferLength;
			ULONG POINTER_ALIGNMENT InputBufferLength;
			ULONG POINTER_ALIGNMENT FsControlCode;
			PVOID Type3InputBuffer;
		} FileSystemControl;
		//
		// System service parameters for:  NtLockFile/NtUnlockFile
		//

		struct {
			PLARGE_INTEGER Length;
			ULONG POINTER_ALIGNMENT Key;
			LARGE_INTEGER ByteOffset;
		} LockControl;

		//
		// System service parameters for:  NtFlushBuffersFile
		//
		// No extra user-supplied parameters.
		//



		//
		// System service parameters for:  NtCancelIoFile
		//
		// No extra user-supplied parameters.
		//



		//
		// System service parameters for:  NtDeviceIoControlFile
		//
		// Note that the user's output buffer is stored in the UserBuffer field
		// and the user's input buffer is stored in the SystemBuffer field.
		//

		struct {
			ULONG OutputBufferLength;
			ULONG POINTER_ALIGNMENT InputBufferLength;
			ULONG POINTER_ALIGNMENT IoControlCode;
			PVOID Type3InputBuffer;
		} DeviceIoControl;

		//
		// System service parameters for:  NtQuerySecurityObject
		//

		struct {
			SECURITY_INFORMATION SecurityInformation;
			ULONG POINTER_ALIGNMENT Length;
		} QuerySecurity;

		//
		// System service parameters for:  NtSetSecurityObject
		//

		struct {
			SECURITY_INFORMATION SecurityInformation;
			PSECURITY_DESCRIPTOR SecurityDescriptor;
		} SetSecurity;

		//
		// Non-system service parameters.
		//
		// Parameters for MountVolume
		//

		struct {
			PVPB Vpb;
			PDEVICE_OBJECT DeviceObject;
		} MountVolume;

		//
		// Parameters for VerifyVolume
		//

		struct {
			PVPB Vpb;
			PDEVICE_OBJECT DeviceObject;
		} VerifyVolume;

		//
		// Parameters for Scsi with internal device contorl.
		//

		struct {
			struct _SCSI_REQUEST_BLOCK *Srb;
		} Scsi;



		//
		// System service parameters for:  NtQueryQuotaInformationFile
		//

		struct {
			ULONG Length;
			PSID StartSid;
			PFILE_GET_QUOTA_INFORMATION SidList;
			ULONG SidListLength;
		} QueryQuota;

		//
		// System service parameters for:  NtSetQuotaInformationFile
		//

		struct {
			ULONG Length;
		} SetQuota;



		//
		// Parameters for IRP_MN_QUERY_DEVICE_RELATIONS
		//

		struct {
			DEVICE_RELATION_TYPE Type;
		} QueryDeviceRelations;

		//
		// Parameters for IRP_MN_QUERY_INTERFACE
		//

		struct {
			CONST GUID *InterfaceType;
			USHORT Size;
			USHORT Version;
			PINTERFACE Interface;
			PVOID InterfaceSpecificData;
		} QueryInterface;

		//
		// Parameters for IRP_MN_QUERY_CAPABILITIES
		//

		struct {
			PDEVICE_CAPABILITIES Capabilities;
		} DeviceCapabilities;

		//
		// Parameters for IRP_MN_FILTER_RESOURCE_REQUIREMENTS
		//

		struct {
			PIO_RESOURCE_REQUIREMENTS_LIST IoResourceRequirementList;
		} FilterResourceRequirements;

		//
		// Parameters for IRP_MN_READ_CONFIG and IRP_MN_WRITE_CONFIG
		//

		struct {
			ULONG WhichSpace;
			PVOID Buffer;
			ULONG Offset;
			ULONG POINTER_ALIGNMENT Length;
		} ReadWriteConfig;

		//
		// Parameters for IRP_MN_SET_LOCK
		//

		struct {
			BOOLEAN Lock;
		} SetLock;

		//
		// Parameters for IRP_MN_QUERY_ID
		//

		struct {
			BUS_QUERY_ID_TYPE IdType;
		} QueryId;

		//
		// Parameters for IRP_MN_QUERY_DEVICE_TEXT
		//

		struct {
			DEVICE_TEXT_TYPE DeviceTextType;
			LCID POINTER_ALIGNMENT LocaleId;
		} QueryDeviceText;

		//
		// Parameters for IRP_MN_DEVICE_USAGE_NOTIFICATION
		//

		struct {
			BOOLEAN InPath;
			BOOLEAN Reserved[3];
			DEVICE_USAGE_NOTIFICATION_TYPE POINTER_ALIGNMENT Type;
		} UsageNotification;

		//
		// Parameters for IRP_MN_WAIT_WAKE
		//

		struct {
			SYSTEM_POWER_STATE PowerState;
		} WaitWake;

		//
		// Parameter for IRP_MN_POWER_SEQUENCE
		//

		struct {
			PPOWER_SEQUENCE PowerSequence;
		} PowerSequence;

		//
		// Parameters for IRP_MN_SET_POWER and IRP_MN_QUERY_POWER
		//

#if (NTDDI_VERSION >= NTDDI_VISTA)
		struct {
			union {
				ULONG SystemContext;
				SYSTEM_POWER_STATE_CONTEXT SystemPowerStateContext;
			};
			POWER_STATE_TYPE POINTER_ALIGNMENT Type;
			POWER_STATE POINTER_ALIGNMENT State;
			POWER_ACTION POINTER_ALIGNMENT ShutdownType;
		} Power;
#else
		struct {
			ULONG SystemContext;
			POWER_STATE_TYPE POINTER_ALIGNMENT Type;
			POWER_STATE POINTER_ALIGNMENT State;
			POWER_ACTION POINTER_ALIGNMENT ShutdownType;
		} Power;
#endif // (NTDDI_VERSION >= NTDDI_VISTA)

		//
		// Parameters for StartDevice
		//

		struct {
			PCM_RESOURCE_LIST AllocatedResources;
			PCM_RESOURCE_LIST AllocatedResourcesTranslated;
		} StartDevice;

		//
		// Parameters for Cleanup
		//
		// No extra parameters supplied
		//

		//
		// WMI Irps
		//

		struct {
			ULONG_PTR ProviderId;
			PVOID DataPath;
			ULONG BufferSize;
			PVOID Buffer;
		} WMI;

		//
		// Others - driver-specific
		//

		struct {
			PVOID Argument1;
			PVOID Argument2;
			PVOID Argument3;
			PVOID Argument4;
		} Others;

	} Parameters;

	//
	// Save a pointer to this device driver's device object for this request
	// so it can be passed to the completion routine if needed.
	//

	PDEVICE_OBJECT DeviceObject;

	//
	// The following location contains a pointer to the file object for this
	// request.
	//

	PFILE_OBJECT FileObject;

	//
	// The following routine is invoked depending on the flags in the above
	// flags field.
	//

	PIO_COMPLETION_ROUTINE CompletionRoutine;

	//
	// The following is used to store the address of the context parameter
	// that should be passed to the CompletionRoutine.
	//

	PVOID Context;

} IO_STACK_LOCATION, *PIO_STACK_LOCATION;

typedef struct __declspec( align( 16 ) ) _IRP {
	CSHORT Type;
	USHORT Size;

	//
	// Define the common fields used to control the IRP.
	//

	//
	// Define a pointer to the Memory Descriptor List (MDL) for this I/O
	// request.  This field is only used if the I/O is "direct I/O".
	//

	PMDL MdlAddress;

	//
	// Flags word - used to remember various flags.
	//

	ULONG Flags;

	//
	// The following union is used for one of three purposes:
	//
	//    1. This IRP is an associated IRP.  The field is a pointer to a master
	//       IRP.
	//
	//    2. This is the master IRP.  The field is the count of the number of
	//       IRPs which must complete (associated IRPs) before the master can
	//       complete.
	//
	//    3. This operation is being buffered and the field is the address of
	//       the system space buffer.
	//

	union {
		struct _IRP *MasterIrp;
		__volatile LONG IrpCount;
		PVOID SystemBuffer;
	} AssociatedIrp;

	//
	// Thread list entry - allows queueing the IRP to the thread pending I/O
	// request packet list.
	//

	LIST_ENTRY ThreadListEntry;

	//
	// I/O status - final status of operation.
	//

	IO_STATUS_BLOCK IoStatus;

	//
	// Requestor mode - mode of the original requestor of this operation.
	//

	KPROCESSOR_MODE RequestorMode;

	//
	// Pending returned - TRUE if pending was initially returned as the
	// status for this packet.
	//

	BOOLEAN PendingReturned;

	//
	// Stack state information.
	//

	CHAR StackCount;
	CHAR CurrentLocation;

	//
	// Cancel - packet has been canceled.
	//

	BOOLEAN Cancel;

	//
	// Cancel Irql - Irql at which the cancel spinlock was acquired.
	//

	KIRQL CancelIrql;

	//
	// ApcEnvironment - Used to save the APC environment at the time that the
	// packet was initialized.
	//

	CCHAR ApcEnvironment;

	//
	// Allocation control flags.
	//

	UCHAR AllocationFlags;

	//
	// User parameters.
	//

	PIO_STATUS_BLOCK UserIosb;
	PKEVENT UserEvent;
	union {
		struct {
			union {
				PIO_APC_ROUTINE UserApcRoutine;
				PVOID IssuingProcess;
			};
			PVOID UserApcContext;
		} AsynchronousParameters;
		LARGE_INTEGER AllocationSize;
	} Overlay;

	//
	// CancelRoutine - Used to contain the address of a cancel routine supplied
	// by a device driver when the IRP is in a cancelable state.
	//

	__volatile PDRIVER_CANCEL CancelRoutine;

	//
	// Note that the UserBuffer parameter is outside of the stack so that I/O
	// completion can copy data back into the user's address space without
	// having to know exactly which service was being invoked.  The length
	// of the copy is stored in the second half of the I/O status block. If
	// the UserBuffer field is NULL, then no copy is performed.
	//

	PVOID UserBuffer;

	//
	// Kernel structures
	//
	// The following section contains kernel structures which the IRP needs
	// in order to place various work information in kernel controller system
	// queues.  Because the size and alignment cannot be controlled, they are
	// placed here at the end so they just hang off and do not affect the
	// alignment of other fields in the IRP.
	//

	union {

		struct {

			union {

				//
				// DeviceQueueEntry - The device queue entry field is used to
				// queue the IRP to the device driver device queue.
				//

				KDEVICE_QUEUE_ENTRY DeviceQueueEntry;

				struct {

					//
					// The following are available to the driver to use in
					// whatever manner is desired, while the driver owns the
					// packet.
					//

					PVOID DriverContext[4];

				} ;

			} ;

			//
			// Thread - pointer to caller's Thread Control Block.
			//

			PETHREAD Thread;

			//
			// Auxiliary buffer - pointer to any auxiliary buffer that is
			// required to pass information to a driver that is not contained
			// in a normal buffer.
			//

			PCHAR AuxiliaryBuffer;

			//
			// The following unnamed structure must be exactly identical
			// to the unnamed structure used in the minipacket header used
			// for completion queue entries.
			//

			struct {

				//
				// List entry - used to queue the packet to completion queue, among
				// others.
				//

				LIST_ENTRY ListEntry;

				union {

					//
					// Current stack location - contains a pointer to the current
					// IO_STACK_LOCATION structure in the IRP stack.  This field
					// should never be directly accessed by drivers.  They should
					// use the standard functions.
					//

					struct _IO_STACK_LOCATION *CurrentStackLocation;

					//
					// Minipacket type.
					//

					ULONG PacketType;
				};
			};

			//
			// Original file object - pointer to the original file object
			// that was used to open the file.  This field is owned by the
			// I/O system and should not be used by any other drivers.
			//

			PFILE_OBJECT OriginalFileObject;

		} Overlay;

		//
		// APC - This APC control block is used for the special kernel APC as
		// well as for the caller's APC, if one was specified in the original
		// argument list.  If so, then the APC is reused for the normal APC for
		// whatever mode the caller was in and the "special" routine that is
		// invoked before the APC gets control simply deallocates the IRP.
		//

		KAPC Apc;

		//
		// CompletionKey - This is the key that is used to distinguish
		// individual I/O operations initiated on a single file handle.
		//

		PVOID CompletionKey;

	} Tail;

} IRP;

#ifndef UNDEFINE_IRP
typedef IRP *PIRP;
#else
typedef PVOID PIRP; 
#endif //UNDEFINE_IRP

//
// Define driver unload routine type.
//
__drv_functionClass(DRIVER_UNLOAD)
__drv_requiresIRQL(PASSIVE_LEVEL)
__drv_sameIRQL
typedef
VOID
DRIVER_UNLOAD (
			   __in struct _DRIVER_OBJECT *DriverObject
			   );

typedef DRIVER_UNLOAD *PDRIVER_UNLOAD;

struct _KDPC;

//
// Define the actions that a driver execution routine may request of the
// adapter/controller allocation routines upon return.
//

typedef enum _IO_ALLOCATION_ACTION {
	KeepObject = 1,
	DeallocateObject,
	DeallocateObjectKeepRegisters
} IO_ALLOCATION_ACTION, *PIO_ALLOCATION_ACTION;

typedef
__drv_functionClass(DRIVER_CONTROL)
__drv_sameIRQL
IO_ALLOCATION_ACTION
DRIVER_CONTROL (
				__in struct _DEVICE_OBJECT *DeviceObject,
				__inout struct _IRP *Irp,
				__in PVOID MapRegisterBase,
				__in PVOID Context
				);
typedef DRIVER_CONTROL *PDRIVER_CONTROL;

__drv_functionClass(KDEFERRED_ROUTINE)
__drv_maxIRQL(DISPATCH_LEVEL)
__drv_minIRQL(DISPATCH_LEVEL)
__drv_requiresIRQL(DISPATCH_LEVEL)
__drv_sameIRQL
typedef
VOID
KDEFERRED_ROUTINE (
				   __in struct _KDPC *Dpc,
				   __in_opt PVOID DeferredContext,
				   __in_opt PVOID SystemArgument1,
				   __in_opt PVOID SystemArgument2
				   );

typedef KDEFERRED_ROUTINE *PKDEFERRED_ROUTINE;

typedef struct _KDPC {
	UCHAR Type;
	UCHAR Importance;
	volatile USHORT Number;
	LIST_ENTRY DpcListEntry;
	PKDEFERRED_ROUTINE DeferredRoutine;
	PVOID DeferredContext;
	PVOID SystemArgument1;
	PVOID SystemArgument2;
	__volatile PVOID DpcData;
} KDPC, *PKDPC, *PRKDPC;

typedef struct _WAIT_CONTEXT_BLOCK {
	KDEVICE_QUEUE_ENTRY WaitQueueEntry;
	PDRIVER_CONTROL DeviceRoutine;
	PVOID DeviceContext;
	ULONG NumberOfMapRegisters;
	PVOID DeviceObject;
	PVOID CurrentIrp;
	PKDPC BufferChainingDpc;
} WAIT_CONTEXT_BLOCK, *PWAIT_CONTEXT_BLOCK;

//
// Define Volume Parameter Block (VPB) flags.
//

#define VPB_MOUNTED                     0x00000001
#define VPB_LOCKED                      0x00000002
#define VPB_PERSISTENT                  0x00000004
#define VPB_REMOVE_PENDING              0x00000008
#define VPB_RAW_MOUNT                   0x00000010
#define VPB_DIRECT_WRITES_ALLOWED       0x00000020

typedef struct __declspec(align( 16 ) ) _DEVICE_OBJECT {
	CSHORT Type;
	USHORT Size;
	LONG ReferenceCount;
	struct _DRIVER_OBJECT *DriverObject;
	struct _DEVICE_OBJECT *NextDevice;
	struct _DEVICE_OBJECT *AttachedDevice;
	struct _IRP *CurrentIrp;
	PIO_TIMER Timer;
	ULONG Flags;                                // See above:  DO_...
	ULONG Characteristics;                      // See ntioapi:  FILE_...
	__volatile PVPB Vpb;
	PVOID DeviceExtension;
	DEVICE_TYPE DeviceType;
	CCHAR StackSize;
	union {
		LIST_ENTRY ListEntry;
		WAIT_CONTEXT_BLOCK Wcb;
	} Queue;
	ULONG AlignmentRequirement;
	KDEVICE_QUEUE DeviceQueue;
	KDPC Dpc;

	//
	//  The following field is for exclusive use by the filesystem to keep
	//  track of the number of Fsp threads currently using the device
	//

	ULONG ActiveThreadCount;
	PSECURITY_DESCRIPTOR SecurityDescriptor;
	KEVENT DeviceLock;

	USHORT SectorSize;
	USHORT Spare1;

	struct _DEVOBJ_EXTENSION  *DeviceObjectExtension;
	PVOID  Reserved;

} DEVICE_OBJECT;

typedef struct _DEVICE_OBJECT *PDEVICE_OBJECT; 

#define IRP_MJ_CREATE                   0x00
#define IRP_MJ_CREATE_NAMED_PIPE        0x01
#define IRP_MJ_CLOSE                    0x02
#define IRP_MJ_READ                     0x03
#define IRP_MJ_WRITE                    0x04
#define IRP_MJ_QUERY_INFORMATION        0x05
#define IRP_MJ_SET_INFORMATION          0x06
#define IRP_MJ_QUERY_EA                 0x07
#define IRP_MJ_SET_EA                   0x08
#define IRP_MJ_FLUSH_BUFFERS            0x09
#define IRP_MJ_QUERY_VOLUME_INFORMATION 0x0a
#define IRP_MJ_SET_VOLUME_INFORMATION   0x0b
#define IRP_MJ_DIRECTORY_CONTROL        0x0c
#define IRP_MJ_FILE_SYSTEM_CONTROL      0x0d
#define IRP_MJ_DEVICE_CONTROL           0x0e
#define IRP_MJ_INTERNAL_DEVICE_CONTROL  0x0f
#define IRP_MJ_SHUTDOWN                 0x10
#define IRP_MJ_LOCK_CONTROL             0x11
#define IRP_MJ_CLEANUP                  0x12
#define IRP_MJ_CREATE_MAILSLOT          0x13
#define IRP_MJ_QUERY_SECURITY           0x14
#define IRP_MJ_SET_SECURITY             0x15
#define IRP_MJ_POWER                    0x16
#define IRP_MJ_SYSTEM_CONTROL           0x17
#define IRP_MJ_DEVICE_CHANGE            0x18
#define IRP_MJ_QUERY_QUOTA              0x19
#define IRP_MJ_SET_QUOTA                0x1a
#define IRP_MJ_PNP                      0x1b
#define IRP_MJ_PNP_POWER                IRP_MJ_PNP      // Obsolete....
#define IRP_MJ_MAXIMUM_FUNCTION         0x1b

typedef struct _DRIVER_OBJECT {
	CSHORT Type;
	CSHORT Size;

	//
	// The following links all of the devices created by a single driver
	// together on a list, and the Flags word provides an extensible flag
	// location for driver objects.
	//

	PDEVICE_OBJECT DeviceObject;
	ULONG Flags;

	//
	// The following section describes where the driver is loaded.  The count
	// field is used to count the number of times the driver has had its
	// registered reinitialization routine invoked.
	//

	PVOID DriverStart;
	ULONG DriverSize;
	PVOID DriverSection;
	PDRIVER_EXTENSION DriverExtension;

	//
	// The driver name field is used by the error log thread
	// determine the name of the driver that an I/O request is/was bound.
	//

	UNICODE_STRING DriverName;

	//
	// The following section is for registry support.  Thise is a pointer
	// to the path to the hardware information in the registry
	//

	PUNICODE_STRING HardwareDatabase;

	//
	// The following section contains the optional pointer to an array of
	// alternate entry points to a driver for "fast I/O" support.  Fast I/O
	// is performed by invoking the driver routine directly with separate
	// parameters, rather than using the standard IRP call mechanism.  Note
	// that these functions may only be used for synchronous I/O, and when
	// the file is cached.
	//

	PFAST_IO_DISPATCH FastIoDispatch;

	//
	// The following section describes the entry points to this particular
	// driver.  Note that the major function dispatch table must be the last
	// field in the object so that it remains extensible.
	//

	PDRIVER_INITIALIZE DriverInit;
	PDRIVER_STARTIO DriverStartIo;
	PDRIVER_UNLOAD DriverUnload;
	PDRIVER_DISPATCH MajorFunction[IRP_MJ_MAXIMUM_FUNCTION + 1];

} DRIVER_OBJECT;
typedef struct _DRIVER_OBJECT *PDRIVER_OBJECT; 

typedef enum _MODE {
	KernelMode,
	UserMode,
	MaximumMode
} MODE; 

typedef enum _KWAIT_REASON {
	Executive,
	FreePage,
	PageIn,
	PoolAllocation,
	DelayExecution,
	Suspended,
	UserRequest,
	WrExecutive,
	WrFreePage,
	WrPageIn,
	WrPoolAllocation,
	WrDelayExecution,
	WrSuspended,
	WrUserRequest,
	WrEventPair,
	WrQueue,
	WrLpcReceive,
	WrLpcReply,
	WrVirtualMemory,
	WrPageOut,
	WrRendezvous,
	WrKeyedEvent,
	WrTerminated,
	WrProcessInSwap,
	WrCpuRateControl,
	WrCalloutStack,
	WrKernel,
	WrResource,
	WrPushLock,
	WrMutex,
	WrQuantumEnd,
	WrDispatchInt,
	WrPreempted,
	WrYieldExecution,
	WrFastMutex,
	WrGuardedMutex,
	WrRundown,
	MaximumWaitReason
} KWAIT_REASON;

#ifndef STATUS_SUCCESS 
#define STATUS_SUCCESS 0 
#endif //STATUS_SUCCESS 

typedef unsigned short USRHORT;
typedef LONG NTSTATUS; 
#define VOID void

#ifndef MAKEWORD
#define MAKEWORD(a, b)      ((WORD)(((BYTE)((DWORD)(a) & 0xff)) | ((WORD)((BYTE)((DWORD)(b) & 0xff))) << 8))
#endif //MAKEWORD

#ifndef MAKELONG
#define MAKELONG(a, b)      ((LONG)(((WORD)((DWORD)(a) & 0xffff)) | ((DWORD)((WORD)((DWORD)(b) & 0xffff))) << 16))
#endif //MAKELONG

#ifndef LOWORD
#define LOWORD(l)           ((WORD)((DWORD)(l) & 0xffff))
#endif //LOWORD

#ifndef HIWORD
#define HIWORD(l)           ((WORD)((DWORD)(l) >> 16))
#endif //HIWORD

#ifndef LOBYTE
#define LOBYTE(w)           ((BYTE)((DWORD)(w) & 0xff))
#endif //LOBYTE

#ifndef HIBYTE
#define HIBYTE(w)           ((BYTE)((DWORD)(w) >> 8))
#endif //HIBYTE

#define XCHG(x)         MAKEWORD( HIBYTE(x), LOBYTE(x) )

#define DXCHG(x)        MAKELONG( XCHG(HIWORD(x)), XCHG(LOWORD(x)) )

#define LONIBBLE(b) ((BYTE) ((b) & 0x0F))

#define HINIBBLE(b)     ((BYTE) ((b) >> 4))

#define HEX(b)          (HexTable[LONIBBLE(b)])

#define SWAPBYTES(w)    ((w) = XCHG(w))

#define SWAPWORDS(d)    ((d) = DXCHG(d))

#ifndef FORCEINLINE
#if (_MSC_VER >= 1200)
#define FORCEINLINE __forceinline
#else
#define FORCEINLINE __inline
#endif
#endif

#ifndef LIST_OPERA_DEFINED
FORCEINLINE
VOID
InitializeListHead(
    PLIST_ENTRY ListHead
    )
{
    ListHead->Flink = ListHead->Blink = ListHead;
}

BOOLEAN
FORCEINLINE
IsListEmpty(
    __in const LIST_ENTRY * ListHead
    )
{
    return (BOOLEAN)(ListHead->Flink == ListHead);
}

FORCEINLINE
BOOLEAN
RemoveEntryList(
    __in PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Blink;
    PLIST_ENTRY Flink;

    Flink = Entry->Flink;
    Blink = Entry->Blink;
    Blink->Flink = Flink;
    Flink->Blink = Blink;
    return (BOOLEAN)(Flink == Blink);
}

FORCEINLINE
PLIST_ENTRY
RemoveHeadList(
    __inout PLIST_ENTRY ListHead
    )
{
    PLIST_ENTRY Flink;
    PLIST_ENTRY Entry;

    Entry = ListHead->Flink;
    Flink = Entry->Flink;
    ListHead->Flink = Flink;
    Flink->Blink = ListHead;
    return Entry;
}


FORCEINLINE
PLIST_ENTRY
RemoveTailList(
    __inout PLIST_ENTRY ListHead
    )
{
    PLIST_ENTRY Blink;
    PLIST_ENTRY Entry;

    Entry = ListHead->Blink;
    Blink = Entry->Blink;
    ListHead->Blink = Blink;
    Blink->Flink = ListHead;
    return Entry;
}


FORCEINLINE
VOID
InsertTailList(
    __inout PLIST_ENTRY ListHead,
    __inout __drv_aliasesMem PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Blink;

    Blink = ListHead->Blink;
    Entry->Flink = ListHead;
    Entry->Blink = Blink;
    Blink->Flink = Entry;
    ListHead->Blink = Entry;
}


FORCEINLINE 
VOID
InsertHeadList(
    __inout PLIST_ENTRY ListHead,
    __inout __drv_aliasesMem PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Flink;

    Flink = ListHead->Flink;
    Entry->Flink = Flink;
    Entry->Blink = ListHead;
    Flink->Blink = Entry;
    ListHead->Flink = Entry;
}
#endif //LIST_OPERA_DEFINED

typedef struct __FILTER_IPV4_ADDR
{
	ULONG BlockIP;
} FILTER_IPV4_ADDR, *PFILTER_IPV4_ADDR;

void logfile(const char *format, ...); 
#define DBGPRINT( _x_ ) 
#define MEMDBGPRINT( _x_ ) 

#define KeAcquireSpinLock( lock, irql ) 
#define KeReleaseSpinLock( lock, irql ) 
#define KeInitializeSpinLock( lock ) 

#define ALLOC_TAG_POOL( size ) malloc( size )

#define MmIsAddressValid( x ) TRUE

#define INLINE __inline 
INLINE PVOID ALLOC_ZERO_TAG_POOL( ULONG Size ) 
{
	PVOID Buff;
	Buff = malloc( Size ); 
	if( NULL == Buff )
	{
		return NULL;
	}

	memset( Buff, 0, Size );
	return Buff;
}

INLINE VOID FREE_TAG_POOL( PVOID Buff )
{
	free( Buff );
}

typedef struct __INFO_HEAD
{
	LIST_ENTRY ListEntry; 
	ULONG InfoId; 
} INFO_HEAD, *PINFO_HEAD; 

INLINE PVOID ExAllocatePoolWithTag( CHAR type, ULONG size, ULONG tag )
{
	return ALLOC_TAG_POOL( size ); 
}

//
// Priority increment definitions.  The comment for each definition gives
// the names of the system services that use the definition when satisfying
// a wait.
//

//
// Priority increment used when satisfying a wait on an executive event
// (NtPulseEvent and NtSetEvent)
//

#define EVENT_INCREMENT                 1

//
// Priority increment when no I/O has been done.  This is used by device
// and file system drivers when completing an IRP (IoCompleteRequest).
//

#define IO_NO_INCREMENT                 0


//
// Priority increment for completing CD-ROM I/O.  This is used by CD-ROM device
// and file system drivers when completing an IRP (IoCompleteRequest)
//

#define IO_CD_ROM_INCREMENT             1

//
// Priority increment for completing disk I/O.  This is used by disk device
// and file system drivers when completing an IRP (IoCompleteRequest)
//

#define IO_DISK_INCREMENT               1



//
// Priority increment for completing keyboard I/O.  This is used by keyboard
// device drivers when completing an IRP (IoCompleteRequest)
//

#define IO_KEYBOARD_INCREMENT           6


//
// Priority increment for completing mailslot I/O.  This is used by the mail-
// slot file system driver when completing an IRP (IoCompleteRequest).
//

#define IO_MAILSLOT_INCREMENT           2


//
// Priority increment for completing mouse I/O.  This is used by mouse device
// drivers when completing an IRP (IoCompleteRequest)
//

#define IO_MOUSE_INCREMENT              6


//
// Priority increment for completing named pipe I/O.  This is used by the
// named pipe file system driver when completing an IRP (IoCompleteRequest).
//

#define IO_NAMED_PIPE_INCREMENT         2

//
// Priority increment for completing network I/O.  This is used by network
// device and network file system drivers when completing an IRP
// (IoCompleteRequest).
//

#define IO_NETWORK_INCREMENT            2


//
// Priority increment for completing parallel I/O.  This is used by parallel
// device drivers when completing an IRP (IoCompleteRequest)
//

#define IO_PARALLEL_INCREMENT           1

//
// Priority increment for completing serial I/O.  This is used by serial device
// drivers when completing an IRP (IoCompleteRequest)
//

#define IO_SERIAL_INCREMENT             2

//
// Priority increment for completing sound I/O.  This is used by sound device
// drivers when completing an IRP (IoCompleteRequest)
//

#define IO_SOUND_INCREMENT              8

//
// Priority increment for completing video I/O.  This is used by video device
// drivers when completing an IRP (IoCompleteRequest)
//

#define IO_VIDEO_INCREMENT              1

//
// Priority increment used when satisfying a wait on an executive semaphore
// (NtReleaseSemaphore)
//

#define SEMAPHORE_INCREMENT             1

typedef struct _OBJECT_TYPE *POBJECT_TYPE;

typedef struct _OBJECT_HANDLE_INFORMATION {
	ULONG HandleAttributes;
	ACCESS_MASK GrantedAccess;
} OBJECT_HANDLE_INFORMATION, *POBJECT_HANDLE_INFORMATION;


//typedef struct __LIST_ENTRY
//{
//	PVOID FLink;
//	PVOID BLink;
//} LIST_ENTRY;

#ifndef DUMP_MEM
#define DUMP_MEM DumpMemory
#endif //DUMP_MEM

FORCEINLINE
VOID DumpMemory ( PVOID mem, INT32 size )
{
	unsigned char str[20];
	unsigned char *m = ( BYTE* )mem;
	int i,j;

	for (j = 0; j < size / 8; j++)
	{
		memset (str,0,sizeof str);
		for (i = 0; i < 8; i++) 
		{
			if (m[i] > ' ' && m[i] <= '~')
				str[i]=m[i];
			else
				str[i]='.';
		}

		DBGPRINT( ( "0x%08p  %02x %02x %02x %02x %02x %02x %02x %02x  %s\n",
			m, m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], str ) );

		m+=8;
	}

	//memset (str,0,sizeof str);
	for (i = 0; i < ( size % 8 ); i++) 
	{
		if (m[i] > ' ' && m[i] <= '~')
			str[i]=m[i];
		else
			str[i]='.';
	}

	str[ i ] = '\0';

	DBGPRINT( ( "0x%08p  ", m ) );
	for (i = 0; i < ( size % 8 ); i++) 
	{
		DBGPRINT( ( "%02x ", m[ i ] ) );
	}

	DBGPRINT( ( "%s \n", str ) );
}

FORCEINLINE
VOID PrintIPAddr( ULONG *IPAddr )
{
	BYTE *_IPAddr = ( BYTE* )IPAddr;
	DBGPRINT( ( "%d.%d.%d.%d \n", 
		*_IPAddr, 
		*( _IPAddr + 1 ), 
		*( _IPAddr + 2 ), 
		*( _IPAddr + 3 )
		) );
}

INLINE WCHAR ascii_wupper( WCHAR ch )
{
	if( ch >= L'a' && ch <= L'z' )
	{
		ch = ch + L'A' - L'a'; 
	}

	return ch; 
}

INLINE CHAR ascii_upper( CHAR ch )
{
	if( ch >= 'a' && ch <= 'z' )
	{
		ch = ch + 'A' - 'a'; 
	}

	return ch; 
}

INLINE VOID unicode_str_to_upper( LPWSTR str )
{
	ASSERT( str != NULL ); 

	while( *str != L'\0' )
	{
		*str = ascii_wupper( *str ); 

		str ++; 
	}
}

INLINE VOID ansi_str_to_upper( LPSTR str )
{
	ASSERT( str != NULL ); 

	while( *str != L'\0' )
	{
		*str = ascii_upper( *str ); 

		str ++; 
	}
}

#define hold_sp_lock( lock, irql ) 
#define release_sp_lock( lock, irql ) 

INLINE NTSTATUS copy_unicode_string_to_buf( PUNICODE_STRING uni_str, LPWSTR buf, ULONG buf_len, ULONG *need_len )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	ULONG _need_len; 


	if( uni_str->Buffer[ uni_str->Length / sizeof( WCHAR ) - 1 ] == L'\0' )
	{
		_need_len = uni_str->Length; 
	}
	else
	{
		_need_len = uni_str->Length + sizeof( WCHAR ); 
	}

	if( need_len != NULL )
	{
		*need_len = _need_len; 
	}

	if( _need_len > buf_len )
	{
		_need_len = buf_len; 
		ntstatus = STATUS_BUFFER_TOO_SMALL; 
	}

	memcpy( buf, uni_str->Buffer, uni_str->Length ); 
	if( buf[ uni_str->Length / sizeof( WCHAR ) - 1 ] != L'\0' )
	{
		buf[ _need_len / sizeof( WCHAR ) - 1 ] = L'\0'; 
	}

	//_return:
	return ntstatus; 
}

FORCEINLINE
__drv_aliasesMem
PIO_STACK_LOCATION
IoGetCurrentIrpStackLocation(
    __in PIRP Irp
)
/*--

Routine Description:

    This routine is invoked to return a pointer to the current stack location
    in an I/O Request Packet (IRP).

Arguments:

    Irp - Pointer to the I/O Request Packet.

Return Value:

    The function value is a pointer to the current stack location in the
    packet.

--*/
{
    //ASSERT(Irp->CurrentLocation <= Irp->StackCount + 1);
    //return Irp->Tail.Overlay.CurrentStackLocation;
	return NULL; 
}
//
//INLINE VOID ExFreePoolWithTag( PVOID pool, ULONG tag )
//{
//	FREE_TAG_POOL( pool ); 
//}

#define free_pool( buf ) free( buf )

typedef enum _POOL_TYPE {
	NonPagedPool,
	PagedPool,
	NonPagedPoolMustSucceed,
	DontUseThisType,
	NonPagedPoolCacheAligned,
	PagedPoolCacheAligned,
	NonPagedPoolCacheAlignedMustS,
	MaxPoolType,

	//
	// Note these per session types are carefully chosen so that the appropriate
	// masking still applies as well as MaxPoolType above.
	//

	NonPagedPoolSession = 32,
	PagedPoolSession = NonPagedPoolSession + 1,
	NonPagedPoolMustSucceedSession = PagedPoolSession + 1,
	DontUseThisTypeSession = NonPagedPoolMustSucceedSession + 1,
	NonPagedPoolCacheAlignedSession = DontUseThisTypeSession + 1,
	PagedPoolCacheAlignedSession = NonPagedPoolCacheAlignedSession + 1,
	NonPagedPoolCacheAlignedMustSSession = PagedPoolCacheAlignedSession + 1,
} POOL_TYPE;
#endif //__RING0_2_RING3_H__