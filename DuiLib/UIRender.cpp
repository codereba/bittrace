#include "StdAfx.h"

///////////////////////////////////////////////////////////////////////////////////////

#include "XUnzip.h"

extern "C"
{
    extern unsigned char *stbi_load_from_memory(unsigned char const *buffer, int len, int *x, int *y, \
        int *comp, int req_comp);
	extern void     stbi_image_free(void *retval_from_stbi_load);

};

namespace DuiLib {

/////////////////////////////////////////////////////////////////////////////////////
//
//

CRenderClip::~CRenderClip()
{
    ASSERT(::GetObjectType(hDC)==OBJ_DC || ::GetObjectType(hDC)==OBJ_MEMDC);
    ASSERT(::GetObjectType(hRgn)==OBJ_REGION);
    ASSERT(::GetObjectType(hOldRgn)==OBJ_REGION);
    ::SelectClipRgn(hDC, hOldRgn);
    ::DeleteObject(hOldRgn);
    ::DeleteObject(hRgn);
}

void CRenderClip::GenerateClip(HDC hDC, RECT rc, CRenderClip& clip)
{
    RECT rcClip = { 0 };
    ::GetClipBox(hDC, &rcClip);
    clip.hOldRgn = ::CreateRectRgnIndirect(&rcClip);
    clip.hRgn = ::CreateRectRgnIndirect(&rc);
    ::ExtSelectClipRgn(hDC, clip.hRgn, RGN_AND);
    clip.hDC = hDC;
    clip.rcItem = rc;
}

void CRenderClip::GenerateRoundClip(HDC hDC, RECT rc, RECT rcItem, int width, int height, CRenderClip& clip)
{
    RECT rcClip = { 0 };
    ::GetClipBox(hDC, &rcClip);
    clip.hOldRgn = ::CreateRectRgnIndirect(&rcClip);
    clip.hRgn = ::CreateRectRgnIndirect(&rc);
    HRGN hRgnItem = ::CreateRoundRectRgn(rcItem.left, rcItem.top, rcItem.right + 1, rcItem.bottom + 1, width, height);
    ::CombineRgn(clip.hRgn, clip.hRgn, hRgnItem, RGN_AND);
    ::ExtSelectClipRgn(hDC, clip.hRgn, RGN_AND);
    clip.hDC = hDC;
    clip.rcItem = rc;
    ::DeleteObject(hRgnItem);
}

void CRenderClip::UseOldClipBegin(HDC hDC, CRenderClip& clip)
{
    ::SelectClipRgn(hDC, clip.hOldRgn);
}

void CRenderClip::UseOldClipEnd(HDC hDC, CRenderClip& clip)
{
    ::SelectClipRgn(hDC, clip.hRgn);
}

/////////////////////////////////////////////////////////////////////////////////////
//
//

static const float OneThird = 1.0f / 3;

static void RGBtoHSL(DWORD ARGB, float* H, float* S, float* L) {
    const float
        R = (float)GetRValue(ARGB),
        G = (float)GetGValue(ARGB),
        B = (float)GetBValue(ARGB),
        nR = (R<0?0:(R>255?255:R))/255,
        nG = (G<0?0:(G>255?255:G))/255,
        nB = (B<0?0:(B>255?255:B))/255,
        m = min(min(nR,nG),nB),
        M = max(max(nR,nG),nB);
    *L = (m + M)/2;
    if (M==m) *H = *S = 0;
    else {
        const float
            f = (nR==m)?(nG-nB):((nG==m)?(nB-nR):(nR-nG)),
            i = (nR==m)?3.0f:((nG==m)?5.0f:1.0f);
        *H = (i-f/(M-m));
        if (*H>=6) *H-=6;
        *H*=60;
        *S = (2*(*L)<=1)?((M-m)/(M+m)):((M-m)/(2-M-m));
    }
}

static void HSLtoRGB(DWORD* ARGB, float H, float S, float L) {
    const float
        q = 2*L<1?L*(1+S):(L+S-L*S),
        p = 2*L-q,
        h = H/360,
        tr = h + OneThird,
        tg = h,
        tb = h - OneThird,
        ntr = tr<0?tr+1:(tr>1?tr-1:tr),
        ntg = tg<0?tg+1:(tg>1?tg-1:tg),
        ntb = tb<0?tb+1:(tb>1?tb-1:tb),
        R = 255*(6*ntr<1?p+(q-p)*6*ntr:(2*ntr<1?q:(3*ntr<2?p+(q-p)*6*(2.0f*OneThird-ntr):p))),
        G = 255*(6*ntg<1?p+(q-p)*6*ntg:(2*ntg<1?q:(3*ntg<2?p+(q-p)*6*(2.0f*OneThird-ntg):p))),
        B = 255*(6*ntb<1?p+(q-p)*6*ntb:(2*ntb<1?q:(3*ntb<2?p+(q-p)*6*(2.0f*OneThird-ntb):p)));
    *ARGB &= 0xFF000000;
    *ARGB |= RGB( (BYTE)(R<0?0:(R>255?255:R)), (BYTE)(G<0?0:(G>255?255:G)), (BYTE)(B<0?0:(B>255?255:B)) );
}

static COLORREF PixelAlpha(COLORREF clrSrc, double src_darken, COLORREF clrDest, double dest_darken)
{
    return RGB (GetRValue (clrSrc) * src_darken + GetRValue (clrDest) * dest_darken, 
        GetGValue (clrSrc) * src_darken + GetGValue (clrDest) * dest_darken, 
        GetBValue (clrSrc) * src_darken + GetBValue (clrDest) * dest_darken);

}

static BOOL WINAPI AlphaBitBlt(HDC hDC, int nDestX, int nDestY, int dwWidth, int dwHeight, HDC hSrcDC, \
                        int nSrcX, int nSrcY, int wSrc, int hSrc, BLENDFUNCTION ftn)
{
    HDC hTempDC = ::CreateCompatibleDC(hDC);
    if (NULL == hTempDC)
        return FALSE;

    //Creates Source DIB
    LPBITMAPINFO lpbiSrc = NULL;
    // Fill in the BITMAPINFOHEADER
    lpbiSrc = (LPBITMAPINFO) new BYTE[sizeof(BITMAPINFOHEADER)];
	if (lpbiSrc == NULL)
	{
		::DeleteDC(hTempDC);
		return FALSE;
	}
    lpbiSrc->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    lpbiSrc->bmiHeader.biWidth = dwWidth;
    lpbiSrc->bmiHeader.biHeight = dwHeight;
    lpbiSrc->bmiHeader.biPlanes = 1;
    lpbiSrc->bmiHeader.biBitCount = 32;
    lpbiSrc->bmiHeader.biCompression = BI_RGB;
    lpbiSrc->bmiHeader.biSizeImage = dwWidth * dwHeight;
    lpbiSrc->bmiHeader.biXPelsPerMeter = 0;
    lpbiSrc->bmiHeader.biYPelsPerMeter = 0;
    lpbiSrc->bmiHeader.biClrUsed = 0;
    lpbiSrc->bmiHeader.biClrImportant = 0;

    COLORREF* pSrcBits = NULL;
    HBITMAP hSrcDib = CreateDIBSection (
        hSrcDC, lpbiSrc, DIB_RGB_COLORS, (void **)&pSrcBits,
        NULL, NULL);

    if ((NULL == hSrcDib) || (NULL == pSrcBits)) 
    {
		delete [] lpbiSrc;
        ::DeleteDC(hTempDC);
        return FALSE;
    }

    HBITMAP hOldTempBmp = (HBITMAP)::SelectObject (hTempDC, hSrcDib);
    ::StretchBlt(hTempDC, 0, 0, dwWidth, dwHeight, hSrcDC, nSrcX, nSrcY, wSrc, hSrc, SRCCOPY);
    ::SelectObject (hTempDC, hOldTempBmp);

    //Creates Destination DIB
    LPBITMAPINFO lpbiDest = NULL;
    // Fill in the BITMAPINFOHEADER
    lpbiDest = (LPBITMAPINFO) new BYTE[sizeof(BITMAPINFOHEADER)];
	if (lpbiDest == NULL)
	{
        delete [] lpbiSrc;
        ::DeleteObject(hSrcDib);
        ::DeleteDC(hTempDC);
        return FALSE;
	}

    lpbiDest->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    lpbiDest->bmiHeader.biWidth = dwWidth;
    lpbiDest->bmiHeader.biHeight = dwHeight;
    lpbiDest->bmiHeader.biPlanes = 1;
    lpbiDest->bmiHeader.biBitCount = 32;
    lpbiDest->bmiHeader.biCompression = BI_RGB;
    lpbiDest->bmiHeader.biSizeImage = dwWidth * dwHeight;
    lpbiDest->bmiHeader.biXPelsPerMeter = 0;
    lpbiDest->bmiHeader.biYPelsPerMeter = 0;
    lpbiDest->bmiHeader.biClrUsed = 0;
    lpbiDest->bmiHeader.biClrImportant = 0;

    COLORREF* pDestBits = NULL;
    HBITMAP hDestDib = CreateDIBSection (
        hDC, lpbiDest, DIB_RGB_COLORS, (void **)&pDestBits,
        NULL, NULL);

    if ((NULL == hDestDib) || (NULL == pDestBits))
    {
        delete [] lpbiSrc;
        ::DeleteObject(hSrcDib);
        ::DeleteDC(hTempDC);
        return FALSE;
    }

    ::SelectObject (hTempDC, hDestDib);
    ::BitBlt (hTempDC, 0, 0, dwWidth, dwHeight, hDC, nDestX, nDestY, SRCCOPY);
    ::SelectObject (hTempDC, hOldTempBmp);

    double src_darken;
    BYTE nAlpha;

    for (int pixel = 0; pixel < dwWidth * dwHeight; pixel++, pSrcBits++, pDestBits++)
    {
        nAlpha = LOBYTE(*pSrcBits >> 24);
        src_darken = (double) (nAlpha * ftn.SourceConstantAlpha) / 255.0 / 255.0;
        if( src_darken < 0.0 ) src_darken = 0.0;
        *pDestBits = PixelAlpha(*pSrcBits, src_darken, *pDestBits, 1.0 - src_darken);
    } //for

    ::SelectObject (hTempDC, hDestDib);
    ::BitBlt (hDC, nDestX, nDestY, dwWidth, dwHeight, hTempDC, 0, 0, SRCCOPY);
    ::SelectObject (hTempDC, hOldTempBmp);

    delete [] lpbiDest;
    ::DeleteObject(hDestDib);

    delete [] lpbiSrc;
    ::DeleteObject(hSrcDib);

    ::DeleteDC(hTempDC);
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////
//
//

DWORD CRenderEngine::AdjustColor(DWORD dwColor, short H, short S, short L)
{
    if( H == 180 && S == 100 && L == 100 ) return dwColor;
    float fH, fS, fL;
    float S1 = S / 100.0f;
    float L1 = L / 100.0f;
    RGBtoHSL(dwColor, &fH, &fS, &fL);
    fH += (H - 180);
    fH = fH > 0 ? fH : fH + 360; 
    fS *= S1;
    fL *= L1;
    HSLtoRGB(&dwColor, fH, fS, fL);
    return dwColor;
}

//#include <GdiPlus.h>
//using namespace Gdiplus; 

LRESULT GetAlphaChannelBits( HBITMAP BmpSrc, PBYTE pDest, SHORT Width, SHORT Height, PBYTE MaskBmpBits )
{
	LRESULT Ret = ERROR_SUCCESS; 
	INT32 _Ret; 
	//LPBYTE pDest = NULL;
	//BITMAPINFO bmi;
	//HBITMAP BmpOut; 

	do 
	{
		if( BmpSrc == NULL )
		{
			Ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( Width <= 0 )
		{
			Ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( Height <= 0 )
		{
			Ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		//if( Width > 32 
		//	|| Height > 32 )
		//{
		//	Ret = ERROR_INVALID_PARAMETER; 
		//	break; 
		//}

		//::ZeroMemory(&bmi, sizeof(BITMAPINFO));
		//bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		//bmi.bmiHeader.biWidth = Width;
		//bmi.bmiHeader.biHeight = -Height;
		//bmi.bmiHeader.biPlanes = 1;
		//bmi.bmiHeader.biBitCount = 32;
		//bmi.bmiHeader.biCompression = BI_RGB;
		//bmi.bmiHeader.biSizeImage = Width * Height * 4;

		//hBitmap = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**)&pDest, NULL, 0);
		//if( !hBitmap ) 
		//{
		//	break;
		//}

		BITMAP bm = { 0 };
		_Ret = GetObject( BmpSrc, sizeof( bm ), &bm ); 
		if( FALSE == _Ret )
		{
			Ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		if( bm.bmBitsPixel < 32 )
		{
			Ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		//BITMAPINFO* bmi = (BITMAPINFO*) _alloca(sizeof(BITMAPINFOHEADER) + (256 * sizeof(RGBQUAD)));
		//::ZeroMemory(bmi, sizeof(BITMAPINFOHEADER) + (256 * sizeof(RGBQUAD)));
		//bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		//BOOL bRes = ::GetDIBits(hDC, hBmp, 0, bm.bmHeight, NULL, bmi, DIB_RGB_COLORS);
		//if( !bRes || bmi->bmiHeader.biBitCount != 32 ) return;
		//LPBYTE pBitData = (LPBYTE) ::LocalAlloc(LPTR, bm.bmWidth * bm.bmHeight * sizeof(DWORD));
		//if( pBitData == NULL ) return;
		//LPBYTE pData = pBitData;
		//::GetDIBits(hDC, hBmp, 0, bm.bmHeight, pData, bmi, DIB_RGB_COLORS);

		//for( int y = 0; y < Height; y++ ) {
		//	for( int x = 0; x < Width; x++ ) {
		//		pData[0] = (BYTE)((DWORD)pData[0] * pData[3] / 255);
		//		pData[1] = (BYTE)((DWORD)pData[1] * pData[3] / 255);
		//		pData[2] = (BYTE)((DWORD)pData[2] * pData[3] / 255);
		//		pData += 4;
		//	}
		//}
		//::SetDIBits(hDC, hBmp, 0, bm.bmHeight, pBitData, bmi, DIB_RGB_COLORS);
		//::LocalFree(pBitData);

		for( int i = 0; i < Width * Height; i++ ) 
		{
			//pDest[i*4 + 3] = pImage[i*4 + 3];
			//if( pDest[i*4 + 3] < 255 )
			//{
			//	pDest[i*4] = (BYTE)(DWORD(pImage[i*4 + 2])*pImage[i*4 + 3]/255);
			//	pDest[i*4 + 1] = (BYTE)(DWORD(pImage[i*4 + 1])*pImage[i*4 + 3]/255);
			//	pDest[i*4 + 2] = (BYTE)(DWORD(pImage[i*4])*pImage[i*4 + 3]/255); 
			//	bAlphaChannel = true;
			//}
			//else
			//{
			//	pDest[i*4] = pImage[i*4 + 2];
			//	pDest[i*4 + 1] = pImage[i*4 + 1];
			//	pDest[i*4 + 2] = pImage[i*4]; 
			//}

			//#define ICON_MAKE_COLOR ( ULONG )( RGB( 0xff, 0xff, 0xff ) )

			if( *( DWORD* )( &pDest[ i * 4 ] ) != 0 ) 
			{
				MaskBmpBits[ i ] = 0xff; 
			}
			else
			{
				MaskBmpBits[ i ] = 0; 
			}
		}
	}while( FALSE ); 

	return Ret; 
}

LRESULT GenerateAlphaChannelBitmap( HBITMAP BmpSrc, PBYTE pDest, PBYTE MaskBmpBits, SHORT Width, SHORT Height )
{
	LRESULT Ret = ERROR_SUCCESS; 
	INT32 _Ret; 
	//LPBYTE pDest = NULL;
	//BITMAPINFO bmi;
	//HBITMAP BmpOut; 

	do 
	{
		if( BmpSrc == NULL )
		{
			Ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( Width <= 0 )
		{
			Ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( Height <= 0 )
		{
			Ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		//::ZeroMemory(&bmi, sizeof(BITMAPINFO));
		//bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		//bmi.bmiHeader.biWidth = Width;
		//bmi.bmiHeader.biHeight = -Height;
		//bmi.bmiHeader.biPlanes = 1;
		//bmi.bmiHeader.biBitCount = 32;
		//bmi.bmiHeader.biCompression = BI_RGB;
		//bmi.bmiHeader.biSizeImage = Width * Height * 4;

		//hBitmap = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**)&pDest, NULL, 0);
		//if( !hBitmap ) 
		//{
		//	break;
		//}

		BITMAP bm = { 0 };
		_Ret = GetObject( BmpSrc, sizeof( bm ), &bm ); 
		if( FALSE == _Ret )
		{
			Ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		if( bm.bmBitsPixel < 32 )
		{
			Ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		//BITMAPINFO* bmi = (BITMAPINFO*) _alloca(sizeof(BITMAPINFOHEADER) + (256 * sizeof(RGBQUAD)));
		//::ZeroMemory(bmi, sizeof(BITMAPINFOHEADER) + (256 * sizeof(RGBQUAD)));
		//bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		//BOOL bRes = ::GetDIBits(hDC, hBmp, 0, bm.bmHeight, NULL, bmi, DIB_RGB_COLORS);
		//if( !bRes || bmi->bmiHeader.biBitCount != 32 ) return;
		//LPBYTE pBitData = (LPBYTE) ::LocalAlloc(LPTR, bm.bmWidth * bm.bmHeight * sizeof(DWORD));
		//if( pBitData == NULL ) return;
		//LPBYTE pData = pBitData;
		//::GetDIBits(hDC, hBmp, 0, bm.bmHeight, pData, bmi, DIB_RGB_COLORS);

		//for( int y = 0; y < Height; y++ ) {
		//	for( int x = 0; x < Width; x++ ) {
		//		pData[0] = (BYTE)((DWORD)pData[0] * pData[3] / 255);
		//		pData[1] = (BYTE)((DWORD)pData[1] * pData[3] / 255);
		//		pData[2] = (BYTE)((DWORD)pData[2] * pData[3] / 255);
		//		pData += 4;
		//	}
		//}
		//::SetDIBits(hDC, hBmp, 0, bm.bmHeight, pBitData, bmi, DIB_RGB_COLORS);
		//::LocalFree(pBitData);

		for( int i = 0; i < Width * Height; i++ ) 
		{
			//pDest[i*4 + 3] = pImage[i*4 + 3];
			//if( pDest[i*4 + 3] < 255 )
			//{
			//	pDest[i*4] = (BYTE)(DWORD(pImage[i*4 + 2])*pImage[i*4 + 3]/255);
			//	pDest[i*4 + 1] = (BYTE)(DWORD(pImage[i*4 + 1])*pImage[i*4 + 3]/255);
			//	pDest[i*4 + 2] = (BYTE)(DWORD(pImage[i*4])*pImage[i*4 + 3]/255); 
			//	bAlphaChannel = true;
			//}
			//else
			//{
			//	pDest[i*4] = pImage[i*4 + 2];
			//	pDest[i*4 + 1] = pImage[i*4 + 1];
			//	pDest[i*4 + 2] = pImage[i*4]; 
			//}

//#define ICON_MAKE_COLOR ( ULONG )( RGB( 0xff, 0xff, 0xff ) )

			if( MaskBmpBits[ i ] != 0 ) 
			{
				//ASSERT( pDest[i*4 + 3] == 0 
				//	|| pDest[i*4 + 3] == 0xff ); 

				if( *( DWORD* )( &pDest[ i * 4 ] ) != 0 )
				{
					__Trace( _T( "the mask color of the icon is not 0: 0x%0.8x\n" ), 
						*( DWORD* )( &pDest[ i * 4 ] ) ); 
				}

				//ASSERT( *( DWORD* )( &pDest[ i * 4 ] ) == 0 ); 

				pDest[i*4] = (BYTE)0;
				pDest[i*4 + 1] = (BYTE)0;
				pDest[i*4 + 2] = (BYTE)0; 
				pDest[i*4 + 3] = (BYTE)0;
			}
		}
	}while( FALSE ); 

	return Ret; 
}

HBITMAP ConvertIconToBitmap( HICON hIcon, INT32 *cx, INT32 *cy ) 
{
#if 0
	INT32 Ret; 
	ULONG _Ret; 
	HBITMAP hBmp = NULL; 
	BITMAP bmp; 
	HDC bmpDC = NULL; 
	HDC iconDC = NULL; 
	ICONINFO IconInfo; 
	HBITMAP hIconBmp; 
	HOBJECT OldObj; 
	HOBJECT OldObj2; 
	DWORD dwWidth;
	DWORD dwHeight; 
	
	do 
	{

		Ret = GetIconInfo( hIcon, &IconInfo ); 

		if( Ret == FALSE )   
		{
			break; 
		}

		bmpDC = GetDC( NULL ); 
		if( bmpDC == NULL )
		{
			break; 
		}

		iconDC = CreateCompatibleDC( &bmpDC ); 
		if( iconDC != NULL )
		{
			break; 
		}

		hIconBmp = GetObject( IconInfo.hbmColor, sizeof( BITMAP ), &bmp ); 

		if( hIconBmp == NULL ) 
		{
			break; 
		}

		dwWidth = IconInfo.xHotspot * 2;
		dwHeight = IconInfo.yHotspot*2; 

		hBmp = CreateBitmap( dwWidth, dwHeight, bmp.bmPlanes, 
			bmp.bmBitsPixel, NULL ); 

		if( hBmp == NULL )
		{
			break; 
		}

		OldObj = SelectObject( iconDC, IconInfo.hbmColor ); 
		OldObj2 = SelectObject( bmpDC, hBmp ); 

		Ret = BitBlt( bmpDC, 0, 0, dwWidth, dwHeight, iconDC, 0, 0, SRCCOPY ); 
		if( Ret == FALSE )
		{
			DeleteBitmap( hBmp ); 
			hBmp = NULL; 
		}

		SelectObject( iconDC, OldObj ); 
		SelectObject( bmpDC, OldObj2 ); 
	}while( FALSE ); 
	
	if( bmpDC != NULL )
	{
		ReleaseDC( NULL, bmpDC ); 
	}

	if( iconDC != NULL )
	{
		DeleteDC( iconDC ); 
	}
#endif //0

	//Bitmap *IconBmp = Bitmap::FromHICON( hIcon ); 

	//Bitmap::FromBITMAPINFO,

	//	Bitmap::FromResource,

	//	Bitmap::FromStream,

	//	Bitmap::GetHBITMAP,

	//	Bitmap::GetHICON

	INT32 Ret; 
	HDC Screen_Handle = NULL; 
	HDC Device_Handle = NULL; 
	HBITMAP Bitmap_Handle = NULL; 
	HBITMAP Old_Bitmap = NULL; 
	ULONG IconHeight = GetSystemMetrics( SM_CYICON ); 
	ULONG IconWidth = GetSystemMetrics( SM_CXICON ); 
	BYTE *MaskBmpBits = NULL; 

	do 
	{
		if( cx == NULL )
		{
			break; 
		}

		if( cy == NULL )
		{
			break; 
		}

		*cx = 0; 
		*cy = 0; 

		MaskBmpBits = ( BYTE* )malloc( sizeof( BYTE ) * IconHeight * IconWidth ); 
		if( MaskBmpBits == NULL )
		{
			break; 
		}

		Screen_Handle = GetDC( NULL ); 
		if( Screen_Handle == NULL )
		{
			break; 
		}

		Device_Handle = CreateCompatibleDC(Screen_Handle);
		if( Device_Handle == NULL )
		{
			break; 
		}

		BITMAPINFO bmi = { 0 };
		LPDWORD pDest = NULL;

		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = IconWidth;
		bmi.bmiHeader.biHeight = IconHeight;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;
		bmi.bmiHeader.biCompression = BI_RGB;
		bmi.bmiHeader.biSizeImage = IconWidth * IconHeight * sizeof(DWORD);

		Bitmap_Handle = ::CreateDIBSection( Device_Handle, &bmi, DIB_RGB_COLORS, (LPVOID*) &pDest, NULL, 0);

		//Bitmap_Handle = CreateCompatibleBitmap( Screen_Handle, 
		//	IconWidth,
		//	IconHeight );

		if( Bitmap_Handle == NULL )
		{
			break; 
		}

		Old_Bitmap = ( HBITMAP )SelectObject( Device_Handle, Bitmap_Handle );

		//for( ; ; )
		//{
		//	//Ret = DrawIcon( Device_Handle, 
		//	//	0,
		//	//	0, 
		//	//	hIcon ); 

		//	Ret = DrawIconEx( Screen_Handle, 
		//		0, 
		//		0, 
		//		hIcon, 
		//		IconWidth,
		//		IconHeight, 
		//		0, 
		//		NULL, 
		//		DI_NORMAL ); 

		//	Sleep( 100 ); 
		//}

		//RECT RectMask; 
		//HBRUSH BrBk; 
		//RectMask.left = 0; 
		//RectMask.top = 0; 
		//RectMask.right = IconWidth; 
		//RectMask.bottom = IconHeight; 

		//BrBk = CreateSolidBrush( RGB( 0xff, 00, 0xff ) ); 

		//if( BrBk == NULL )
		//{
		//	DeleteBitmap( Bitmap_Handle ); 
		//	Bitmap_Handle = NULL; 

		//	break; 
		//}

		//Ret = FillRect( Device_Handle, &RectMask, BrBk ); 
		//
		//DeleteBrush( BrBk ); 

		//if( Ret == FALSE )
		//{
		//	DeleteBitmap( Bitmap_Handle ); 
		//	Bitmap_Handle = NULL; 
		//	break; 
		//}

		do 
		{
			Ret = DrawIconEx( Device_Handle, 
				0, 
				0, 
				hIcon, 
				IconWidth,
				IconHeight, 
				0, 
				NULL, 
				DI_MASK ); 

			//BitBlt( Device_Handle, 0, 0, 32, 32, Screen_Handle, 100, 100, SRCCOPY ); 

			//GdiFlush(); 

			//for( ; ; )
			//{
			//	BitBlt( Screen_Handle, 0, 0, 32, 32, Device_Handle, 0, 0, SRCCOPY ); 

			//	Sleep( 200 ); 
			//}

			if( Ret == FALSE )
			{
				DeleteBitmap( Bitmap_Handle ); 
				Bitmap_Handle = NULL; 
				break; 
			}

			Ret = GetAlphaChannelBits( Bitmap_Handle, ( PBYTE )pDest, ( SHORT )IconWidth, ( SHORT )IconHeight, MaskBmpBits ); 
			if( Ret != ERROR_SUCCESS )
			{
				DeleteBitmap( Bitmap_Handle ); 
				Bitmap_Handle = NULL; 
				break; 
			}

			Ret = DrawIconEx( Device_Handle, 
				0, 
				0, 
				hIcon, 
				IconWidth,
				IconHeight, 
				0, 
				NULL, 
				DI_IMAGE ); 

			if( Ret == FALSE )
			{
				DeleteBitmap( Bitmap_Handle ); 
				Bitmap_Handle = NULL; 
				break; 
			}

			Ret = GenerateAlphaChannelBitmap( Bitmap_Handle, ( PBYTE )pDest, MaskBmpBits, ( SHORT )IconWidth, ( SHORT )IconHeight ); 
			if( Ret != ERROR_SUCCESS )
			{
				DeleteBitmap( Bitmap_Handle ); 
				Bitmap_Handle = NULL; 
				break; 
			}

		}while( FALSE );

		SelectObject( Device_Handle, Old_Bitmap ); 

		if( Bitmap_Handle != NULL )
		{
			*cx = IconWidth; 
			*cy = IconHeight; 
		}

	}while( FALSE );

	if( MaskBmpBits != NULL )
	{
		free( MaskBmpBits ); 
	}

	if( Device_Handle != NULL )
	{
		DeleteDC( Device_Handle );
	}

	if( Screen_Handle != NULL )
	{
		ReleaseDC( NULL,Screen_Handle );
	}

	return Bitmap_Handle;
}

typedef enum _FILE_DATA_TYPE
{
	FILE_DATA_PE, 
	FILE_DATA_OTHER, 
} FILE_DATA_TYPE, *PFILE_DATA_TYPE; 

FILE_DATA_TYPE GetFileDataType( HANDLE hFile )
{
#define FILE_HEADER_DATA_SIZE 2
	INT32 Ret; 
	FILE_DATA_TYPE DataType = FILE_DATA_OTHER; 
	BYTE FileHeaderData[ FILE_HEADER_DATA_SIZE ]; 
	ULONG Readed; 
	LARGE_INTEGER offset = { 0 }; 
	LARGE_INTEGER now_offset; 

	do 
	{
		if( hFile == INVALID_HANDLE_VALUE 
			|| hFile == NULL )
		{
			ASSERT( FALSE ); 
			break; 
		}

		Ret = SetFilePointerEx( hFile, offset, &now_offset, SEEK_SET ); 
		if( Ret == FALSE )
		{
			Ret = GetLastError(); 
			//log_trace( ( DBG_MSG_AND_ERROR_OUT, "set file pointer failed \n" ) ); 
			break; 
		}

		Ret = ReadFile( hFile, 
			FileHeaderData, 
			sizeof( FileHeaderData ), 
			&Readed, 
			NULL ); 

		if( FALSE == Ret )
		{
			Ret = SetFilePointerEx( hFile, offset, &now_offset, SEEK_SET ); 
			if( Ret == FALSE )
			{
				Ret = GetLastError(); 
				//log_trace( ( DBG_MSG_AND_ERROR_OUT, "set file pointer failed \n" ) ); 
				break; 
			}

			break; 
		}

		Ret = SetFilePointerEx( hFile, offset, &now_offset, SEEK_SET ); 
		if( Ret == FALSE )
		{
			Ret = GetLastError(); 
			//log_trace( ( DBG_MSG_AND_ERROR_OUT, "set file pointer failed \n" ) ); 
			break; 
		}

		if( Readed != sizeof( FileHeaderData ) )
		{
			break; 
		}

		if( FileHeaderData[ 0 ] == 'M' 
			&& FileHeaderData[ 1 ] == 'Z' )
		{
			//DBG_BP(); 
			DataType = FILE_DATA_PE; 
			break; 
		}

	}while( FALSE );

	return DataType; 
}

LRESULT get_pe_file_icon( LPCWSTR file_name,  
							   HICON *icon )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	SHFILEINFOW file_info; 

	file_info.hIcon = NULL; 

	do 
	{
		if( icon == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( file_name == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		//if( file_name_len == 0 )
		//{
		//	ret = ERROR_INVALID_PARAMETER; 
		//	break; 
		//}

		*icon = NULL; 

		file_info.dwAttributes = 0; 
		file_info.hIcon = NULL; 
		file_info.iIcon = -1; 
		*file_info.szDisplayName = L'\0'; 
		*file_info.szTypeName = L'\0'; 

		hr = ( HRESULT )SHGetFileInfo( file_name, 
			FILE_ATTRIBUTE_NORMAL, 
			&file_info, 
			sizeof( file_info ), 
			SHGFI_LARGEICON | SHGFI_USEFILEATTRIBUTES | SHGFI_ICON );
		//ExtractIconEx(); 

		if( hr == 0 )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		if( file_info.hIcon == NULL )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		*icon = file_info.hIcon; 

	}while( FALSE );

	if( ret != ERROR_SUCCESS )
	{
		if( file_info.hIcon != NULL )
		{
			DestroyIcon( file_info.hIcon ); 
		}
	}
#ifdef _DEBUG
	else
	{
		ASSERT( *icon != NULL ); 
	}
#endif //_DEBUG

	return ret; 
}

TImageInfo* CRenderEngine::LoadImage(STRINGorID bitmap, LPCTSTR type, DWORD mask, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
    LPBYTE pData = NULL;
    DWORD dwSize = 0;
	HBITMAP hBitmap = NULL; 
	TImageInfo* data = NULL; 
	bool bAlphaChannel = false;
	INT32 cx = 0; 
	INT32 cy = 0; 
	INT32 n = 0;

	//BOOLEAN ImageLoaded = FALSE; 

	do 
	{
		if( type == NULL ) 
		{
			CStdString sFile;  
			BOOLEAN load_file = FALSE; 

			if( 0 == ( flags & LOAD_FILE_FROM_ABS_PATH ) )
			{
				if( CPaintManagerUI::is_res_zipped() == FALSE ) 
				{
					sFile = CPaintManagerUI::GetResourcePath(); 
					sFile += bitmap.m_lpstr;
					load_file = TRUE; 
				}
			}
			else
			{
				sFile = bitmap.m_lpstr; 
				load_file = TRUE; 
			}

			if( TRUE == load_file )
			{
				HANDLE hFile = ::CreateFile(sFile.GetData(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, \
					FILE_ATTRIBUTE_NORMAL, NULL);
				
				if( hFile == INVALID_HANDLE_VALUE ) 
				{
					__Trace( _T( "open the image file %s error %u\n" ), 
						sFile.GetData(), 
						GetLastError() ); 
					
					return NULL;
				}

				DWORD dwRead = 0;
				FILE_DATA_TYPE DataType; 

				DataType = GetFileDataType( hFile ); 
				if( DataType == FILE_DATA_PE )
				{
					HICON icon; 
					CloseHandle( hFile ); 

					ret = get_pe_file_icon( sFile.GetData(), &icon ); 
					if( ret != ERROR_SUCCESS )
					{
						break; 
					}

					ASSERT( icon != NULL ); 

					hBitmap = ConvertIconToBitmap( icon, &cx, &cy ); 
					if( hBitmap == NULL )
					{
						ASSERT( cx == 0 ); 
						ASSERT( cy == 0 ); 

						DestroyIcon( icon ); 
						break; 
					}

					ASSERT( cx > 0 ); 
					ASSERT( cy > 0 ); 

					DestroyIcon( icon ); 
					bAlphaChannel = true; 
					
					//ImageLoaded = TRUE; 
					break; 
				}

				dwSize = ::GetFileSize(hFile, NULL);
				
				if( dwSize == 0 )
				{
					break;
				}

				pData = new BYTE[ dwSize ];
				if( pData == NULL )
				{
					break; 
				}

				::ReadFile( hFile, pData, dwSize, &dwRead, NULL );
				::CloseHandle( hFile );

				if( dwRead != dwSize ) 
				{
					delete[] pData;
					pData = NULL; 
					break; 
				}
			}
			else 
			{
				STRINGorID res_type; 
				STRINGorID res_name; 

				CPaintManagerUI::GetResourceZipResType( res_type, res_name ); 
				if( read_data_from_zip( CPaintManagerUI::GetResourceZip(), res_type, res_name, bitmap, &pData, &dwSize ) != ERROR_SUCCESS )
				{
					ASSERT( pData == NULL ); 
					ASSERT( dwSize == 0 ); 

#ifdef _DEBUG
					if( pData != NULL )
					{
						pData = NULL; 
						ASSERT( FALSE ); 
					}
#endif //_DEBUG
					break; 

				}
			}
		}
		else 
		{
			HRSRC hResource = ::FindResource(CPaintManagerUI::GetResourceDll(), bitmap.m_lpstr, type);

			if( hResource == NULL ) 
			{
				 break; 
			}

			HGLOBAL hGlobal = ::LoadResource(CPaintManagerUI::GetResourceDll(), hResource);
			if( hGlobal == NULL ) 
			{
				FreeResource(hResource);
				break;
			}

			dwSize = ::SizeofResource(CPaintManagerUI::GetResourceDll(), hResource);
			if( dwSize == 0 ) 
			{
				break;
			}

			pData = new BYTE[ dwSize ];
			if( pData == NULL )
			{
				break; 
			}

			::CopyMemory(pData, (LPBYTE)::LockResource(hGlobal), dwSize);
			::FreeResource(hResource);
		}
	}while( FALSE );

	do 
	{
		if( pData != NULL 
			&& hBitmap == NULL )
		{
			LPBYTE pImage = NULL;

			pImage = stbi_load_from_memory(pData, dwSize, &cx, &cy, &n, 4);
			delete[] pData;
			pData = NULL; 

			if( !pImage ) 
			{
				break; 
			}

			do 
			{
				LPBYTE pDest = NULL;
				BITMAPINFO bmi;
				::ZeroMemory(&bmi, sizeof(BITMAPINFO));
				bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
				bmi.bmiHeader.biWidth = cx;
				bmi.bmiHeader.biHeight = -cy;
				bmi.bmiHeader.biPlanes = 1;
				bmi.bmiHeader.biBitCount = 32;
				bmi.bmiHeader.biCompression = BI_RGB;
				bmi.bmiHeader.biSizeImage = cx * cy * 4;

				hBitmap = ::CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**)&pDest, NULL, 0);
				if( !hBitmap ) 
				{
					break;
				}

				for( int i = 0; i < cx * cy; i++ ) 
				{
					pDest[i*4 + 3] = pImage[i*4 + 3];
					if( pDest[i*4 + 3] < 255 )
					{
						pDest[i*4] = (BYTE)(DWORD(pImage[i*4 + 2])*pImage[i*4 + 3]/255);
						pDest[i*4 + 1] = (BYTE)(DWORD(pImage[i*4 + 1])*pImage[i*4 + 3]/255);
						pDest[i*4 + 2] = (BYTE)(DWORD(pImage[i*4])*pImage[i*4 + 3]/255); 
						bAlphaChannel = true;
					}
					else
					{
						pDest[i*4] = pImage[i*4 + 2];
						pDest[i*4 + 1] = pImage[i*4 + 1];
						pDest[i*4 + 2] = pImage[i*4]; 
					}

					if( *( DWORD* )( &pDest[ i * 4 ] ) == mask ) 
					{
						pDest[i*4] = (BYTE)0;
						pDest[i*4 + 1] = (BYTE)0;
						pDest[i*4 + 2] = (BYTE)0; 
						pDest[i*4 + 3] = (BYTE)0;
						bAlphaChannel = true;
					}
				}

			}while( FALSE ); 

			stbi_image_free(pImage); 
		}
	}while( FALSE ); 

	if( hBitmap != NULL )
	{
		data = new TImageInfo; 

		ASSERT( cx > 0 ); 
		ASSERT( cy > 0 ); 
		
		if( data != NULL )
		{
			data->hBitmap = hBitmap;
			data->nX = cx;
			data->nY = cy;
			data->alphaChannel = bAlphaChannel;
		}
	}

	return data;
}

void CRenderEngine::DrawImage(HDC hDC, HBITMAP hBitmap, const RECT& rc, const RECT& rcPaint,
                                    const RECT& rcBmpPart, const RECT& rcCorners, bool alphaChannel, 
                                    BYTE uFade, bool hole, bool xtiled, bool ytiled)
{
    ASSERT(::GetObjectType(hDC)==OBJ_DC || ::GetObjectType(hDC)==OBJ_MEMDC);

    typedef BOOL (WINAPI *LPALPHABLEND)(HDC, int, int, int, int,HDC, int, int, int, int, BLENDFUNCTION);
    static LPALPHABLEND lpAlphaBlend = (LPALPHABLEND) ::GetProcAddress(::GetModuleHandle(_T("msimg32.dll")), "AlphaBlend");

    if( lpAlphaBlend == NULL ) lpAlphaBlend = AlphaBitBlt;
    if( hBitmap == NULL ) return;

    HDC hCloneDC = ::CreateCompatibleDC(hDC);
    HBITMAP hOldBitmap = (HBITMAP) ::SelectObject(hCloneDC, hBitmap);
    ::SetStretchBltMode(hDC, COLORONCOLOR);

    RECT rcTemp = {0};
    RECT rcDest = {0};
    if( lpAlphaBlend && (alphaChannel || uFade < 255) ) {
        BLENDFUNCTION bf = { AC_SRC_OVER, 0, uFade, AC_SRC_ALPHA };
        // middle
        if( !hole ) {
            rcDest.left = rc.left + rcCorners.left;
            rcDest.top = rc.top + rcCorners.top;
            rcDest.right = rc.right - rc.left - rcCorners.left - rcCorners.right;
            rcDest.bottom = rc.bottom - rc.top - rcCorners.top - rcCorners.bottom;
            rcDest.right += rcDest.left;
            rcDest.bottom += rcDest.top;
            if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
                if( !xtiled && !ytiled ) {
                    rcDest.right -= rcDest.left;
                    rcDest.bottom -= rcDest.top;
                    lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
                        rcBmpPart.left + rcCorners.left, rcBmpPart.top + rcCorners.top, \
                        rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right, \
                        rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom, bf);
                }
                else if( xtiled && ytiled ) {
                    LONG lWidth = rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right;
                    LONG lHeight = rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom;
                    int iTimesX = (rcDest.right - rcDest.left + lWidth - 1) / lWidth;
                    int iTimesY = (rcDest.bottom - rcDest.top + lHeight - 1) / lHeight;
                    for( int j = 0; j < iTimesY; ++j ) {
                        LONG lDestTop = rcDest.top + lHeight * j;
                        LONG lDestBottom = rcDest.top + lHeight * (j + 1);
                        LONG lDrawHeight = lHeight;
                        if( lDestBottom > rcDest.bottom ) {
                            lDrawHeight -= lDestBottom - rcDest.bottom;
                            lDestBottom = rcDest.bottom;
                        }
                        for( int i = 0; i < iTimesX; ++i ) {
                            LONG lDestLeft = rcDest.left + lWidth * i;
                            LONG lDestRight = rcDest.left + lWidth * (i + 1);
                            LONG lDrawWidth = lWidth;
                            if( lDestRight > rcDest.right ) {
                                lDrawWidth -= lDestRight - rcDest.right;
                                lDestRight = rcDest.right;
                            }
                            lpAlphaBlend(hDC, rcDest.left + lWidth * i, rcDest.top + lHeight * j, 
                                lDestRight - lDestLeft, lDestBottom - lDestTop, hCloneDC, 
                                rcBmpPart.left + rcCorners.left, rcBmpPart.top + rcCorners.top, lDrawWidth, lDrawHeight, bf);
                        }
                    }
                }
                else if( xtiled ) {
                    LONG lWidth = rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right;
                    int iTimes = (rcDest.right - rcDest.left + lWidth - 1) / lWidth;
                    for( int i = 0; i < iTimes; ++i ) {
                        LONG lDestLeft = rcDest.left + lWidth * i;
                        LONG lDestRight = rcDest.left + lWidth * (i + 1);
                        LONG lDrawWidth = lWidth;
                        if( lDestRight > rcDest.right ) {
                            lDrawWidth -= lDestRight - rcDest.right;
                            lDestRight = rcDest.right;
                        }
                        lpAlphaBlend(hDC, lDestLeft, rcDest.top, lDestRight - lDestLeft, rcDest.bottom, 
                            hCloneDC, rcBmpPart.left + rcCorners.left, rcBmpPart.top + rcCorners.top, \
                            lDrawWidth, rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom, bf);
                    }
                }
                else { // ytiled
                    LONG lHeight = rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom;
                    int iTimes = (rcDest.bottom - rcDest.top + lHeight - 1) / lHeight;
                    for( int i = 0; i < iTimes; ++i ) {
                        LONG lDestTop = rcDest.top + lHeight * i;
                        LONG lDestBottom = rcDest.top + lHeight * (i + 1);
                        LONG lDrawHeight = lHeight;
                        if( lDestBottom > rcDest.bottom ) {
                            lDrawHeight -= lDestBottom - rcDest.bottom;
                            lDestBottom = rcDest.bottom;
                        }
                        lpAlphaBlend(hDC, rcDest.left, rcDest.top + lHeight * i, rcDest.right, lDestBottom - lDestTop, 
                            hCloneDC, rcBmpPart.left + rcCorners.left, rcBmpPart.top + rcCorners.top, \
                            rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right, lDrawHeight, bf);                    
                    }
                }
            }
        }

        // left-top
        if( rcCorners.left > 0 && rcCorners.top > 0 ) {
            rcDest.left = rc.left;
            rcDest.top = rc.top;
            rcDest.right = rcCorners.left;
            rcDest.bottom = rcCorners.top;
            rcDest.right += rcDest.left;
            rcDest.bottom += rcDest.top;
            if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
                rcDest.right -= rcDest.left;
                rcDest.bottom -= rcDest.top;
                lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
                    rcBmpPart.left, rcBmpPart.top, rcCorners.left, rcCorners.top, bf);
            }
        }
        // top
        if( rcCorners.top > 0 ) {
            rcDest.left = rc.left + rcCorners.left;
            rcDest.top = rc.top;
            rcDest.right = rc.right - rc.left - rcCorners.left - rcCorners.right;
            rcDest.bottom = rcCorners.top;
            rcDest.right += rcDest.left;
            rcDest.bottom += rcDest.top;
            if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
                rcDest.right -= rcDest.left;
                rcDest.bottom -= rcDest.top;
                lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
                    rcBmpPart.left + rcCorners.left, rcBmpPart.top, rcBmpPart.right - rcBmpPart.left - \
                    rcCorners.left - rcCorners.right, rcCorners.top, bf);
            }
        }
        // right-top
        if( rcCorners.right > 0 && rcCorners.top > 0 ) {
            rcDest.left = rc.right - rcCorners.right;
            rcDest.top = rc.top;
            rcDest.right = rcCorners.right;
            rcDest.bottom = rcCorners.top;
            rcDest.right += rcDest.left;
            rcDest.bottom += rcDest.top;
            if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
                rcDest.right -= rcDest.left;
                rcDest.bottom -= rcDest.top;
                lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
                    rcBmpPart.right - rcCorners.right, rcBmpPart.top, rcCorners.right, rcCorners.top, bf);
            }
        }
        // left
        if( rcCorners.left > 0 ) {
            rcDest.left = rc.left;
            rcDest.top = rc.top + rcCorners.top;
            rcDest.right = rcCorners.left;
            rcDest.bottom = rc.bottom - rc.top - rcCorners.top - rcCorners.bottom;
            rcDest.right += rcDest.left;
            rcDest.bottom += rcDest.top;
            if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
                rcDest.right -= rcDest.left;
                rcDest.bottom -= rcDest.top;
                lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
                    rcBmpPart.left, rcBmpPart.top + rcCorners.top, rcCorners.left, rcBmpPart.bottom - \
                    rcBmpPart.top - rcCorners.top - rcCorners.bottom, bf);
            }
        }
        // right
        if( rcCorners.right > 0 ) {
            rcDest.left = rc.right - rcCorners.right;
            rcDest.top = rc.top + rcCorners.top;
            rcDest.right = rcCorners.right;
            rcDest.bottom = rc.bottom - rc.top - rcCorners.top - rcCorners.bottom;
            rcDest.right += rcDest.left;
            rcDest.bottom += rcDest.top;
            if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
                rcDest.right -= rcDest.left;
                rcDest.bottom -= rcDest.top;
                lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
                    rcBmpPart.right - rcCorners.right, rcBmpPart.top + rcCorners.top, rcCorners.right, \
                    rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom, bf);
            }
        }
        // left-bottom
        if( rcCorners.left > 0 && rcCorners.bottom > 0 ) {
            rcDest.left = rc.left;
            rcDest.top = rc.bottom - rcCorners.bottom;
            rcDest.right = rcCorners.left;
            rcDest.bottom = rcCorners.bottom;
            rcDest.right += rcDest.left;
            rcDest.bottom += rcDest.top;
            if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
                rcDest.right -= rcDest.left;
                rcDest.bottom -= rcDest.top;
                lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
                    rcBmpPart.left, rcBmpPart.bottom - rcCorners.bottom, rcCorners.left, rcCorners.bottom, bf);
            }
        }
        // bottom
        if( rcCorners.bottom > 0 ) {
            rcDest.left = rc.left + rcCorners.left;
            rcDest.top = rc.bottom - rcCorners.bottom;
            rcDest.right = rc.right - rc.left - rcCorners.left - rcCorners.right;
            rcDest.bottom = rcCorners.bottom;
            rcDest.right += rcDest.left;
            rcDest.bottom += rcDest.top;
            if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
                rcDest.right -= rcDest.left;
                rcDest.bottom -= rcDest.top;
                lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
                    rcBmpPart.left + rcCorners.left, rcBmpPart.bottom - rcCorners.bottom, \
                    rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right, rcCorners.bottom, bf);
            }
        }
        // right-bottom
        if( rcCorners.right > 0 && rcCorners.bottom > 0 ) {
            rcDest.left = rc.right - rcCorners.right;
            rcDest.top = rc.bottom - rcCorners.bottom;
            rcDest.right = rcCorners.right;
            rcDest.bottom = rcCorners.bottom;
            rcDest.right += rcDest.left;
            rcDest.bottom += rcDest.top;
            if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
                rcDest.right -= rcDest.left;
                rcDest.bottom -= rcDest.top;
                lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
                    rcBmpPart.right - rcCorners.right, rcBmpPart.bottom - rcCorners.bottom, rcCorners.right, \
                    rcCorners.bottom, bf);
            }
        }
    }
    else 
    {
        if (rc.right - rc.left == rcBmpPart.right - rcBmpPart.left \
            && rc.bottom - rc.top == rcBmpPart.bottom - rcBmpPart.top \
            && rcCorners.left == 0 && rcCorners.right == 0 && rcCorners.top == 0 && rcCorners.bottom == 0)
        {
            if( ::IntersectRect(&rcTemp, &rcPaint, &rc) ) {
                ::BitBlt(hDC, rcTemp.left, rcTemp.top, rcTemp.right - rcTemp.left, rcTemp.bottom - rcTemp.top, \
                    hCloneDC, rcBmpPart.left + rcTemp.left - rc.left, rcBmpPart.top + rcTemp.top - rc.top, SRCCOPY);
            }
        }
        else
        {
            // middle
            if( !hole ) {
                rcDest.left = rc.left + rcCorners.left;
                rcDest.top = rc.top + rcCorners.top;
                rcDest.right = rc.right - rc.left - rcCorners.left - rcCorners.right;
                rcDest.bottom = rc.bottom - rc.top - rcCorners.top - rcCorners.bottom;
                rcDest.right += rcDest.left;
                rcDest.bottom += rcDest.top;
                if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
                    if( !xtiled && !ytiled ) {
                        rcDest.right -= rcDest.left;
                        rcDest.bottom -= rcDest.top;
                        ::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
                            rcBmpPart.left + rcCorners.left, rcBmpPart.top + rcCorners.top, \
                            rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right, \
                            rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom, SRCCOPY);
                    }
                    else if( xtiled && ytiled ) {
                        LONG lWidth = rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right;
                        LONG lHeight = rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom;
                        int iTimesX = (rcDest.right - rcDest.left + lWidth - 1) / lWidth;
                        int iTimesY = (rcDest.bottom - rcDest.top + lHeight - 1) / lHeight;
                        for( int j = 0; j < iTimesY; ++j ) {
                            LONG lDestTop = rcDest.top + lHeight * j;
                            LONG lDestBottom = rcDest.top + lHeight * (j + 1);
                            LONG lDrawHeight = lHeight;
                            if( lDestBottom > rcDest.bottom ) {
                                lDrawHeight -= lDestBottom - rcDest.bottom;
                                lDestBottom = rcDest.bottom;
                            }
                            for( int i = 0; i < iTimesX; ++i ) {
                                LONG lDestLeft = rcDest.left + lWidth * i;
                                LONG lDestRight = rcDest.left + lWidth * (i + 1);
                                LONG lDrawWidth = lWidth;
                                if( lDestRight > rcDest.right ) {
                                    lDrawWidth -= lDestRight - rcDest.right;
                                    lDestRight = rcDest.right;
                                }
                                ::BitBlt(hDC, rcDest.left + lWidth * i, rcDest.top + lHeight * j, \
                                    lDestRight - lDestLeft, lDestBottom - lDestTop, hCloneDC, \
                                    rcBmpPart.left + rcCorners.left, rcBmpPart.top + rcCorners.top, SRCCOPY);
                            }
                        }
                    }
                    else if( xtiled ) {
                        LONG lWidth = rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right;
                        int iTimes = (rcDest.right - rcDest.left + lWidth - 1) / lWidth;
                        for( int i = 0; i < iTimes; ++i ) {
                            LONG lDestLeft = rcDest.left + lWidth * i;
                            LONG lDestRight = rcDest.left + lWidth * (i + 1);
                            LONG lDrawWidth = lWidth;
                            if( lDestRight > rcDest.right ) {
                                lDrawWidth -= lDestRight - rcDest.right;
                                lDestRight = rcDest.right;
                            }
                            ::StretchBlt(hDC, lDestLeft, rcDest.top, lDestRight - lDestLeft, rcDest.bottom, 
                                hCloneDC, rcBmpPart.left + rcCorners.left, rcBmpPart.top + rcCorners.top, \
                                lDrawWidth, rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom, SRCCOPY);
                        }
                    }
                    else { // ytiled
                        LONG lHeight = rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom;
                        int iTimes = (rcDest.bottom - rcDest.top + lHeight - 1) / lHeight;
                        for( int i = 0; i < iTimes; ++i ) {
                            LONG lDestTop = rcDest.top + lHeight * i;
                            LONG lDestBottom = rcDest.top + lHeight * (i + 1);
                            LONG lDrawHeight = lHeight;
                            if( lDestBottom > rcDest.bottom ) {
                                lDrawHeight -= lDestBottom - rcDest.bottom;
                                lDestBottom = rcDest.bottom;
                            }
                            ::StretchBlt(hDC, rcDest.left, rcDest.top + lHeight * i, rcDest.right, lDestBottom - lDestTop, 
                                hCloneDC, rcBmpPart.left + rcCorners.left, rcBmpPart.top + rcCorners.top, \
                                rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right, lDrawHeight, SRCCOPY);                    
                        }
                    }
                }
            }
            
            // left-top
            if( rcCorners.left > 0 && rcCorners.top > 0 ) {
                rcDest.left = rc.left;
                rcDest.top = rc.top;
                rcDest.right = rcCorners.left;
                rcDest.bottom = rcCorners.top;
                rcDest.right += rcDest.left;
                rcDest.bottom += rcDest.top;
                if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
                    rcDest.right -= rcDest.left;
                    rcDest.bottom -= rcDest.top;
                    ::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
                        rcBmpPart.left, rcBmpPart.top, rcCorners.left, rcCorners.top, SRCCOPY);
                }
            }
            // top
            if( rcCorners.top > 0 ) {
                rcDest.left = rc.left + rcCorners.left;
                rcDest.top = rc.top;
                rcDest.right = rc.right - rc.left - rcCorners.left - rcCorners.right;
                rcDest.bottom = rcCorners.top;
                rcDest.right += rcDest.left;
                rcDest.bottom += rcDest.top;
                if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
                    rcDest.right -= rcDest.left;
                    rcDest.bottom -= rcDest.top;
                    ::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
                        rcBmpPart.left + rcCorners.left, rcBmpPart.top, rcBmpPart.right - rcBmpPart.left - \
                        rcCorners.left - rcCorners.right, rcCorners.top, SRCCOPY);
                }
            }
            // right-top
            if( rcCorners.right > 0 && rcCorners.top > 0 ) {
                rcDest.left = rc.right - rcCorners.right;
                rcDest.top = rc.top;
                rcDest.right = rcCorners.right;
                rcDest.bottom = rcCorners.top;
                rcDest.right += rcDest.left;
                rcDest.bottom += rcDest.top;
                if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
                    rcDest.right -= rcDest.left;
                    rcDest.bottom -= rcDest.top;
                    ::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
                        rcBmpPart.right - rcCorners.right, rcBmpPart.top, rcCorners.right, rcCorners.top, SRCCOPY);
                }
            }
            // left
            if( rcCorners.left > 0 ) {
                rcDest.left = rc.left;
                rcDest.top = rc.top + rcCorners.top;
                rcDest.right = rcCorners.left;
                rcDest.bottom = rc.bottom - rc.top - rcCorners.top - rcCorners.bottom;
                rcDest.right += rcDest.left;
                rcDest.bottom += rcDest.top;
                if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
                    rcDest.right -= rcDest.left;
                    rcDest.bottom -= rcDest.top;
                    ::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
                        rcBmpPart.left, rcBmpPart.top + rcCorners.top, rcCorners.left, rcBmpPart.bottom - \
                        rcBmpPart.top - rcCorners.top - rcCorners.bottom, SRCCOPY);
                }
            }
            // right
            if( rcCorners.right > 0 ) {
                rcDest.left = rc.right - rcCorners.right;
                rcDest.top = rc.top + rcCorners.top;
                rcDest.right = rcCorners.right;
                rcDest.bottom = rc.bottom - rc.top - rcCorners.top - rcCorners.bottom;
                rcDest.right += rcDest.left;
                rcDest.bottom += rcDest.top;
                if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
                    rcDest.right -= rcDest.left;
                    rcDest.bottom -= rcDest.top;
                    ::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
                        rcBmpPart.right - rcCorners.right, rcBmpPart.top + rcCorners.top, rcCorners.right, \
                        rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom, SRCCOPY);
                }
            }
            // left-bottom
            if( rcCorners.left > 0 && rcCorners.bottom > 0 ) {
                rcDest.left = rc.left;
                rcDest.top = rc.bottom - rcCorners.bottom;
                rcDest.right = rcCorners.left;
                rcDest.bottom = rcCorners.bottom;
                rcDest.right += rcDest.left;
                rcDest.bottom += rcDest.top;
                if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
                    rcDest.right -= rcDest.left;
                    rcDest.bottom -= rcDest.top;
                    ::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
                        rcBmpPart.left, rcBmpPart.bottom - rcCorners.bottom, rcCorners.left, rcCorners.bottom, SRCCOPY);
                }
            }
            // bottom
            if( rcCorners.bottom > 0 ) {
                rcDest.left = rc.left + rcCorners.left;
                rcDest.top = rc.bottom - rcCorners.bottom;
                rcDest.right = rc.right - rc.left - rcCorners.left - rcCorners.right;
                rcDest.bottom = rcCorners.bottom;
                rcDest.right += rcDest.left;
                rcDest.bottom += rcDest.top;
                if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
                    rcDest.right -= rcDest.left;
                    rcDest.bottom -= rcDest.top;
                    ::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
                        rcBmpPart.left + rcCorners.left, rcBmpPart.bottom - rcCorners.bottom, \
                        rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right, rcCorners.bottom, SRCCOPY);
                }
            }
            // right-bottom
            if( rcCorners.right > 0 && rcCorners.bottom > 0 ) {
                rcDest.left = rc.right - rcCorners.right;
                rcDest.top = rc.bottom - rcCorners.bottom;
                rcDest.right = rcCorners.right;
                rcDest.bottom = rcCorners.bottom;
                rcDest.right += rcDest.left;
                rcDest.bottom += rcDest.top;
                if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
                    rcDest.right -= rcDest.left;
                    rcDest.bottom -= rcDest.top;
                    ::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
                        rcBmpPart.right - rcCorners.right, rcBmpPart.bottom - rcCorners.bottom, rcCorners.right, \
                        rcCorners.bottom, SRCCOPY);
                }
            }
        }
    }

    ::SelectObject(hCloneDC, hOldBitmap);
    ::DeleteDC(hCloneDC);
}

