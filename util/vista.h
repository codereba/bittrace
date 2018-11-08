#ifndef _VISTA_INC_
#define _VISTA_INC_

#include <atlsecurity.h>

BOOL	WINAPI	IsVistaLater();
BOOL	WINAPI	IsOpenUAC();

#define MSGFLT_ADD		1
#define MSGFLT_REMOVE	2
BOOL	WINAPI	Vista_ChangeWindowMessageFilter(UINT uMessage, int nFlag);


inline BOOL SetSecurityDescriptorToLowIntegrity(ATL::CSecurityDesc &sd)
{
	LPCWSTR LOW_INTEGRITY_SDDL_SACL_W = L"S:(ML;;NW;;;LW)";

	BOOL  bRet           = FALSE;
	PACL  pSacl          = NULL;
	BOOL  fSaclPresent   = FALSE;
	BOOL  fSaclDefaulted = FALSE;
	PSECURITY_DESCRIPTOR pSD = NULL;
	ATL::CSacl LowIntegrityAcl;
	OSVERSIONINFOW VersionInfo = {0};
	VersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
	if( !GetVersionExW( &VersionInfo ) )
	{
		return FALSE;
	}

	if( VersionInfo.dwMajorVersion < 6 )
	{
		return TRUE;
	}

	if (ConvertStringSecurityDescriptorToSecurityDescriptorW(LOW_INTEGRITY_SDDL_SACL_W,SDDL_REVISION_1,&pSD,NULL))
	{
		if(GetSecurityDescriptorSacl(pSD,&fSaclPresent,&pSacl,&fSaclDefaulted))
		{
			ACL_SIZE_INFORMATION SaclSize = {0}; 
			if (::GetAclInformation(pSacl,  &SaclSize,  sizeof(SaclSize),  AclSizeInformation))
			{
				// The CSecurityDesc destructor expects the memory to have been malloced. 
				PACL pNewSacl = static_cast<PACL>(malloc(SaclSize.AclBytesInUse)); 
				if( pNewSacl )
				{
					::CopyMemory(pNewSacl, pSacl, SaclSize.AclBytesInUse); 

					CSacl OldSacl; 
					sd.GetSacl( &OldSacl );
					if(::SetSecurityDescriptorSacl( const_cast<SECURITY_DESCRIPTOR*>(sd.GetPSECURITY_DESCRIPTOR()), fSaclPresent, pNewSacl, fSaclDefaulted))
					{
						bRet = TRUE;
					} 
					else
					{
						free( pNewSacl );
					}
				}
			} 
		}
		LocalFree ( pSD );
	}

	return bRet;
}
#endif /* _VISTA_INC_ */