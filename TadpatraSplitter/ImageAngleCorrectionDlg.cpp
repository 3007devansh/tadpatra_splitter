// ImageAngleCorrectionDlg.cpp : implementation file
//

#include "pch.h"
#include "TadpatraSplitter.h"
#include "ImageAngleCorrectionDlg.h"
#include "afxdialogex.h"
#include "Logger.h"
#include "CommonUtil.h"

#include <filesystem>
#include <opencv2/core/core.hpp>
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;
using namespace std::filesystem;

#pragma warning (disable : 4244)

// CImageAngleCorrectionDlg dialog

IMPLEMENT_DYNAMIC(CImageAngleCorrectionDlg, CDialog)

CImageAngleCorrectionDlg::CImageAngleCorrectionDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DIALOG_IMAGE_ANGLE_CORRECTION, pParent)
#ifdef _DEBUG
	, m_strFolderName(_T("C:\\Users\\hardi\\Documents\\Hardik\\TP_200201_Splitted"))
#else
	, m_strFolderName(_T(""))
#endif
	//, m_pPictureBoxCtrl(NULL)
	, m_nCurrentImageIndex(0)
	, m_nAntiClockwiseRotationAngle(1)
	, m_nClockwiseRotationAngle(1)
	, m_bOvewriteRotatedFile(TRUE)
	, m_dZoomFactor(1.0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CImageAngleCorrectionDlg::~CImageAngleCorrectionDlg()
{
}

void CImageAngleCorrectionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_FOLDER, m_strFolderName);
	DDX_Text(pDX, IDC_EDIT_ROTATE_ANTI_CLOCKWISE, m_nAntiClockwiseRotationAngle);
	DDX_Text(pDX, IDC_EDIT_ROTATE_CLOCKWISE, m_nClockwiseRotationAngle);
	DDX_Control(pDX, IDC_SPIN_ROTATE_ANTICLOCKWISE, m_AntiClockwiseRotationAngleSpinCtrl);
	DDX_Control(pDX, IDC_SPIN_ROTATE_CLOCKWISE, m_ClockwiseRotationAngleSpinCtrl);
	//DDX_Control(pDX, IDC_STATIC_STATUS_CURR_FILE, m_ctrlCurrFileName);
	DDX_Text(pDX, IDC_EDIT_STATUS_CURR_FILE, m_strCurrFile);
	DDX_Check(pDX, IDC_CHECK_OVERWRITE_ROTATED_FILE, m_bOvewriteRotatedFile);
	DDX_Control(pDX, IDC_STATIC_PICTURE_BOX, m_CustomPictureCtrl);
}

BEGIN_MESSAGE_MAP(CImageAngleCorrectionDlg, CDialog)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BUTTON_FOLDER_BROWSE_IMAGE_ROTATION, &CImageAngleCorrectionDlg::OnFolderBrowseButtonClicked)
	ON_BN_CLICKED(IDC_BUTTON_PREVIOUS_IMAGE, &CImageAngleCorrectionDlg::OnPreviousImageButtonClicked)
	ON_BN_CLICKED(IDC_BUTTON_NEXT_IMAGE, &CImageAngleCorrectionDlg::OnNextImageButtonClicked)
	ON_BN_CLICKED(IDC_BUTTON_OPEN_INPUT_FOLDER, &CImageAngleCorrectionDlg::OnOpenInputFolderButtonClicked)
	ON_BN_CLICKED(IDC_BUTTON_ROTATE_ANTICLOCKWISE, &CImageAngleCorrectionDlg::OnRotateAntiClockwiseButtonClicked)
	ON_BN_CLICKED(IDC_BUTTON_ROTATE_CLOCKWISE, &CImageAngleCorrectionDlg::OnRotateClockwiseButtonClicked)
	ON_WM_SHOWWINDOW()
	ON_WM_SETFOCUS()
	ON_WM_ACTIVATE()
END_MESSAGE_MAP()