bool DrawImage(HDC hDC, CPaintManagerUI* pManager, const RECT& rc, const RECT& rcPaint, const CStdString& sImageName, \
		const CStdString& sImageResType, RECT rcItem, RECT rcBmpPart, RECT rcCorner, DWORD dwMask, BYTE bFade, \
		bool bHole, bool bTiledX, bool bTiledY, ULONG flags = 0 )
{
	const TImageInfo* data = NULL;
	if( sImageResType.IsEmpty() ) {
		data = pManager->GetImageEx((LPCTSTR)sImageName, NULL, dwMask, flags );
	}
	else {
		data = pManager->GetImageEx((LPCTSTR)sImageName, (LPCTSTR)sImageResType, dwMask, flags );
	}
	if( !data ) return false;    

	if( rcBmpPart.left == 0 && rcBmpPart.right == 0 && rcBmpPart.top == 0 && rcBmpPart.bottom == 0 ) {
		rcBmpPart.right = data->nX;
		rcBmpPart.bottom = data->nY;
	}
	if (rcBmpPart.right > data->nX) rcBmpPart.right = data->nX;
	if (rcBmpPart.bottom > data->nY) rcBmpPart.bottom = data->nY;

	RECT rcTemp;
	if( !::IntersectRect(&rcTemp, &rcItem, &rc) ) return true;
	if( !::IntersectRect(&rcTemp, &rcItem, &rcPaint) ) return true;

	CRenderEngine::DrawImage(hDC, data->hBitmap, rcItem, rcPaint, rcBmpPart, rcCorner, data->alphaChannel, bFade, bHole, bTiledX, bTiledY);

	return true;
}

