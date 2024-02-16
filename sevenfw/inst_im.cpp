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

#include "common_func.h"
#include "wdk_macros.h"
#include <stdio.h>
#include <tchar.h>
#include "inst_im.h"
#include <Strsafe.h>

#pragma comment( lib, "Setupapi.lib" )

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


ndis_drv_installer::ndis_drv_installer():m_hwnd(NULL),m_IsSetup(false)
{

}

ndis_drv_installer::ndis_drv_installer(HWND hWnd)
{
	m_hwnd=hWnd;
}

ndis_drv_installer::~ndis_drv_installer()
{

}

void ndis_drv_installer::set_main_wnd( HWND hWnd )
{
	m_hwnd = hWnd;	
}

LRESULT ndis_drv_installer::install_ndis_drv( ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	DWORD _ret; 
	WCHAR img_file_path[ MAX_PATH ] = { 0 }; 
	WCHAR *exe_file_name; 
	WCHAR file_in_sys_dir[ _MAX_PATH ]={0};
	WCHAR szDir[_MAX_DIR]={0};
	WCHAR szDrive[_MAX_DRIVE]={0};
	WCHAR szInfPath[_MAX_PATH]={0};
	WCHAR szInfPath2[_MAX_PATH]={0};
	LPWSTR lpszPnpID = NULL;
	HRESULT hr; 
	LPWSTR lock_id = NULL;
	INetCfg *net_cfg = NULL;

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__  ) ); 
	
	m_IsSetup = false; 
	_ret = GetModuleFileNameW( NULL, img_file_path, MAX_PATH );
	if( _ret == 0 )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	exe_file_name = wcsrchr( img_file_path, L'\\' ); 
	if( exe_file_name == NULL )
	{
		ASSERT( FALSE ); 
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	_ret = GetSystemDirectoryW( file_in_sys_dir, MAX_PATH );
	if( _ret == 0 ) 
	{
		ret = GetLastError(); 
		goto _return; 
	}

	hr = StringCchCatW( file_in_sys_dir, ARRAY_SIZE( file_in_sys_dir ), const_cast< LPCWSTR >( L"\\drivers\\" ) );
	if( FAILED( hr ) )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		log_trace( ( MSG_ERROR, "comprise the file name on system failed \n" ) ); 
		goto _return; 
	}

	hr = StringCchCopyW( m_exe_module_name, ARRAY_SIZE( m_exe_module_name ), exe_file_name + 1 );
	if( FAILED( hr ) )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		ASSERT( FALSE ); 
		goto _return; 
	}

	_wsplitpath( file_in_sys_dir, 
		szDrive, 
		szDir,
		NULL, 
		NULL);	
	hr = StringCchCopyW( szInfPath, sizeof( szInfPath ), szDrive ); 
	if( FAILED( hr ) )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		log_trace( ( MSG_ERROR, "comprise the inf file name on system failed \n" ) ); 
		ASSERT( FALSE ); ; 
		goto _return; 
	}

	hr = StringCchCatW( szInfPath, sizeof( szInfPath ), szDir ); 
	if( FAILED( hr ) )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		log_trace( ( MSG_ERROR, "comprise the inf file name on system failed \n" ) ); 
		ASSERT( FALSE ); ; 
		goto _return; 
	}

	hr = StringCchCopyW( szInfPath2, sizeof( szInfPath2 ), szInfPath ); 
	if( FAILED( hr ) )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		log_trace( ( MSG_ERROR, "comprise the inf file name on system failed \n" ) ); 
		ASSERT( FALSE ); ; 
		goto _return; 
	}

	if( 0 == ( flags & INSTALL_SEVENFW_WIN7_DRV ) )
	{
		hr = StringCchCatW( szInfPath, sizeof( szInfPath ), L"sevenfw_m.inf" );
		if( FAILED( hr ) )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			log_trace( ( MSG_ERROR, "comprise the inf file name on system failed \n" ) ); 
			ASSERT( FALSE ); ; 
			goto _return; 
		}

		_ret = ( DWORD )SetupCopyOEMInfW(
			szInfPath,
			szInfPath2, // Other files are in the
			// same dir. as primary INF
			SPOST_PATH,     // First param is path to INF
			0, /*SP_COPY_DELETESOURCE*/            // Default copy style
			NULL,           // Name of the INF after
			// it's copied to %windir%\inf
			0,              // Max buf. size for the above
			NULL,           // Required size if non-null
			NULL);           // Optionally get the filename part of Inf name after it is copied.

		if( _ret == FALSE )
		{
			//ret = GetLastError(); 
			SAFE_SET_ERROR_CODE( ret ); 

			log_trace( ( DBG_MSG_AND_ERROR_OUT, "copy sevenfw inf files to system directory failed \n" ) ); 
			goto _return; 
		}
	}

	hr = StringCchCopyW( szInfPath, sizeof( szInfPath ), szDrive ); 
	if( FAILED( hr ) )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		log_trace( ( MSG_ERROR, "comprise the inf file name on system failed \n" ) ); 
		ASSERT( FALSE ); ; 
		goto _return; 
	}

	hr = StringCchCatW( szInfPath, sizeof( szInfPath ), szDir ); 
	if( FAILED( hr ) )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		log_trace( ( MSG_ERROR, "comprise the inf file name on system failed \n" ) ); 
		ASSERT( FALSE ); ; 
		goto _return; 
	}

	if( 0 != ( flags & INSTALL_SEVENFW_WIN7_DRV ) )
	{
		hr = StringCchCatW( szInfPath, sizeof( szInfPath ), L"sevenfwex.inf" );
	}
	else
	{
		hr = StringCchCatW( szInfPath, sizeof( szInfPath ), L"sevenfw.inf" );
	}

	if( FAILED( hr ) )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		log_trace( ( MSG_ERROR, "comprise the inf file name on system failed \n" ) ); 
		ASSERT( FALSE ); ; 
		goto _return; 
	}

	hr = GetPnpID( szInfPath, &lpszPnpID ); 
	if( FAILED( hr ) )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "get pnp id from the inf file failed\n" ) ); 
		goto _return; 
	}

	hr = HrGetINetCfg( TRUE, m_exe_module_name, &net_cfg, &lock_id );
	if( FAILED( hr ) || hr == S_FALSE )
	{
		if ( ( hr == NETCFG_E_NO_WRITE_LOCK) && lock_id )
		{
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "no other free net configuration wirte lock exist %d\n", hr ) ); 
		}
		else
		{
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "get net configuration if failed %d\n", hr ) ); 
		}

		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	hr = HrInstallNetComponent( net_cfg,
		lpszPnpID,
		&GUID_DEVCLASS_NETSERVICE,
		szInfPath );
	if ( FAILED( hr ) )	
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "install net component failed 0x%0.8x\n", hr ) ); 
		goto _return; 
	}

	hr = net_cfg->Apply(); 
	if ( FAILED( hr ) )	
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "apply net component configure failed 0x%0.8x\n", hr ) ); 
		goto _return; 
	}

	m_IsSetup = true; 

