#ifndef __COMMON_API_H__
#define __COMMON_API_H__

#ifndef INLINE 
#define INLINE __inline
#endif //INLINE 

#define DRIVER_WORK_STATE_SW_ID 0
typedef struct _sw_ctrl
{
	ULONG id; 
	ULONG state; 
} sw_ctrl, *psw_ctrl; 

#define IOCTL_DRIVER_WORK_STATE CTL_CODE( FILE_DEVICE_UNKNOWN, 0x0998, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_SET_DBG_LOG_LEVEL CTL_CODE( FILE_DEVICE_UNKNOWN, 0x0999, METHOD_BUFFERED, FILE_ANY_ACCESS )

INLINE LPCWSTR get_common_driver_io_ctl_desc_w( ULONG ctl_code )
{
	LPCWSTR desc = L"UNKNOWN CONTROL CODE"; 

	switch( ctl_code )
	{
	case IOCTL_DRIVER_WORK_STATE:
		desc = L"IOCTL_DRIVER_WORK_STATE"; 
		break; 
	case IOCTL_SET_DBG_LOG_LEVEL:
		desc = L"IOCTL_SET_DBG_LOG_LEVEL"; 
		break; 
	default:
		break; 
	}

	return desc; 
}

INLINE LPCSTR get_common_driver_io_ctl_desc_a( ULONG ctl_code )
{
	LPCSTR desc = "UNKNOWN CONTROL CODE"; 

	switch( ctl_code )
	{
	case IOCTL_DRIVER_WORK_STATE:
		desc = "IOCTL_DRIVER_WORK_STATE"; 
		break; 
	case IOCTL_SET_DBG_LOG_LEVEL:
		desc = "IOCTL_SET_DBG_LOG_LEVEL"; 
		break; 
	default:
		break; 
	}

	return desc; 
}

#ifdef DRIVER
INLINE NTSTATUS _set_dbg_log_level( PVOID input_buf, ULONG input_len )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	ULONG log_level; 

	do 
	{
		if( input_len < sizeof( ULONG ) )
		{
			ntstatus = STATUS_INVALID_PARAMETER; 
			break; 
		}

		log_level = *( ULONG* )input_buf; 

		set_dbg_log_level( log_level ); 
	} while ( FALSE ); 

	return ntstatus; 
}
#endif //DRIVER

#endif //__COMMON_API_H__