#include "pch.h"
#include "CommonUtil.h"

#include <winver.h>                   //Include file for VerQueryValue
#pragma comment (lib, "version.lib")  //Lib for VerQueryValue

#pragma warning (disable : 4244)
#pragma warning (disable : 4996)

Mat CCommonUtil::ReadImageIntoMatObj(const string& strImagePath)
{
	Mat objImageMatBuffer;

	if (!strImagePath.empty())
		objImageMatBuffer = cv::imread(strImagePath, IMREAD_COLOR);
	return objImageMatBuffer;
}

void CCommonUtil::ReadImageIntoMatObj(const string& strImagePath, Mat& objImage)
{
    if (!strImagePath.empty())
        objImage = cv::imread(strImagePath, IMREAD_COLOR);
}

void CCommonUtil::LoadImageInPictureBox(CStatic* pPictureBoxCtrl, Mat srcDrawRect)
{
    if (pPictureBoxCtrl)
    {
        Mat tempDrawRect;
        cvtColor(srcDrawRect, tempDrawRect, COLOR_BGR2BGRA);

        RECT rect;
        pPictureBoxCtrl->GetWindowRect(&rect);

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
            pPictureBoxCtrl->SetBitmap(hBitmap);
            DeleteObject(hBitmap);
        }
        //UpdateWindow();
    }
}

std::string CCommonUtil::GetAppVersion()
{
    DWORD dwHandle;
    TCHAR fileName[MAX_PATH];

    GetModuleFileName(NULL, fileName, MAX_PATH);
    DWORD dwSize = GetFileVersionInfoSize(fileName, &dwHandle);
    TCHAR buffer[2048];

    VS_FIXEDFILEINFO* pvFileInfo = NULL;
    UINT fiLen = 0;

    if ((dwSize > 0) && GetFileVersionInfo(fileName, dwHandle, dwSize, &buffer))
    {
        VerQueryValue(&buffer, "\\", (LPVOID*)&pvFileInfo, &fiLen);
    }

    if (fiLen > 0)
    {
        char buf[25];
        int len = sprintf(buf, "%hu.%hu.%hu.%hu",
            HIWORD(pvFileInfo->dwFileVersionMS),
            LOWORD(pvFileInfo->dwFileVersionMS),
            HIWORD(pvFileInfo->dwFileVersionLS),
            LOWORD(pvFileInfo->dwFileVersionLS)
        );

        return std::string(buf, len);
    }
    else
    {
        return std::string("(Unknown)");
    }
}

void CCommonUtil::GetAllFileNames(const CString& strFolderName, CStringArray& filePaths)
{
    string strImageFolderName = strFolderName.GetString();
    strImageFolderName += "\\*.JPG";

    CFileFind finder;
    // start working for files
    BOOL bWorking = finder.FindFile(strImageFolderName.c_str());

    while (bWorking)
    {
        bWorking = finder.FindNextFile();

        // skip . and .. files
        if (!finder.IsDots())
        {
            string strFileName = strFolderName.GetString();
            strFileName += "\\";
            strFileName += finder.GetFileName().GetString();

            filePaths.Add(strFileName.c_str());
        }
    }
}
