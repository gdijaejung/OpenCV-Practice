
#include "global.h"
#include "linecontour.h"


using namespace cv;


cLineContour::cLineContour() :
m_contours(4)
{

}

cLineContour::cLineContour(const vector<cv::Point> &contours) :
m_contours(4)
{
	Init(contours);
}

cLineContour::cLineContour(const vector<cv::KeyPoint> &contours) :
m_contours(4)
{
	Init(contours);
}


cLineContour::cLineContour(const cv::Point &center, const float size) :
m_contours(4)
{
	Init(center, size);
}

cLineContour::~cLineContour()
{

}


// �ʱ�ȭ.
bool cLineContour::Init(const vector<cv::Point> &contours)
{
	OrderedLineContours(contours, m_contours);
	return true;
}


// �ʱ�ȭ.
bool cLineContour::Init(const vector<cv::KeyPoint> &keypoints)
{
	if (keypoints.size() < 3)
		return false;

	vector<Point> keys(keypoints.size());
	for (u_int i = 0; i < keypoints.size(); ++i)
		keys[i] = Point(keypoints[i].pt);

	OrderedLineContours(keys, m_contours);

	return true;
}


// ���� center�� �߽����� size��ŭ Ű���� ���� �����.
//
// 0 -- 1 ---- 2 ---- + ----- 3 ----- 4
bool cLineContour::Init(const cv::Point &center, const float size)
{
	m_contours[0] = center + Point(-(int)(size / 2.f), 0);
	m_contours[1] = center + Point(-(int)(size / 4.f), 0);
	m_contours[2] = center + Point((int)(size / 4.f), 0);
	m_contours[3] = center + Point((int)(size / 2.f), 0);
	return true;
}


// �ڽ� ���.
void cLineContour::Draw(cv::Mat &dst, const cv::Scalar &color, const int thickness)
// color = cv::Scalar(0, 0, 0), thickness = 1
{
	DrawLines(dst, m_contours, color, thickness, false);
}


// �簢���� ������ �����Ѵ�.
Point cLineContour::Center() const
{
	int cnt = 0;
	Point center;
	for each (auto &pt in m_contours)
	{
		if (pt != Point(0, 0))
		{
			center += pt;
			cnt++;
		}
	}

	if (cnt <= 0)
		return Point(0, 0);

	center = Point(center.x / cnt, center.y / cnt);
	return center;
}


// �簢���� ������ �߽����� �������Ѵ�.
// void cLineContour::ScaleCenter(const float scale)
// {
// 	const Point center = Center();
// 
// 	for (u_int i = 0; i < m_contours.size(); ++i)
// 	{
// 		m_contours[i] = center + ((m_contours[i] - center) * scale);
// 	}
// }
