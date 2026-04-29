#pragma once

#include <iostream>
#include <filesystem>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

//#include "GridLinesPictureCtrl.h"

using namespace cv;
using namespace std;
using namespace std::filesystem;

static const int MIN_BORDER_PIXEL_MARGIN = 0;
static const int MAX_BORDER_PIXEL_MARGIN = 150;
static const int OUTPUT_STRING_SIZE = 256;
static const int FILE_NAME_SIZE = 512;

class CCommonUtil
{
public:
	static Mat ReadImageIntoMatObj(const string& strImagePath);
	static void ReadImageIntoMatObj(const string& strImagePath, Mat& objImage);
	static void LoadImageInPictureBox(CStatic* pPictureBoxCtrl, Mat srcDrawRect);
	static void GetAllFileNames(const CString& strFolderName, CStringArray& filePaths);
	static std::string GetAppVersion();

private:
	CCommonUtil();	//Making constructor private, so as to prevent creating its object.
};