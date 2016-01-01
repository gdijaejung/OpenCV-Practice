
#include "global.h"
#include "detectrectpoint.h"

using namespace cv;


cDetectRectPoint::cDetectRectPoint()
{
	SimpleBlobDetector::Params params;

	// Change thresholds
	params.minThreshold = 1;
	params.maxThreshold = 20;

	// Filter by Area.
	params.filterByArea = true;
	params.minArea = g_config.m_detectPointMinArea;
	params.maxArea = g_config.m_detectPointMaxArea;

	// Filter by Circularity
	params.filterByCircularity = true;
	params.minCircularity = 0.1f;

	// Filter by Convexity
	params.filterByConvexity = true;
	params.minConvexity = 0.87f;

	// Filter by Inertia
	params.filterByInertia = true;
	params.minInertiaRatio = 0.01f;

	Init(params);
}

cDetectRectPoint::~cDetectRectPoint()
{

}


// �Ķ���� �ʱ�ȭ.
void cDetectRectPoint::Init(const SimpleBlobDetector::Params &params)
{
	m_params = params;
}


// �簢�� ����Ʈ�� �ν��Ѵ�.
// �νĿ� �����ϸ� true�� �����Ѵ�.
bool cDetectRectPoint::Detect(cCamera &camera0, const Mat &skewTransform)
{
	bool isSearch = true;
	int detectCount = 0;
	int thresholdValue = g_config.m_binaryThreshold; // �Ӱ谪(Threshold) ����
	std::vector<Point> detectPoints(4);

	cvNamedWindow("HUV05-binarization", 0);
	cvResizeWindow("HUV05-binarization", 640, 480);
	cvMoveWindow("HUV05-binarization", -660, 520);
	cvCreateTrackbar("Threshold", "HUV05-binarization", &thresholdValue, 255, NULL);


	Mat src(camera0.Resolution().height, camera0.Resolution().width, CV_8UC(3));
	while (isSearch) 
	{
		Mat &cam = camera0.ShowCaptureUndistortion(skewTransform, src);
		if (cam.empty())
			continue;

		Mat grayscaleMat = src.clone();
		cvtColor(src, grayscaleMat, CV_RGB2GRAY); // �÷��� ������� ��ȯ

		Mat binaryMat(grayscaleMat.size(), grayscaleMat.type());
		cv::threshold(grayscaleMat, binaryMat, thresholdValue, 255, cv::THRESH_BINARY);


		//----------------------------------------------------------------------------------------------
		//  �̹������� �����͸� ã�´�.
		SimpleBlobDetector detector(m_params);

		// Detect blobs.
		std::vector<KeyPoint> keypoints;
		detector.detect(binaryMat, keypoints);
	
		// Draw detected blobs as red circles.
		// DrawMatchesFlags::DRAW_RICH_KEYPOINTS flag ensures the size of the circle corresponds to the size of blob
		Mat im_with_keypoints;
		drawKeypoints(binaryMat, keypoints, im_with_keypoints, Scalar::all(-1), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
		imshow("HUV05-binarization", im_with_keypoints);

		if (keypoints.size() == 4)
		{
			// 4���� ����Ʈ�� ���������� ã����, ��ġ�� �����ϰ�, Detect�� �����Ѵ�.
			++detectCount;
			if (detectCount > 20)
			{
				isSearch = false;

				// 4���� ����Ʈ�� ������� ���߾� �����Ѵ�.
				vector<Point> keys(4);
				for (u_int i = 0; i < keypoints.size(); ++i)
					keys[i] = Point(keypoints[i].pt);
				OrderedContours(keys, detectPoints);

				DrawLines(im_with_keypoints, detectPoints, Scalar(0, 255, 0), 2);
				imshow("HUV05-binarization", im_with_keypoints);
			}
		}
		else
		{
			detectCount = 0;
		}

		if (cvWaitKey(10) >= 0)
			break;
	}

	cvDestroyWindow("HUV05-binarization");

	m_contour = detectPoints;

	return true;
}


// Skew ��ȯ ����� ����ؼ� �����Ѵ�.
// scale �� ��ŭ m_contour �� �÷� ��´�.
Mat cDetectRectPoint::SkewTransform(const int width, const int height, const float scale)
{
	return ::SkewTransform(m_contour, width, height, scale);
}
