//---------------------------------------------------------------------
#ifndef _H_ZoomCtrl
#define _H_ZoomCtrl

#include "scrollhelper.h"

//---------------------------------------------------------------------
class CCustomPictureCtrl : public CWnd
{
public:
						CCustomPictureCtrl();
						~CCustomPictureCtrl();

	double				GetZoomFactor() const		{ return m_dZoomFactor; }
	void				SetZoomFactor(double d)		{ m_dZoomFactor = d; }
	void				AdjustScrollbars();
	void				SetPictureCtrlClientRect(const CRect& rectClientRect)
	{
		m_rectPicCtrl = rectClientRect;
	}
	void				SetBitmap(const HBITMAP& hBitmap)
	{
		m_hBitmap = hBitmap;
	}

	//{{AFX_MSG(CDendroCtl)
	afx_msg void		OnPaint();
	afx_msg void		OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void		OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL		OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	LRESULT				WindowProc(UINT mess, WPARAM wParam, LPARAM lParam);
	afx_msg int			OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg BOOL		OnEraseBkgnd(CDC* pDC);
	afx_msg void		OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void		OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void		OnLButtonUp(UINT nFlags, CPoint point);

protected:
	virtual void		Draw(CDC *pDC);
	void				PrepDC(CDC *pDC, const CRect& rVirt, const CRect& rScreen);

	CRect				m_rectPicCtrl;
	CRect				m_rVirt, m_rScreen;	
	CRect rcdraw = CRect(50, 50, 100, 100);
	CRect rcRightTop = CRect(90, 50, 100, 60);
	CRect rcRightBottom = CRect(90, 90, 100, 100);
	CRect rcLeftTop = CRect(50, 50, 60, 60);
	CRect rcLeftBottom = CRect(50, 90, 60, 100);
	CScrollHelper		m_ScrollHelper;
	double				m_dZoomFactor;
	int xShift = 0, yShift = 0;
	BOOL bclicked = FALSE;
	BOOL bclickedLT = FALSE;
	BOOL bclickedLB = FALSE;
	BOOL bclickedRT = FALSE;
	BOOL bclickedRB = FALSE;
	POINT oPointMove;
	HBITMAP m_hBitmap;
};
//---------------------------------------------------------------------
#endif // _H_ZoomCtrl
