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

 
#undef _USE_WINSOCK
#include "stdafx.h"
#include "bitsafe_setup.h"
#include "uac_func.h"
#include "install_conf.h"

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;								// 当前实例
TCHAR szTitle[MAX_LOADSTRING];					// 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];			// 主窗口类名

// 此代码模块中包含的函数的前向声明:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

#define BITSAFE_INSTALL_DIR L"bitsafe"
#define BITSAFE_LINK_NAME L"bitsafe.lnk"
#define PATH_DELIM L"\\"

LRESULT _get_user_path( LPWSTR file_name, ULONG buf_len ); 
LRESULT get_bitsafe_install_path( LPWSTR install_path, ULONG buf_len )
{
	LRESULT ret; 
	BOOL _ret; 
	WCHAR user_path[ MAX_PATH ]; 

	ret = _get_user_path( user_path, MAX_PATH ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	if( wcslen( user_path ) + 1 + CONST_STR_LEN( PATH_DELIM ) + CONST_STR_LEN( BITSAFE_INSTALL_DIR ) > buf_len )
	{
		ret = ERROR_BUFFER_TOO_SMALL; 
		goto _return; 
	}

	wcscpy( install_path, user_path ); 
	wcsncat( install_path, PATH_DELIM, buf_len - wcslen( install_path ) ); 
	wcsncat( install_path, BITSAFE_INSTALL_DIR, buf_len - wcslen( install_path ) ); 

	if( install_path[ buf_len - 1 ] != L'\0' )
	{
		install_path[ buf_len - 1 ] = L'\0'; 
	}

	_ret = CreateDirectory( install_path, NULL ); 
	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		if( ret == ERROR_ALREADY_EXISTS )
		{
			ret = ERROR_SUCCESS; 
		}

		goto _return; 
	}

_return:
	return ret; 
}

#define UPDATE_DRV_EVENT_NAME _T( "bitsafe_drv_update" )
LRESULT update_other_files( LPCWSTR exe_file )
{
	LRESULT ret = ERROR_SUCCESS; 
#define MAX_CMD_LINE 512
	WCHAR cmd_line[ MAX_CMD_LINE ]; 
	HANDLE update_event = NULL; 
	ULONG wait_ret; 

	ASSERT( exe_file != NULL ); 

	wcscpy( cmd_line, L"\"" ); 

	wcsncat( cmd_line, exe_file, MAX_CMD_LINE - wcslen( cmd_line ) ); 

	//if( cmd_line[ wcslen( cmd_line ) - 1 ] != L'\\' ) 
	//{
	//	wcsncat( cmd_line, L"\\", MAX_CMD_LINE - wcslen( cmd_line ) ); 
	//}

	wcsncat( cmd_line, L"\" " L"/installed", MAX_CMD_LINE - wcslen( cmd_line ) ); 

	update_event = CreateEvent( NULL, TRUE, FALSE, UPDATE_DRV_EVENT_NAME ); 
	if( update_event == NULL )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	ret = run_proc_sync( cmd_line ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	wait_ret = WaitForSingleObject( update_event, 10000 ); 
	if( wait_ret != WAIT_OBJECT_0 )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

_return:
	if( update_event != NULL )
	{
		CloseHandle( update_event ); 
	}

	return ret; 
}

INLINE LRESULT _get_desktop_path( LPWSTR file_name, ULONG buf_len )\
{
	LRESULT ret; 

	ret = get_desktop_path( file_name, buf_len ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = correct_user_path( file_name, buf_len ); 

_return:
	return ret; 
}

LRESULT get_bitsafe_link_path( LPWSTR link_path, ULONG buf_len )
{
	LRESULT ret; 
	WCHAR desktop_path[ MAX_PATH ]; 
	//WCHAR user_name[ MAX_PATH ]; 
	//WCHAR domain_name[ MAX_PATH ]; 

	ret = _get_desktop_path( desktop_path, MAX_PATH ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	if( wcslen( desktop_path ) + 1 + CONST_STR_LEN( PATH_DELIM ) + CONST_STR_LEN( BITSAFE_LINK_NAME ) > buf_len )
	{
		ret = ERROR_BUFFER_TOO_SMALL; 
		goto _return; 
	}

	wcscpy( link_path, desktop_path ); 
	wcsncat( link_path, PATH_DELIM, buf_len - wcslen( link_path ) ); 
	wcsncat( link_path, BITSAFE_LINK_NAME, buf_len - wcslen( link_path ) ); 

	if( link_path[ buf_len - 1 ] != L'\0' )
	{
		link_path[ buf_len - 1 ] = L'\0'; 
	}

_return:
	return ret; 
}

typedef struct _install_file_info
{
	LPCWSTR file_name; 
	ULONG res_id; 
} install_file_info, *pinstall_file_info; 

install_file_info all_install_files[] = 
{
	{ L"bitsafe.exe", IDR_BITSAFE_FILE }, 
	{ L"crashsender.exe", IDR_CRASH_SENDER_FILE }, 
	{ L"crashrpt_lang.ini", IDR_CRASH_RPT_LANG }, 
	{ L"dbghelp.dll", IDR_DBG_HELP_FILE }
};

#define INSTALL_SOURCE_FILE_TYPE L"SOURCE_FILE"

#include "shlobj.h"
LRESULT _create_link( LPCWSTR file_name, 
					LPCWSTR link_file_name )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hres ;
	IShellLink *psl = NULL;
	IPersistFile *ppf = NULL ;
	BOOLEAN com_inited = FALSE; 

	//WORD wsz[ MAX_PATH] ;
	hres = CoInitialize( NULL ); 
	if( FAILED( hres ) )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	com_inited = TRUE; 

	hres = CoCreateInstance( CLSID_ShellLink, 
		NULL,
		CLSCTX_INPROC_SERVER, 
		IID_IShellLink,
		( void ** )&psl ) ;

	if( FAILED( hres) )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	psl->SetPath( file_name ); 
	//psl ->SetHotkey( MAKEWORD( 'R',
	//	HOTKEYF_SHIFT | HOTKEYF_CONTROL ) ) ; 

	hres = psl->QueryInterface( IID_IPersistFile,
		(void**)&ppf ); 

	if( FAILED( hres) )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}
	
	//MultiByteToWideChar( CP_ACP, 
	//	0, 
	//	link_file_name, 
	//	-1, 
	//	link_file_name, 
	//	MAX_PATH ); 

	hres = ppf->Save( link_file_name, STGM_READWRITE ); 

	if( FAILED( hres) )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

_return:

	if( ppf != NULL )
	{
		ppf->Release();
	}
	
	if( psl != NULL )
	{
		psl->Release();
	}

	if( com_inited == TRUE )
	{
		CoUninitialize(); 
	}

	return ret;
}

LRESULT notify_shell( LONG event_id, LPCWSTR dest_path )
{    
	LPWSTR tmp;

	SHChangeNotify( event_id,
		SHCNF_FLUSH | SHCNF_PATH,
		dest_path, 
		0 ); 

	tmp = ( LPWSTR )dest_path + lstrlen( dest_path ) - 1; 

	for( ; *tmp != L'\\'; tmp--); 

	*tmp = '\0'; 

	SHChangeNotify( SHCNE_UPDATEDIR | SHCNE_INTERRUPT, 
		SHCNF_FLUSH | SHCNF_PATH, 
		dest_path, 0 ); 

	return ERROR_SUCCESS; 
}

LRESULT create_link( LPCWSTR file_name, LPCWSTR link_file_name )
{
	LRESULT ret; 

	ret = _create_link( file_name, link_file_name ); 

	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}


	notify_shell( SHCNE_CREATE|SHCNE_INTERRUPT,  
		link_file_name ); 

_return:
	return ret; 
}

