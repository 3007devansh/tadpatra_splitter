#pragma once

#include "CustomPictureCtrl.h"

#include <opencv2/core/core.hpp>

using namespace cv;

// CImageAngleCorrectionDlg dialog

class CImageAngleCorrectionDlg : public CDialog
{
	DECLARE_DYNAMIC(CImageAngleCorrectionDlg)

public:
	CImageAngleCorrectionDlg(CWnd* pParent = nullptr);   // Standard constructor
	virtual ~CImageAngleCorrectionDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_IMAGE_ANGLE_CORRECTION };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

private:
	CString m_strFolderName;
	//CStatic* m_pPictureBoxCtrl;
	CCustomPictureCtrl m_CustomPictureCtrl;
	double m_dZoomFactor;
	CString m_strCurrFile;
	CStringArray m_arrImageFiles;
	//Mat m_objImage;
	int m_nCurrentImageIndex;
	int m_nAntiClockwiseRotationAngle;
	int m_nClockwiseRotationAngle;
	CSpinButtonCtrl m_AntiClockwiseRotationAngleSpinCtrl;
	CSpinButtonCtrl m_ClockwiseRotationAngleSpinCtrl;
	CStatic m_ctrlCurrFileName;
	BOOL m_bOvewriteRotatedFile;
	HICON m_hIcon;

public:
	BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnFolderBrowseButtonClicked();
	afx_msg void OnPreviousImageButtonClicked();
	afx_msg void OnNextImageButtonClicked();

	void PreviewImage(const CString& strImageFilePath);
	afx_msg void OnOpenInputFolderButtonClicked();
	afx_msg void OnRotateAntiClockwiseButtonClicked();
	afx_msg void OnRotateClockwiseButtonClicked();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};