_return: 
	if( net_cfg != NULL )
	{
		hr = HrReleaseINetCfg( net_cfg, TRUE ); 
		if( FAILED( hr ) )
		{
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "!!!release net configure if failed 0x%0.8x\n", hr ) ); 
		}
	}

	if( lpszPnpID != NULL )
	{
		CoTaskMemFree( lpszPnpID );
	}
		
	if( lock_id != NULL )
	{
		CoTaskMemFree( lock_id );
	}

	if( FAILED( hr ) )
	{
		log_trace( ( MSG_INFO, "install failed return 0x%0.8x \n", hr ) ); ;
	}

	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__  ) ); 
	return ret; 
}

HRESULT ndis_drv_installer::uninstall_ndis_drv( ULONG flags )
{	
	INetCfg               *net_cfg = NULL;
	INetCfgComponent      *component = NULL;
	INetCfgClass          *cfg_class = NULL;
	INetCfgClassSetup     *cfg_setup = NULL;
	LPWSTR                lock_id = NULL;
	GUID                  class_guid;
	OBO_TOKEN             obo;
	HRESULT               hr;
	
	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	hr = HrGetINetCfg( TRUE, m_exe_module_name, &net_cfg, &lock_id ); 

	log_trace( ( MSG_INFO, "HrGetINetCfg return 0x%0.8x \n", hr ) ); 

	if( hr != S_OK )
	{
		if( ( hr == NETCFG_E_NO_WRITE_LOCK ) )
		{
			log_trace( ( MSG_INFO | DBG_ERROR_MSG, "net configure lock write lock is locked \n" ) ); 
		}
		else
		{
			log_trace( ( MSG_INFO | DBG_ERROR_MSG, "can't uninstall��maybe is the inf file is currupt!" ) );
		}

		goto _return; 
	}
	if( 0 != ( flags & INSTALL_SEVENFW_WIN7_DRV ) )
	{
		hr = net_cfg->FindComponent(L"MS_SevenfwEx", &component ); 
	}
	else
	{
		hr = net_cfg->FindComponent(L"ms_sevenfw", &component ); 
	}

	if ( hr != S_OK )
	{
		log_trace( ( MSG_INFO | DBG_ERROR_MSG, "find the net component %s failed %d\n", "ms_sevenfw", hr ) ); 
		goto _return; 
	}
	hr = component->GetClassGuid( &class_guid );	
	if ( hr != S_OK )
	{
		log_trace( ( MSG_INFO | DBG_ERROR_MSG, "get the net component class guid failed %d \n", hr ) ); 
		goto _return; 
	}

	hr = net_cfg->QueryNetCfgClass( &class_guid,
		IID_INetCfgClass,
		(PVOID *)&cfg_class );

	if( hr != S_OK )
	{
		log_trace( ( MSG_INFO, "get the net component's class failed %d\n", hr ) ); 
		goto _return; 
	}

	hr = cfg_class->QueryInterface( IID_INetCfgClassSetup,
		( LPVOID* )&cfg_setup );

	if ( hr != S_OK )
	{
		ASSERT( cfg_setup != NULL ); 
		log_trace( ( MSG_INFO | DBG_ERROR_MSG, "get the net configuration class setup if failed, return %d\n", hr ) ); 
		goto _return; 
	}

	ZeroMemory( &obo, sizeof( OBO_TOKEN ) );
	obo.Type = OBO_USER;

	hr = cfg_setup->DeInstall( component,
		&obo, NULL ); 

	if ( ( hr != S_OK ) && ( hr != NETCFG_S_REBOOT ) )
	{
		log_trace( ( MSG_INFO, "uninstall net component failed, return %d\n", hr ) ); ;
		goto _return; 
	}

	hr = net_cfg->Apply();

	if ( ( hr != S_OK ) && ( hr != NETCFG_S_REBOOT ) )
	{
		log_trace( ( MSG_INFO | DBG_ERROR_MSG, "apply uninstall configuration failed return %d\n", hr ) );
	}
	else
	{
		m_IsSetup = false;
		log_trace( ( MSG_INFO, "apply uninstall configuration successfully" ) ); 
	}
_return:

	if( cfg_setup != NULL )
	{
		ReleaseRef( cfg_setup );
	}

	if( component != NULL )
	{
		ReleaseRef( component );
	}

	if( lock_id != NULL )
	{
		CoTaskMemFree( lock_id );
	}

	if( net_cfg != NULL )
	{
		HrReleaseINetCfg( net_cfg, TRUE );
	}

	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__ ) ); 
	return hr;
}

