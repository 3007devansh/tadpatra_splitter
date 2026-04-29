// CBatchProcessingDlg.cpp : implementation file
//

#include "pch.h"
#include "TadpatraSplitter.h"
#include "BatchProcessingDlg.h"
#include "afxdialogex.h"
#include "Logger.h"
#include "CommonUtil.h"

using namespace cv;
using namespace Logging;

const char IMAGE_TYPE_JPG[] = "JPG";
const char IMAGE_TYPE_PNG[] = "PNG";

#pragma warning (disable : 4267)

// CBatchProcessingDlg dialog
CBatchProcessingDlg::CBatchProcessingDlg(CWnd* pParent)
	: CDialog(IDD_DIALOG_BATCH_PROCESSING, pParent)
#ifdef _DEBUG
	, m_strInputFolderName(_T("C:\\Users\\hardi\\Documents\\Hardik\\TP_200201"))
#else
	, m_strInputFolderName(_T(""))
#endif
	, m_strOutputFolderName(_T(""))
	, m_nBorderPixelMargin(100)
	, m_pPictureBoxCtrl(NULL)
	, m_bStopRequested(FALSE)
	, m_hSplittingThreadHandle(NULL)
	, m_bPreviewImage(TRUE)
	, m_bFirstFolioIsTitleImage(TRUE)
{
}

CBatchProcessingDlg::~CBatchProcessingDlg()
{
}

void CBatchProcessingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_INPUT_FOLDER, m_strInputFolderName);
	DDX_Text(pDX, IDC_EDIT_OUTPUT_FOLDER, m_strOutputFolderName);
	DDX_Control(pDX, IDC_COMBO_IMAGE_TYPE, m_cmbImageType);
	DDX_Text(pDX, IDC_EDIT_BORDER_PIXEL_MARGIN, m_nBorderPixelMargin);
	DDV_MinMaxInt(pDX, m_nBorderPixelMargin, MIN_BORDER_PIXEL_MARGIN, MAX_BORDER_PIXEL_MARGIN);
	DDX_Control(pDX, IDC_SPIN_BORDER_PIXEL_MARGIN, m_SpinCtrl);
	DDX_Control(pDX, IDC_STATIC_STATUS, m_ctrlStatusText);
	DDX_Check(pDX, IDC_CHECK_PREVIEW_IMAGE, m_bPreviewImage);
	DDX_Check(pDX, IDC_CHECK_FIRST_FOLIO_IS_TITLE_IMAGE, m_bFirstFolioIsTitleImage);
}


BEGIN_MESSAGE_MAP(CBatchProcessingDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_INPUT_FOLDER_BROWSE, &CBatchProcessingDlg::OnInputFolderBrowseButtonClicked)
	ON_BN_CLICKED(IDC_BUTTON_OPEN_INPUT_FOLDER, &CBatchProcessingDlg::OnOpenInputFolderButtonClicked)
	ON_BN_CLICKED(IDC_BUTTON_OUTPUT_FOLDER_BROWSE, &CBatchProcessingDlg::OnOutputFolderBrowseButtonClicked)
	ON_BN_CLICKED(IDC_BUTTON_OPEN_OUTPUT_FOLDER, &CBatchProcessingDlg::OnOpenOutputFolderButtonClicked)
	ON_BN_CLICKED(IDC_BUTTON_CLEAN_OUTPUT_FOLDER, &CBatchProcessingDlg::OnCleanOutputFolderButtonClicked)
	ON_BN_CLICKED(IDC_BUTTON_START_SPLITTING, &CBatchProcessingDlg::OnStartSplittingButtonClicked)
	ON_BN_CLICKED(IDC_BUTTON_STOP_SPLITTING, &CBatchProcessingDlg::OnStopSplittingButtonClicked)
END_MESSAGE_MAP()


// CBatchProcessingDlg message handlers

BOOL CBatchProcessingDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (!m_strInputFolderName.IsEmpty())
	{
		m_strOutputFolderName = m_strInputFolderName;
		m_strOutputFolderName += "_Splitted";
	}

	m_cmbImageType.AddString(CA2CT(IMAGE_TYPE_JPG));
	m_cmbImageType.AddString(CA2CT(IMAGE_TYPE_PNG));
	m_cmbImageType.SetCurSel(0);

	m_SpinCtrl.SetRange(MIN_BORDER_PIXEL_MARGIN, MAX_BORDER_PIXEL_MARGIN);

	m_pPictureBoxCtrl = (CStatic*)GetDlgItem(IDC_STATIC_PICTURE_BOX);

	m_ctrlStatusText.SetWindowText("Select Input/Output folders and click 'Start Splitting'.");

	GetDlgItem(IDC_BUTTON_OPEN_INPUT_FOLDER)->SetWindowText("Open Input Folder");

	GetDlgItem(IDC_BUTTON_CLEAN_OUTPUT_FOLDER)->ShowWindow(SW_HIDE);

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