bool CRenderEngine::DrawImageString(HDC hDC, CPaintManagerUI* pManager, const RECT& rc, const RECT& rcPaint, 
                                          LPCTSTR pStrImage, LPCTSTR pStrModify)
{
	if ((pManager == NULL) || (hDC == NULL)) return false;

    // 1、aaa.jpg
    // 2、file='aaa.jpg' res='' restype='0' dest='0,0,0,0' source='0,0,0,0' corner='0,0,0,0' 
    // mask='#FF0000' fade='255' hole='false' xtiled='false' ytiled='false'

    CStdString sImageName = pStrImage;
    CStdString sImageResType;
#ifdef _DEBUG
	ULONG image_flags = 0xffffffff; 
#else
	ULONG image_flags = 0; 
#endif //_DEBUG

	RECT rcItem = rc;
    RECT rcBmpPart = {0};
    RECT rcCorner = {0};
    DWORD dwMask = 0;
    BYTE bFade = 0xFF;
    bool bHole = false;
    bool bTiledX = false;
    bool bTiledY = false;

	int image_count = 0;

    CStdString sItem;
    CStdString sValue;
    LPTSTR pstr = NULL;

    for( int i = 0; i < 2; ++i,image_count = 0 ) 
	{
        if( i == 1 )
		{
            pStrImage = pStrModify;
		}

        if( !pStrImage ) 
		{
			continue;
		}

        while( *pStrImage != _T('\0') ) 
		{
            sItem.Empty();
            sValue.Empty();

            while( *pStrImage > _T('\0') 
				&& *pStrImage <= _T(' ') ) 
			{
				pStrImage = ::CharNext(pStrImage); 
			}

            while( *pStrImage != _T('\0') 
				&& *pStrImage != _T('=') 
				&& *pStrImage > _T(' ') ) 
			{
                LPTSTR pstrTemp = ::CharNext(pStrImage);
                
				while( pStrImage < pstrTemp) 
				{
                    sItem += *pStrImage++;
                }
            }

            while( *pStrImage > _T('\0') 
				&& *pStrImage <= _T(' ') ) 
			{
				pStrImage = ::CharNext(pStrImage);
			}

			if( *pStrImage++ != _T('=') ) break;
            
			while( *pStrImage > _T('\0') && *pStrImage <= _T(' ') ) 
			{
				pStrImage = ::CharNext(pStrImage);
			}
			
			if( *pStrImage++ != _T('\'') ) 
			{
				break;
			}
			
			while( *pStrImage != _T('\0') && *pStrImage != _T('\'') ) 
			{
                LPTSTR pstrTemp = ::CharNext(pStrImage);
                while( pStrImage < pstrTemp) 
				{
                    sValue += *pStrImage++;
                }
            }

            if( *pStrImage++ != _T('\'') ) 
			{
				break;
			}

            if( !sValue.IsEmpty() ) 
			{
				if( sItem == _T( "filepath" ) )
				{
					if( image_count > 0 )
					{
						DuiLib::DrawImage(hDC, pManager, rc, rcPaint, sImageName, sImageResType,
						rcItem, rcBmpPart, rcCorner, dwMask, bFade, bHole, bTiledX, bTiledY, image_flags );
					}

					image_flags = LOAD_FILE_FROM_ABS_PATH; 
					sImageName = sValue;
					++image_count;
				}
				else if( sItem == _T("file") || sItem == _T("res") ) 
				{
					if( image_count > 0 )
					{
						DuiLib::DrawImage(hDC, pManager, rc, rcPaint, sImageName, sImageResType,
						rcItem, rcBmpPart, rcCorner, dwMask, bFade, bHole, bTiledX, bTiledY, image_flags );
					}

					image_flags = 0; 
					sImageName = sValue;
					++image_count;
				}
				else if( sItem == _T("restype") ) 
				{
					if( image_count > 0 )
					{
						DuiLib::DrawImage(hDC, pManager, rc, rcPaint, sImageName, sImageResType,
							rcItem, rcBmpPart, rcCorner, dwMask, bFade, bHole, bTiledX, bTiledY, image_flags );
					}

					image_flags = 0; 
                    sImageResType = sValue;
					++image_count;
                }
                else if( sItem == _T("dest") )
				{
                    rcItem.left = rc.left + _tcstol(sValue.GetData(), &pstr, 10);  ASSERT(pstr);    
                    rcItem.top = rc.top + _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
                    rcItem.right = rc.left + _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
					if (rcItem.right > rc.right) rcItem.right = rc.right;
                    rcItem.bottom = rc.top + _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
					if (rcItem.bottom > rc.bottom) rcItem.bottom = rc.bottom;
                }
                else if( sItem == _T("source") )
				{
                    rcBmpPart.left = _tcstol(sValue.GetData(), &pstr, 10);  ASSERT(pstr);    
                    rcBmpPart.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
                    rcBmpPart.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
                    rcBmpPart.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);  
                }
                else if( sItem == _T("corner") ) 
				{
                    rcCorner.left = _tcstol(sValue.GetData(), &pstr, 10);  ASSERT(pstr);    
                    rcCorner.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
                    rcCorner.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
                    rcCorner.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
                }
                else if( sItem == _T("mask") )
				{
                    if( sValue[0] == _T('#')) dwMask = _tcstoul(sValue.GetData() + 1, &pstr, 16);
                    else dwMask = _tcstoul(sValue.GetData(), &pstr, 16);
                }
                else if( sItem == _T("fade") ) 
				{
                    bFade = (BYTE)_tcstoul(sValue.GetData(), &pstr, 10);
                }
                else if( sItem == _T("hole") ) 
				{
                    bHole = (_tcscmp(sValue.GetData(), _T("true")) == 0);
                }
                else if( sItem == _T("xtiled") ) 
				{
                    bTiledX = (_tcscmp(sValue.GetData(), _T("true")) == 0);
                }
                else if( sItem == _T("ytiled") ) 
				{
                    bTiledY = (_tcscmp(sValue.GetData(), _T("true")) == 0);
                }
            }

            if( *pStrImage++ != _T( ' ' ) ) 
			{
				break;
			}
		}
    }