BOOL CImageAngleCorrectionDlg::OnEraseBkgnd(CDC* pDC)
{
	//if ((0 != m_arrImageFiles.GetSize()) && 
	//	(m_nCurrentImageIndex >= 0) &&
	//	(m_nCurrentImageIndex <= (m_arrImageFiles.GetSize() - 1))
	//	)
	//{
	//	PreviewImage(m_arrImageFiles[m_nCurrentImageIndex]);
	//}
	return TRUE;
}

// CImageAngleCorrectionDlg message handlers
BOOL CImageAngleCorrectionDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	//m_pPictureBoxCtrl = (CStatic*)GetDlgItem(IDC_STATIC_PICTURE_BOX);
	CButton* pPreviousImageButton = (CButton*)GetDlgItem(IDC_BUTTON_PREVIOUS_IMAGE);
	if (pPreviousImageButton)
	{
		pPreviousImageButton->ModifyStyle(0, BS_BITMAP);
		HBITMAP hIcon = (HBITMAP)LoadImage(
			AfxGetApp()->m_hInstance,
			MAKEINTRESOURCE(IDB_BITMAP_PREVIOUS_IMAGE),
			IMAGE_BITMAP,
			0, 0, // use actual size
			LR_DEFAULTCOLOR
		);
		if (hIcon)
			pPreviousImageButton->SetBitmap(hIcon);
	}

	CButton* pNextImageButton = (CButton*)GetDlgItem(IDC_BUTTON_NEXT_IMAGE);
	if (pNextImageButton)
	{
		pNextImageButton->ModifyStyle(0, BS_BITMAP);
		HBITMAP hIcon = (HBITMAP)LoadImage(
			AfxGetApp()->m_hInstance,
			MAKEINTRESOURCE(IDB_BITMAP_NEXT_IMAGE),
			IMAGE_BITMAP,
			0, 0, // use actual size
			LR_DEFAULTCOLOR
		);
		if (hIcon)
			pNextImageButton->SetBitmap(hIcon);
	}

	CButton* pRotateAntiClockwise = (CButton*)GetDlgItem(IDC_BUTTON_ROTATE_ANTICLOCKWISE);
	if (pRotateAntiClockwise)
	{
		pRotateAntiClockwise->ModifyStyle(0, BS_BITMAP);
		HBITMAP hIcon = (HBITMAP)LoadImage(
			AfxGetApp()->m_hInstance,
			MAKEINTRESOURCE(IDB_BITMAP_ROTATE_ANTICLOCKWISE),
			IMAGE_BITMAP,
			0, 0, // use actual size
			LR_DEFAULTCOLOR
		);
		if (hIcon)
			pRotateAntiClockwise->SetBitmap(hIcon);
	}

	CButton* pRotateClockwise = (CButton*)GetDlgItem(IDC_BUTTON_ROTATE_CLOCKWISE);
	if (pRotateClockwise)
	{
		pRotateClockwise->ModifyStyle(0, BS_BITMAP);
		HBITMAP hIcon = (HBITMAP)LoadImage(
			AfxGetApp()->m_hInstance,
			MAKEINTRESOURCE(IDB_BITMAP_ROTATE_CLOCKWISE),
			IMAGE_BITMAP,
			0, 0, // use actual size
			LR_DEFAULTCOLOR
		);
		if (hIcon)
			pRotateClockwise->SetBitmap(hIcon);
	}

	m_AntiClockwiseRotationAngleSpinCtrl.SetRange(1, 45);
	m_ClockwiseRotationAngleSpinCtrl.SetRange(1, 45);

	UpdateData(FALSE);

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	CRect rectPictureCtrl = { 0, 0, 0, 0 };
	GetDlgItem(IDC_STATIC_PICTURE_BOX)->GetClientRect(&rectPictureCtrl);
	m_CustomPictureCtrl.SetPictureCtrlClientRect(rectPictureCtrl);

	//if (!m_strFolderName.IsEmpty())
	//{
	//	std::filesystem::directory_entry fsInputFolderName(m_strFolderName.GetString());
	//	if (fsInputFolderName.exists())
	//	{
	//		OnPreviousImageButtonClicked();
	//	}
	//}

	GetDlgItem(IDC_BUTTON_NEXT_IMAGE)->SetFocus();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

