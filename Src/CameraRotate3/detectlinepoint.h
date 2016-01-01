//
// ������ ��Ÿ���� 2�� �̻��� ����Ʈ�� �ν��ؼ� Skew transform�� ����Ѵ�.
//
#pragma once


class cDetectLinePoint
{
public:
	cDetectLinePoint();
	virtual ~cDetectLinePoint();

	void Init(const SimpleBlobDetector::Params &params);
	bool Detect(cCamera &camera0, const Mat &skewTransform);
	const vector<cv::Point>& GetDetectPoints();
	Mat SkewTransform(const int width, const int height, const float scaleW, const float scaleH);


protected:
	// 0 --- 1 --- 2 --- N
	vector<cv::Point> m_contour;	// Line ������ (�ε��� ����)

	SimpleBlobDetector::Params m_params;
};


inline const vector<cv::Point>& cDetectLinePoint::GetDetectPoints() {
	return m_contour;
}
