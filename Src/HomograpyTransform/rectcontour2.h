//
// �簢 �ڽ��� ǥ���ϴ� Ŭ����.
// cRectContour Ŭ������ vector�� ���� �ʰ�, �迭�� ���� ��.
// OrderedContours() ��, �ε��� ��ȭ�� �����Ѵ�.
//
#pragma once


#include <vector>
#include<opencv2/opencv.hpp>
using std::vector;
typedef unsigned int u_int;


class cRectContour2
{
public:
	cRectContour2();
	cRectContour2(const vector<cv::Point> &contours);
	cRectContour2(const vector<cv::KeyPoint> &contours);
	cRectContour2(const cv::Point &center, const float size);
	virtual ~cRectContour2();

	bool Init(const cv::Point contours[4]);
	bool Init(const vector<cv::Point> &contours);
	bool Init(const vector<cv::Point2f> &contours);
	bool Init(const vector<cv::KeyPoint> &keypoints);
	bool Init(const cv::Point &center, const float size);
	void Normalize();
	void Draw(cv::Mat &dst, const cv::Scalar &color = cv::Scalar(0, 0, 0), const int thickness = 1);

	int Width() const;
	int Height() const;
	cv::Point At(const int index) const;
	cv::Point Center() const;
	void ScaleCenter(const float scale);
	void Scale(const float vscale, const float hscale);

	cRectContour2& operator = (const cRectContour2 &rhs);


public:
	cv::Point m_contours[4];
	int m_chIndices[4];
};