INT CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData)
{
	if (uMsg == BFFM_INITIALIZED) SendMessage(hwnd, BFFM_SETSELECTION, TRUE, pData);
	return 0;
}

void CImageAngleCorrectionDlg::OnFolderBrowseButtonClicked()
{
	UpdateData(TRUE);

	TCHAR szDir[MAX_PATH];
	BROWSEINFO bInfo;
	ZeroMemory(&bInfo, sizeof(BROWSEINFO));
	bInfo.hwndOwner = this->m_hWnd;
	bInfo.pidlRoot = NULL;
	bInfo.pszDisplayName = szDir; // Address of a buffer to receive the display name of the folder selected by the user
	bInfo.lpszTitle = "Please select image folder"; // Title of the dialog
	bInfo.ulFlags = 0;
	bInfo.lpfn = BrowseCallbackProc;

#ifdef _DEBUG
	bInfo.lParam = (LPARAM)"D:\\Hardik_Shah\\TP_200201_Splitted";
#else
	bInfo.lParam = 0;
#endif

	if (!m_strFolderName.IsEmpty())
		bInfo.lParam = (LPARAM)m_strFolderName.GetString();

	bInfo.iImage = -1;

	LPITEMIDLIST lpItem = SHBrowseForFolder(&bInfo);
	if (lpItem != NULL)
	{
		SHGetPathFromIDList(lpItem, szDir);
		m_strFolderName = szDir;
		UpdateData(FALSE);

		CCommonUtil::GetAllFileNames(m_strFolderName, m_arrImageFiles);

		//m_ctrlCurrFileName.SetWindowText(CA2CT(""));
		m_strCurrFile = "";
		UpdateData(FALSE);

		m_nCurrentImageIndex = 0;

		if (0 != m_arrImageFiles.GetSize())
			PreviewImage(m_arrImageFiles[m_nCurrentImageIndex]);
		GetDlgItem(IDC_BUTTON_NEXT_IMAGE)->SetFocus();
	}
}

void CImageAngleCorrectionDlg::OnPreviousImageButtonClicked()
{
	UpdateData(TRUE);

	if (0 == m_arrImageFiles.GetSize())
		CCommonUtil::GetAllFileNames(m_strFolderName, m_arrImageFiles);

	if (0 < m_arrImageFiles.GetSize())
	{
		if (m_nCurrentImageIndex > 0)
		{
			m_nCurrentImageIndex--;

			string strCurrFile = "Current File: ";
			strCurrFile += m_arrImageFiles[m_nCurrentImageIndex].GetString();
			//m_ctrlCurrFileName.SetWindowText(CA2CT(strCurrFile.c_str()));
			m_strCurrFile = strCurrFile.c_str();
			PreviewImage(m_arrImageFiles[m_nCurrentImageIndex]);
		}
		else
		{
			string strCurrFile = "Current File: ";
			strCurrFile += m_arrImageFiles[m_nCurrentImageIndex].GetString();
			//m_ctrlCurrFileName.SetWindowText(CA2CT(strCurrFile.c_str()));
			m_strCurrFile = strCurrFile.c_str();

			PreviewImage(m_arrImageFiles[m_nCurrentImageIndex]);
		}
		UpdateData(FALSE);
	}
}

void CImageAngleCorrectionDlg::OnNextImageButtonClicked()
{
	UpdateData(TRUE);

	if (0 == m_arrImageFiles.GetSize())
		CCommonUtil::GetAllFileNames(m_strFolderName, m_arrImageFiles);

	if ((0 < m_arrImageFiles.GetSize()) && (m_nCurrentImageIndex < (m_arrImageFiles.GetSize() - 1)))
	{
		m_nCurrentImageIndex++;

		string strCurrFile = "Current File: ";
		strCurrFile += m_arrImageFiles[m_nCurrentImageIndex].GetString();
		//m_ctrlCurrFileName.SetWindowText(CA2CT(strCurrFile.c_str()));
		m_strCurrFile = strCurrFile.c_str();
		UpdateData(FALSE);

		PreviewImage(m_arrImageFiles[m_nCurrentImageIndex]);
	}
}

