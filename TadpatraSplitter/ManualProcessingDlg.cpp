// CManualProcessingDlg.cpp : implementation file
//

#include "pch.h"
#include "TadpatraSplitter.h"
#include "ManualProcessingDlg.h"
#include "afxdialogex.h"
#include "Logger.h"
#include "CommonUtil.h"

#include <iostream>
#include <filesystem>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;
using namespace std::filesystem;
using namespace Logging;

// CManualProcessingDlg dialog

CManualProcessingDlg::CManualProcessingDlg(CWnd* pParent)
	: CDialog(IDD_DIALOG_MANUAL_PROCESSING, pParent)
	, m_strFileNameFrontSide(_T(""))
	, m_strFileNameBackSide(_T(""))
	, m_pPictureBoxFrontSide(NULL)
	, m_pPictureBoxBackSide(NULL)
	, m_strOutputFolderName(_T(""))
	, m_strManuScriptNumber(_T(""))
	, m_nBorderPixelMargin(100)
{

}

CManualProcessingDlg::~CManualProcessingDlg()
{
}

void CManualProcessingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_FILE_NAME_FRONT_SIDE, m_strFileNameFrontSide);
	DDX_Text(pDX, IDC_EDIT_FILE_NAME_BACK_SIDE, m_strFileNameBackSide);
	DDX_Text(pDX, IDC_EDIT_OUTPUT_FOLDER, m_strOutputFolderName);
	DDX_Control(pDX, IDC_STATIC_STATUS, m_ctrlStatusText);
	DDX_Text(pDX, IDC_EDIT_MANUSCRIPT_NUMBER, m_strManuScriptNumber);
	DDX_Text(pDX, IDC_EDIT_BORDER_PIXEL_MARGIN, m_nBorderPixelMargin);
	DDV_MinMaxInt(pDX, m_nBorderPixelMargin, MIN_BORDER_PIXEL_MARGIN, MAX_BORDER_PIXEL_MARGIN);
	DDX_Control(pDX, IDC_SPIN_BORDER_PIXEL_MARGIN, m_SpinCtrl);
}


BEGIN_MESSAGE_MAP(CManualProcessingDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_FILE_NAME_FRONT_SIDE, &CManualProcessingDlg::OnFileNameFrontSideButtonClicked)
	ON_BN_CLICKED(IDC_BUTTON_FILE_NAME_BACK_SIDE, &CManualProcessingDlg::OnFileNameBackSideButtonClicked)
	ON_BN_CLICKED(IDC_BUTTON_OUTPUT_FOLDER_BROWSE, &CManualProcessingDlg::OnOutputFolderBrowseButtonClicked)
	ON_BN_CLICKED(IDC_BUTTON_OPEN_OUTPUT_FOLDER, &CManualProcessingDlg::OnOpenOutputFolderButtonClicked)
	ON_BN_CLICKED(IDC_BUTTON_START_SPLITTING, &CManualProcessingDlg::OnStartSplittingButtonClicked)
END_MESSAGE_MAP()

// CManualProcessingDlg message handlers
BOOL CManualProcessingDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_pPictureBoxFrontSide = (CStatic*)GetDlgItem(IDC_STATIC_FRONT_SIDE);
	m_pPictureBoxBackSide = (CStatic*)GetDlgItem(IDC_STATIC_BACK_SIDE);

	m_SpinCtrl.SetRange(MIN_BORDER_PIXEL_MARGIN, MAX_BORDER_PIXEL_MARGIN);

	m_ctrlStatusText.SetWindowText("Manually select Front and Backside Image of Manuscript and click Start Splitting.");

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CManualProcessingDlg::OnFileNameFrontSideButtonClicked()
{
	CFileDialog fOpenDlg(TRUE, "jpg", "", OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,
		"JPG Files (*.jpg)|*.jpg||", this);

	fOpenDlg.m_pOFN->lpstrTitle = "JPG Files";

	if (fOpenDlg.DoModal() == IDOK)
	{
		m_strFileNameFrontSide = (LPCTSTR)fOpenDlg.GetPathName();
		m_strFileNameFrontSide.Trim();

		if (!m_strFileNameFrontSide.IsEmpty())
		{
			path p = m_strFileNameFrontSide.GetString();
			m_strOutputFolderName = p.parent_path().string().c_str();
			m_strOutputFolderName += "_Splitted";

			//std::string temp = m_strFileNameFrontSide.GetString();
			//std::string str1 = temp.substr(0, temp.find_last_of("/\\"));
			//str1 = str1.substr(0, str1.find_last_of("/\\"));
			UpdateData(FALSE);
			ShowImagePreview(m_strFileNameFrontSide, m_pPictureBoxFrontSide);
		}
	}
}

