#pragma once



// ���󿡼� rect�� ã�� �����Ѵ�.
bool FindRectContour(IplImage *image, IplImage *binImage, 
	const int minArea, const int maxArea, OUT vector<cv::Point> &out);


