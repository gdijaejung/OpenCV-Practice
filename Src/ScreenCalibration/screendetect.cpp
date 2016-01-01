
#include "global.h"
#include "screendetect.h"

using namespace cv;



// ��ũ���� �ν��ؼ� ũ��� ����Ʈ�� �����Ѵ�.
// ���� �ν����� ���ߴٸ� Rect(0,0,0,0) �� �����Ѵ�.
// isManualMode : true �̸�, threshold ���� ���� �����Ѵ�.
//				  false �̸�, �ڵ����� �����ȴ�.
Rect ScreenDetectAuto(std::vector<cv::Point> &out, const int minScreenArea, OUT int &outThreshold)
{
	Mat screen(g_screen.GetHeight(), g_screen.GetWidth(), CV_8UC(3));
	screen.setTo(Scalar(255, 255, 255));
	namedWindow("Screen", CV_WINDOW_NORMAL);
	cvSetWindowProperty("Screen", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);

	const bool startBlank = true;
	const int w = g_screen.GetWidth();
	const int h = g_screen.GetHeight();
	const int div = 32;
	imshow("Screen", screen);

	//-----------------------------------------------------------------------------------------
	// ī�޶� ���� ���� ���.
	int threshold = 33; // �Ӱ谪(Threshold) ����
	bool upThreashold = false;
	bool detectScreen = false;
	int detectScreenCount = 0;
	int detectThreshold = threshold;
	std::vector<cv::Point> screenLines(4);
	Rect screenRect;

	IplImage *camera = 0;
	IplImage *binImage = 0;
	IplImage *binOutput = 0;
	cvNamedWindow("HUV05-camera", 0);
	cvNamedWindow("HUV05-binarization", 0);
	cvNamedWindow("HUV05-screen", 0);

	cvResizeWindow("HUV05-camera", 640, 480);
	cvResizeWindow("HUV05-binarization", 640, 480);
	cvResizeWindow("HUV05-screen", 640, 480);

	cvMoveWindow("HUV05-camera", -660, 0);
	cvMoveWindow("HUV05-binarization", -660, 500);
	cvMoveWindow("HUV05-screen", -1320, 500);
	cvCreateTrackbar("Threshold", "HUV05-camera", &threshold, 255, NULL); // "HUV05-camera" �����쿡 bar �� ���δ�. 

	while (1) {
		camera = g_camera.GetCapture();
		if (!camera)
			continue;

		// ī�޶� ������ ����ȭ �Ѵ�.
		if (!binImage) {
			binImage = cvCreateImage({ 640, 480 }, IPL_DEPTH_8U, 1); // ��� �̹��� ����
			binOutput = cvCreateImage({ 640, 480 }, IPL_DEPTH_8U, 1); // ��� �̹��� ����
		}

		cvCvtColor(camera, binImage, CV_RGB2GRAY); // �÷��� ������� ��ȯ
		// ������ �� �ȼ�(x,y) ���� threshold ���� �ʰ��� 255 ��, �� ���ϴ� 0 ���� ��ȯ
		cvThreshold(binImage, binOutput, threshold, 255, CV_THRESH_BINARY);
		binOutput->origin = camera->origin; // ������ ������ ������ �ٷ� �����


		if (cvWaitKey(10) >= 0)
			break;

		cvShowImage("HUV05-camera", camera);
		cvShowImage("HUV05-binarization", binOutput);
		//imshow("HUV05-binarization", bw);


		//----------------------------------------------------------------------------------------------
		// �ܰ��� �̾Ƴ���.
		Mat src(camera);

		// Use Canny instead of threshold to catch squares with gradient shading
		cv::Mat bw;
		cv::Canny(Mat(binOutput), bw, 0, 50, 5);

		// Find contours
		std::vector<std::vector<cv::Point> > contours;
		cv::findContours(bw.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

		std::vector<cv::Point> approx;
		cv::Mat dst(camera);// src.clone();

		int rectIndex = -1;
		bool findMask = false;
		Rect bigScreen;

		for (u_int i = 0; i < contours.size(); i++)
		{
			// Approximate contour with accuracy proportional
			// to the contour perimeter
			cv::approxPolyDP(cv::Mat(contours[i]), approx, cv::arcLength(cv::Mat(contours[i]), true)*0.02, true);

			// Skip small or non-convex objects 
			if (std::fabs(cv::contourArea(contours[i])) < 100 || !cv::isContourConvex(approx))
				continue;

			if (approx.size() == 4) // �簢���� ã�´�.
			{
				// Number of vertices of polygonal curve
				int vtc = approx.size();

				// Get the cosines of all corners
				std::vector<double> cos;
				for (int j = 2; j < vtc + 1; j++)
					cos.push_back(angle(approx[j%vtc], approx[j - 2], approx[j - 1]));

				// Sort ascending the cosine values
				std::sort(cos.begin(), cos.end());

				// Get the lowest and the highest cosine
				double mincos = cos.front();
				double maxcos = cos.back();

				// Use the degrees obtained above and the number of vertices
				// to determine the shape of the contour
				if (vtc == 4 && mincos >= -0.1f && maxcos <= 0.8f)
				{
					setLabel(dst, "RECT", contours[i]);

					line(dst, approx[0], approx[1], Scalar(255, 0, 0), 2);
					line(dst, approx[1], approx[2], Scalar(255, 0, 0), 2);
					line(dst, approx[2], approx[3], Scalar(255, 0, 0), 2);
					line(dst, approx[3], approx[0], Scalar(255, 0, 0), 2);

					// ���� ū �ڽ��� ã�´�.
					Point center = approx[0];
					center += approx[1];
					center += approx[2];
					center += approx[3];

					// �߽��� ���
					center.x /= 4;
					center.y /= 4;

					// �ð�������� left top ���� ������� 0, 1, 2, 3 ��ġ�� ����Ʈ�� ����ȴ�.
					for (int k = 0; k < 4; ++k)
					{
						if ((approx[k].x < center.x) && (approx[k].y < center.y))
							screenLines[0] = approx[k];
						if ((approx[k].x < center.x) && (approx[k].y > center.y))
							screenLines[3] = approx[k];
						if ((approx[k].x > center.x) && (approx[k].y < center.y))
							screenLines[1] = approx[k];
						if ((approx[k].x > center.x) && (approx[k].y > center.y))
							screenLines[2] = approx[k];
					}

					Rect rect((int)screenLines[0].x, (int)screenLines[0].y,
						(int)(screenLines[1].x - screenLines[0].x),
						(int)(screenLines[2].y - screenLines[0].y));

					if ((rect.width * rect.height) > (bigScreen.width * bigScreen.height))
					{
						bigScreen = rect;

						// bigScreen ũ�Ⱑ ������� Ŀ�� �Ѵ�. �۴ٸ� ã�� ���Ѱɷ� �����Ѵ�.
						if (rect.width * rect.height > minScreenArea)
							rectIndex = i;
					}
					//break;
				}
			}
		}

		// �ڽ��� ã�� ���ߴٸ�, threshold�� �����.
		if (!detectScreen && (rectIndex < 0))
		{
			if (upThreashold)
			{
				threshold = min(threshold + 1, 255);
				if (threshold >= 255)
					upThreashold = false;
			}
			else
			{
				threshold = max(threshold - 1, 0);
				if (threshold <= 0)
					upThreashold = true;
			}

			setTrackbarPos("Threshold", "HUV05-camera", threshold);
		}

		// �ڽ��� ã��, threshold ���� �����ϴٸ�, detectScreenCount�� ������Ų��.
		if (!detectScreen)
		{
			if ((rectIndex >= 0) && (threshold == detectThreshold))
			{
				++detectScreenCount;

				// ���������� ��ũ�� ��ġ�� ã�Ҵٸ�, threshold�� �����ϴ� ���� �����.				
				if (detectScreenCount > 30)
				{
					detectScreen = true;
					screenRect = bigScreen;
				}
			}
			else
			{
				detectScreenCount = 1;
			}

			// �ڽ��� ã�Ҵٸ�, ���� threshold ���� �����Ѵ�.
			if (rectIndex >= 0)
			{
				detectThreshold = threshold;
			}
		}

		if (detectScreen)
			break;

		imshow("HUV05-screen", dst);
	}


	cvDestroyWindow("HUV05-camera");
	cvDestroyWindow("HUV05-binarization");
	cvDestroyWindow("HUV05-screen");
	cvDestroyWindow("Screen");

	out = screenLines;
	outThreshold = threshold;
	return screenRect;
}



// ������ ��ũ���� ã�´�.
cv::Rect ScreenDetectManual(std::vector<Point> &out,
	const int minScreenArea, OUT int &outThreshold)
{
	Mat screen(g_screen.GetHeight(), g_screen.GetWidth(), CV_8UC(3));
	screen.setTo(Scalar(255, 255, 255));
	namedWindow("Screen", CV_WINDOW_NORMAL);
	cvSetWindowProperty("Screen", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);

	const bool startBlank = true;
	const int w = g_screen.GetWidth();
	const int h = g_screen.GetHeight();
	const int div = 32;
	imshow("Screen", screen);

	//-----------------------------------------------------------------------------------------
	// ī�޶� ���� ���� ���.
	int threshold = 33; // �Ӱ谪(Threshold) ����
	bool detectScreen = false;
	int detectScreenCount = 0;
	std::vector<cv::Point> screenLines(4);
	Rect screenRect;

	IplImage *camera = 0;
	IplImage *binImage = 0;
	IplImage *binOutput = 0;
	cvNamedWindow("HUV05-camera", 0);
	cvNamedWindow("HUV05-binarization", 0);
	cvNamedWindow("HUV05-screen", 0);

	cvResizeWindow("HUV05-camera", 640, 480);
	cvResizeWindow("HUV05-binarization", 640, 480);
	cvResizeWindow("HUV05-screen", 640, 480);

	cvMoveWindow("HUV05-camera", -660, 0);
	cvMoveWindow("HUV05-binarization", -660, 500);
	cvMoveWindow("HUV05-screen", -1320, 500);
	cvCreateTrackbar("Threshold", "HUV05-camera", &threshold, 255, NULL); // "HUV05-camera" �����쿡 bar �� ���δ�. 

	while (1) {
		camera = g_camera.GetCapture();
		if (!camera)
			continue;

		// ī�޶� ������ ����ȭ �Ѵ�.
		if (!binImage) {
			binImage = cvCreateImage({ 640, 480 }, IPL_DEPTH_8U, 1); // ��� �̹��� ����
			binOutput = cvCreateImage({ 640, 480 }, IPL_DEPTH_8U, 1); // ��� �̹��� ����
		}

		cvCvtColor(camera, binImage, CV_RGB2GRAY); // �÷��� ������� ��ȯ
		// ������ �� �ȼ�(x,y) ���� threshold ���� �ʰ��� 255 ��, �� ���ϴ� 0 ���� ��ȯ
		cvThreshold(binImage, binOutput, threshold, 255, CV_THRESH_BINARY);
		binOutput->origin = camera->origin; // ������ ������ ������ �ٷ� �����


		if (cvWaitKey(10) >= 0)
			break;

		cvShowImage("HUV05-camera", camera);
		cvShowImage("HUV05-binarization", binOutput);
		//imshow("HUV05-binarization", bw);


		//----------------------------------------------------------------------------------------------
		// �ܰ��� �̾Ƴ���.
		Mat src(camera);

		// Use Canny instead of threshold to catch squares with gradient shading
		cv::Mat bw;
		cv::Canny(Mat(binOutput), bw, 0, 50, 5);

		// Find contours
		std::vector<std::vector<cv::Point> > contours;
		cv::findContours(bw.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

		std::vector<cv::Point> approx;
		cv::Mat dst(camera);// src.clone();

		int rectIndex = -1;
		bool findMask = false;
		Rect bigScreen;

		for (u_int i = 0; i < contours.size(); i++)
		{
			// Approximate contour with accuracy proportional
			// to the contour perimeter
			cv::approxPolyDP(cv::Mat(contours[i]), approx, cv::arcLength(cv::Mat(contours[i]), true)*0.02, true);

			// Skip small or non-convex objects 
			if (std::fabs(cv::contourArea(contours[i])) < 100 || !cv::isContourConvex(approx))
				continue;

			if (approx.size() == 4) // �簢���� ã�´�.
			{
				// Number of vertices of polygonal curve
				int vtc = approx.size();

				// Get the cosines of all corners
				std::vector<double> cos;
				for (int j = 2; j < vtc + 1; j++)
					cos.push_back(angle(approx[j%vtc], approx[j - 2], approx[j - 1]));

				// Sort ascending the cosine values
				std::sort(cos.begin(), cos.end());

				// Get the lowest and the highest cosine
				double mincos = cos.front();
				double maxcos = cos.back();

				// Use the degrees obtained above and the number of vertices
				// to determine the shape of the contour
				if (vtc == 4 && mincos >= -0.1f && maxcos <= 0.8f)
				{
					// ���� ū �ڽ��� ã�´�.
					Point center = approx[0];
					center += approx[1];
					center += approx[2];
					center += approx[3];

					// �߽��� ���
					center.x /= 4;
					center.y /= 4;

					vector<cv::Point> lines(4);
					// �ð�������� left top ���� ������� 0, 1, 2, 3 ��ġ�� ����Ʈ�� ����ȴ�.
					for (int k = 0; k < 4; ++k)
					{
						if ((approx[k].x < center.x) && (approx[k].y < center.y))
							lines[0] = approx[k];
						if ((approx[k].x < center.x) && (approx[k].y > center.y))
							lines[3] = approx[k];
						if ((approx[k].x > center.x) && (approx[k].y < center.y))
							lines[1] = approx[k];
						if ((approx[k].x > center.x) && (approx[k].y > center.y))
							lines[2] = approx[k];
					}

					Rect rect((int)lines[0].x, (int)lines[0].y,
						(int)(lines[1].x - lines[0].x),
						(int)(lines[2].y - lines[0].y));

					if ((rect.width * rect.height) > (bigScreen.width * bigScreen.height))
					{
						bigScreen = rect;

						// bigScreen ũ�Ⱑ ������� Ŀ�� �Ѵ�. �۴ٸ� ã�� ���Ѱɷ� �����Ѵ�.
						// �ּ� 200 X 200 ���� �����Ѵ�.
						if (rect.width * rect.height > minScreenArea)
						{
							rectIndex = i;
							screenLines = lines;
						}
					}
					//break;
				}
			}
		}


		// �ڽ��� ã��, threshold ���� �����ϴٸ�, detectScreenCount�� ������Ų��.
		if (!detectScreen)
		{
			if (rectIndex >= 0)
			{
				++detectScreenCount;

				// ���������� ��ũ�� ��ġ�� ã�Ҵٸ�, threshold�� �����ϴ� ���� �����.				
				if (detectScreenCount > 30)
				{
					detectScreen = true;
					screenRect = bigScreen;
				}
			}
			else
			{
				detectScreenCount = 1;
			}
		}

		if (detectScreen)
			break;


		setLabel(dst, "Screen", screenLines);
		line(dst, screenLines[0], screenLines[1], Scalar(255, 0, 0), 2);
		line(dst, screenLines[1], screenLines[2], Scalar(255, 0, 0), 2);
		line(dst, screenLines[2], screenLines[3], Scalar(255, 0, 0), 2);
		line(dst, screenLines[3], screenLines[0], Scalar(255, 0, 0), 2);


		imshow("HUV05-screen", dst);
	}

	cvWaitKey(1000);


	cvDestroyWindow("HUV05-camera");
	cvDestroyWindow("HUV05-binarization");
	cvDestroyWindow("HUV05-screen");
	cvDestroyWindow("Screen");

	out = screenLines;
	outThreshold = threshold;
	return screenRect;
}