void CImageAngleCorrectionDlg::PreviewImage(const CString& strImageFilePath)
{
	std::filesystem::directory_entry fsImageFilePath(strImageFilePath.GetString());

	if (fsImageFilePath.exists())
	{
		Mat mainImg = CCommonUtil::ReadImageIntoMatObj(strImageFilePath.GetString());
		if (!mainImg.empty())
		{
			//m_objImage = mainImg.clone();

			Mat tempDrawRect;
			cvtColor(mainImg, tempDrawRect, COLOR_BGR2BGRA);

			RECT rect;
			m_CustomPictureCtrl.GetWindowRect(&rect);

			int nWidth = rect.right - rect.left;
			//int nHeightLimit = 0;
			//nHeightLimit = rect.bottom - rect.top;
			float ratio = (float)tempDrawRect.rows / (float)tempDrawRect.cols;
			int nHeiht = ratio * nWidth;
			//if (nHeightLimit < nHeiht)
			//{
			//    nHeiht = nHeightLimit;
			//}

			resize(tempDrawRect, tempDrawRect, cv::Size(nWidth, nHeiht));
			HBITMAP hBitmap = CreateBitmap(tempDrawRect.cols, tempDrawRect.rows, 1, 32, tempDrawRect.data);
			if (hBitmap)
			{
				m_CustomPictureCtrl.SetBitmap(hBitmap);
			}

			//CCommonUtil::LoadImageInPictureBox(m_pPictureBoxCtrl, mainImg);
			//CCommonUtil::LoadImageInPictureBox(&m_CustomPictureCtrl, mainImg);

			UpdateWindow();
			m_CustomPictureCtrl.SendMessage(WM_PAINT);
		}
	}
}

void CImageAngleCorrectionDlg::OnOpenInputFolderButtonClicked()
{
	UpdateData(TRUE);
	m_strFolderName.Trim();
	if (!m_strFolderName.IsEmpty())
		ShellExecuteA(NULL, "open", m_strFolderName, NULL, NULL, SW_SHOWDEFAULT);
}

void CImageAngleCorrectionDlg::OnRotateAntiClockwiseButtonClicked()
{
	UpdateData(TRUE);

	if (0 >= m_arrImageFiles.GetSize())
		return;

	CWaitCursor cWaitCursor;

	std::filesystem::directory_entry fsImageFolderPath(m_arrImageFiles[m_nCurrentImageIndex].GetString());
	double scale = 1.0;

	if (fsImageFolderPath.exists())
	{
		Mat mainImg = CCommonUtil::ReadImageIntoMatObj(m_arrImageFiles[m_nCurrentImageIndex].GetString());
		if (!mainImg.empty())
		{
			double dAngle = (double)m_nAntiClockwiseRotationAngle;

			Point2f center(mainImg.cols / 2.0f, mainImg.rows  / 2.0);

			// using getRotationMatrix2D() to get the rotation matrix
			Mat rotation_matix = getRotationMatrix2D(center, dAngle, scale);

			// Determine bounding rectangle, center not relevant
			cv::Rect2f bbox = cv::RotatedRect(cv::Point2f(), mainImg.size(), dAngle).boundingRect2f();

			// Adjust transformation matrix
			rotation_matix.at<double>(0, 2) += bbox.width / 2.0 - mainImg.cols / 2.0;
			rotation_matix.at<double>(1, 2) += bbox.height / 2.0 - mainImg.rows / 2.0;

			// we will save the resulting image in rotated_image matrix
			Mat rotated_image;

			// rotate the image using warpAffine
			warpAffine(mainImg, rotated_image, rotation_matix, bbox.size(), cv::INTER_LINEAR, cv::BORDER_REPLICATE);

			if (!rotated_image.empty())
			{
				//CCommonUtil::LoadImageInPictureBox(m_pPictureBoxCtrl, rotated_image);

				if (m_bOvewriteRotatedFile)
				{
					cv::imwrite(m_arrImageFiles[m_nCurrentImageIndex].GetString(), rotated_image);
				}
				else
				{
					if (AfxMessageBox("Are you sure you want to ovewrite the rotated file over original file?", MB_YESNO) == IDYES)
					{
						cv::imwrite(m_arrImageFiles[m_nCurrentImageIndex].GetString(), rotated_image);
					}
					else
					{
						//CCommonUtil::LoadImageInPictureBox(m_pPictureBoxCtrl, mainImg);
					}
				}
				UpdateWindow();
				m_CustomPictureCtrl.SendMessage(WM_PAINT);
			}
		}	//if (!mainImg.empty())
	}
}

