
#include "global.h"
#include "screencalib.h"
#include "findrectcontour.h"
#include <sstream>

using namespace cv;


// ��ũ�� ���� ���� �����Ѵ�.
void ScreenCalibration( const vector<Point> &screenContour )
{
	const int w = g_screen.GetWidth();
	const int h = g_screen.GetHeight();
	const int rows = 5;
	const int cols = 5;
	const int cellW = w / cols;
	const int cellH = h / rows;
	g_screen.CreateCellMappingTable(cols, rows);

	int threshold = 254;
//	int threshold = 200;
	IplImage *camera = 0;
	IplImage *binImage = 0;
	IplImage *binOutput = 0;

	Mat screen(h, w, CV_8UC(3));
	namedWindow("Screen", CV_WINDOW_NORMAL);
	cvSetWindowProperty("Screen", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);

	cvNamedWindow("HUV05-camera", 0);
	cvNamedWindow("HUV05-binarization", 0);
	cvNamedWindow("HUV05-screen", 0);

	cvResizeWindow("HUV05-camera", 640, 480);
	cvResizeWindow("HUV05-binarization", 640, 480);
	cvResizeWindow("HUV05-screen", 640, 480);

	cvMoveWindow("HUV05-camera", -660, 0);
	cvMoveWindow("HUV05-binarization", -660, 520);
	cvMoveWindow("HUV05-screen", -660, 0);
	cvCreateTrackbar("Threshold", "HUV05-camera", &threshold, 255, NULL); // "HUV05-camera" �����쿡 bar �� ���δ�. 
	cvCreateTrackbar("Threshold", "HUV05-screen", &threshold, 255, NULL); // "HUV05-camera" �����쿡 bar �� ���δ�. 

	std::vector<vector<Point> > contours(1);
	contours[0] = screenContour;

	cvWaitKey(300);


	int count = 0;
	bool isDelay = false;
	int delayTime = 0;

	while (count < (rows * cols))
	{
		const int r = count / rows;
		const int c = count % cols;
		screen.setTo(Scalar(0, 0, 0));
		rectangle(screen, Rect(c*cellW, r*cellH, cellW, cellH), Scalar(255, 255, 255), CV_FILLED);
		imshow("Screen", screen);

		camera = g_camera.GetCapture();

		// ��ũ�� ��ġ ���� �����´�.
		Mat src(camera);
		Mat mask = Mat::zeros(src.rows, src.cols, CV_8UC1);
		drawContours(mask, contours, 0, Scalar(255), CV_FILLED);
		Mat maskingScreen(src.rows, src.cols, CV_8UC3);
		maskingScreen.setTo(Scalar(0, 0, 0));
		src.copyTo(maskingScreen, mask);

		IplImage dummyScreen(maskingScreen); // ����ȭ ó���� ���� ���� ��ũ�� ����


		// ī�޶� ������ ����ȭ �Ѵ�.
		if (!binImage) {
			binImage = cvCreateImage({ 640, 480 }, IPL_DEPTH_8U, 1); // ��� �̹��� ����
			binOutput = cvCreateImage({ 640, 480 }, IPL_DEPTH_8U, 1); // ��� �̹��� ����
		}

		cvCvtColor(&dummyScreen, binImage, CV_RGB2GRAY); // �÷��� ������� ��ȯ
		// ������ �� �ȼ�(x,y) ���� threshold ���� �ʰ��� 255 ��, �� ���ϴ� 0 ���� ��ȯ
		cvThreshold(binImage, binOutput, threshold, 255, CV_THRESH_BINARY_INV);
		binOutput->origin = dummyScreen.origin; // ������ ������ ������ �ٷ� ��� ��

		cvShowImage("HUV05-camera", &dummyScreen);
		cvShowImage("HUV05-binarization", binOutput);

		if (cvWaitKey(10) >= 0)
			break;

		if (isDelay)
		{
			if (timeGetTime() - delayTime > 300)
				isDelay = false;
			continue;
		}
		

		Mat dst(&dummyScreen);
		vector<Point> rectLines;
		const pair<float,float> rate = g_screen.GetResolutionRecognitionRate();
		const float minArea = cellH*cellW * rate.first  * 0.7f;
		const float maxArea = cellH*cellW * rate.first * 2.f;
		if (FindRectContour(binOutput, binOutput, (int)minArea, (int)maxArea, rectLines))
		{
			setLabel(dst, "RECT", rectLines);
			line(dst, rectLines[0], rectLines[1], Scalar(255, 0, 0), 2);
			line(dst, rectLines[1], rectLines[2], Scalar(255, 0, 0), 2);
			line(dst, rectLines[2], rectLines[3], Scalar(255, 0, 0), 2);
			line(dst, rectLines[3], rectLines[0], Scalar(255, 0, 0), 2);

			
			g_screen.SetCellMappingTable(count, Rect(c*cellW, r*cellH, cellW, cellH), rectLines);

			isDelay = true;
			delayTime = timeGetTime();
			++count;
		}

		imshow("HUV05-screen", dst);
	}


	//-------------------------------------------------------------------------------
	// ��� ����ȭ�� ���
	g_screen.CalculateCellMapping(); // ���� ���̺��� �ϼ��Ѵ�.

	screen.setTo(Scalar(0, 0, 0));
	imshow("Screen", screen);
	cvWaitKey(30);

	camera = g_camera.GetCapture();
	int num = 0;
	Mat dst(camera);	
	for each(auto &table in g_screen.GetCellMappingTable())
	{
		std::stringstream ss;
		ss << "RECT " << num++;
		setLabel(dst, ss.str(), table.cameraCell);
		line(dst, table.cameraCell[0], table.cameraCell[1], Scalar(255, 0, 0), 1);
		line(dst, table.cameraCell[1], table.cameraCell[2], Scalar(255, 0, 0), 1);
		line(dst, table.cameraCell[2], table.cameraCell[3], Scalar(255, 0, 0), 1);
		line(dst, table.cameraCell[3], table.cameraCell[0], Scalar(255, 0, 0), 1);
	}
	imshow("HUV05-screen", dst);

	num = 0;
	for each(auto &table in g_screen.GetCellMappingTable())
	{
		std::stringstream ss;
		ss << "RECT " << num;

		// ȭ�鿡 ���� ����Ѵ�.
		setLabel(screen, ss.str(), table.screenCell2, Scalar(0,0,255));
		line(screen, table.screenCell2[0], table.screenCell2[1], Scalar(0, 0, 255), 2);
		line(screen, table.screenCell2[1], table.screenCell2[2], Scalar(0, 0, 255), 2);
		line(screen, table.screenCell2[2], table.screenCell2[3], Scalar(0, 0, 255), 2);
		line(screen, table.screenCell2[3], table.screenCell2[0], Scalar(0, 0, 255), 2);

		
		vector<Point> rect(4);
		for (int i = 0; i < 4; ++i)
		{
			KeyPoint kpos;
			kpos.pt = Point(table.cameraCell[i].x, table.cameraCell[i].y);
			rect[i] = g_screen.GetPointPos(kpos, num);
		}

		setLabel(screen, ss.str(), rect);
		line(screen, rect[0], rect[1], Scalar(0, 255, 255), 1);
		line(screen, rect[1], rect[2], Scalar(0, 255, 255), 1);
		line(screen, rect[2], rect[3], Scalar(0, 255, 255), 1);
		line(screen, rect[3], rect[0], Scalar(0, 255, 255), 1);

		++num;
	}
	imshow("Screen", screen);
	//-------------------------------------------------------------------------------
	

	cvWaitKey();

	cvDestroyWindow("HUV05-camera");
	cvDestroyWindow("HUV05-binarization");
	cvDestroyWindow("HUV05-screen");
	cvDestroyWindow("Screen");
}
