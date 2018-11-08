
#ifdef _DLL_UILIB
#ifdef UILIB_EXPORTS
#define UILIB_API __declspec(dllexport)
#else
#define UILIB_API __declspec(dllimport)
#endif
#else
#define UILIB_API 
#endif //_DLL

/*********************************************************************
notice all class is only support single thread in the duilib.
don't modify the member of one instance of every class simuteniously.
*********************************************************************/

#include "crt_mem_leak_dbg.h"

//#define _DEBUG_MEM_LEAKS
//#ifdef _DEBUG_MEM_LEAKS
//
//#ifndef _DEBUG_MEM_LEAKS_DEFINED
//#define _DEBUG_MEM_LEAKS_DEFINED
//#define CRTDBG_MAP_ALLOC 
//#define _CRTDBG_MAP_ALLOC 
//
//#include <stdlib.h> 
//#include <crtdbg.h> 
//
//#ifndef DBG_NORMAL_BLOCK
//#define DBG_NORMAL_BLOCK new ( _NORMAL_BLOCK, __FILE__, __LINE__)
//#endif //DBG_NORMAL_BLOCK
//
//#ifndef DEBUG_CLIENT_BLOCK 
//#define DEBUG_CLIENT_BLOCK new( _CLIENT_BLOCK, __FILE__, __LINE__)
//#endif //DEBUG_CLIENT_BLOCK 
//
////#define new DEBUG_CLIENTBLOCK 
//#define new DBG_NORMAL_BLOCK
//
////#ifdef new
////#undef new
////#endif //new 
//
////#ifdef _DEBUG_MEM_LEAKS
////
////#ifdef DBG_NORMAL_BLOCK
////#define new DBG_NORMAL_BLOCK
////#endif //DBG_NORMAL_BLOCK
////
////#endif //_DEBUG_MEM_LEAKS
//#endif //_DEBUG_MEM_LEAKS_DEFINED
//#endif //_DEBUG_MEM_LEAKS

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <stddef.h>
#include <richedit.h>
#include <tchar.h>
#include <assert.h>

#include "UIBase.h"
#include "UIClassId.h"
#include "UIAnim.h"
#include "UIManager.h"
#include "UIDelegate.h"
#include "UIControl.h"
#include "UIItemManager.h"
#include "UIContainer.h"
#include "UICommonControls.h"
#include "UIList.h"
#include "UIListEx.h"
#include "UICachedContainer.h"
#include "UIListSubElementEx.h"
#include "UIListBodyEx.h"
#include "UICombo.h"
#include "UIActiveX.h"
#include "UIRichEdit.h"

#include "UIMarkup.h"
#include "UIDlgBuilder.h"
#include "UIRender.h"
//#include "UITree.h"
#include "UIImageList.h"

#include "Internal.h"

extern DuiLib::CAnimationSpooler m_anim;