#ifdef _DEBUG
	if( image_flags == 0xffffffff )
	{
		image_flags = 0; 
	}
#endif //_DEBUG

	DuiLib::DrawImage( hDC, pManager, rc, rcPaint, sImageName, sImageResType,
		rcItem, rcBmpPart, rcCorner, dwMask, bFade, bHole, bTiledX, bTiledY, image_flags );

    return true;
}

//inline void CRenderEngine::_DrawColor(HDC hDC, const RECT& rc, DWORD color, DWORD fade )
//{
//
//	// Create a new 32bpp bitmap with room for an alpha channel
//	BITMAPINFO bmi = { 0 };
//	RECT rcBmpPart = {0, 0, 1, 1};
//	RECT rcCorners = {0};
//	LPDWORD pDest; 
//	HBITMAP hBitmap; 
//
//	if( color <= 0x00FFFFFF ) 
//	{
//		goto _return;
//	}
//
//	if( color >= 0xFF000000 )
//	{
//		goto _return; 
//	}
//
//	bmi.bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
//	bmi.bmiHeader.biWidth = 1;
//	bmi.bmiHeader.biHeight = 1;
//	bmi.bmiHeader.biPlanes = 1;
//	bmi.bmiHeader.biBitCount = 32;
//	bmi.bmiHeader.biCompression = BI_RGB;
//	bmi.bmiHeader.biSizeImage = 1 * 1 * sizeof( DWORD );
//	pDest = NULL;
//	hBitmap = ::CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, (LPVOID*) &pDest, NULL, 0);
//
//	if( hBitmap == NULL )
//	{
//		goto _return;
//	}
//
//	*pDest = color;
//
//	DrawImage(hDC, hBitmap, rc, rc, rcBmpPart, rcCorners, true, ( BYTE )fade );
//	::DeleteObject(hBitmap); 
//
//_return:
//	return; 
//}