HRESULT ndis_drv_installer::HrGetINetCfg( 
	IN BOOL fGetWriteLock, 
	IN LPWSTR lpszAppName, 
	OUT INetCfg** net_cfg, 
	LPWSTR *lock_id )
{
    INetCfg      *_net_cfg = NULL;
    INetCfgLock  *cfg_lock= NULL;
    HRESULT      hr = S_OK;

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__  ) ); 

    *net_cfg = NULL;

    if ( lock_id != NULL )
    {
        *lock_id = NULL;
    }

    hr = CoInitialize( NULL );

    if ( FAILED( hr ) ) 
	{
		log_trace( ( MSG_INFO, "CoInitialize failed return 0x%0.8x\n", hr ) ); 
		goto _return; 
	}

	hr = CoCreateInstance( CLSID_CNetCfg,
		NULL, CLSCTX_INPROC_SERVER,
		IID_INetCfg,
		( void** )&_net_cfg ); 

	if ( FAILED( hr ) ) 
	{
		log_trace( ( MSG_INFO, "CoCreateInstance failed return 0x%0.8x\n", hr ) ); 
		goto _return; 
	}

	if( fGetWriteLock == TRUE ) 
	{
		hr = _net_cfg->QueryInterface( IID_INetCfgLock,
			(LPVOID *)&cfg_lock );
		if ( FAILED( hr ) ) 
		{
			log_trace( ( MSG_INFO, "QueryInterface failed return 0x%0.8x\n", hr ) ); 
			goto _return; 
		}

		hr = cfg_lock->AcquireWriteLock( 10,
			lpszAppName,
			lock_id );

		if( hr != S_OK ) 
		{
			log_trace( ( MSG_INFO, "AcquireWriteLock failed return 0x%0.8x\n", hr ) ); 
			goto _return; 
		}
	}

	hr = _net_cfg->Initialize( NULL );

	if ( FAILED( hr  ) ) 
	{

		if ( cfg_lock != NULL ) 
		{
			cfg_lock->ReleaseWriteLock();
		}

		log_trace( ( MSG_INFO, "Initialize failed return 0x%0.8x\n", hr ) ); 
		goto _return; 
	}

	*net_cfg = _net_cfg;
