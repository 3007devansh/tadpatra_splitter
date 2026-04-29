#include "pch.h"
#include "CustomPictureCtrl.h"
#include <atlimage.h>

#pragma warning(disable : 4244)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//---------------------------------------------------------------------
CCustomPictureCtrl::CCustomPictureCtrl() : m_hBitmap(NULL), m_rectPicCtrl(0, 0, 0, 0), m_dZoomFactor(1.0)
{
    m_ScrollHelper.AttachWnd(this);
}
//---------------------------------------------------------------------
CCustomPictureCtrl::~CCustomPictureCtrl()
{
}
//---------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CCustomPictureCtrl, CWnd)
	ON_WM_PAINT()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_MOUSEACTIVATE()
    ON_WM_MOUSEWHEEL()
    ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

//---------------------------------------------------------------------
void CCustomPictureCtrl::Draw(CDC *pDC)
{
	if (!m_hBitmap)
		return;

	CRect rVirt(0, 0, m_rectPicCtrl.right, m_rectPicCtrl.bottom);
	PrepDC(pDC, rVirt, m_rectPicCtrl);

	CImage image;
	//image.Load("C:\\Users\\hardi\\Documents\\Hardik\\ZoomCtrl\\Image.bmp");
	image.Attach(m_hBitmap);

	CBitmap bmp;
	bmp.Attach(image.Detach());

	//CBitmap bmp;
	//bmp.Attach(m_hBitmap);

	CDC bmDC;
	bmDC.CreateCompatibleDC(pDC);
	CBitmap* pOldbmp = bmDC.SelectObject(&bmp);
	BITMAP  bi;
	bmp.GetBitmap(&bi);
	pDC->SetStretchBltMode(COLORONCOLOR);
	pDC->StretchBlt(0, 0, m_rectPicCtrl.Width(), m_rectPicCtrl.Height(), &bmDC, 0, 0, bi.bmWidth, bi.bmHeight, SRCCOPY);

	HPEN  hPenGreen = ::CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
	HBRUSH  hOldBrush = (HBRUSH)pDC->SelectObject(::GetStockObject(HOLLOW_BRUSH));
	HPEN hOldPen = (HPEN)pDC->SelectObject(hPenGreen);
	pDC->Rectangle(rcdraw);
	pDC->Rectangle(rcLeftTop);
	pDC->Rectangle(rcLeftBottom);
	pDC->Rectangle(rcRightTop);
	pDC->Rectangle(rcRightBottom);

	DeleteObject(bmp);
	DeleteObject(hOldPen);
	DeleteObject(hOldBrush);
	DeleteObject(hPenGreen);
	DeleteObject(&pOldbmp);

	ReleaseDC(pDC);
	ReleaseDC(&bmDC);
}
//---------------------------------------------------------------------
void CCustomPictureCtrl::OnPaint()
{
	CPaintDC dc(this);
	Draw(&dc);
}
//---------------------------------------------------------------------
void CCustomPictureCtrl::PrepDC(CDC *pDC, const CRect& rVirt, const CRect& rScreen)
{
	m_rVirt = rVirt;
	m_rScreen = rScreen;

	pDC->IntersectClipRect(&rScreen);

	pDC->SetMapMode(MM_ANISOTROPIC);
	pDC->SetWindowExt(rVirt.Width(), rVirt.Height());

	LONG wid = (LONG)(m_dZoomFactor * (double)rScreen.Width());
	LONG hgt = (LONG)(m_dZoomFactor * (double)rScreen.Height());
	pDC->SetViewportExt(wid, hgt);

	CSize scrollPos = m_ScrollHelper.GetScrollPos();
	pDC->SetViewportOrg(-scrollPos.cx, -scrollPos.cy);
}
//---------------------------------------------------------------------
void CCustomPictureCtrl::AdjustScrollbars()
{
	int xMax = (int)((double)m_rScreen.Width() * m_dZoomFactor);
	int yMax = (int)((double)m_rScreen.Height() * m_dZoomFactor);
	m_ScrollHelper.SetPageSize(m_rScreen.Width(), m_rScreen.Height());
	m_ScrollHelper.SetDisplaySize(xMax, yMax);
}
//---------------------------------------------------------------------
void CCustomPictureCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    m_ScrollHelper.OnHScroll(nSBCode, nPos, pScrollBar);
	Invalidate();
}
//---------------------------------------------------------------------
void CCustomPictureCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    m_ScrollHelper.OnVScroll(nSBCode, nPos, pScrollBar);
	Invalidate();
}
//---------------------------------------------------------------------
BOOL CCustomPictureCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    return m_ScrollHelper.OnMouseWheel(nFlags, zDelta, pt);
}
//---------------------------------------------------------------------
LRESULT CCustomPictureCtrl::WindowProc(UINT msg, WPARAM wParam, LPARAM lParam) 
{
    if (msg == WM_NCHITTEST || msg == WM_NCLBUTTONDOWN || msg == WM_NCLBUTTONDBLCLK) 
        return ::DefWindowProc(m_hWnd, msg, wParam, lParam); 
    return CWnd::WindowProc(msg, wParam, lParam); 
} 
//---------------------------------------------------------------------
int CCustomPictureCtrl::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	int status = CWnd::OnMouseActivate(pDesktopWnd, nHitTest, message);
    SetFocus();
	return status;
}
//---------------------------------------------------------------------
BOOL CCustomPictureCtrl::OnEraseBkgnd(CDC* pDC)
{
    return TRUE;
}
//---------------------------------------------------------------------
void CCustomPictureCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (((point.x + xShift) / m_dZoomFactor) > rcdraw.left + 10 && ((point.x + xShift) / m_dZoomFactor) < rcdraw.right - 10 && ((point.y + yShift) / m_dZoomFactor) > rcdraw.top + 10 && ((point.y + yShift) / m_dZoomFactor) < rcdraw.bottom - 10)
	{
		bclicked = TRUE;
		oPointMove.x = (point.x + xShift) / m_dZoomFactor;
		oPointMove.y = (point.y + yShift) / m_dZoomFactor;
	}

	if (((point.x + xShift) / m_dZoomFactor) > rcLeftTop.left && ((point.x + xShift) / m_dZoomFactor) < rcLeftTop.right && ((point.y + yShift) / m_dZoomFactor) > rcLeftTop.top && ((point.y + yShift) / m_dZoomFactor) < rcLeftTop.bottom)
	{
		bclickedLT = TRUE;
		oPointMove.x = (point.x + xShift) / m_dZoomFactor;
		oPointMove.y = (point.y + yShift) / m_dZoomFactor;
	}

	if (((point.x + xShift) / m_dZoomFactor) > rcLeftBottom.left && ((point.x + xShift) / m_dZoomFactor) < rcLeftBottom.right && ((point.y + yShift) / m_dZoomFactor) > rcLeftBottom.top && ((point.y + yShift) / m_dZoomFactor) < rcLeftBottom.bottom)
	{
		bclickedLB = TRUE;
		oPointMove.x = (point.x + xShift) / m_dZoomFactor;
		oPointMove.y = (point.y + yShift) / m_dZoomFactor;
	}

	if (((point.x + xShift) / m_dZoomFactor) > rcRightTop.left && ((point.x + xShift) / m_dZoomFactor) < rcRightTop.right && ((point.y + yShift) / m_dZoomFactor) > rcRightTop.top && ((point.y + yShift) / m_dZoomFactor) < rcRightTop.bottom)
	{
		bclickedRT = TRUE;
		oPointMove.x = (point.x + xShift) / m_dZoomFactor;
		oPointMove.y = (point.y + yShift) / m_dZoomFactor;
	}

	if (((point.x + xShift) / m_dZoomFactor) > rcRightBottom.left && ((point.x + xShift) / m_dZoomFactor) < rcRightBottom.right && ((point.y + yShift) / m_dZoomFactor) > rcRightBottom.top && ((point.y + yShift) / m_dZoomFactor) < rcRightBottom.bottom)
	{
		bclickedRB = TRUE;
		oPointMove.x = (point.x + xShift) / m_dZoomFactor;
		oPointMove.y = (point.y + yShift) / m_dZoomFactor;
	}
}
//---------------------------------------------------------------------
void CCustomPictureCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	bclicked = FALSE;
	bclickedLT = FALSE;
	bclickedLB = FALSE;
	bclickedRT = FALSE;
	bclickedRB = FALSE;
}
//---------------------------------------------------------------------
void CCustomPictureCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	if (bclicked)
	{
		int iMoveDiffX = ((point.x + xShift) / m_dZoomFactor) - oPointMove.x;
		int iMoveDiffY = ((point.y + yShift) / m_dZoomFactor) - oPointMove.y;
		oPointMove.x = (point.x + xShift) / m_dZoomFactor;
		oPointMove.y = (point.y + yShift) / m_dZoomFactor;

		rcdraw.left += iMoveDiffX;
		rcdraw.top += iMoveDiffY;
		rcdraw.right += iMoveDiffX;
		rcdraw.bottom += iMoveDiffY;

		rcLeftTop.left += iMoveDiffX;
		rcLeftTop.top += iMoveDiffY;
		rcLeftTop.right += iMoveDiffX;
		rcLeftTop.bottom += iMoveDiffY;

		rcLeftBottom.left += iMoveDiffX;
		rcLeftBottom.top += iMoveDiffY;
		rcLeftBottom.right += iMoveDiffX;
		rcLeftBottom.bottom += iMoveDiffY;

		rcRightTop.left += iMoveDiffX;
		rcRightTop.top += iMoveDiffY;
		rcRightTop.right += iMoveDiffX;
		rcRightTop.bottom += iMoveDiffY;

		rcRightBottom.left += iMoveDiffX;
		rcRightBottom.top += iMoveDiffY;
		rcRightBottom.right += iMoveDiffX;
		rcRightBottom.bottom += iMoveDiffY;
		CClientDC dc(this);
		Draw(&dc);
	}
	else if (bclickedLT)
	{
		int iMoveDiffX = ((point.x + xShift) / m_dZoomFactor) - oPointMove.x;
		int iMoveDiffY = ((point.y + yShift) / m_dZoomFactor) - oPointMove.y;
		oPointMove.x = (point.x + xShift) / m_dZoomFactor;
		oPointMove.y = (point.y + yShift) / m_dZoomFactor;

		rcdraw.top += iMoveDiffY;
		rcdraw.left += iMoveDiffX;

		rcLeftTop.left += iMoveDiffX;
		rcLeftTop.top += iMoveDiffY;
		rcLeftTop.right += iMoveDiffX;
		rcLeftTop.bottom += iMoveDiffY;

		rcLeftBottom.left += iMoveDiffX;
		rcLeftBottom.right += iMoveDiffX;

		rcRightTop.top += iMoveDiffY;
		rcRightTop.bottom += iMoveDiffY;

		CClientDC dc(this);
		Draw(&dc);
	}
	else if (bclickedLB)
	{
		int iMoveDiffX = ((point.x + xShift) / m_dZoomFactor) - oPointMove.x;
		int iMoveDiffY = ((point.y + yShift) / m_dZoomFactor) - oPointMove.y;
		oPointMove.x = (point.x + xShift) / m_dZoomFactor;
		oPointMove.y = (point.y + yShift) / m_dZoomFactor;

		rcdraw.bottom += iMoveDiffY;
		rcdraw.left += iMoveDiffX;

		rcLeftBottom.left += iMoveDiffX;
		rcLeftBottom.top += iMoveDiffY;
		rcLeftBottom.right += iMoveDiffX;
		rcLeftBottom.bottom += iMoveDiffY;

		rcRightBottom.top += iMoveDiffY;
		rcRightBottom.bottom += iMoveDiffY;

		rcLeftTop.left += iMoveDiffX;
		rcLeftTop.right += iMoveDiffX;

		CClientDC dc(this);
		Draw(&dc);
	}
	else if (bclickedRT)
	{
		int iMoveDiffX = ((point.x + xShift) / m_dZoomFactor) - oPointMove.x;
		int iMoveDiffY = ((point.y + yShift) / m_dZoomFactor) - oPointMove.y;
		oPointMove.x = (point.x + xShift) / m_dZoomFactor;
		oPointMove.y = (point.y + yShift) / m_dZoomFactor;

		rcdraw.top += iMoveDiffY;
		rcdraw.right += iMoveDiffX;

		rcRightTop.left += iMoveDiffX;
		rcRightTop.top += iMoveDiffY;
		rcRightTop.right += iMoveDiffX;
		rcRightTop.bottom += iMoveDiffY;

		rcLeftTop.top += iMoveDiffY;
		rcLeftTop.bottom += iMoveDiffY;

		rcRightBottom.left += iMoveDiffX;
		rcRightBottom.right += iMoveDiffX;

		CClientDC dc(this);
		Draw(&dc);
	}
	else if (bclickedRB)
	{
	int iMoveDiffX = ((point.x + xShift) / m_dZoomFactor) - oPointMove.x;
	int iMoveDiffY = ((point.y + yShift) / m_dZoomFactor) - oPointMove.y;
	oPointMove.x = (point.x + xShift) / m_dZoomFactor;
	oPointMove.y = (point.y + yShift) / m_dZoomFactor;

	rcdraw.bottom += iMoveDiffY;
	rcdraw.right += iMoveDiffX;

	rcRightBottom.left += iMoveDiffX;
	rcRightBottom.top += iMoveDiffY;
	rcRightBottom.right += iMoveDiffX;
	rcRightBottom.bottom += iMoveDiffY;

	rcLeftBottom.top += iMoveDiffY;
	rcLeftBottom.bottom += iMoveDiffY;

	rcRightTop.left += iMoveDiffX;
	rcRightTop.right += iMoveDiffX;

	CClientDC dc(this);
	Draw(&dc);
	}
}
//---------------------------------------------------------------------