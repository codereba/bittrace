#ifndef __UIRENDER_H__
#define __UIRENDER_H__

#pragma once

namespace DuiLib {
/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CRenderClip
{
public:
    ~CRenderClip();
    RECT rcItem;
    HDC hDC;
    HRGN hRgn;
    HRGN hOldRgn;

    static void GenerateClip(HDC hDC, RECT rc, CRenderClip& clip);
    static void GenerateRoundClip(HDC hDC, RECT rc, RECT rcItem, int width, int height, CRenderClip& clip);
    static void UseOldClipBegin(HDC hDC, CRenderClip& clip);
    static void UseOldClipEnd(HDC hDC, CRenderClip& clip);
};

/////////////////////////////////////////////////////////////////////////////////////
//

#define LOAD_FILE_FROM_ABS_PATH 0x000000001

class UILIB_API CRenderEngine
{
public:
    static DWORD AdjustColor(DWORD dwColor, short H, short S, short L);
    static TImageInfo* LoadImage(STRINGorID bitmap, LPCTSTR type = NULL, DWORD mask = 0, ULONG flags = 0 );
    static void DrawImage(HDC hDC, HBITMAP hBitmap, const RECT& rc, const RECT& rcPaint, \
        const RECT& rcBmpPart, const RECT& rcCorners, bool alphaChannel, BYTE uFade = 255, 
        bool hole = false, bool xtiled = false, bool ytiled = false);
    static bool DrawImageString(HDC hDC, CPaintManagerUI* pManager, const RECT& rcItem, const RECT& rcPaint, 
        LPCTSTR pStrImage, LPCTSTR pStrModify = NULL);

	inline static void _DrawColor(HDC hDC, const RECT& rc, DWORD color, DWORD fade )
	{

		// Create a new 32bpp bitmap with room for an alpha channel
		BITMAPINFO bmi = { 0 };
		RECT rcBmpPart = {0, 0, 1, 1};
		RECT rcCorners = {0};
		LPDWORD pDest; 
		HBITMAP hBitmap; 

		if( color <= 0x00FFFFFF ) 
		{
			goto _return;
		}

		if( color >= 0xFF000000 )
		{
			goto _return; 
		}

		bmi.bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
		bmi.bmiHeader.biWidth = 1;
		bmi.bmiHeader.biHeight = 1;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;
		bmi.bmiHeader.biCompression = BI_RGB;
		bmi.bmiHeader.biSizeImage = 1 * 1 * sizeof( DWORD );
		pDest = NULL;
		hBitmap = ::CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, (LPVOID*) &pDest, NULL, 0);

		if( hBitmap == NULL )
		{
			goto _return;
		}

		*pDest = color;

		DrawImage(hDC, hBitmap, rc, rc, rcBmpPart, rcCorners, true, ( BYTE )fade );
		::DeleteObject(hBitmap); 

_return:
		return; 
	}

	static void DrawFadeBkColor(HDC hDC, const RECT& rc, DWORD color, DWORD fade ); 
    static void DrawColor(HDC hDC, const RECT& rc, DWORD color);
    static void DrawGradient(HDC hDC, const RECT& rc, DWORD dwFirst, DWORD dwSecond, bool bVertical, int nSteps);
	static void DrawAlphaGradient(HDC hDC, const RECT& rc, DWORD dwFirst, bool bVertical, int nSteps, INT32 nMaxAlpha ); 

    // 以下函数中的颜色参数alpha值无效
	inline static void DrawLine(HDC hDC, LONG left, LONG top, LONG right, LONG bottom, int nSize, DWORD dwPenColor ); 
    static void DrawLine(HDC hDC, const RECT& rc, int nSize, DWORD dwPenColor);
    static void DrawRect(HDC hDC, const RECT& rc, int nSize, DWORD dwPenColor);
    static void DrawRoundRect(HDC hDC, const RECT& rc, int width, int height, int nSize, DWORD dwPenColor);
    static void DrawText(HDC hDC, CPaintManagerUI* pManager, RECT& rc, LPCTSTR pstrText, \
        DWORD dwTextColor, int iFont, UINT uStyle);
    static void DrawHtmlText(HDC hDC, CPaintManagerUI* pManager, RECT& rc, LPCTSTR pstrText, 
        DWORD dwTextColor, RECT* pLinks, CStdString* sLinks, int& nLinkRects, UINT uStyle);
    static HBITMAP GenerateBitmap(CPaintManagerUI* pManager, CControlUI* pControl, RECT rc);
};

HBITMAP ConvertIconToBitmap( HICON hIcon ); 

} // namespace DuiLib

#endif // __UIRENDER_H__
