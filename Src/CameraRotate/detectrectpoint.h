//
// �簢���� ��Ÿ���� 4���� ����Ʈ�� �ν��ؼ� Skew transform�� ����Ѵ�.
//
#pragma once


class cDetectRectPoint
{
public:
	cDetectRectPoint();
	virtual ~cDetectRectPoint();

	void Init(const SimpleBlobDetector::Params &params);
	bool Detect(cCamera &camera0, const Mat &skewTransform);
	const vector<cv::Point>& GetDetectPoints();
	Mat SkewTransform(const int width, const int height, const float scale=1);


protected:
	vector<cv::Point> m_contour;	// Rect ������ (�ε��� ����)
	// 0 ---------- 1
	// |            |
	// |            |
	// |            |
	// 2 ---------- 3

	SimpleBlobDetector::Params m_params;
};


inline const vector<cv::Point>& cDetectRectPoint::GetDetectPoints() {
	return m_contour;
}