void CRenderEngine::DrawFadeBkColor(HDC hDC, const RECT& rc, DWORD color, DWORD fade )
{
	if( color <= 0x00FFFFFF ) 
	{
		goto _return;
	}

	if( color >= 0xFF000000 )
	{
		goto _return; 
	}

	_DrawColor( hDC, rc, color, fade ); 

_return:
	return; 
}

void CRenderEngine::DrawColor(HDC hDC, const RECT& rc, DWORD color)
{
	if( color <= 0x00FFFFFF ) 
	{
		goto _return;
	}

	if( color >= 0xFF000000 )
	{
		::SetBkColor( hDC, RGB( GetBValue( color ), GetGValue( color ), GetRValue( color ) ) );
		::ExtTextOut( hDC, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL );
	}
	else
	{
		_DrawColor( hDC, rc, color, 255 ); 
	}

_return:
	return; 
}

void CRenderEngine::DrawGradient(HDC hDC, const RECT& rc, DWORD dwFirst, DWORD dwSecond, bool bVertical, int nSteps)
{
    typedef BOOL (WINAPI *LPALPHABLEND)(HDC, int, int, int, int,HDC, int, int, int, int, BLENDFUNCTION);
    static LPALPHABLEND lpAlphaBlend = (LPALPHABLEND) ::GetProcAddress(::GetModuleHandle(_T("msimg32.dll")), "AlphaBlend");
    if( lpAlphaBlend == NULL ) lpAlphaBlend = AlphaBitBlt;
    typedef BOOL (WINAPI *PGradientFill)(HDC, PTRIVERTEX, ULONG, PVOID, ULONG, ULONG);
    static PGradientFill lpGradientFill = (PGradientFill) ::GetProcAddress(::GetModuleHandle(_T("msimg32.dll")), "GradientFill");

    BYTE bAlpha = (BYTE)(((dwFirst >> 24) + (dwSecond >> 24)) >> 1);
    if( bAlpha == 0 ) return;
    int cx = rc.right - rc.left;
    int cy = rc.bottom - rc.top;
    RECT rcPaint = rc;
    HDC hPaintDC = hDC;
    HBITMAP hPaintBitmap = NULL;
    HBITMAP hOldPaintBitmap = NULL;
    if( bAlpha < 255 ) {
        rcPaint.left = rcPaint.top = 0;
        rcPaint.right = cx;
        rcPaint.bottom = cy;
        hPaintDC = ::CreateCompatibleDC(hDC);
        hPaintBitmap = ::CreateCompatibleBitmap(hDC, cx, cy);
        ASSERT(hPaintDC);
        ASSERT(hPaintBitmap);
        hOldPaintBitmap = (HBITMAP) ::SelectObject(hPaintDC, hPaintBitmap);
    }
    if( lpGradientFill != NULL ) 
    {
        TRIVERTEX triv[2] = 
        {
            { rcPaint.left, rcPaint.top, GetBValue(dwFirst) << 8, GetGValue(dwFirst) << 8, GetRValue(dwFirst) << 8, 0xFF00 },
            { rcPaint.right, rcPaint.bottom, GetBValue(dwSecond) << 8, GetGValue(dwSecond) << 8, GetRValue(dwSecond) << 8, 0xFF00 }
        };
        GRADIENT_RECT grc = { 0, 1 };
        lpGradientFill(hPaintDC, triv, 2, &grc, 1, bVertical ? GRADIENT_FILL_RECT_V : GRADIENT_FILL_RECT_H);
    }
    else 
    {
        // Determine how many shades
        int nShift = 1;
        if( nSteps >= 64 ) nShift = 6;
        else if( nSteps >= 32 ) nShift = 5;
        else if( nSteps >= 16 ) nShift = 4;
        else if( nSteps >= 8 ) nShift = 3;
        else if( nSteps >= 4 ) nShift = 2;
        int nLines = 1 << nShift;
        for( int i = 0; i < nLines; i++ ) {
            // Do a little alpha blending
            BYTE bR = (BYTE) ((GetBValue(dwSecond) * (nLines - i) + GetBValue(dwFirst) * i) >> nShift);
            BYTE bG = (BYTE) ((GetGValue(dwSecond) * (nLines - i) + GetGValue(dwFirst) * i) >> nShift);
            BYTE bB = (BYTE) ((GetRValue(dwSecond) * (nLines - i) + GetRValue(dwFirst) * i) >> nShift);
            // ... then paint with the resulting color
            HBRUSH hBrush = ::CreateSolidBrush(RGB(bR,bG,bB));
            RECT r2 = rcPaint;
            if( bVertical ) {
                r2.bottom = rc.bottom - ((i * (rc.bottom - rc.top)) >> nShift);
                r2.top = rc.bottom - (((i + 1) * (rc.bottom - rc.top)) >> nShift);
                if( (r2.bottom - r2.top) > 0 ) ::FillRect(hDC, &r2, hBrush);
            }
            else {
                r2.left = rc.right - (((i + 1) * (rc.right - rc.left)) >> nShift);
                r2.right = rc.right - ((i * (rc.right - rc.left)) >> nShift);
                if( (r2.right - r2.left) > 0 ) ::FillRect(hPaintDC, &r2, hBrush);
            }
            ::DeleteObject(hBrush);
        }
    }
    if( bAlpha < 255 ) {
        BLENDFUNCTION bf = { AC_SRC_OVER, 0, bAlpha, AC_SRC_ALPHA };
        lpAlphaBlend(hDC, rc.left, rc.top, cx, cy, hPaintDC, 0, 0, cx, cy, bf);
        ::SelectObject(hPaintDC, hOldPaintBitmap);
        ::DeleteObject(hPaintBitmap);
        ::DeleteDC(hPaintDC);
    }
}