void CManualProcessingDlg::OnFileNameBackSideButtonClicked()
{
	CFileDialog fOpenDlg(TRUE, "jpg", "", OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,
		"JPG Files (*.jpg)|*.jpg||", this);

	fOpenDlg.m_pOFN->lpstrTitle = "JPG Files";

	if (fOpenDlg.DoModal() == IDOK)
	{
		m_strFileNameBackSide = (LPCTSTR)fOpenDlg.GetPathName();
		m_strFileNameBackSide.Trim();

		if (!m_strFileNameBackSide.IsEmpty())
		{
			UpdateData(FALSE);
			ShowImagePreview(m_strFileNameBackSide, m_pPictureBoxBackSide);
		}
	}
}

void CManualProcessingDlg::ShowImagePreview(const CString& strImageFile, CStatic* pPictureBoxCtrl)
{
	string strFileName = strImageFile.GetString();
	std::filesystem::directory_entry fsImageFolderPath(strFileName);

	if (fsImageFolderPath.exists())
	{
		Mat mainImg = CCommonUtil::ReadImageIntoMatObj(strFileName);
		if (!mainImg.empty())
		{
			CCommonUtil::LoadImageInPictureBox(pPictureBoxCtrl, mainImg);
		}
	}
}

void CManualProcessingDlg::OnOutputFolderBrowseButtonClicked()
{
	TCHAR szDir[MAX_PATH];
	BROWSEINFO bInfo;
	ZeroMemory(&bInfo, sizeof(BROWSEINFO));
	bInfo.hwndOwner = this->m_hWnd;
	bInfo.pidlRoot = NULL;
	bInfo.pszDisplayName = szDir; // Address of a buffer to receive the display name of the folder selected by the user
	bInfo.lpszTitle = "Please select output image folder"; // Title of the dialog
	bInfo.ulFlags = 0;
	bInfo.lpfn = NULL;
	bInfo.lParam = 0;
	bInfo.iImage = -1;

	LPITEMIDLIST lpItem = SHBrowseForFolder(&bInfo);
	if (lpItem != NULL)
	{
		SHGetPathFromIDList(lpItem, szDir);
		m_strOutputFolderName = szDir;
		UpdateData(FALSE);
	}
}

void CManualProcessingDlg::OnOpenOutputFolderButtonClicked()
{
	UpdateData(TRUE);
	m_strOutputFolderName.Trim();
	if (!m_strOutputFolderName.IsEmpty())
		ShellExecuteA(NULL, "open", m_strOutputFolderName, NULL, NULL, SW_SHOWDEFAULT);
}