INT CALLBACK CBatchProcessingDlg::BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData)
{
	if (uMsg == BFFM_INITIALIZED) ::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, pData);
	return 0;
}

void CBatchProcessingDlg::OnInputFolderBrowseButtonClicked()
{
	UpdateData(TRUE);

	TCHAR szDir[MAX_PATH];
	BROWSEINFO bInfo;
	ZeroMemory(&bInfo, sizeof(BROWSEINFO));
	bInfo.hwndOwner = this->m_hWnd;
	bInfo.pidlRoot = NULL;
	bInfo.pszDisplayName = szDir; // Address of a buffer to receive the display name of the folder selected by the user
	bInfo.lpszTitle = "Please select Input image folder"; // Title of the dialog
	bInfo.ulFlags = 0;
	bInfo.lpfn = BrowseCallbackProc;
	bInfo.lParam = 0;

	if (!m_strInputFolderName.IsEmpty())
		bInfo.lParam = (LPARAM)m_strInputFolderName.GetString();

	bInfo.iImage = -1;

	LPITEMIDLIST lpItem = SHBrowseForFolder(&bInfo);
	if (lpItem != NULL)
	{
		SHGetPathFromIDList(lpItem, szDir);
		m_strInputFolderName = szDir;

		if (!m_strInputFolderName.IsEmpty())
		{
			m_strOutputFolderName = m_strInputFolderName;
			m_strOutputFolderName += "_Splitted";
		}
		UpdateData(FALSE);
	}
}

void CBatchProcessingDlg::OnOutputFolderBrowseButtonClicked()
{
	UpdateData(TRUE);

	TCHAR szDir[MAX_PATH];
	BROWSEINFO bInfo;
	ZeroMemory(&bInfo, sizeof(BROWSEINFO));
	bInfo.hwndOwner = this->m_hWnd;
	bInfo.pidlRoot = NULL;
	bInfo.pszDisplayName = szDir; // Address of a buffer to receive the display name of the folder selected by the user
	bInfo.lpszTitle = "Please select Output image folder"; // Title of the dialog
	bInfo.ulFlags = BIF_RETURNONLYFSDIRS;
	bInfo.lpfn = BrowseCallbackProc;
	bInfo.lParam = 0;

	if (!m_strOutputFolderName.IsEmpty())
		bInfo.lParam = (LPARAM)m_strOutputFolderName.GetString();

	bInfo.iImage = -1;

	LPITEMIDLIST lpItem = SHBrowseForFolder(&bInfo);
	if (lpItem != NULL)
	{
		SHGetPathFromIDList(lpItem, szDir);
		m_strOutputFolderName = szDir;
		UpdateData(FALSE);
	}
}

void CBatchProcessingDlg::OnOpenInputFolderButtonClicked()
{
	UpdateData(TRUE);
	m_strInputFolderName.Trim();
	if (!m_strInputFolderName.IsEmpty())
		ShellExecuteA(NULL, "open", m_strInputFolderName, NULL, NULL, SW_SHOWDEFAULT);
}

void CBatchProcessingDlg::OnOpenOutputFolderButtonClicked()
{
	UpdateData(TRUE);
	m_strOutputFolderName.Trim();
	if (!m_strOutputFolderName.IsEmpty())
		ShellExecuteA(NULL, "open", m_strOutputFolderName, NULL, NULL, SW_SHOWDEFAULT);
}

void CBatchProcessingDlg::WaitForImageSplittingThread()
{
	if (m_hSplittingThreadHandle)
	{
		DWORD dwWaitResult = WaitForSingleObject(m_hSplittingThreadHandle, 500);
		if (WAIT_OBJECT_0 == dwWaitResult)
		{
			CloseHandle(m_hSplittingThreadHandle);
			m_hSplittingThreadHandle = NULL;
		}
	}
}