_return:

	if( cfg_lock != NULL )
	{
		ReleaseRef( cfg_lock );
	}

	if ( FAILED( hr ) ) 
	{
		if( _net_cfg != NULL )
		{
			ReleaseRef( _net_cfg ); 
		}

		CoUninitialize();
	}

	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__  ) ); 
    return hr;
}

HRESULT ndis_drv_installer::HrReleaseINetCfg( IN INetCfg* net_cfg, IN BOOL fHasWriteLock )
{
    INetCfgLock    *cfg_lock= NULL;
    HRESULT        hr = S_OK;

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__  ) ); 

    hr = net_cfg->Uninitialize();

    if ( FAILED( hr ) )
	{
		goto _return; 
	}
	
	if( fHasWriteLock == TRUE ) 
	{
        hr = net_cfg->QueryInterface( IID_INetCfgLock,
                                  ( LPVOID* )&cfg_lock );
        if( FAILED( hr ) ) 
		{
			goto _return; 
		}

		hr = cfg_lock->ReleaseWriteLock(); 
    }

_return:
	if( cfg_lock != NULL )
	{
		ReleaseRef( cfg_lock);
	}

	if( net_cfg != NULL )
	{
		ReleaseRef( net_cfg );
	}


    CoUninitialize();

	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__  ) ); 
    return hr;
}


HRESULT ndis_drv_installer::HrInstallNetComponent( IN INetCfg *net_cfg, 
                               IN LPWSTR component_id, 
                               IN const GUID *class_guid, 
                               IN LPWSTR inf_file )
{
    DWORD     dwError;
    HRESULT   hr = S_OK;
    WCHAR     Drive[_MAX_DRIVE];
    WCHAR     Dir[_MAX_DIR];
    WCHAR     DestDir[_MAX_DRIVE+_MAX_DIR];

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__  ) ); 

	_wsplitpath(inf_file,Drive,        
		Dir,
		NULL,
		NULL);
	
	StringCchCopyW(DestDir,sizeof(DestDir),Drive); 
	StringCchCatW(DestDir,sizeof(DestDir),Dir);
	if( FALSE == SetupCopyOEMInfW( 
		(LPWSTR)inf_file, 
		(LPWSTR)DestDir, // Other files are in the 
		SPOST_PATH,    // First param is path to INF 
		NULL,// Default copy style 
		NULL,          // Name of the INF after 
		// it's copied to %windir%\inf 
		0,              // Max buf. size for the above 
		NULL,          // Required size if non-null 
		NULL)          // Optionally get the filename 
		// part of Inf name after it is copied. 
		) 
	{
		dwError = GetLastError();
		hr = HRESULT_FROM_WIN32( dwError );	
		
		log_trace( ( MSG_INFO, "copy inf file failed %d\n", hr ) ); 
		
		goto _return; 
	}

	hr = HrInstallComponent( net_cfg, component_id, class_guid );
	
	if ( FAILED( hr ) ) 
	{
		log_trace( ( MSG_INFO, "install net component failed %d\n", hr ) ); 
		goto _return; 
	}

	hr = net_cfg->Apply();
	m_IsSetup = true;
	log_trace( ( MSG_INFO, "apply net component install setup %d\n", hr ) ); 

