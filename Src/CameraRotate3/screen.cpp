
#include "global.h"
#include "screen.h"

using namespace cv;

cScreen g_screen;


cScreen::cScreen() : 
	m_oldT(0)
{
}

cScreen::~cScreen()
{
	cvDestroyWindow("Screen");
}


// ��ũ�� �ػ󵵿� ���󿡼� �νĵ� ��ũ���� ũ�⸦ �����Ѵ�.
void cScreen::Init(const cv::Rect &screenResolution, const vector<cv::Point> &screenContour)
{
	m_screenResolution = screenResolution;
	m_screenContour = screenContour;

	namedWindow("Screen", 0);
	cvResizeWindow("Screen", screenResolution.width, screenResolution.height);

	m_screen = Mat(screenResolution.height, screenResolution.width, CV_8UC(3));
	m_screen.setTo(Scalar(255, 255, 255));
	imshow("Screen", m_screen);
//	namedWindow("Screen", CV_WINDOW_NORMAL);
//	cvSetWindowProperty("Screen", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
}


// ��ũ�� �ػ� ����.
void cScreen::InitResolution(const cv::Rect &screenResolution)
{
	m_screenResolution = screenResolution;

// 	namedWindow("Screen", 0);
// 	cvResizeWindow("Screen", screenResolution.width, screenResolution.height);
	namedWindow("Screen", CV_WINDOW_NORMAL);
	cvSetWindowProperty("Screen", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);

	m_screen = Mat(screenResolution.height, screenResolution.width, CV_8UC(3));
	m_screen.setTo(Scalar(255, 255, 255));
	imshow("Screen", m_screen);
}


// ȭ�鿡 �ֻ��ϴ� ��ũ���� �ػ��� �ʺ� �����Ѵ�.
int cScreen::GetWidth()
{
	return m_screenResolution.width;
}


// ȭ�鿡 �ֻ��ϴ� ��ũ���� �ػ��� ���̸� �����Ѵ�.
int cScreen::GetHeight()
{
	return m_screenResolution.height;
}


// ���󿡼� �νĵ� ��ũ���� �ʺ� �����Ѵ�.
int cScreen::GetRecogWidth()
{
	return m_screenContour[1].x - m_screenContour[0].x;
}


// ���󿡼� �νĵ� ��ũ���� ���̸� �����Ѵ�.
int cScreen::GetRecogHeight()
{
	return m_screenContour[3].y - m_screenContour[0].y;
}


// ȭ�� �ػ� �� ���� �ν��� ������ ������ �����Ѵ�.
// width, height ������� �����Ѵ�.
pair<float, float> cScreen::GetResolutionRecognitionRate()
{
	const float rate = (float)(GetRecogWidth() * GetRecogHeight()) / (float)(GetWidth() * GetHeight());
	return pair<float, float>(rate, rate);
}


// ü������ ������ ����Ѵ�.
void cScreen::ShowChessBoard(const int rows, const int cols)
{
	m_screen.setTo(Scalar(255, 255, 255));
	horizLines(m_screen, m_screenResolution.width, m_screenResolution.height, 5);
	imshow("Screen", m_screen);
}