void CRenderEngine::DrawAlphaGradient(HDC hDC, const RECT& rc, DWORD dwFirst, bool bVertical, int nSteps, INT32 nMaxAlpha)
{
	typedef BOOL (WINAPI *LPALPHABLEND)(HDC, int, int, int, int,HDC, int, int, int, int, BLENDFUNCTION);
	INT32 bAlpha; 
	static LPALPHABLEND lpAlphaBlend = ( LPALPHABLEND ) ::GetProcAddress( ::GetModuleHandle( _T( "msimg32.dll" ) ), "AlphaBlend" );
	
	ASSERT( nMaxAlpha > 0 && nMaxAlpha <= 255 ); 

	if( lpAlphaBlend == NULL ) 
	{
		lpAlphaBlend = AlphaBitBlt;
	}
	typedef BOOL ( WINAPI *PGradientFill )( HDC, PTRIVERTEX, ULONG, PVOID, ULONG, ULONG );
	static PGradientFill lpGradientFill = ( PGradientFill ) ::GetProcAddress( ::GetModuleHandle( _T( "msimg32.dll" ) ), "GradientFill" );

	int cx = rc.right - rc.left;
	int cy = rc.bottom - rc.top;
	RECT rcPaint = rc;
	HDC hPaintDC = hDC;
	HBITMAP hPaintBitmap = NULL;
	HBITMAP hOldPaintBitmap = NULL;

	rcPaint.left = rcPaint.top = 0;
	rcPaint.right = cx;
	rcPaint.bottom = cy;
	hPaintDC = ::CreateCompatibleDC(hDC);
	hPaintBitmap = ::CreateCompatibleBitmap(hDC, cx, cy);
	ASSERT(hPaintDC);
	ASSERT(hPaintBitmap);
	hOldPaintBitmap = (HBITMAP) ::SelectObject(hPaintDC, hPaintBitmap);

	// Determine how many shades
	int nShift = 1;
	if( nSteps >= 64 ) nShift = 6;
	else if( nSteps >= 32 ) nShift = 5;
	else if( nSteps >= 16 ) nShift = 4;
	else if( nSteps >= 8 ) nShift = 3;
	else if( nSteps >= 4 ) nShift = 2;
	int nLines = 1 << nShift;

	// Do a little alpha blending
	BYTE bR = ( BYTE )GetBValue( dwFirst );
	BYTE bG = ( BYTE )GetGValue( dwFirst );
	BYTE bB = ( BYTE )GetRValue( dwFirst );

	// ... then paint with the resulting color
	HBRUSH hBrush = ::CreateSolidBrush(RGB(bR,bG,bB));
	RECT r2 = rcPaint;
	::FillRect(hPaintDC, &r2, hBrush);
	::DeleteObject(hBrush);

	for( int i = 0; i < nLines; i++ ) 
	{
		RECT r2 = rcPaint;
		if( bVertical ) 
		{
			r2.bottom = rc.bottom - ((i * (rc.bottom - rc.top)) >> nShift);
			bAlpha = ( ( nMaxAlpha * ( rc.bottom - r2.bottom ) ) / rcPaint.bottom ); 
			if( bAlpha < 0 )
			{
				bAlpha = 0; 
			}
			else if( bAlpha > nMaxAlpha )
			{
				bAlpha = nMaxAlpha; 
			}

			r2.top = rc.bottom - (((i + 1) * (rc.bottom - rc.top)) >> nShift);
			
			if( (r2.bottom - r2.top) > 0 ) 
			{
				BLENDFUNCTION bf = { AC_SRC_OVER, 0, bAlpha, AC_SRC_ALPHA };
				lpAlphaBlend(hDC, 
					rc.left, 
					r2.top, 
					r2.right - r2.left, 
					r2.bottom - r2.top, 
					hPaintDC, 
					0, 
					0, 
					r2.right - r2.left, 
					r2.bottom - r2.top, 
					bf );
			}
		}
		else 
		{
			r2.left = rc.right - (((i + 1) * (rc.right - rc.left)) >> nShift);
			r2.right = rc.right - ((i * (rc.right - rc.left)) >> nShift);
			if( (r2.right - r2.left) > 0 )
			{
				BLENDFUNCTION bf = { AC_SRC_OVER, 0, bAlpha, AC_SRC_ALPHA };
				lpAlphaBlend(hDC, 
					r2.left, 
					rc.top, 
					r2.right - r2.left, 
					r2.bottom - r2.top, 
					hPaintDC, 
					0, 
					0, 
					r2.right - r2.left, 
					r2.bottom - r2.top, 
					bf );
			}
		}
	}

	::SelectObject(hPaintDC, hOldPaintBitmap);
	::DeleteObject(hPaintBitmap);
	::DeleteDC(hPaintDC);
}

void CRenderEngine::DrawLine(HDC hDC, const RECT& rc, int nSize, DWORD dwPenColor)
{
	DrawLine( hDC, rc.left, rc.top, rc.right, rc.bottom, nSize, dwPenColor ); 
}

inline void CRenderEngine::DrawLine(HDC hDC, LONG left, LONG top, LONG right, LONG bottom, int nSize, DWORD dwPenColor )
{
	HPEN hPen; 
	POINT ptTemp = { 0 };
	HPEN hOldPen; 

	ASSERT(::GetObjectType( hDC ) == OBJ_DC || ::GetObjectType( hDC ) == OBJ_MEMDC );
	hPen = ::CreatePen(PS_SOLID, nSize, RGB(GetBValue(dwPenColor), GetGValue(dwPenColor), GetRValue(dwPenColor)));
	
	if( hPen == NULL )
	{
		return; 
	}

	hOldPen = (HPEN)::SelectObject(hDC, hPen);
	::MoveToEx(hDC, left, top, &ptTemp);
	::LineTo(hDC, right, bottom);
	::SelectObject(hDC, hOldPen);
	::DeleteObject(hPen);
}

void CRenderEngine::DrawRect(HDC hDC, const RECT& rc, int nSize, DWORD dwPenColor)
{
	HPEN hPen; 
	HPEN hOldPen; 

	ASSERT(::GetObjectType(hDC)==OBJ_DC || ::GetObjectType(hDC)==OBJ_MEMDC);

	hPen = ::CreatePen( PS_SOLID | PS_INSIDEFRAME, 
		nSize, 
		RGB( GetBValue( dwPenColor ), 
		GetGValue( dwPenColor ), 
		GetRValue( dwPenColor ) ) );
	if( hPen == NULL )
	{
		goto _return; 
	}

    hOldPen = ( HPEN )::SelectObject( hDC, hPen );
    ::SelectObject(hDC, ::GetStockObject(HOLLOW_BRUSH));
    ::Rectangle(hDC, rc.left, rc.top, rc.right, rc.bottom);
    ::SelectObject(hDC, hOldPen);
    ::DeleteObject(hPen);

_return:
	return; 
}

void CRenderEngine::DrawRoundRect(HDC hDC, const RECT& rc, int nSize, int width, int height, DWORD dwPenColor)
{
	HPEN hPen; 
	HPEN hOldPen; 

    ASSERT( ::GetObjectType( hDC ) == OBJ_DC || ::GetObjectType( hDC ) == OBJ_MEMDC ); 

    hPen = ::CreatePen(PS_SOLID | PS_INSIDEFRAME, nSize, RGB(GetBValue(dwPenColor), GetGValue(dwPenColor), GetRValue(dwPenColor)));
	if( hPen == NULL )
	{
		goto _return; 
	}

	hOldPen = (HPEN)::SelectObject(hDC, hPen);

	::SelectObject(hDC, ::GetStockObject(HOLLOW_BRUSH));
    ::RoundRect(hDC, rc.left, rc.top, rc.right, rc.bottom, width, height);
    ::SelectObject(hDC, hOldPen);
    ::DeleteObject(hPen);

_return:
	return; 
}

void CRenderEngine::DrawText(HDC hDC, CPaintManagerUI* pManager, RECT& rc, LPCTSTR pstrText, DWORD dwTextColor, int iFont, UINT uStyle)
{
    ASSERT(::GetObjectType(hDC)==OBJ_DC || ::GetObjectType(hDC)==OBJ_MEMDC);
    ::SetBkMode(hDC, TRANSPARENT);
    ::SetTextColor(hDC, RGB(GetBValue(dwTextColor), GetGValue(dwTextColor), GetRValue(dwTextColor)));
    HFONT hOldFont = (HFONT)::SelectObject(hDC, pManager->GetFont(iFont));
    ::DrawText(hDC, pstrText, -1, &rc, uStyle | DT_NOPREFIX);
    ::SelectObject(hDC, hOldFont);
}

