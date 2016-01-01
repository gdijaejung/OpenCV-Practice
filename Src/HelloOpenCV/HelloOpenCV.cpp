// HelloOpenCV.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

#pragma comment(lib, "opencv_core2411d.lib")
#pragma comment(lib, "opencv_highgui2411d.lib")
#pragma comment(lib, "opencv_imgproc2411d.lib")
#pragma comment(lib, "opencv_features2d2411d.lib")
#pragma comment(lib, "opencv_nonfree2411d.lib")
#pragma comment(lib, "winmm.lib")


int _tmain(int argc, _TCHAR* argv[])
{
	
	Mat f = imread("D:/opencv/sources/samples/cpp/baboon.jpg");
	if (f.empty())
		return 0;

	namedWindow("Hello");
	imshow("Hello", f);
//	putText(f, "Hello, monkey", Point(50, 60), Font();
	waitKey();

	return 0;
}
