#pragma once


#include <iostream>
#include <filesystem>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;
using namespace std::filesystem;

// CBatchProcessingDlg dialog

class CBatchProcessingDlg : public CDialog
{
public:
	CBatchProcessingDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CBatchProcessingDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_BATCH_PROCESSING };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(CBatchProcessingDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnInputFolderBrowseButtonClicked();
	afx_msg void OnOpenInputFolderButtonClicked();

	afx_msg void OnOutputFolderBrowseButtonClicked();
	afx_msg void OnOpenOutputFolderButtonClicked();
	afx_msg void OnStartSplittingButtonClicked();
	afx_msg void OnStopSplittingButtonClicked();
	afx_msg void OnCleanOutputFolderButtonClicked();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CString m_strInputFolderName;
	CString m_strOutputFolderName;
	CComboBox m_cmbImageType;
	int m_nBorderPixelMargin;
	CSpinButtonCtrl m_SpinCtrl;
	CStatic* m_pPictureBoxCtrl;
	BOOL m_bPreviewImage;
	BOOL m_bFirstFolioIsTitleImage;
	CStatic m_ctrlStatusText;

	BOOL m_bStopRequested;

	typedef struct _RECT_COORDINATES
	{
		Rect m_objectRect;
		int m_nObjectIndex;
	} RECT_COORDINATES;

	//CWinThread* m_pSplittingThreadHandle;
	HANDLE m_hSplittingThreadHandle;
	static DWORD WINAPI ImageSplittingThreadStaticFunc(LPVOID lpParam);
	DWORD ImageSplittingThreadFunc();

public:


	CString GetInputFolderName() const
	{
		return m_strInputFolderName;
	}

	CString GetOutputFolderName() const
	{
		return m_strOutputFolderName;
	}

	inline CComboBox* GetImageTypeComboBoxlHandle()
	{
		return &m_cmbImageType;
	}

	inline int GetBorderPixelMargin() const
	{
		return m_nBorderPixelMargin;
	}

	inline CStatic* GetPictureBoxHandle()
	{
		return m_pPictureBoxCtrl;
	}

	inline BOOL IsStopRequested() const
	{
		return m_bStopRequested;
	}

	static void DeleteDirectoryContents(const std::filesystem::path& directory);
	void WaitForImageSplittingThread();
	static INT CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData);
};
