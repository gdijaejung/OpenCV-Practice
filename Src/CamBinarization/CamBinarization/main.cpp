// ī�޶� ������ ����ȭ�ؼ� ����ϴ� ����.
// http://t9t9.com/110 ���� ����.

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "cvaux.h"

#pragma comment(lib, "opencv_core2411d.lib")
#pragma comment(lib, "opencv_highgui2411d.lib")
#pragma comment(lib, "opencv_imgproc2411.lib")

using namespace cv;

void main()
{
	int threshold = 128; // �Ӱ谪(Threshold) ����
	IplImage* image = 0;
	IplImage* output = 0;
	IplImage* gray = 0;

	CvCapture* capture = cvCaptureFromCAM(0);
	cvNamedWindow("camera", 0);
	cvNamedWindow("output", 0);
	cvResizeWindow("camera", 640, 480);
	cvResizeWindow("output", 640, 480);
	cvCreateTrackbar("Threthold", "camera", &threshold, 255, NULL); // "camera" �����쿡 bar �� ���̰� 

	cvMoveWindow("camera", 0, 0);
	cvMoveWindow("output", 660, 0);

	while (1) {
		cvGrabFrame(capture);
		image = cvRetrieveFrame(capture);
		if (!image)
			continue;

		cvShowImage("camera", image);

		if (!output){
			gray = cvCreateImage({ 640, 480 }, IPL_DEPTH_8U, 1); // ��� �̹��� ����
			output = cvCreateImage({ 640, 480 }, IPL_DEPTH_8U, 1); // ��� �̹��� ����
		}

		cvCvtColor(image, gray, CV_RGB2GRAY); // �÷��� ������� ��ȯ

		if (cvWaitKey(10) >= 0)
			break;
		
		// ������ �� �ȼ�(x,y) ���� threshold ���� �ʰ��� 255 ��, �� ���ϴ� 0 ���� ��ȯ
		cvThreshold(gray, output, threshold, 255, CV_THRESH_BINARY);
		output->origin = image->origin; // ������ ������ ������ �ٷ� �����

		cvShowImage("output", output);
	}

	cvReleaseCapture(&capture);
	cvDestroyWindow("camera");
	cvDestroyWindow("output");
}