_return:
	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__  ) ); 
    return hr;
}


HRESULT ndis_drv_installer::HrInstallComponent(IN INetCfg* net_cfg,
                           IN LPWSTR szComponentId,
                           IN const GUID* class_guid)
{
    INetCfgClassSetup   *cfg_setup = NULL;
    INetCfgComponent    *component = NULL;
    OBO_TOKEN           oho;
    HRESULT             hr = S_OK;

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__  ) ); 

    ZeroMemory( &oho, sizeof( oho ) ); 
    oho.Type = OBO_USER;

    hr = net_cfg->QueryNetCfgClass ( class_guid,
                                 IID_INetCfgClassSetup,
                                 (void**)&cfg_setup );
    if( FAILED( hr ) ) 
	{
		goto _return; 
	}

	hr = cfg_setup->Install( szComponentId,
		&oho,
		0,
		0,       // Upgrade from build number.
		NULL,    // Answerfile name
		NULL,    // Answerfile section name
		&component ); // Reference after the component
	
	if( FAILED( hr ) ) 
	{
		goto _return; 
	}

_return: 
	if( component != NULL )
	{
		ReleaseRef( component );
	}

	if( cfg_setup != NULL )
	{
		ReleaseRef( cfg_setup );
	}

	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__  ) ); 
	return hr;
}


HRESULT ndis_drv_installer::HrUninstallNetComponent(IN INetCfg* net_cfg,
                                IN LPWSTR szComponentId)
{
    INetCfgComponent    *component = NULL;
    INetCfgClass        *cfg_class = NULL;
    INetCfgClassSetup   *cfg_setup = NULL;
    OBO_TOKEN           oho;
    GUID                class_guid;
    HRESULT             hr = S_OK;

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__  ) ); 


    ZeroMemory( &oho,sizeof(oho));
    oho.Type = OBO_USER;
    hr = net_cfg->FindComponent( szComponentId, &component );

    if( FAILED( hr ) )
	{
		log_trace( ( MSG_INFO, "find the net conponet failed %d \n", hr ) ); 
		goto _return; 		;
	}	

	hr = component->GetClassGuid( &class_guid );
	if ( FAILED( hr ) )
	{
		log_trace( ( MSG_INFO, "get the net component class id failed %d\n", hr ) ); 
		goto _return; 
	}

	hr = net_cfg->QueryNetCfgClass( 
		&class_guid,
		IID_INetCfgClass,
		(void**)&cfg_class ); 

	if ( FAILED( hr ) ){
		log_trace( ( MSG_INFO, "query net configuration class failed %d \n", hr ) ); 
		goto _return; 
	}

	hr = cfg_class->QueryInterface(IID_INetCfgClassSetup,(void**)&cfg_setup );
	if ( FAILED( hr ) )
	{
		log_trace( ( MSG_INFO, "get the net component class setup if failed %d \n", hr ) );
		goto _return; 
	}

	hr = cfg_setup->DeInstall( component,
		&oho,
		NULL ); 

	if( FAILED( hr ) )
	{
		log_trace( ( MSG_INFO, "uninstall the net component failed %d\n", hr ) ); 
		goto _return; 
	}

	hr = net_cfg->Apply();

_return:
	if( cfg_setup != NULL )
	{
		ReleaseRef( cfg_setup );
	}

	if( cfg_class != NULL )
	{
		ReleaseRef( cfg_class );
	}

	if( component != NULL )
	{
		ReleaseRef( component );
	}

	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__  ) ); 
    return hr;
}

HRESULT ndis_drv_installer::HrGetComponentEnum (INetCfg* net_cfg,
                            IN const GUID* class_guid,
                            OUT IEnumNetCfgComponent **ppencc)
{
    INetCfgClass  *pncclass;
    HRESULT       hr;

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__  ) ); 
    *ppencc = NULL;

    hr = net_cfg->QueryNetCfgClass( class_guid,
                                IID_INetCfgClass,
                                (PVOID *)&pncclass );

    if ( FAILED( hr ) ) 
	{
		goto _return; 
	}
	

	hr = pncclass->EnumComponents( ppencc );

	ReleaseRef( pncclass );