#include "bitsafe_common.h"
#include "service.h"
#include "one_instance.h"
#include "msg_box.h"
#include "install_dlg.h"

/********************************************************************************************************
更新过程设计:
1.进行整体程序主版本号的判断
2.判断那些文件需要更新
3.判断驱动是否需重启
4.进行文件的更新
5.重启驱动,或重启机器,或放弃更新

PROCMON式轻量级运行方式:
1.程序启动,加载驱动,马上卸载驱动,这样每次运行程序,都是对驱动的一次安装过程,这样更新的过程就比较简单.
2.向内核提出请求,得到内核的版本号,当内核的版本号与应用层不一致时,简单的停止执行,直到之前的内核退出为至.

第1版本的设计使用轻量级实现方案.
********************************************************************************************************/
LRESULT install_bitsafe()
{
	LRESULT ret; 
	LRESULT _ret; 
	WCHAR install_path[ MAX_PATH ]; 
	WCHAR install_file_name[ MAX_PATH ]; 
	WCHAR link_file_name[ MAX_PATH ]; 
	INT32 i; 
	ULONG prepare_work; 

#ifdef _DEBUG
	ret = del_must_restart_mark(); 
#endif //_DEBUG

	//ret = show_install_dlg( NULL, NULL, NULL ); 
	//if( ret != ERROR_SUCCESS )
	//{
	//	log_trace( ( MSG_ERROR, "show install dialog error 0x%0.8x\n", ret ) ); 
	//}

	//ret = check_must_restart_mark(); 
	//if( ret == ERROR_SUCCESS )
	//{
	//	DBG_BP(); 
	//	show_msg( NULL, _get_string_by_id( TEXT_COMMON_MUST_RESTART, _T( "在系统未重启前,比特安全的功能无法正常加载." ) ) ); 

	//	goto _return; 
	//}

	ret = get_bitsafe_install_path( install_path, MAX_PATH ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = stop_service( SERVICE_NAME ); ;
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_INFO, "service stop failed \n" ) ); 
		goto _return; 
	}

	ret = kill_app_instance( FALSE ); 
	if( ret != ERROR_SUCCESS 
		&& ret != ERROR_MAIN_WND_PROC_NOT_FOUND  )
	{
		goto _return; 
	}

	ret = init_uninstall_context( install_path, NULL, NULL ); 

	ret = uninstall_bitsafe_by_conf_file( &prepare_work ); 
	if( ret != ERROR_SUCCESS )
	{
		DBG_BP(); 

		//ret = update_other_files( install_file_name ); 
		//if( ret != ERROR_SUCCESS )
		//{
		//	log_trace( ( MSG_ERROR, "uninstall other component failed\n" ) ); 
		//}

		log_trace( ( MSG_ERROR, "uninstall bitsafe by configuration error 0x%0.8x\n", ret ) ); 

		//goto _return; 
	}

	ret = ERROR_SUCCESS; 

	for( i = 0; i < ARRAY_SIZE( all_install_files ); i ++ )
	{
		if( wcslen( all_install_files[ i ].file_name ) + wcslen( install_path ) + CONST_STR_LEN( PATH_DELIM ) + 1 > MAX_PATH )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			ASSERT( FALSE ); 
			goto _return; 
		}

		wcscpy( install_file_name, install_path ); 
		wcsncat( install_file_name, PATH_DELIM, MAX_PATH - wcslen( install_file_name ) );
		wcscat( install_file_name, all_install_files[ i ].file_name ); 

		//MessageBox( NULL, install_file_name, NULL, 0 ); 

		//DBG_BP(); 
		ret = output_file_from_res( NULL, 
			MAKEINTRESOURCE( all_install_files[ i ].res_id ), 
			INSTALL_SOURCE_FILE_TYPE, 
			install_file_name ); 
 
		if( ret != ERROR_SUCCESS )
		{
			//DBG_BP(); 
			log_trace( ( MSG_ERROR, "output the file %ws from resource ( id:%d )failed\n", install_file_name, all_install_files[ i ].res_id  )  ); 
			goto _return; 
		}
	}

	wcscpy( install_file_name, install_path ); 
	wcsncat( install_file_name, PATH_DELIM, MAX_PATH - wcslen( install_file_name ) );
	wcscat( install_file_name, all_install_files[ 0 ].file_name ); 

	ret = update_other_files( install_file_name ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "uninstall other component failed\n" ) ); 
	}

	//DBG_BP(); 
	ret = get_bitsafe_link_path( link_file_name, MAX_PATH ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	//MessageBox( NULL, link_file_name, NULL, 0 ); 

	ret = create_link( install_file_name, link_file_name ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "create link %ws failed\n", link_file_name ) ); 
		//goto _return; 
	}


	log_trace( ( MSG_ERROR, "uninstall bitsafe by configuration error 0x%0.8x\n", ret ) ); 


	if( PREPARE_BY_RESTART != prepare_work )
	{
		_ret = run_proc( install_file_name ); 
	}