void CBatchProcessingDlg::DeleteDirectoryContents(const std::filesystem::path& fspDirectory)
{
	if (std::filesystem::directory_entry(fspDirectory).exists())
	{
		for (const auto& entry : std::filesystem::directory_iterator(fspDirectory))
			std::filesystem::remove_all(entry.path());
	}
}

void CBatchProcessingDlg::OnCleanOutputFolderButtonClicked()
{
	UpdateData(TRUE);

	if (AfxMessageBox("Are you sure you want to clean output folder and delete all processed files?", MB_YESNO) == IDYES)
	{
		CWaitCursor cWaitCursor;

		string strOutputFolderName = m_strOutputFolderName.GetString();
		std::filesystem::path fsOutputFolderPath(strOutputFolderName);
		CBatchProcessingDlg::DeleteDirectoryContents(fsOutputFolderPath);
	}
}

void CBatchProcessingDlg::OnStartSplittingButtonClicked()
{
	m_bStopRequested = FALSE;
	UpdateData(TRUE);

	m_strInputFolderName.Trim();
	if (m_strInputFolderName.IsEmpty())
	{
		AfxMessageBox("Select Input Image Folder.");
		CButton* pButton = (CButton*)GetDlgItem(IDC_BUTTON_INPUT_FOLDER_BROWSE);
		if (pButton)
		{
			pButton->SetFocus();
			return;
		}
	}

	std::filesystem::directory_entry fsInputFolderName(m_strInputFolderName.GetString());
	if (!fsInputFolderName.exists())
	{
		AfxMessageBox("Input Image Folder doesn't exist. Please select valid file.");
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_INPUT_FOLDER);
		if (pEdit)
		{
			pEdit->SetSel(0, -1);
			pEdit->SetFocus();
			return;
		}
	}

	m_strOutputFolderName.Trim();
	if (m_strOutputFolderName.IsEmpty())
	{
		AfxMessageBox("Select Output Image Folder.");
		CButton* pButton = (CButton*)GetDlgItem(IDC_BUTTON_OUTPUT_FOLDER_BROWSE);
		if (pButton)
		{
			pButton->SetFocus();
			return;
		}
	}

	//std::filesystem::directory_entry fsOutputFolderName(m_strOutputFolderName.GetString());
	//if (!fsOutputFolderName.exists())
	//{
	//	AfxMessageBox("Output Image Folder doesn't exist. Please select valid file.");
	//	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_OUTPUT_FOLDER);
	//	if (pEdit)
	//	{
	//		pEdit->SetSel(0, -1);
	//		pEdit->SetFocus();
	//		return;
	//	}
	//}

	if ((m_nBorderPixelMargin < MIN_BORDER_PIXEL_MARGIN) || (m_nBorderPixelMargin > MAX_BORDER_PIXEL_MARGIN))
	{
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_BORDER_PIXEL_MARGIN);
		if (pEdit)
		{
			pEdit->SetSel(0, -1);
			pEdit->SetFocus();
			return;
		}
	}


	CWaitCursor cWaitCursor;

	if (!m_hSplittingThreadHandle)
	{
		DWORD dwTransferThreadID = 0;
		m_hSplittingThreadHandle = ::CreateThread(NULL, 0, &ImageSplittingThreadStaticFunc, this, 0, &dwTransferThreadID);
		if (NULL == m_hSplittingThreadHandle)
		{
			char szOutputString[OUTPUT_STRING_SIZE] = { 0 };
			_snprintf_s(szOutputString, sizeof(szOutputString), sizeof(szOutputString),
				"%s - Line# [%d] - Failed to create ImageSplittingThreadStaticFunc thread.", __FUNCTION__, __LINE__);
			LOG_ERROR(szOutputString);
		}
		else
		{
			LOG_DEBUG("Thread started.");
		}
	}
}

DWORD WINAPI CBatchProcessingDlg::ImageSplittingThreadStaticFunc(LPVOID lpParam)
{
	DWORD dwRetVal = 0;

	CBatchProcessingDlg* pTadpatraSplitterDlg = reinterpret_cast<CBatchProcessingDlg*> (lpParam);
	if (pTadpatraSplitterDlg)
	{
		dwRetVal = pTadpatraSplitterDlg->ImageSplittingThreadFunc();
	}

	return dwRetVal;
}