void CImageAngleCorrectionDlg::OnRotateClockwiseButtonClicked()
{
	UpdateData(TRUE);

	if (0 >= m_arrImageFiles.GetSize())
		return;

	CWaitCursor cWaitCursor;

	std::filesystem::directory_entry fsImageFolderPath(m_arrImageFiles[m_nCurrentImageIndex].GetString());
	double scale = 1.0;

	if (fsImageFolderPath.exists())
	{
		Mat mainImg;
		mainImg = CCommonUtil::ReadImageIntoMatObj(m_arrImageFiles[m_nCurrentImageIndex].GetString());
		if (!mainImg.empty())
		{
			double dAngle = (-1 * (double)m_nClockwiseRotationAngle);

			Point2f center(mainImg.cols / 2.0f, mainImg.rows / 2.0f);

			// Using getRotationMatrix2D() to get the rotation matrix
			Mat rotation_matix = getRotationMatrix2D(center, dAngle, scale);

			// Determine bounding rectangle, center not relevant
			cv::Rect2f bbox = cv::RotatedRect(cv::Point2f(), mainImg.size(), dAngle).boundingRect2f();
			
			// Adjust transformation matrix
			rotation_matix.at<double>(0, 2) += bbox.width / 2.0 - mainImg.cols / 2.0;
			rotation_matix.at<double>(1, 2) += bbox.height / 2.0 - mainImg.rows / 2.0;

			// we will save the resulting image in rotated_image matrix
			Mat rotated_image;

			// rotate the image using warpAffine
			warpAffine(mainImg, rotated_image, rotation_matix, bbox.size(), cv::INTER_LINEAR, cv::BORDER_REPLICATE);

			if (!rotated_image.empty())
			{
				//CCommonUtil::LoadImageInPictureBox(m_pPictureBoxCtrl, rotated_image);

				if (m_bOvewriteRotatedFile)
				{
					cv::imwrite(m_arrImageFiles[m_nCurrentImageIndex].GetString(), rotated_image);
				}
				else
				{
					if (AfxMessageBox("Are you sure you want to ovewrite the rotated file over original file?", MB_YESNO) == IDYES)
					{
						cv::imwrite(m_arrImageFiles[m_nCurrentImageIndex].GetString(), rotated_image);
					}
					else
					{
						//CCommonUtil::LoadImageInPictureBox(m_pPictureBoxCtrl, mainImg);
					}
				}
				UpdateWindow();
				m_CustomPictureCtrl.SendMessage(WM_PAINT);
			}
		}	//if (!mainImg.empty())
	}
}

BOOL CImageAngleCorrectionDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_LEFT)
		{
			OnPreviousImageButtonClicked(); // Previous Image
			return TRUE;
		}
		else if (pMsg->wParam == VK_RIGHT)
		{
			OnNextImageButtonClicked(); // Next Image
			return TRUE;
		}
		else if (pMsg->wParam == VK_UP)
		{
			if ((GetAsyncKeyState(VK_LCONTROL) < 0) || (GetAsyncKeyState(VK_RCONTROL) < 0))
			{
				OnRotateAntiClockwiseButtonClicked();
				return TRUE;
			}
		}
		else if (pMsg->wParam == VK_DOWN)
		{
			if ((GetAsyncKeyState(VK_LCONTROL) < 0) || (GetAsyncKeyState(VK_RCONTROL) < 0))
			{
				OnRotateClockwiseButtonClicked();
				return TRUE;
			}
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CImageAngleCorrectionDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	if (bShow)
	{
		GetDlgItem(IDC_BUTTON_NEXT_IMAGE)->SetFocus();
	}
}

void CImageAngleCorrectionDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}