_return:

	_ret = do_install_prepare_work(); 
	
	if( ret != ERROR_SUCCESS )
	{
		LPCWSTR tmp_text; 
		dlg_ret_state ret_state; 
		
		tmp_text = _get_string_by_id( TEXT_INSTALL_COMPATIBILITY_ERROR, _T( "由于其它程序或系统环境的问题, 安装失败, 现在重启系统?" ) ); 

		ret = show_msg( NULL, tmp_text, &ret_state );  
		if( ret == ERROR_SUCCESS )
		{
			if( ret_state == OK_STATE )
			{
				windows_shutdown( EWX_REBOOT | EWX_FORCE );  
			}
		}
	}
	return ret; 
}

INT32 APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: 在此放置代码。
	LRESULT ret; 
	MSG msg;
	HACCEL hAccelTable;

	CPaintManagerUI::SetInstance( hInstance ); 
	CPaintManagerUI::SetResourcePath( CPaintManagerUI::GetInstancePath() );
	//CPaintManagerUI::SetResourceZip( UI_FILE_NAME );
	CPaintManagerUI::SetResourceZipResType( INSTALL_SOURCE_FILE_TYPE, IDR_SETUP_RES_ZIP ); 

   	ret = install_bitsafe(); 
	goto _return; 

_return:
	return ( INT32 )ret; 

	// 初始化全局字符串
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_BITSAFE_SETUP, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 执行应用程序初始化:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_BITSAFE_SETUP);

	// 主消息循环:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
//  注释: 
//
//    仅当希望在已添加到 Windows 95 的
//    “RegisterClassEx”函数之前此代码与 Win32 系统兼容时，
//    才需要此函数及其用法。调用此函数
//    十分重要，这样应用程序就可以获得关联的
//   “格式正确的”小图标。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_BITSAFE_SETUP);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCTSTR)IDC_BITSAFE_SETUP;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

//
//   函数: InitInstance(HANDLE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释: 
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // 将实例句柄存储在全局变量中

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  函数: WndProc(HWND, unsigned, WORD, LONG)
//
//  目的: 处理主窗口的消息。
//
//  WM_COMMAND	- 处理应用程序菜单
//  WM_PAINT	- 绘制主窗口
//  WM_DESTROY	- 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message) 
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam); 
		wmEvent = HIWORD(wParam); 
		// 分析菜单选择:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: 在此添加任意绘图代码...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// “关于”框的消息处理程序。
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}
