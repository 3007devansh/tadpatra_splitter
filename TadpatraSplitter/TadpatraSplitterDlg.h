
// TadpatraSplitterDlg.h : header file
//

#pragma once

#include "BatchProcessingDlg.h"
#include "ManualProcessingDlg.h"
#include "ImageAngleCorrectionDlg.h"

// CTadpatraSplitterDlg dialog
class CTadpatraSplitterDlg : public CDialog
{
// Construction
public:
	CTadpatraSplitterDlg(CWnd* pParent = nullptr);	// standard constructor
	~CTadpatraSplitterDlg();

	CBatchProcessingDlg m_objBatchProcessingDlg;
	CManualProcessingDlg m_objManualProcessingDlg;
	CImageAngleCorrectionDlg m_objImageAngleCorrectionDlgDlg;

	void ShowWindowNumber(int number);

// Dialog Data
	enum { IDD = IDD_DIALOG_TADPATRASPLITTER };
	CTabCtrl	m_cTab;

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

private:
	CRect m_rSettingsRect;

public:
	afx_msg void OnClose();
};