_return:
	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__  ) ); 
    return hr;
}

HRESULT ndis_drv_installer::HrGetFirstComponent (IN IEnumNetCfgComponent* pencc,
                             OUT INetCfgComponent **ppncc)
{
    HRESULT  hr;
    ULONG    ulCount;

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__  ) ); 
    
	*ppncc = NULL;

    pencc->Reset();

	hr = pencc->Next( 1,ppncc,&ulCount );

	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__  ) ); 
	return hr;
}

HRESULT ndis_drv_installer::HrGetNextComponent (IN IEnumNetCfgComponent* pencc,
                            OUT INetCfgComponent **ppncc)
{
    HRESULT  hr;
    ULONG    ulCount;

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__  ) ); 

    *ppncc = NULL;

    hr = pencc->Next(1,ppncc,&ulCount );

	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__  ) ); 
    return hr;
}

HRESULT ndis_drv_installer::HrFindNetComponentByPnpId (IN INetCfg *net_cfg,
                                   IN LPWSTR lpszPnpDevNodeId,
                                   OUT INetCfgComponent **ppncc)
{
    IEnumNetCfgComponent    *pencc;
    LPWSTR                  pszPnpId;
    HRESULT                 hr;
    BOOL                    fFound;

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__  ) ); 
	
    hr = HrGetComponentEnum( net_cfg,
                             &GUID_DEVCLASS_NET,
                             &pencc );
    if( FAILED( hr ) ) 
	{
		goto _return; 
	}

	hr = HrGetFirstComponent( pencc, ppncc );

	fFound = FALSE;

	while( hr == S_OK ) 
	{
		hr = ( *ppncc )->GetPnpDevNodeId( &pszPnpId );
		if( hr == S_OK ) 
		{
			fFound = wcscmp( pszPnpId, lpszPnpDevNodeId ) == 0;

			CoTaskMemFree( pszPnpId );
			if ( fFound ) 
			{
				break;
			}
		}

		ReleaseRef( *ppncc );
		hr = HrGetNextComponent( pencc, ppncc );
	}

	ReleaseRef( pencc );

_return:
	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__  ) ); 
    return hr;
}


HRESULT ndis_drv_installer::HrGetBindingPathEnum (IN INetCfgComponent *component,
                              IN DWORD dwBindingType,
                              OUT IEnumNetCfgBindingPath **ppencbp)
{
    INetCfgComponentBindings *pnccb = NULL;
    HRESULT                  hr;

    *ppencbp = NULL;

    hr = component->QueryInterface( IID_INetCfgComponentBindings,
                               (PVOID *)&pnccb );

    if ( FAILED( hr ) ) 
	{
		goto _return; 
	}

	hr = pnccb->EnumBindingPaths( dwBindingType,
		ppencbp );

	ReleaseRef( pnccb );

_return:
	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__  ) ); 
    return hr;
}


HRESULT ndis_drv_installer::HrGetFirstBindingPath (IN IEnumNetCfgBindingPath *pencbp,
                               OUT INetCfgBindingPath **ppncbp)
{
    ULONG   ulCount;
    HRESULT hr;
	
	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__  ) ); 

    *ppncbp = NULL;

    pencbp->Reset();
    hr = pencbp->Next( 1,ppncbp, &ulCount );

	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__  ) ); 
    return hr;
}

HRESULT ndis_drv_installer::HrGetNextBindingPath (IN IEnumNetCfgBindingPath *pencbp,
                              OUT INetCfgBindingPath **ppncbp)
{
    ULONG   ulCount;
    HRESULT hr;
	
	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__  ) ); 

    *ppncbp = NULL;
    hr = pencbp->Next( 1,ppncbp,&ulCount );
	
	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__  ) ); 
    return hr;
}


HRESULT ndis_drv_installer::HrGetBindingInterfaceEnum (IN INetCfgBindingPath *pncbp,
                                   OUT IEnumNetCfgBindingInterface **ppencbi)
{
    HRESULT hr;
	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__  ) ); 
    *ppencbi = NULL;
    hr = pncbp->EnumBindingInterfaces( ppencbi );
	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__  ) ); 
    return hr;
}