DWORD CBatchProcessingDlg::ImageSplittingThreadFunc()
{
	char szOutputString[OUTPUT_STRING_SIZE] = { 0 };

	try
	{
		string strImageFolderName = m_strInputFolderName.GetString();
		string strOutputFolderName = m_strOutputFolderName.GetString();

		std::filesystem::directory_entry fsOutputFolderPath(strOutputFolderName);
		if (!fsOutputFolderPath.exists())
		{
			//Create the directory.
			if (!create_directory(strOutputFolderName))
			{
				LOG_ERROR("Failed to create output directory.");
				return 0;
			}
		}

		CString strSelectedImageType;
		m_cmbImageType.GetLBText(m_cmbImageType.GetCurSel(), strSelectedImageType);

		strImageFolderName += "\\*.";

		if (strSelectedImageType.Compare(CString(IMAGE_TYPE_JPG)) == 0)
			strImageFolderName += IMAGE_TYPE_JPG;
		else if (strSelectedImageType.Compare(CString(IMAGE_TYPE_PNG)) == 0)
			strImageFolderName += IMAGE_TYPE_PNG;

		int nRunningIndex = 0;
		int nTileNumber = 1;
		BOOL bFrontSide = TRUE;

		std::filesystem::path pathInputFolderName = m_strInputFolderName.GetString();
		std::string strManuScriptNumber = pathInputFolderName.stem().string();

		CFileFind finder;
		// start working for files
		BOOL bWorking = finder.FindFile(strImageFolderName.c_str());
		BOOL bIndexPageFound = FALSE;

		int nPreviousNoOfStrips = 0;

		char szOutputFileName[FILE_NAME_SIZE];
		int nSimilarWidthCount = 0;

		ULONGLONG dwStartTime = GetTickCount64();
		while (bWorking)
		{
			bWorking = finder.FindNextFile();

			//Skip . and .. files
			if (!finder.IsDots())
			{
				string strFileName = m_strInputFolderName.GetString();
				strFileName += "\\";
				strFileName += finder.GetFileName().GetString();
				string strOriginalFileName = finder.GetFileTitle();	//Store original FileTitle in variable to use later.

				if (m_bFirstFolioIsTitleImage && !bIndexPageFound)
				{
					memset(szOutputFileName, 0, sizeof(szOutputFileName));

					_snprintf_s(szOutputFileName, sizeof(szOutputFileName), sizeof(szOutputFileName),
						"%s-%05d-%d-%c(%s)",
						strManuScriptNumber.c_str(), 0, 0, 'X',
						strOriginalFileName.c_str());

					string output_path = strOutputFolderName + "\\" + szOutputFileName + ".jpg";
					CopyFile(strFileName.c_str(), output_path.c_str(), FALSE);
					bIndexPageFound = TRUE;
					continue;
				}

				std::filesystem::directory_entry fsImageFilePath(strFileName);

				if (fsImageFilePath.exists())
				{
					Mat mainImg = CCommonUtil::ReadImageIntoMatObj(strFileName);

					if (!mainImg.empty())
					{
						string strStatusText = "Processing " + strFileName;
						m_ctrlStatusText.SetWindowText(CA2CT(strStatusText.c_str()));
						nRunningIndex++;

						if ((nRunningIndex >= 3) && (nRunningIndex % 2) == 1)
							nTileNumber++;

						if (m_bPreviewImage)
							CCommonUtil::LoadImageInPictureBox(m_pPictureBoxCtrl, mainImg);
						else
							CCommonUtil::LoadImageInPictureBox(m_pPictureBoxCtrl, Mat());

						cv::Mat grayImg, binImg;
						cvtColor(mainImg, grayImg, COLOR_BGR2GRAY);	//Converting to 8-bit grayscale image.
						threshold(grayImg, binImg, 0, 255, THRESH_OTSU | THRESH_BINARY_INV); //Remove / Separate white background image from the foreground image.

						//cv::imwrite("D:\\test.jpg", binImg);

						vector< vector< cv::Point>> contours;
						//std::vector<cv::Rect> object_coordinates;
						findContours(binImg, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);

						vector<RECT_COORDINATES> objectDetails;

						Rect rectBoundingRect;
						double dContourArea;
						for (int i = 0; i < contours.size(); i++)
						{
							rectBoundingRect = boundingRect(contours[i]);
							dContourArea = contourArea(contours[i]);
							if (dContourArea > 10000)
							{
								RECT_COORDINATES structObjectDetails;
								structObjectDetails.m_objectRect = rectBoundingRect;
								structObjectDetails.m_nObjectIndex = i;

								objectDetails.push_back(structObjectDetails);
							}
						}

						vector<vector<cv::Point> > hull(contours.size());

						//sort(object_coordinates.begin(), object_coordinates.end(), [&](const auto& a, const auto& b) {return a.y < b.y; });
						sort(objectDetails.begin(), objectDetails.end(), [&](const auto& a, const auto& b) {return a.m_objectRect.y < b.m_objectRect.y; });

						size_t nNumberOfStrips = objectDetails.size();

						//First Step - Check if Number of Strips are greater than Previous Number of Strips
						//if ((0 != nPreviousNoOfStrips) &&
						//	(nNumberOfStrips > nPreviousNoOfStrips))
						if (1 == nNumberOfStrips)
						{
							contours.clear();
							objectDetails.clear();

							//Ignore current image file and just copy it as-is to output folder.
							memset(szOutputFileName, 0, sizeof(szOutputFileName));

							_snprintf_s(szOutputFileName, sizeof(szOutputFileName), sizeof(szOutputFileName),
								"%s-%05d-%d-%c(%s)",
								strManuScriptNumber.c_str(), nTileNumber, 0, bFrontSide ? 'A' : 'B',
								strOriginalFileName.c_str());

							string output_path = strOutputFolderName + "\\" + szOutputFileName + ".jpg";
							CopyFile(strFileName.c_str(), output_path.c_str(), FALSE);

							goto SkipBrokenStrips;
						}
						else
						{
							int nBrokenStripsPossibilityCount = 0;
							//Find out if the splitted objects have similar width or
							//these Tadpatra may be of broken images.
							for (size_t i = 1; i < nNumberOfStrips; i++)
							{
								int nAdjacentWidthDiff = abs(objectDetails[i].m_objectRect.width - objectDetails[i - 1].m_objectRect.width);
								if (nAdjacentWidthDiff >=
									((objectDetails[i].m_objectRect.width * 10) / 100)
									)
								{
									//It looks like we have borken Tadpatra images.
									//So, don't process such images and just ignore the entire Tile.
									nBrokenStripsPossibilityCount++;
								}
							}

							nSimilarWidthCount = 0;

							if (nNumberOfStrips <= 5)
							{
								for (size_t i = 0; i < nNumberOfStrips; i++)
								{
									for (size_t j = 0; j < nNumberOfStrips; j++)
									{
										if (j != i)
										{
											int nWidthDiff = abs(objectDetails[i].m_objectRect.width - objectDetails[j].m_objectRect.width);
											if (nWidthDiff <= 200)
											{
												nSimilarWidthCount++;
											}
										}
									}
								}
							}

							//if ((0 != nBrokenStripsPossibilityCount) && (nNumberOfStrips >= 3))
							//{
							//	int nAdjacentWidthDiff = abs(objectDetails[0].m_objectRect.width - objectDetails[nNumberOfStrips-1].m_objectRect.width);
							//	if (nAdjacentWidthDiff >=
							//		((objectDetails[0].m_objectRect.width * 10) / 100)
							//		)
							//	{
							//		//It looks like we have borken Tadpatra images.
							//		//So, don't process such images and just ignore the entire Tile.
							//		nBrokenStripsPossibilityCount++;
							//	}
							//}
							//							if ((0 != nBrokenStripsPossibilityCount) && (nNumberOfStrips - nBrokenStripsPossibilityCount) >= 2)
							if ((0 == nSimilarWidthCount) &&
									((nBrokenStripsPossibilityCount >= 2) || 
									((nBrokenStripsPossibilityCount != 0) && ((nNumberOfStrips - nBrokenStripsPossibilityCount) <= 2)) ||
									((nBrokenStripsPossibilityCount != 0) && (nBrokenStripsPossibilityCount >= (nNumberOfStrips / 2 )))
									)
								)
							{
								//Ignore current image file and just copy it as-is to output folder.
								memset(szOutputFileName, 0, sizeof(szOutputFileName));

								_snprintf_s(szOutputFileName, sizeof(szOutputFileName), sizeof(szOutputFileName),
									"%s-%05d-%d-%c(%s)",
									strManuScriptNumber.c_str(), nTileNumber, 0, bFrontSide ? 'A' : 'B',
									strOriginalFileName.c_str());

								string output_path = strOutputFolderName + "\\" + szOutputFileName + ".jpg";
								CopyFile(strFileName.c_str(), output_path.c_str(), FALSE);

								goto SkipBrokenStrips;
							}
						}

						for (int i = 0; i < objectDetails.size(); i++)
						{
							//cv::Mat cropped_object = mainImg(object_coordinates[i]);
							cv::convexHull(contours[objectDetails[i].m_nObjectIndex], hull[i], false, true);
							Rect boundingBox = cv::boundingRect(hull[i]);
							for (int j = 0; j < hull[i].size(); j++)
							{
								hull[i][j].x -= boundingBox.x;
								hull[i][j].y -= boundingBox.y;
							}

							cv::Mat cropped_object_BinImg = Mat::zeros(objectDetails[i].m_objectRect.size(), CV_8UC1);
							drawContours(cropped_object_BinImg, hull, i, Scalar(255, 255, 255), FILLED, LINE_8);
							cv::bitwise_not(cropped_object_BinImg, cropped_object_BinImg);
							cv::Mat cropped_object_ColorImg = mainImg.clone()(objectDetails[i].m_objectRect);
							cv::Mat cropped_object_TempImg;
							cv::cvtColor(cropped_object_BinImg, cropped_object_TempImg, COLOR_GRAY2BGR);
							cv::bitwise_or(cropped_object_ColorImg, cropped_object_TempImg, cropped_object_ColorImg);

							cv::Mat borderedImage(cropped_object_ColorImg.rows + 2 * m_nBorderPixelMargin, cropped_object_ColorImg.cols + 2 * m_nBorderPixelMargin, cropped_object_ColorImg.type(), cv::Scalar(255, 255, 255));
							cropped_object_ColorImg.copyTo(borderedImage(cv::Rect(m_nBorderPixelMargin, m_nBorderPixelMargin, cropped_object_ColorImg.cols, cropped_object_ColorImg.rows)));

							memset(szOutputFileName, 0, sizeof(szOutputFileName));

							int temp;
							if (bFrontSide)
							{
								temp = i + 1;
							}
							else
							{
								temp = nNumberOfStrips;
								nNumberOfStrips--;
							}
							_snprintf_s(szOutputFileName, sizeof(szOutputFileName), sizeof(szOutputFileName),
								"%s-%05d-%d-%c(%s)",
								strManuScriptNumber.c_str(), nTileNumber, temp, bFrontSide ? 'A' : 'B',
								strOriginalFileName.c_str());

							string output_path = strOutputFolderName + "\\" + szOutputFileName + ".jpg";
							cv::imwrite(output_path, borderedImage);
							//::Sleep(1);
						}	//for (...)

						//Assign Current count of Number of Strips to nPreviousNoOfStrips.
						nPreviousNoOfStrips = objectDetails.size();

						hull.clear();
						contours.clear();
						objectDetails.clear();
					}	//if (!mainImg.empty())
				}	//if (fsImageFilePath.exists())

			SkipBrokenStrips:
				bFrontSide = !bFrontSide;
			}	//if (!finder.IsDots())
			//::Sleep(1);
			
			CButton *pCheckBox = (CButton*)GetDlgItem(IDC_CHECK_PREVIEW_IMAGE);
			if (pCheckBox)
			{
				m_bPreviewImage = pCheckBox->GetCheck();
			}

			if (IsStopRequested() && bFrontSide)
				break;
		}	//while (bWorking)

		ULONGLONG dwEndTime = GetTickCount64();

		_snprintf_s(szOutputString, sizeof(szOutputString), sizeof(szOutputString),
			"Splitting completed! Total %d files processed in [%llu] milli-seconds.", nRunningIndex, dwEndTime - dwStartTime);
		m_ctrlStatusText.SetWindowText(szOutputString);
	}
	catch (const std::exception& ex)
	{
		_snprintf_s(szOutputString, sizeof(szOutputString), sizeof(szOutputString),
			"%s - Line# [%d] - Exception occurred - [%s].", __FUNCTION__, __LINE__, ex.what());
		LOG_ERROR(szOutputString);
	}
	catch (...)
	{
		_snprintf_s(szOutputString, sizeof(szOutputString), sizeof(szOutputString),
			"%s - Line# [%d] - Exception occurred.", __FUNCTION__, __LINE__);
		LOG_ERROR(szOutputString);
	}

	CloseHandle(m_hSplittingThreadHandle);
	m_hSplittingThreadHandle = NULL;

	return 0;
}

void CBatchProcessingDlg::OnStopSplittingButtonClicked()
{
	m_bStopRequested = TRUE;

	CWaitCursor cWaitCursor;

	WaitForImageSplittingThread();
	m_ctrlStatusText.SetWindowText("Splitting stopped.");
}
