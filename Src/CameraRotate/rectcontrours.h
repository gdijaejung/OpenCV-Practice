//
// 사각 박스를 표현하는 클래스.
//
//
#pragma once


class cRectContours
{
public:
	cRectContours();
	cRectContours(const vector<cv::Point> &contours);
	cRectContours(const vector<cv::KeyPoint> &contours);
	virtual ~cRectContours();

	bool Init(const vector<cv::Point> &contours);
	bool Init(const vector<cv::KeyPoint> &keypoints);
	bool Init(const cv::Point &center, const float size);
	void Draw(cv::Mat &dst, const cv::Scalar &color=cv::Scalar(0,0,0), const int thickness=1);

	cv::Point Center() const;
	void ScaleCenter(const float scale);


protected:
	vector<cv::Point> m_contours;
};
