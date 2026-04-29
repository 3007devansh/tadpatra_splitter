
// TadpatraSplitterDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "TadpatraSplitter.h"
#include "TadpatraSplitterDlg.h"
#include "afxdialogex.h"
#include <iostream>
#include "Logger.h"
#include "CommonUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CTadpatraSplitterDlg dialog



CTadpatraSplitterDlg::CTadpatraSplitterDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DIALOG_TADPATRASPLITTER, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CTadpatraSplitterDlg::~CTadpatraSplitterDlg()
{
	if (m_hIcon)
	{
		DestroyIcon(m_hIcon);
		m_hIcon = NULL;
	}
}

void CTadpatraSplitterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTadpatraSplitterDlg)
	DDX_Control(pDX, IDC_TAB, m_cTab);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTadpatraSplitterDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SHOWWINDOW()
	ON_WM_SYSCOMMAND()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, OnSelchangeTab)
END_MESSAGE_MAP()


// CTadpatraSplitterDlg message handlers

BOOL CTadpatraSplitterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	CRect tabRect;
	m_cTab.GetWindowRect(tabRect);

	// Set the size and location of the child windows based on the tab control
	m_rSettingsRect.left = 13;
	m_rSettingsRect.top = 44;
	m_rSettingsRect.right = tabRect.Width() - 7;
	m_rSettingsRect.bottom = tabRect.Height() - 38;

	// Create the child windows for the main window class
	m_objBatchProcessingDlg.Create(IDD_DIALOG_BATCH_PROCESSING, this);
	m_objManualProcessingDlg.Create(IDD_DIALOG_MANUAL_PROCESSING, this);
	m_objImageAngleCorrectionDlgDlg.Create(IDD_DIALOG_IMAGE_ANGLE_CORRECTION, this);

	// This is redundant with the default value, considering what OnShowWindow does
	ShowWindowNumber(0);

	// Set the titles for each tab
	TCITEM tabItem;
	tabItem.mask = TCIF_TEXT;

	tabItem.pszText = _T("  Batch Processing   ");
	m_cTab.InsertItem(0, &tabItem);

	tabItem.pszText = _T("  Manual Processing   ");
	m_cTab.InsertItem(1, &tabItem);

	tabItem.pszText = _T("  Image Angle Correction    ");
	m_cTab.InsertItem(2, &tabItem);

	CString strWindowTitle;
	GetWindowText(strWindowTitle);
	strWindowTitle += " - ";

	string strProductVersion;
	strProductVersion = CCommonUtil::GetAppVersion();
	strWindowTitle += strProductVersion.c_str();
	SetWindowText(strWindowTitle);

	UpdateData(FALSE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTadpatraSplitterDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTadpatraSplitterDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTadpatraSplitterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CTadpatraSplitterDlg::ShowWindowNumber(int number)
{
	int windowCount = 3;

	// Validate the parameter
	if ((number >= 0) && (number < windowCount))
	{
		// Create and assign pointers to each window
		CDialog *pDlgPointers[3];

		pDlgPointers[0] = &m_objBatchProcessingDlg;
		pDlgPointers[1] = &m_objManualProcessingDlg;
		pDlgPointers[2] = &m_objImageAngleCorrectionDlgDlg;

		// Hide every window except for the chosen one
		for (int count = 0; count < windowCount; count++)
		{
			if (count != number)
			{
				pDlgPointers[count]->ShowWindow(SW_HIDE);
			}
			else if (count == number)
			{
				pDlgPointers[count]->ShowWindow(SW_SHOW);

				// Show the chosen window and set it's location
				pDlgPointers[count]->SetWindowPos(&wndTop, m_rSettingsRect.left,
					m_rSettingsRect.top, m_rSettingsRect.right,
					m_rSettingsRect.bottom, SWP_SHOWWINDOW);

				m_cTab.SetCurSel(count);
			}
		}
	}
}

void CTadpatraSplitterDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	// When the dialog is shown, display the first window
	if (bShow)
	{
		ShowWindowNumber(0);
	}
}

void CTadpatraSplitterDlg::OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult)
{
	// Get the number of the currently selected tab, and show it
	ShowWindowNumber(m_cTab.GetCurFocus());

	// Do something with the "formal parameters" so the compiler is happy in warning level 4
	pNMHDR = NULL;
	pResult = NULL;
}

void CTadpatraSplitterDlg::OnClose()
{
}
