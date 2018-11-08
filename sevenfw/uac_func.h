#ifndef __UAC_FUNC_H__
#define __UAC_FUNC_H__

BOOL IsUserInAdminGroup(); 
BOOL IsRunAsAdmin(); 
BOOL IsProcessElevated(); 
DWORD GetProcessIntegrityLevel(); 

LRESULT correct_user_path( LPWSTR file_name, ULONG buf_len ); 

LRESULT get_user_name_from_token( HANDLE dest_token, 
								 LPTSTR user_name, 
								 ULONG max_user_name_len, 
								 LPTSTR domain_name, 
								 ULONG max_domain_name_len ); 

LRESULT get_current_user_name( LPTSTR user_name, 
							  ULONG max_user_name_len, 
							  LPTSTR domain_name, 
							  ULONG max_domain_name_len ); 

LRESULT get_logged_on_user_name( LPTSTR user_name, ULONG max_user_name_len ); 


#endif //__UAC_FUNC_H__