void CManualProcessingDlg::OnStartSplittingButtonClicked()
{
	UpdateData(TRUE);

	m_strFileNameFrontSide.Trim();
	if (m_strFileNameFrontSide.IsEmpty())
	{
		AfxMessageBox("Select Manuscript Frontside image.");
		CButton* pButton = (CButton*)GetDlgItem(IDC_BUTTON_FILE_NAME_FRONT_SIDE);
		if (pButton)
		{
			pButton->SetFocus();
			return;
		}
	}

	std::filesystem::directory_entry fsFrontSideFileName(m_strFileNameFrontSide.GetString());
	if (!fsFrontSideFileName.exists())
	{
		AfxMessageBox("Manuscript Frontside image file doesn't exist. Please select valid file.");
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_FILE_NAME_FRONT_SIDE);
		if (pEdit)
		{
			pEdit->SetSel(0, -1);
			pEdit->SetFocus();
			return;
		}
	}

	m_strFileNameBackSide.Trim();
	if (m_strFileNameBackSide.IsEmpty())
	{
		AfxMessageBox("Select Manuscript Backside image.");
		CButton* pButton = (CButton *)GetDlgItem(IDC_BUTTON_FILE_NAME_BACK_SIDE);
		if (pButton)
		{
			pButton->SetFocus();
			return;
		}
	}

	std::filesystem::directory_entry fsBackSideFileName(m_strFileNameBackSide.GetString());
	if (!fsBackSideFileName.exists())
	{
		AfxMessageBox("Manuscript Backside image file doesn't exist. Please select valid file.");
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_FILE_NAME_BACK_SIDE);
		if (pEdit)
		{
			pEdit->SetSel(0, -1);
			pEdit->SetFocus();
			return;
		}
	}

	m_strManuScriptNumber.Trim();
	if (m_strManuScriptNumber.IsEmpty())
	{
		AfxMessageBox("Please enter proper Manuscript Number.");
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_MANUSCRIPT_NUMBER);
		if (pEdit)
		{
			pEdit->SetSel(0, -1);
			pEdit->SetFocus();
			return;
		}
	}

	char szOutputString[OUTPUT_STRING_SIZE] = { 0 };

	try
	{
		for (int nManuScriptNumber=0; nManuScriptNumber <2; nManuScriptNumber++)
		{
			Mat mainImg;

			string strFileName = "";
			if (nManuScriptNumber == 0)
				strFileName = m_strFileNameFrontSide.GetString();
			else
				strFileName = m_strFileNameBackSide.GetString();

			mainImg = CCommonUtil::ReadImageIntoMatObj(strFileName);

			if (!mainImg.empty())
			{
				string strStatusText = "Processing " + strFileName;
				m_ctrlStatusText.SetWindowText(CA2CT(strStatusText.c_str()));

				cv::Mat grayImg, binImg;
				cvtColor(mainImg, grayImg, COLOR_BGR2GRAY);	//Converting to 8-bit grayscale image.
				threshold(grayImg, binImg, 0, 255, THRESH_OTSU | THRESH_BINARY_INV); //Remove / Separate white background image from the foreground image.

				vector< vector< cv::Point>> contours;
				std::vector<cv::Rect> object_coordinates;
				findContours(binImg, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);

				for (int i = 0; i < contours.size(); i++)
				{
					Rect bb = boundingRect(contours[i]);
					double area = contourArea(contours[i]);
					if (area > 10000)	// && contours[i].size() > 50)
					{
						object_coordinates.push_back(bb);
					}
				}
				contours.clear();

				sort(object_coordinates.begin(), object_coordinates.end(), [&](const auto& a, const auto& b) {return a.y < b.y; });

				size_t nNumberOfFolio = object_coordinates.size();
				for (int i = 0; i < nNumberOfFolio; i++)
				{
					cv::Mat cropped_object = mainImg(object_coordinates[i]);

					cv::Mat borderedImage(cropped_object.rows + 2 * m_nBorderPixelMargin, cropped_object.cols + 2 * m_nBorderPixelMargin, cropped_object.type(), cv::Scalar(255, 255, 255));
					cropped_object.copyTo(borderedImage(cv::Rect(m_nBorderPixelMargin, m_nBorderPixelMargin, cropped_object.cols, cropped_object.rows)));

					path strPath = path(strFileName.c_str()).stem();

					char szOutputFileName[FILE_NAME_SIZE];
					memset(szOutputFileName, 0, sizeof(szOutputFileName));
					_snprintf_s(szOutputFileName, sizeof(szOutputFileName), sizeof(szOutputFileName),
						"%s-%04d-%c(%s)",
						m_strManuScriptNumber.GetString(), i + 1, (nManuScriptNumber==0) ? 'A' : 'B', strPath.string().c_str());

					string output_path = m_strOutputFolderName.GetString();
					output_path += "\\";
					output_path += szOutputFileName;
					output_path += ".jpg";

					if (std::filesystem::exists(output_path))
					{
						string strSkipFileMessage;
						strSkipFileMessage = "Output file ";
						strSkipFileMessage += output_path;
						strSkipFileMessage += " already exists. Are you sure you want to overwrite it ?";
						if (AfxMessageBox(strSkipFileMessage.c_str(), MB_YESNO) == IDYES)
						{
							cv::imwrite(output_path, borderedImage);
						}
						else
						{
							strSkipFileMessage = "Skipping " + strFileName;
							m_ctrlStatusText.SetWindowText(strSkipFileMessage.c_str());
						}
					}
					::Sleep(1);
				}

				object_coordinates.clear();
			}	//if (!mainImg.empty())
			::Sleep(10);
		}	//for (int nManuScriptNumber=0; nManuScriptNumber <2; nManuScriptNumber++)
		AfxMessageBox("Manual processing completed.", MB_OK);
	}
	catch (const std::exception& ex)
	{
		_snprintf_s(szOutputString, sizeof(szOutputString), sizeof(szOutputString),
			"%s - Line# [%d] - Exception occurred - [%s].", __FUNCTION__, __LINE__, ex.what());
		LOG_ERROR(szOutputString);
		m_ctrlStatusText.SetWindowText("Error occurred while processing file.");
	}
	catch (...)
	{
		_snprintf_s(szOutputString, sizeof(szOutputString), sizeof(szOutputString),
			"%s - Line# [%d] - Exception occurred.", __FUNCTION__, __LINE__);
		LOG_ERROR(szOutputString);
		m_ctrlStatusText.SetWindowText("Error occurred while processing file.");
	}
}