void CRenderEngine::DrawHtmlText(HDC hDC, CPaintManagerUI* pManager, RECT& rc, LPCTSTR pstrText, DWORD dwTextColor, RECT* prcLinks, CStdString* sLinks, int& nLinkRects, UINT uStyle)
{
    // 考虑到在xml编辑器中使用<>符号不方便，可以使用{}符号代替
    // 支持标签嵌套（如<l><b>text</b></l>），但是交叉嵌套是应该避免的（如<l><b>text</l></b>）
    // The string formatter supports a kind of "mini-html" that consists of various short tags:
    //
    //   Bold:             <b>text</b>
    //   Color:            <c #xxxxxx>text</c>  where x = RGB in hex
    //   Font:             <f x>text</f>        where x = font id
    //   Italic:           <i>text</i>
    //   Image:            <i x y z>            where x = image name and y = imagelist num and z(optional) = imagelist id
    //   Link:             <a x>text</a>        where x(optional) = link content, normal like app:notepad or http:www.xxx.com
    //   NewLine           <n>                  
    //   Paragraph:        <p x>text</p>        where x = extra pixels indent in p
    //   Raw Text:         <r>text</r>
    //   Selected:         <s>text</s>
    //   Underline:        <u>text</u>
    //   X Indent:         <x i>                where i = hor indent in pixels
    //   Y Indent:         <y i>                where i = ver indent in pixels 

    ASSERT(::GetObjectType(hDC)==OBJ_DC || ::GetObjectType(hDC)==OBJ_MEMDC);
    if( pstrText == NULL ) return;
    if( ::IsRectEmpty(&rc) ) return;

    bool bDraw = (uStyle & DT_CALCRECT) == 0;

    CStdPtrArray aFontArray(10);
    CStdPtrArray aColorArray(10);
    CStdPtrArray aPIndentArray(10);

    RECT rcClip = { 0 };
    ::GetClipBox(hDC, &rcClip);
    HRGN hOldRgn = ::CreateRectRgnIndirect(&rcClip);
    HRGN hRgn = ::CreateRectRgnIndirect(&rc);
    if( bDraw ) ::ExtSelectClipRgn(hDC, hRgn, RGN_AND);

    TEXTMETRIC* pTm = &pManager->GetDefaultFontInfo()->tm;
    HFONT hOldFont = (HFONT) ::SelectObject(hDC, pManager->GetDefaultFontInfo()->hFont);
    ::SetBkMode(hDC, TRANSPARENT);
    ::SetTextColor(hDC, RGB(GetBValue(dwTextColor), GetGValue(dwTextColor), GetRValue(dwTextColor)));
    DWORD dwBkColor = pManager->GetDefaultSelectedBkColor();
    ::SetBkColor(hDC, RGB(GetBValue(dwBkColor), GetGValue(dwBkColor), GetRValue(dwBkColor)));

    // If the drawstyle include a alignment, we'll need to first determine the text-size so
    // we can draw it at the correct position...
    if( (uStyle & DT_SINGLELINE) != 0 && (uStyle & DT_VCENTER) != 0 && (uStyle & DT_CALCRECT) == 0 ) {
        RECT rcText = { 0, 0, 9999, 100 };
        int nLinks = 0;
        DrawHtmlText(hDC, pManager, rcText, pstrText, dwTextColor, NULL, NULL, nLinks, uStyle | DT_CALCRECT);
        rc.top = rc.top + ((rc.bottom - rc.top) / 2) - ((rcText.bottom - rcText.top) / 2);
        rc.bottom = rc.top + (rcText.bottom - rcText.top);
    }
    if( (uStyle & DT_SINGLELINE) != 0 && (uStyle & DT_CENTER) != 0 && (uStyle & DT_CALCRECT) == 0 ) {
        RECT rcText = { 0, 0, 9999, 100 };
        int nLinks = 0;
        DrawHtmlText(hDC, pManager, rcText, pstrText, dwTextColor, NULL, NULL, nLinks, uStyle | DT_CALCRECT);
        rc.left = rc.left + ((rc.right - rc.left) / 2) - ((rcText.right - rcText.left) / 2);
        rc.right = rc.left + (rcText.right - rcText.left);
    }
    if( (uStyle & DT_SINGLELINE) != 0 && (uStyle & DT_RIGHT) != 0 && (uStyle & DT_CALCRECT) == 0 ) {
        RECT rcText = { 0, 0, 9999, 100 };
        int nLinks = 0;
        DrawHtmlText(hDC, pManager, rcText, pstrText, dwTextColor, NULL, NULL, nLinks, uStyle | DT_CALCRECT);
        rc.left = rc.right - (rcText.right - rcText.left);
    }

    bool bHoverLink = false;
    CStdString sHoverLink;
    POINT ptMouse = pManager->GetMousePos();
    for( int i = 0; !bHoverLink && i < nLinkRects; i++ ) {
        if( ::PtInRect(prcLinks + i, ptMouse) ) {
            sHoverLink = *(CStdString*)(sLinks + i);
            bHoverLink = true;
        }
    }

    POINT pt = { rc.left, rc.top };
    int iLinkIndex = 0;
    int cyLine = pTm->tmHeight + pTm->tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1);
    int cyMinHeight = 0;
    int cxMaxWidth = 0;
    POINT ptLinkStart = { 0 };
    bool bLineEnd = false;
    bool bInRaw = false;
    bool bInLink = false;
    bool bInSelected = false;
    int iLineLinkIndex = 0;

    // 排版习惯是图文底部对齐，所以每行绘制都要分两步，先计算高度，再绘制
    CStdPtrArray aLineFontArray;
    CStdPtrArray aLineColorArray;
    CStdPtrArray aLinePIndentArray;
    LPCTSTR pstrLineBegin = pstrText;
    bool bLineInRaw = false;
    bool bLineInLink = false;
    bool bLineInSelected = false;
    int cyLineHeight = 0;
    bool bLineDraw = false; // 行的第二阶段：绘制
    while( *pstrText != _T('\0') ) {
        if( pt.x >= rc.right || *pstrText == _T('\n') || bLineEnd ) {
            if( *pstrText == _T('\n') ) pstrText++;
            if( bLineEnd ) bLineEnd = false;
            if( !bLineDraw ) {
                if( bInLink && iLinkIndex < nLinkRects ) {
                    ::SetRect(&prcLinks[iLinkIndex++], ptLinkStart.x, ptLinkStart.y, MIN(pt.x, rc.right), pt.y + cyLine);
                    CStdString *pStr1 = (CStdString*)(sLinks + iLinkIndex - 1);
                    CStdString *pStr2 = (CStdString*)(sLinks + iLinkIndex);
                    *pStr2 = *pStr1;
                }
                for( int i = iLineLinkIndex; i < iLinkIndex; i++ ) {
                    prcLinks[i].bottom = pt.y + cyLine;
                }
                if( bDraw ) {
                    bInLink = bLineInLink;
                    iLinkIndex = iLineLinkIndex;
                }
            }
            else {
                if( bInLink && iLinkIndex < nLinkRects ) iLinkIndex++;
                bLineInLink = bInLink;
                iLineLinkIndex = iLinkIndex;
            }
            if( (uStyle & DT_SINGLELINE) != 0 && (!bDraw || bLineDraw) ) break;
            if( bDraw ) bLineDraw = !bLineDraw; // !
            pt.x = rc.left;
            if( !bLineDraw ) pt.y += cyLine;
            if( pt.y > rc.bottom && bDraw ) break;
            ptLinkStart = pt;
            cyLine = pTm->tmHeight + pTm->tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1);
            if( pt.x >= rc.right ) break;
        }
        else if( !bInRaw && ( *pstrText == _T('<') || *pstrText == _T('{') )
            && ( pstrText[1] >= _T('a') && pstrText[1] <= _T('z') )
            && ( pstrText[2] == _T(' ') || pstrText[2] == _T('>') || pstrText[2] == _T('}') ) ) {
                pstrText++;
                LPCTSTR pstrNextStart = NULL;
                switch( *pstrText ) {
            case _T('a'):  // Link
                {
                    pstrText++;
                    while( *pstrText > _T('\0') && *pstrText <= _T(' ') ) pstrText = ::CharNext(pstrText);
                    if( iLinkIndex < nLinkRects && !bLineDraw ) {
                        CStdString *pStr = (CStdString*)(sLinks + iLinkIndex);
                        pStr->Empty();
                        while( *pstrText != _T('\0') && *pstrText != _T('>') && *pstrText != _T('}') ) {
                            LPCTSTR pstrTemp = ::CharNext(pstrText);
                            while( pstrText < pstrTemp) {
                                *pStr += *pstrText++;
                            }
                        }
                    }

                    DWORD clrColor = pManager->GetDefaultLinkFontColor();
                    if( bHoverLink && iLinkIndex < nLinkRects ) {
                        CStdString *pStr = (CStdString*)(sLinks + iLinkIndex);
                        if( sHoverLink == *pStr ) clrColor = pManager->GetDefaultLinkHoverFontColor();
                    }
                    //else if( prcLinks == NULL ) {
                    //    if( ::PtInRect(&rc, ptMouse) )
                    //        clrColor = pManager->GetDefaultLinkHoverFontColor();
                    //}
                    aColorArray.Add((LPVOID)clrColor);
                    ::SetTextColor(hDC,  RGB(GetBValue(clrColor), GetGValue(clrColor), GetRValue(clrColor)));
                    TFontInfo* pFontInfo = pManager->GetDefaultFontInfo();
                    if( aFontArray.GetSize() > 0 ) pFontInfo = (TFontInfo*)aFontArray.GetAt(aFontArray.GetSize() - 1);
                    if( pFontInfo->bUnderline == false ) {
                        HFONT hFont = pManager->GetFont(pFontInfo->sFontName, pFontInfo->iSize, pFontInfo->bBold, true, pFontInfo->bItalic);
                        if( hFont == NULL ) hFont = pManager->AddFont(pFontInfo->sFontName, pFontInfo->iSize, pFontInfo->bBold, true, pFontInfo->bItalic);
                        pFontInfo = pManager->GetFontInfo(hFont);
                        aFontArray.Add(pFontInfo);
                        pTm = &pFontInfo->tm;
                        ::SelectObject(hDC, pFontInfo->hFont);
                        cyLine = MAX(cyLine, pTm->tmHeight + pTm->tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1));
                    }
                    ptLinkStart = pt;
                    bInLink = true;
                }
                break;
            case _T('b'):  // Bold
                {
                    pstrText++;
                    TFontInfo* pFontInfo = pManager->GetDefaultFontInfo();
                    if( aFontArray.GetSize() > 0 ) pFontInfo = (TFontInfo*)aFontArray.GetAt(aFontArray.GetSize() - 1);
                    if( pFontInfo->bBold == false ) {
                        HFONT hFont = pManager->GetFont(pFontInfo->sFontName, pFontInfo->iSize, true, pFontInfo->bUnderline, pFontInfo->bItalic);
                        if( hFont == NULL ) hFont = pManager->AddFont(pFontInfo->sFontName, pFontInfo->iSize, true, pFontInfo->bUnderline, pFontInfo->bItalic);
                        pFontInfo = pManager->GetFontInfo(hFont);
                        aFontArray.Add(pFontInfo);
                        pTm = &pFontInfo->tm;
                        ::SelectObject(hDC, pFontInfo->hFont);
                        cyLine = MAX(cyLine, pTm->tmHeight + pTm->tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1));
                    }
                }
                break;
            case _T('c'):  // Color
                {
                    pstrText++;
                    while( *pstrText > _T('\0') && *pstrText <= _T(' ') ) pstrText = ::CharNext(pstrText);
                    if( *pstrText == _T('#')) pstrText++;
                    DWORD clrColor = _tcstol(pstrText, const_cast<LPTSTR*>(&pstrText), 16);
                    aColorArray.Add((LPVOID)clrColor);
                    ::SetTextColor(hDC, RGB(GetBValue(clrColor), GetGValue(clrColor), GetRValue(clrColor)));
                }
                break;
            case _T('f'):  // Font
                {
                    pstrText++;
                    while( *pstrText > _T('\0') && *pstrText <= _T(' ') ) pstrText = ::CharNext(pstrText);
                    LPCTSTR pstrTemp = pstrText;
                    int iFont = (int) _tcstol(pstrText, const_cast<LPTSTR*>(&pstrText), 10);
                    //if( isdigit(*pstrText) ) { // debug版本会引起异常
                    if( pstrTemp != pstrText ) {
                        TFontInfo* pFontInfo = pManager->GetFontInfo(iFont);
                        aFontArray.Add(pFontInfo);
                        pTm = &pFontInfo->tm;
                        ::SelectObject(hDC, pFontInfo->hFont);
                    }
                    else {
                        CStdString sFontName;
                        int iFontSize = 10;
                        CStdString sFontAttr;
                        bool bBold = false;
                        bool bUnderline = false;
                        bool bItalic = false;
                        while( *pstrText != _T('\0') && *pstrText != _T('>') && *pstrText != _T('}') && *pstrText != _T(' ') ) {
                            pstrTemp = ::CharNext(pstrText);
                            while( pstrText < pstrTemp) {
                                sFontName += *pstrText++;
                            }
                        }
                        while( *pstrText > _T('\0') && *pstrText <= _T(' ') ) pstrText = ::CharNext(pstrText);
                        if( isdigit(*pstrText) ) {
                            iFontSize = (int) _tcstol(pstrText, const_cast<LPTSTR*>(&pstrText), 10);
                        }
                        while( *pstrText > _T('\0') && *pstrText <= _T(' ') ) pstrText = ::CharNext(pstrText);
                        while( *pstrText != _T('\0') && *pstrText != _T('>') && *pstrText != _T('}') ) {
                            pstrTemp = ::CharNext(pstrText);
                            while( pstrText < pstrTemp) {
                                sFontAttr += *pstrText++;
                            }
                        }
                        sFontAttr.MakeLower();
                        if( sFontAttr.Find(_T("bold")) >= 0 ) bBold = true;
                        if( sFontAttr.Find(_T("underline")) >= 0 ) bUnderline = true;
                        if( sFontAttr.Find(_T("italic")) >= 0 ) bItalic = true;
                        HFONT hFont = pManager->GetFont(sFontName, iFontSize, bBold, bUnderline, bItalic);
                        if( hFont == NULL ) hFont = pManager->AddFont(sFontName, iFontSize, bBold, bUnderline, bItalic);
                        TFontInfo* pFontInfo = pManager->GetFontInfo(hFont);
                        aFontArray.Add(pFontInfo);
                        pTm = &pFontInfo->tm;
                        ::SelectObject(hDC, pFontInfo->hFont);
                    }
                    cyLine = MAX(cyLine, pTm->tmHeight + pTm->tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1));
                }
                break;
            case _T('i'):  // Italic or Image
                {
                    pstrNextStart = pstrText - 1;
                    pstrText++;
                    int iWidth = 0;
                    int iHeight = 0;
                    while( *pstrText > _T('\0') && *pstrText <= _T(' ') ) pstrText = ::CharNext(pstrText);
                    const TImageInfo* pImageInfo = NULL;
                    CStdString sName;
                    while( *pstrText != _T('\0') && *pstrText != _T('>') && *pstrText != _T('}') && *pstrText != _T(' ') ) {
                        LPCTSTR pstrTemp = ::CharNext(pstrText);
                        while( pstrText < pstrTemp) {
                            sName += *pstrText++;
                        }
                    }
                    if( sName.IsEmpty() ) { // Italic
                        pstrNextStart = NULL;
                        TFontInfo* pFontInfo = pManager->GetDefaultFontInfo();
                        if( aFontArray.GetSize() > 0 ) pFontInfo = (TFontInfo*)aFontArray.GetAt(aFontArray.GetSize() - 1);
                        if( pFontInfo->bItalic == false ) {
                            HFONT hFont = pManager->GetFont(pFontInfo->sFontName, pFontInfo->iSize, pFontInfo->bBold, pFontInfo->bUnderline, true);
                            if( hFont == NULL ) hFont = pManager->AddFont(pFontInfo->sFontName, pFontInfo->iSize, pFontInfo->bBold, pFontInfo->bUnderline, true);
                            pFontInfo = pManager->GetFontInfo(hFont);
                            aFontArray.Add(pFontInfo);
                            pTm = &pFontInfo->tm;
                            ::SelectObject(hDC, pFontInfo->hFont);
                            cyLine = MAX(cyLine, pTm->tmHeight + pTm->tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1));
                        }
                    }
                    else {
                        while( *pstrText > _T('\0') && *pstrText <= _T(' ') ) pstrText = ::CharNext(pstrText);
                        int iImageListNum = (int) _tcstol(pstrText, const_cast<LPTSTR*>(&pstrText), 10);
                        if( iImageListNum <= 0 ) iImageListNum = 1;
                        while( *pstrText > _T('\0') && *pstrText <= _T(' ') ) pstrText = ::CharNext(pstrText);
                        int iImageListIndex = (int) _tcstol(pstrText, const_cast<LPTSTR*>(&pstrText), 10);
                        if( iImageListIndex < 0 || iImageListIndex >= iImageListNum ) iImageListIndex = 0;

                        pImageInfo = pManager->GetImageEx((LPCTSTR)sName);
                        if( pImageInfo ) {
                            iWidth = pImageInfo->nX;
                            iHeight = pImageInfo->nY;
                            if( iImageListNum > 1 ) iWidth /= iImageListNum;

                            if( pt.x + iWidth > rc.right && pt.x > rc.left && (uStyle & DT_SINGLELINE) == 0 ) {
                                bLineEnd = true;
                            }
                            else {
                                pstrNextStart = NULL;
                                if( bDraw && bLineDraw ) {
                                    CRect rcImage(pt.x, pt.y + cyLineHeight - iHeight, pt.x + iWidth, pt.y + cyLineHeight);
                                    if( iHeight < cyLineHeight ) { 
                                        rcImage.bottom -= (cyLineHeight - iHeight) / 2;
                                        rcImage.top = rcImage.bottom -  iHeight;
                                    }
                                    CRect rcBmpPart(0, 0, iWidth, iHeight);
                                    rcBmpPart.left = iWidth * iImageListIndex;
                                    rcBmpPart.right = iWidth * (iImageListIndex + 1);
                                    CRect rcCorner(0, 0, 0, 0);
                                    DrawImage(hDC, pImageInfo->hBitmap, rcImage, rcImage, rcBmpPart, rcCorner, \
                                        pImageInfo->alphaChannel, 255);
                                }

                                cyLine = MAX(iHeight, cyLine);
                                pt.x += iWidth;
                                cyMinHeight = pt.y + iHeight;
                                cxMaxWidth = MAX(cxMaxWidth, pt.x);
                            }
                        }
                        else pstrNextStart = NULL;
                    }
                }
                break;
            case _T('n'):  // Newline
                {
                    pstrText++;
                    if( (uStyle & DT_SINGLELINE) != 0 ) break;
                    bLineEnd = true;
                }
                break;
            case _T('p'):  // Paragraph
                {
                    pstrText++;
                    if( pt.x > rc.left ) bLineEnd = true;
                    while( *pstrText > _T('\0') && *pstrText <= _T(' ') ) pstrText = ::CharNext(pstrText);
                    int cyLineExtra = (int)_tcstol(pstrText, const_cast<LPTSTR*>(&pstrText), 10);
                    aPIndentArray.Add((LPVOID)cyLineExtra);
                    cyLine = MAX(cyLine, pTm->tmHeight + pTm->tmExternalLeading + cyLineExtra);
                }
                break;
            case _T('r'):  // Raw Text
                {
                    pstrText++;
                    bInRaw = true;
                }
                break;
            case _T('s'):  // Selected text background color
                {
                    pstrText++;
                    bInSelected = !bInSelected;
                    if( bDraw && bLineDraw ) {
                        if( bInSelected ) ::SetBkMode(hDC, OPAQUE);
                        else ::SetBkMode(hDC, TRANSPARENT);
                    }
                }
                break;
            case _T('u'):  // Underline text
                {
                    pstrText++;
                    TFontInfo* pFontInfo = pManager->GetDefaultFontInfo();
                    if( aFontArray.GetSize() > 0 ) pFontInfo = (TFontInfo*)aFontArray.GetAt(aFontArray.GetSize() - 1);
                    if( pFontInfo->bUnderline == false ) {
                        HFONT hFont = pManager->GetFont(pFontInfo->sFontName, pFontInfo->iSize, pFontInfo->bBold, true, pFontInfo->bItalic);
                        if( hFont == NULL ) hFont = pManager->AddFont(pFontInfo->sFontName, pFontInfo->iSize, pFontInfo->bBold, true, pFontInfo->bItalic);
                        pFontInfo = pManager->GetFontInfo(hFont);
                        aFontArray.Add(pFontInfo);
                        pTm = &pFontInfo->tm;
                        ::SelectObject(hDC, pFontInfo->hFont);
                        cyLine = MAX(cyLine, pTm->tmHeight + pTm->tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1));
                    }
                }
                break;
            case _T('x'):  // X Indent
                {
                    pstrText++;
                    while( *pstrText > _T('\0') && *pstrText <= _T(' ') ) pstrText = ::CharNext(pstrText);
                    int iWidth = (int) _tcstol(pstrText, const_cast<LPTSTR*>(&pstrText), 10);
                    pt.x += iWidth;
                    cxMaxWidth = MAX(cxMaxWidth, pt.x);
                }
                break;
            case _T('y'):  // Y Indent
                {
                    pstrText++;
                    while( *pstrText > _T('\0') && *pstrText <= _T(' ') ) pstrText = ::CharNext(pstrText);
                    cyLine = (int) _tcstol(pstrText, const_cast<LPTSTR*>(&pstrText), 10);
                }
                break;
                }
                if( pstrNextStart != NULL ) pstrText = pstrNextStart;
                else {
                    while( *pstrText != _T('\0') && *pstrText != _T('>') && *pstrText != _T('}') ) pstrText = ::CharNext(pstrText);
                    pstrText = ::CharNext(pstrText);
                }
        }
        else if( !bInRaw && ( *pstrText == _T('<') || *pstrText == _T('{') ) && pstrText[1] == _T('/') )
        {
            pstrText++;
            pstrText++;
            switch( *pstrText )
            {
            case _T('c'):
                {
                    pstrText++;
                    aColorArray.Remove(aColorArray.GetSize() - 1);
                    DWORD clrColor = dwTextColor;
                    if( aColorArray.GetSize() > 0 ) clrColor = (int)aColorArray.GetAt(aColorArray.GetSize() - 1);
                    ::SetTextColor(hDC, RGB(GetBValue(clrColor), GetGValue(clrColor), GetRValue(clrColor)));
                }
                break;
            case _T('p'):
                pstrText++;
                if( pt.x > rc.left ) bLineEnd = true;
                aPIndentArray.Remove(aPIndentArray.GetSize() - 1);
                cyLine = MAX(cyLine, pTm->tmHeight + pTm->tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1));
                break;
            case _T('s'):
                {
                    pstrText++;
                    bInSelected = !bInSelected;
                    if( bDraw && bLineDraw ) {
                        if( bInSelected ) ::SetBkMode(hDC, OPAQUE);
                        else ::SetBkMode(hDC, TRANSPARENT);
                    }
                }
                break;
            case _T('a'):
                {
                    if( iLinkIndex < nLinkRects ) {
                        if( !bLineDraw ) ::SetRect(&prcLinks[iLinkIndex], ptLinkStart.x, ptLinkStart.y, MIN(pt.x, rc.right), pt.y + pTm->tmHeight + pTm->tmExternalLeading);
                        iLinkIndex++;
                    }
                    aColorArray.Remove(aColorArray.GetSize() - 1);
                    DWORD clrColor = dwTextColor;
                    if( aColorArray.GetSize() > 0 ) clrColor = (int)aColorArray.GetAt(aColorArray.GetSize() - 1);
                    ::SetTextColor(hDC, RGB(GetBValue(clrColor), GetGValue(clrColor), GetRValue(clrColor)));
                    bInLink = false;
                }
            case _T('b'):
            case _T('f'):
            case _T('i'):
            case _T('u'):
                {
                    pstrText++;
                    aFontArray.Remove(aFontArray.GetSize() - 1);
                    TFontInfo* pFontInfo = (TFontInfo*)aFontArray.GetAt(aFontArray.GetSize() - 1);
                    if( pFontInfo == NULL ) pFontInfo = pManager->GetDefaultFontInfo();
                    if( pTm->tmItalic && pFontInfo->bItalic == false ) {
                        ABC abc;
                        ::GetCharABCWidths(hDC, _T(' '), _T(' '), &abc);
                        pt.x += abc.abcC / 2; // 简单修正一下斜体混排的问题, 正确做法应该是http://support.microsoft.com/kb/244798/en-us
                    }
                    pTm = &pFontInfo->tm;
                    ::SelectObject(hDC, pFontInfo->hFont);
                    cyLine = MAX(cyLine, pTm->tmHeight + pTm->tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1));
                }
                break;
            }
            while( *pstrText != _T('\0') && *pstrText != _T('>') && *pstrText != _T('}') ) pstrText = ::CharNext(pstrText);
            pstrText = ::CharNext(pstrText);
        }
        else if( !bInRaw &&  *pstrText == _T('<') && pstrText[2] == _T('>') && (pstrText[1] == _T('{')  || pstrText[1] == _T('}')) )
        {
            SIZE szSpace = { 0 };
            ::GetTextExtentPoint32(hDC, &pstrText[1], 1, &szSpace);
            if( bDraw && bLineDraw ) ::TextOut(hDC, pt.x, pt.y + cyLineHeight - pTm->tmHeight - pTm->tmExternalLeading, &pstrText[1], 1);
            pt.x += szSpace.cx;
            cxMaxWidth = MAX(cxMaxWidth, pt.x);
            pstrText++;pstrText++;pstrText++;
        }
        else if( !bInRaw &&  *pstrText == _T('{') && pstrText[2] == _T('}') && (pstrText[1] == _T('<')  || pstrText[1] == _T('>')) )
        {
            SIZE szSpace = { 0 };
            ::GetTextExtentPoint32(hDC, &pstrText[1], 1, &szSpace);
            if( bDraw && bLineDraw ) ::TextOut(hDC, pt.x,  pt.y + cyLineHeight - pTm->tmHeight - pTm->tmExternalLeading, &pstrText[1], 1);
            pt.x += szSpace.cx;
            cxMaxWidth = MAX(cxMaxWidth, pt.x);
            pstrText++;pstrText++;pstrText++;
        }
        else if( !bInRaw &&  *pstrText == _T(' ') )
        {
            SIZE szSpace = { 0 };
            ::GetTextExtentPoint32(hDC, _T(" "), 1, &szSpace);
            // Still need to paint the space because the font might have
            // underline formatting.
            if( bDraw && bLineDraw ) ::TextOut(hDC, pt.x,  pt.y + cyLineHeight - pTm->tmHeight - pTm->tmExternalLeading, _T(" "), 1);
            pt.x += szSpace.cx;
            cxMaxWidth = MAX(cxMaxWidth, pt.x);
            pstrText++;
        }
        else
        {
            POINT ptPos = pt;
            int cchChars = 0;
            int cchSize = 0;
            int cchLastGoodWord = 0;
            int cchLastGoodSize = 0;
            LPCTSTR p = pstrText;
            LPCTSTR pstrNext;
            SIZE szText = { 0 };
            if( !bInRaw && *p == _T('<') || *p == _T('{') ) p++, cchChars++, cchSize++;
            while( *p != _T('\0') && *p != _T('\n') ) {
                // This part makes sure that we're word-wrapping if needed or providing support
                // for DT_END_ELLIPSIS. Unfortunately the GetTextExtentPoint32() call is pretty
                // slow when repeated so often.
                // TODO: Rewrite and use GetTextExtentExPoint() instead!
                if( bInRaw ) {
                    if( ( *p == _T('<') || *p == _T('{') ) && p[1] == _T('/') 
                        && p[2] == _T('r') && ( p[3] == _T('>') || p[3] == _T('}') ) ) {
                            p += 4;
                            bInRaw = false;
                            break;
                    }
                }
                else {
                    if( *p == _T('<') || *p == _T('{') ) break;
                }
                pstrNext = ::CharNext(p);
                cchChars++;
                cchSize += (int)(pstrNext - p);
                szText.cx = cchChars * pTm->tmMaxCharWidth;
                if( pt.x + szText.cx >= rc.right ) {
                    ::GetTextExtentPoint32(hDC, pstrText, cchSize, &szText);
                }
                if( pt.x + szText.cx > rc.right ) {
                    if( pt.x + szText.cx > rc.right && pt.x != rc.left) {
                        cchChars--;
                        cchSize -= (int)(pstrNext - p);
                    }
                    if( (uStyle & DT_WORDBREAK) != 0 && cchLastGoodWord > 0 ) {
                        cchChars = cchLastGoodWord;
                        cchSize = cchLastGoodSize;                 
                    }
                    if( (uStyle & DT_END_ELLIPSIS) != 0 && cchChars > 0 ) {
                        cchChars -= 1;
                        LPCTSTR pstrPrev = ::CharPrev(pstrText, p);
                        if( cchChars > 0 ) {
                            cchChars -= 1;
                            pstrPrev = ::CharPrev(pstrText, pstrPrev);
                            cchSize -= (int)(p - pstrPrev);
                        }
                        else 
                            cchSize -= (int)(p - pstrPrev);
                        pt.x = rc.right;
                    }
                    bLineEnd = true;
                    cxMaxWidth = MAX(cxMaxWidth, pt.x);
                    break;
                }
                if (!( ( p[0] >= _T('a') && p[0] <= _T('z') ) || ( p[0] >= _T('A') && p[0] <= _T('Z') ) )) {
                    cchLastGoodWord = cchChars;
                    cchLastGoodSize = cchSize;
                }
                if( *p == _T(' ') ) {
                    cchLastGoodWord = cchChars;
                    cchLastGoodSize = cchSize;
                }
                p = ::CharNext(p);
            }
            
            ::GetTextExtentPoint32(hDC, pstrText, cchSize, &szText);
            if( bDraw && bLineDraw ) {
                ::TextOut(hDC, ptPos.x, ptPos.y + cyLineHeight - pTm->tmHeight - pTm->tmExternalLeading, pstrText, cchSize);
                if( pt.x >= rc.right && (uStyle & DT_END_ELLIPSIS) != 0 ) 
                    ::TextOut(hDC, ptPos.x + szText.cx, ptPos.y, _T("..."), 3);
            }
            pt.x += szText.cx;
            cxMaxWidth = MAX(cxMaxWidth, pt.x);
            pstrText += cchSize;
        }

        if( pt.x >= rc.right || *pstrText == _T('\n') || *pstrText == _T('\0') ) bLineEnd = true;
        if( bDraw && bLineEnd ) {
            if( !bLineDraw ) {
                aFontArray.Resize(aLineFontArray.GetSize());
                ::CopyMemory(aFontArray.GetData(), aLineFontArray.GetData(), aLineFontArray.GetSize() * sizeof(LPVOID));
                aColorArray.Resize(aLineColorArray.GetSize());
                ::CopyMemory(aColorArray.GetData(), aLineColorArray.GetData(), aLineColorArray.GetSize() * sizeof(LPVOID));
                aPIndentArray.Resize(aLinePIndentArray.GetSize());
                ::CopyMemory(aPIndentArray.GetData(), aLinePIndentArray.GetData(), aLinePIndentArray.GetSize() * sizeof(LPVOID));

                cyLineHeight = cyLine;
                pstrText = pstrLineBegin;
                bInRaw = bLineInRaw;
                bInSelected = bLineInSelected;

                DWORD clrColor = dwTextColor;
                if( aColorArray.GetSize() > 0 ) clrColor = (int)aColorArray.GetAt(aColorArray.GetSize() - 1);
                ::SetTextColor(hDC, RGB(GetBValue(clrColor), GetGValue(clrColor), GetRValue(clrColor)));
                TFontInfo* pFontInfo = (TFontInfo*)aFontArray.GetAt(aFontArray.GetSize() - 1);
                if( pFontInfo == NULL ) pFontInfo = pManager->GetDefaultFontInfo();
                pTm = &pFontInfo->tm;
                ::SelectObject(hDC, pFontInfo->hFont);
                if( bInSelected ) ::SetBkMode(hDC, OPAQUE);
            }
            else {
                aLineFontArray.Resize(aFontArray.GetSize());
                ::CopyMemory(aLineFontArray.GetData(), aFontArray.GetData(), aFontArray.GetSize() * sizeof(LPVOID));
                aLineColorArray.Resize(aColorArray.GetSize());
                ::CopyMemory(aLineColorArray.GetData(), aColorArray.GetData(), aColorArray.GetSize() * sizeof(LPVOID));
                aLinePIndentArray.Resize(aPIndentArray.GetSize());
                ::CopyMemory(aLinePIndentArray.GetData(), aPIndentArray.GetData(), aPIndentArray.GetSize() * sizeof(LPVOID));
                pstrLineBegin = pstrText;
                bLineInSelected = bInSelected;
                bLineInRaw = bInRaw;
            }
        }

        ASSERT(iLinkIndex<=nLinkRects);
    }

    nLinkRects = iLinkIndex;

    // Return size of text when requested
    if( (uStyle & DT_CALCRECT) != 0 ) {
        rc.bottom = MAX(cyMinHeight, pt.y + cyLine);
        rc.right = MIN(rc.right, cxMaxWidth);
    }

    if( bDraw ) ::SelectClipRgn(hDC, hOldRgn);
    ::DeleteObject(hOldRgn);
    ::DeleteObject(hRgn);

    ::SelectObject(hDC, hOldFont);
}

