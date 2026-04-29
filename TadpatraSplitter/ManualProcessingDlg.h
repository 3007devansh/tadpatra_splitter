#pragma once


// CManualProcessingDlg dialog

class CManualProcessingDlg : public CDialog
{
public:
	CManualProcessingDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CManualProcessingDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_MANUAL_PROCESSING };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CString m_strFileNameFrontSide;
	CString m_strFileNameBackSide;
	CStatic* m_pPictureBoxFrontSide;
	CStatic* m_pPictureBoxBackSide;
	CString m_strOutputFolderName;
	CStatic m_ctrlStatusText;
	CString m_strManuScriptNumber;
	int m_nBorderPixelMargin;
	CSpinButtonCtrl m_SpinCtrl;

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnFileNameFrontSideButtonClicked();
	afx_msg void OnFileNameBackSideButtonClicked();
	void ShowImagePreview(const CString& strImageFile, CStatic* pPictureBoxCtrl);
	afx_msg void OnOutputFolderBrowseButtonClicked();
	afx_msg void OnOpenOutputFolderButtonClicked();
	afx_msg void OnStartSplittingButtonClicked();

private:

};