// ī�޶� ��ũ���� ���� �־�� �Ѵ�.
// ��ũ���� �ν��ؼ�, ��ũ�� ũ�� ������ �����Ѵ�.
// minScreenArea : �ּ� ��ũ�� ����.
// ��ũ���� ã���� m_screenContour ���� ������Ʈ �ȴ�.
void cScreen::DetectScreen(cCamera &camera0, const float minScreenArea)
{
	int threshold = g_config.m_screen_BinaryThreshold;
	bool detectScreen = false;
	int detectScreenCount = 0;
	std::vector<cv::Point> screenLines(4);
	Rect screenRect;

	IplImage *camera = 0;
	IplImage *binImage = 0;
	IplImage *binOutput = 0;
	cvNamedWindow("HUV05-screen", 0);
	cvNamedWindow("HUV05-binarization", 0);
	cvResizeWindow("HUV05-binarization", 640, 480);
	cvResizeWindow("HUV05-screen", 640, 480);
	cvMoveWindow("HUV05-binarization", -660, 500);
	cvMoveWindow("HUV05-screen", -1320, 500);
	cvCreateTrackbar("Threshold", "HUV05-binarization", &threshold, 255, NULL);

	while (1) {
		Mat &camera1 = camera0.ShowCaptureUndistortion();
		if (camera1.empty())
			continue;
	
		IplImage camera2(camera1);
		camera = &camera2;


		// ī�޶� ������ ����ȭ �Ѵ�.
		if (!binImage) {
			binImage = cvCreateImage(camera0.Resolution(), IPL_DEPTH_8U, 1); // ��� �̹��� ����
			binOutput = cvCreateImage(camera0.Resolution(), IPL_DEPTH_8U, 1); // ��� �̹��� ����
		}

		cvCvtColor(camera, binImage, CV_RGB2GRAY); // �÷��� ������� ��ȯ
		// ������ �� �ȼ�(x,y) ���� threshold ���� �ʰ��� 255 ��, �� ���ϴ� 0 ���� ��ȯ
		cvThreshold(binImage, binOutput, threshold, 255, CV_THRESH_BINARY);
		binOutput->origin = camera->origin; // ������ ������ ������ �ٷ� �����

		if (cvWaitKey(10) >= 0)
			break;

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
				if (vtc == 4 && mincos >= -0.2f && maxcos <= 0.8f)
				{
 					vector<cv::Point> lines(4);
					OrderedContours(approx, lines);

					const Rect rect((int)lines[0].x, (int)lines[0].y,
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
				}
			}
		}


		if (!detectScreen)
		{
			if (rectIndex >= 0)
			{
				++detectScreenCount;

				// ���������� ��ũ�� ��ġ�� ã�Ҵٸ�, threshold�� �����ϴ� ���� �����.				
				if (detectScreenCount > 10)
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
		DrawLines(dst, screenLines, Scalar(255,0,0), 2);

		imshow("HUV05-screen", dst);
	}

	cvWaitKey(1000);

	cvDestroyWindow("HUV05-binarization");
	cvDestroyWindow("HUV05-screen");


 	m_screenContour = screenLines;
}


// Skew ��ȯ ����� ����ؼ� �����Ѵ�.
// scale �� ��ŭ m_screenContour �� �÷� ��´�.
Mat cScreen::SkewTransform(const int width, const int height, const float scale)
{
	return ::SkewTransform(m_screenContour, width, height, scale);
}



// ��ũ�� �ȿ��� ���� �����̴� �ִϸ��̼��� �Ѵ�.
void cScreen::ShowPingpong(const cv::Point2f &ballPos, const cv::Point2f &velocity, const cv::Size2f &ballSize)
{
	const int w = m_screenResolution.width;
	const int h = m_screenResolution.height;

	// ������ ������.
	const int curT = timeGetTime();
	float incT = (float)(curT - m_oldT) / 1000.f;
	if (incT > 1000.f)
	{
		// ó�� ���۵� ���.
		incT = 1.f;
		m_ballPos = ballPos;
		m_ballVelocity = velocity;
	}
	else if (incT > 0.1f)
	{
		// �ð� ������ �־��� ���.
		incT = 0.1f;
	}

	m_oldT = curT;


	const Point2f pointCurPos = m_ballPos + m_ballVelocity * incT;
	if (pointCurPos.x - ballSize.width/2 < 0 && (m_ballVelocity.x < 0))
		m_ballVelocity.x = -m_ballVelocity.x;
	else if (pointCurPos.x + ballSize.width/2 > w && (m_ballVelocity.x > 0))
		m_ballVelocity.x = -m_ballVelocity.x;
	if (pointCurPos.y - ballSize.height/2 < 0 && (m_ballVelocity.y < 0))
		m_ballVelocity.y = -m_ballVelocity.y;
	else if (pointCurPos.y + ballSize.height/2 > h && (m_ballVelocity.y > 0))
		m_ballVelocity.y = -m_ballVelocity.y;
	m_ballPos = pointCurPos;

	m_screen.setTo(Scalar(255, 255, 255));
	//ShowBoxLines(10, 10, false);
	ellipse(m_screen, RotatedRect(pointCurPos, ballSize, 0), Scalar(0, 0, 0), CV_FILLED);
	imshow("Screen", m_screen);
}


// ���ڹ��� ���� �׸���.
void cScreen::ShowBoxLines(const int rows, const int cols, const bool isClear)
{
	const int w = m_screenResolution.width;
	const int h = m_screenResolution.height;
	const int cx = w / cols;
	const int cy = h / rows;

	if (isClear)
		m_screen.setTo(Scalar(255, 255, 255));

	for (int x = 0; x <= w; x+=cx)
	{
		line(m_screen, Point(x, 0), Point(x, h), Scalar(0, 0, 0));
	}

	for (int y = 0; y <= h; y += cy)
	{
		line(m_screen, Point(0, y), Point(w, y), Scalar(0, 0, 0));
	}
}


// ����Ʈ�� ����Ѵ�.
void cScreen::ShowPoints(const vector<cv::Point2f> &points, const cv::Size2f &size, const cv::Scalar &color, 
	const bool isClear, const cv::Scalar &bgColor)
// color=Scalar(0,0,0), isClear = false, bgColor=cv::Scalar(255, 255, 255)
{
	if (isClear)
		m_screen.setTo(bgColor);

	for each(auto &pt in points)
	{
		ellipse(m_screen, RotatedRect(pt, size, 0), color, CV_FILLED);
	}
	imshow("Screen", m_screen);
}
