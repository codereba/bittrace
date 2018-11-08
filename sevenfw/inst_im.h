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

#ifndef __INSTALL_IM_H__
#define __INSTALL_IM_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "Netcfgx.h"

#ifndef _DEBUG_MEM_LEAKS
#include <comdef.h>
#endif //_DEBUG_MEM_LEAKS

//Platform SDK
#include <devguid.h>
#include <setupapi.h>

class ndis_drv_installer  
{
public:
	HWND m_hwnd;
	WCHAR m_exe_module_name[_MAX_FNAME];
	BOOL m_IsSetup;

public:
	ndis_drv_installer();
	ndis_drv_installer(HWND hWnd);
	virtual ~ndis_drv_installer();
	void set_main_wnd(HWND hWnd);

#define INSTALL_SEVENFW_WIN7_DRV 0x00000001

	LRESULT install_ndis_drv( ULONG flags );
	HRESULT uninstall_ndis_drv( ULONG flags );


	HRESULT GetKeyValue (HINF hInf,LPWSTR lpszSection,
		LPWSTR lpszKey,DWORD  dwIndex,LPWSTR *lppszValue);

	HRESULT GetPnpID(LPWSTR lpszInfFile,LPWSTR *lppszPnpID);
			
	HRESULT HrGetINetCfg (IN BOOL fGetWriteLock,
		IN LPWSTR lpszAppName,
		OUT INetCfg** ppnc,
		LPWSTR *lpszLockedBy);
	
	HRESULT HrReleaseINetCfg (INetCfg* pnc,
		BOOL fHasWriteLock);
	
	HRESULT HrInstallNetComponent (IN INetCfg *pnc,
		IN LPWSTR szComponentId,
		IN const GUID    *pguildClass,
		IN LPWSTR lpszInfFullPath);
	
	HRESULT HrInstallComponent(IN INetCfg* pnc,
		IN LPWSTR szComponentId,
		IN const GUID* pguidClass);
	
	HRESULT HrUninstallNetComponent(IN INetCfg* pnc,
		IN LPWSTR szComponentId);
	
	HRESULT HrGetComponentEnum (INetCfg* pnc,
		IN const GUID* pguidClass,
		IEnumNetCfgComponent **ppencc);
	
	HRESULT HrGetFirstComponent (IEnumNetCfgComponent* pencc,
		INetCfgComponent **ppncc);
	
	HRESULT HrGetNextComponent (IEnumNetCfgComponent* pencc,
		INetCfgComponent **ppncc);
	
	HRESULT HrFindNetComponentByPnpId (IN INetCfg *pnc,
		IN LPWSTR lpszPnpDevNodeId,
		OUT INetCfgComponent **ppncc);
	
	HRESULT HrGetBindingPathEnum (INetCfgComponent *pncc,
		DWORD dwBindingType,
		IEnumNetCfgBindingPath **ppencbp);
	
	HRESULT HrGetFirstBindingPath (IEnumNetCfgBindingPath *pencbp,
		INetCfgBindingPath **ppncbp);
	
	HRESULT HrGetNextBindingPath (IEnumNetCfgBindingPath *pencbp,
		INetCfgBindingPath **ppncbp);
	
	HRESULT HrGetBindingInterfaceEnum (INetCfgBindingPath *pncbp,
		IEnumNetCfgBindingInterface **ppencbi);
	
	HRESULT HrGetFirstBindingInterface (IEnumNetCfgBindingInterface *pencbi,
		INetCfgBindingInterface **ppncbi);
	
	HRESULT HrGetNextBindingInterface (IEnumNetCfgBindingInterface *pencbi,
		INetCfgBindingInterface **ppncbi);
	
VOID ReleaseRef (IUnknown* punk);
};

#endif //__INSTALL_IM_H__