HBITMAP CRenderEngine::GenerateBitmap(CPaintManagerUI* pManager, CControlUI* pControl, RECT rc)
{
    int cx = rc.right - rc.left;
    int cy = rc.bottom - rc.top;

    HDC hPaintDC = ::CreateCompatibleDC(pManager->GetPaintDC());
    HBITMAP hPaintBitmap = ::CreateCompatibleBitmap(pManager->GetPaintDC(), rc.right, rc.bottom);
    ASSERT(hPaintDC);
    ASSERT(hPaintBitmap);
    HBITMAP hOldPaintBitmap = (HBITMAP) ::SelectObject(hPaintDC, hPaintBitmap);
    pControl->DoPaint(hPaintDC, rc);

    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = cx;
    bmi.bmiHeader.biHeight = cy;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = cx * cy * sizeof(DWORD);
    LPDWORD pDest = NULL;
    HDC hCloneDC = ::CreateCompatibleDC(pManager->GetPaintDC());
    HBITMAP hBitmap = ::CreateDIBSection(pManager->GetPaintDC(), &bmi, DIB_RGB_COLORS, (LPVOID*) &pDest, NULL, 0);
    ASSERT(hCloneDC);
    ASSERT(hBitmap);
    if( hBitmap != NULL )
    {
        HBITMAP hOldBitmap = (HBITMAP) ::SelectObject(hCloneDC, hBitmap);
        ::BitBlt(hCloneDC, 0, 0, cx, cy, hPaintDC, rc.left, rc.top, SRCCOPY);
        ::SelectObject(hCloneDC, hOldBitmap);
        ::DeleteDC(hCloneDC);  
        ::GdiFlush();
    }

    // Cleanup
    ::SelectObject(hPaintDC, hOldPaintBitmap);
    ::DeleteObject(hPaintBitmap);
    ::DeleteDC(hPaintDC);

    return hBitmap;
}

} // namespace DuiLib
