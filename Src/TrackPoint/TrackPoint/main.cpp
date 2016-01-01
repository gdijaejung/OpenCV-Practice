// ���󿡼� ����Ʈ�� ã�Ƽ� ǥ���Ѵ�.
// Ư�� ũ�� �̻�, Ư�� ����� ���� �˻��Ѵ�.
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "cvaux.h"
#include "opencv2/nonfree/nonfree.hpp"


using namespace cv;

#pragma comment(lib, "opencv_core2411d.lib")
#pragma comment(lib, "opencv_highgui2411d.lib")
#pragma comment(lib, "opencv_imgproc2411d.lib")
#pragma comment(lib, "opencv_features2d2411d.lib")
#pragma comment(lib, "opencv_nonfree2411d.lib")


void main()
{
	int threshold = 128; // �Ӱ谪(Threshold) ����
	IplImage* image = 0;
	IplImage* output = 0;
	IplImage* gray = 0;

	CvCapture* capture = cvCaptureFromCAM(0);
	cvNamedWindow("HUV05-camera", 0);
	cvNamedWindow("HUV05-binarization", 0);
	cvNamedWindow("HUV05-blob", 0);

	cvResizeWindow("HUV05-camera", 640, 480);
	cvResizeWindow("HUV05-binarization", 640, 480);
	cvResizeWindow("HUV05-blob", 640, 480);
	cvCreateTrackbar("Threthold", "HUV05-camera", &threshold, 255, NULL); // "HUV05-camera" �����쿡 bar �� ���̰� 

	cvMoveWindow("HUV05-camera", 0, 0);
	cvMoveWindow("HUV05-binarization", 660, 0);
	cvMoveWindow("HUV05-blob", 0, 520);

	int fps = 0;
	DWORD oldT = timeGetTime();

	while (1) {
		// check frame per seconds
		++fps;
		DWORD curT = timeGetTime();
		if (curT - oldT > 1000)
		{
			printf("fps = %d\n", fps);
			fps = 0;
			oldT = curT;
		}
		//


		cvGrabFrame(capture);
		image = cvRetrieveFrame(capture);
		if (!image)
			continue;

		cvShowImage("HUV05-camera", image);


		if (!output){
			gray = cvCreateImage({640, 480}, IPL_DEPTH_8U, 1); // ��� �̹��� ����
			output = cvCreateImage({640, 480}, IPL_DEPTH_8U, 1); // ��� �̹��� ����
		}

		cvCvtColor(image, gray, CV_RGB2GRAY); // �÷��� ������� ��ȯ

		if (cvWaitKey(10) >= 0)
			break;

		// ������ �� �ȼ�(x,y) ���� threshold ���� �ʰ��� 255 ��, �� ���ϴ� 0 ���� ��ȯ
		cvThreshold(gray, output, threshold, 255, CV_THRESH_BINARY);
		output->origin = image->origin; // ������ ������ ������ �ٷ� �����
		cvShowImage("HUV05-binarization", output);


		SimpleBlobDetector::Params params;

		// Change thresholds
		params.minThreshold = 1;
		params.maxThreshold = 20;

		// Filter by Area.
		params.filterByArea = true;
		//params.minArea = 1500;
		params.minArea = 10;
		params.maxArea = 100;

		// Filter by Circularity
		params.filterByCircularity = true;
		params.minCircularity = 0.1f;

		// Filter by Convexity
		params.filterByConvexity = true;
		params.minConvexity = 0.87f;

		// Filter by Inertia
		params.filterByInertia = true;
		params.minInertiaRatio = 0.01f;

		SimpleBlobDetector detector(params);

		// Detect blobs.
		std::vector<KeyPoint> keypoints;
		detector.detect(output, keypoints);

		// Draw detected blobs as red circles.
		// DrawMatchesFlags::DRAW_RICH_KEYPOINTS flag ensures the size of the circle corresponds to the size of blob
		Mat im_with_keypoints;
		drawKeypoints(output, keypoints, im_with_keypoints, Scalar::all(-1), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
	
		//cvShowImage("HUV05-camera", image);
		imshow("HUV05-blob", im_with_keypoints);
	}

	cvReleaseCapture(&capture);
	cvDestroyWindow("HUV05-camera");
	cvDestroyWindow("HUV05-binarization");
	cvDestroyWindow("HUV05-blob");
}
