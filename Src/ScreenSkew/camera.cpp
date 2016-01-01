
#include "global.h"
#include "camera.h"

cCamera g_camera;

using namespace cv;


cCamera::cCamera() :
	m_capture(NULL)
{
	// ī�޶� ���� �ְ� ���� �Ķ����
	const double cx = 346.982762f;
	const double cy = 229.626156f;
	const double fx = 676.748756f;
	const double fy = 677.032333f;
	const double k1 = -0.331818f;
	const double k2 = -0.033314f;
	const double k3 = 0.f;
	const double p1 = -0.000313f;
	const double p2 = 0.000841f;
	const double skew_c = 0;
	m_camDistort.SetParams(cx, cy, fx, fy, k1, k2, k3, p1, p2, skew_c);
}

cCamera::~cCamera()
{
	cvDestroyWindow("HUV05-camera");
	cvReleaseCapture(&m_capture);
}


// ī�޶� �ʱ�ȭ.
bool cCamera::Init(const int index) // index=0
{
	m_capture = cvCaptureFromCAM(index);

	cvNamedWindow("HUV05-camera", 0);
	cvResizeWindow("HUV05-camera", 640, 480);
	cvMoveWindow("HUV05-camera", -660, 0);
	return true;
}


// ī�޶� �ν��� ������ �����´�.
IplImage* cCamera::GetCapture()
{
	if (!m_capture)
		return NULL;

	return cvQueryFrame(m_capture);
}


// ī�޶� ��� ���� �ְ���� �ʴ� ������ �����Ѵ�.
Mat& cCamera::GetCaptureUndistortion()
{
	IplImage *image = cvQueryFrame(m_capture);
	if (!image)
		return m_undistortCapture;

	Mat src(image);	
	if (m_undistortCapture.empty())
		m_undistortCapture = src.clone();

	m_camDistort.UndistortImage(src, m_undistortCapture);

	return m_undistortCapture;
}


// ī�޶� �Կ��� ������ �����쿡 ����Ѵ�.
IplImage* cCamera::ShowCapture()
{
	IplImage *image = GetCapture();
	if (!image)
		return NULL;

	cvShowImage("HUV05-camera", image);
	return image;
}


// ī�޶� �Կ��� ������ �����Ͽ� �����쿡 ����Ѵ�.
Mat& cCamera::ShowCaptureUndistortion()
{
	Mat &image = GetCaptureUndistortion();
	if (image.empty())
		return image;

	imshow("HUV05-camera", image);
	return image;
}
