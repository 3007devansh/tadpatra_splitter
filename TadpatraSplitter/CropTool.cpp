// CropTool_Hardik.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

Mat readImage(string imgPath)
{
	Mat matTempBuff = cv::imread(imgPath, IMREAD_COLOR);
	return matTempBuff;
}

int main()
{
	std::cout << "Hello Let's start!\n";
	string inputImagePath = "D:\\MyRepo\\CropTool_Hardik\\Images\\2.jpg";
	string outputFolderPath = "D:\\MyRepo\\CropTool_Hardik\\Images";
	Mat mainImg = readImage(inputImagePath);

	cv::Mat grayImg, binImg;
	cvtColor(mainImg, grayImg, COLOR_BGR2GRAY);
	threshold(grayImg, binImg, 0, 255, THRESH_OTSU | THRESH_BINARY_INV);

	/*cv::imshow("Original Image", mainImg);
	cv::imshow("Binary Image", binImg);
	cv::waitKey(0);
	cv::destroyAllWindows();*/

	vector< vector< cv::Point>> contours;
	std::vector<cv::Rect> object_coordinates;
	findContours(binImg, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
	for (int i = 0; i < contours.size(); i++)
	{
		Rect bb = boundingRect(contours[i]);
		double area = contourArea(contours[i]);
		if (area > 50)
		{
			strObjectDetails
			strObjectDetails.objectRect = bb;
			strObjectDetails.objectIndex = i;
			object_coordinates.push_back(bb);
		}
	}
	contours.clear();

	for (int i = 0; i < object_coordinates.size(); i++)
	{
		cv::Mat cropped_object = mainImg(object_coordinates[i]);
		string output_path = outputFolderPath + "//object_" + std::to_string(i) + ".jpg";
		imwrite(output_path, cropped_object);
		std::cout << "Object " << i << " saved as " << output_path << std::endl;
	}
	object_coordinates.clear();

	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