HRESULT ndis_drv_installer::HrGetFirstBindingInterface (IN IEnumNetCfgBindingInterface *pencbi,
                                    OUT INetCfgBindingInterface **ppncbi)
{
    ULONG   ulCount;
    HRESULT hr;

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__  ) ); 
    *ppncbi = NULL;
    pencbi->Reset();
    hr = pencbi->Next( 1,ppncbi,&ulCount );

	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__  ) ); 
    return hr;
}


HRESULT ndis_drv_installer::HrGetNextBindingInterface (IN IEnumNetCfgBindingInterface *pencbi,
                                   OUT INetCfgBindingInterface **ppncbi)
{
    ULONG   ulCount;
    HRESULT hr;

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__  ) ); 

    *ppncbi = NULL;
    hr = pencbi->Next(1,ppncbi,&ulCount );

	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__  ) ); 
    return hr;
}

VOID ndis_drv_installer::ReleaseRef (IN IUnknown* punk)
{
	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__  ) ); 
	if ( punk ) 
	{
		punk->Release();
	}
	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__  ) ); 
	return;
}

HRESULT ndis_drv_installer::GetPnpID( LPWSTR lpszInfFile, LPWSTR *lppszPnpID )
{
    HINF    hInf;
    LPWSTR  lpszModelSection = NULL;
    HRESULT hr = ERROR_SUCCESS;

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__  ) ); 
    *lppszPnpID = NULL;
	
    hInf = SetupOpenInfFileW(lpszInfFile,
		NULL,
		INF_STYLE_WIN4,
		NULL );
	
    if ( hInf == INVALID_HANDLE_VALUE )
    {	
		hr = HRESULT_FROM_WIN32( GetLastError() ); 
		log_trace( ( MSG_INFO | DBG_ERROR_MSG, "open inf file failed %d\n", hr ) ); ;
		goto _return; 
    }
	
    hr = GetKeyValue( hInf,
		L"Manufacturer",
		NULL,
		1,
		&lpszModelSection );
	
    if ( FAILED( hr ) )
	{
		log_trace( ( MSG_INFO | DBG_ERROR_MSG, "get inf manufacturer key value failed %d \n", hr ) ); 
		goto _return; 
	}

	hr = GetKeyValue( hInf,
		lpszModelSection,
		NULL,
		2,
		lppszPnpID ); 

	if( FAILED( hr ) )
	{
		log_trace( ( MSG_INFO | DBG_ERROR_MSG, "get key of index 2 failed %d\n", hr ) ); 
		goto _return; 
	}
	
_return: 

	if( lpszModelSection != NULL )
	{
		CoTaskMemFree( lpszModelSection );
	}

	SetupCloseInfFile( hInf ); 
	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__  ) ); 
    return hr;
}


HRESULT ndis_drv_installer::GetKeyValue (HINF hInf,LPWSTR lpszSection,LPWSTR lpszKey,DWORD  dwIndex,LPWSTR *lppszValue)
{
    INFCONTEXT  infCtx;
	DWORD dwSizeNeeded;
    HRESULT hr = ERROR_SUCCESS; 
	LPWSTR key_val; 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__  ) ); 
    *lppszValue = NULL;
	
    if ( SetupFindFirstLineW(hInf,
		lpszSection,
		lpszKey,
		&infCtx) == FALSE )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() ); 
		goto _return; 
    }
	
    if ( SetupGetStringField( &infCtx,
		dwIndex,
		NULL,
		0,
		&dwSizeNeeded ) == FALSE )
	{
		DWORD dwErr = GetLastError();
		hr = HRESULT_FROM_WIN32(dwErr);
		goto _return; 
	}

	key_val  = (LPWSTR)CoTaskMemAlloc( sizeof(WCHAR) * dwSizeNeeded );

	if ( key_val == NULL )
	{
		hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
		goto _return; 
	}

	if ( SetupGetStringFieldW(&infCtx,
		dwIndex,
		key_val,
		dwSizeNeeded,
		NULL) == FALSE )
	{

		hr = HRESULT_FROM_WIN32(GetLastError());
		CoTaskMemFree( key_val );
		goto _return; 
	}

	*lppszValue = key_val; 

_return:
	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__  ) ); 
    return hr;
}
