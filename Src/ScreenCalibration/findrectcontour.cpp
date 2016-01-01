
#include "global.h"
#include "findrectcontour.h"

using namespace cv;



// ���󿡼� rect�� ã�� �����Ѵ�.
bool FindRectContour(IplImage *image, IplImage *binImage, 
	const int minArea, const int maxArea,
	OUT vector<cv::Point> &out)
{
	std::vector<cv::Point> rectLines(4);

	Mat src(image);

	// Use Canny instead of threshold to catch squares with gradient shading
	cv::Mat bw;
	cv::Canny(Mat(binImage), bw, 0, 50, 5);

	// Find contours
	std::vector<std::vector<cv::Point> > contours;
	cv::findContours(bw.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	std::vector<cv::Point> approx;
	//cv::Mat dst = src.clone();

	int rectIndex = -1;
	Rect bigRect;

	for (u_int i = 0; i < contours.size(); i++)
	{
		// Approximate contour with accuracy proportional
		// to the contour perimeter
		cv::approxPolyDP(cv::Mat(contours[i]), approx, cv::arcLength(cv::Mat(contours[i]), true)*0.02, true);

		// Skip small or non-convex objects 
		if (std::fabs(cv::contourArea(contours[i])) < 100 || !cv::isContourConvex(approx))
			continue;

		if (approx.size() >= 4) // �簢���� ã�´�.
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
			//if (vtc >= 4 && mincos >= -0.1 && maxcos <= 0.3)
			if (vtc >= 4 && mincos >= -0.25f && maxcos <= 0.8f)
			{
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
						rectLines[0] = approx[k];
					if ((approx[k].x < center.x) && (approx[k].y > center.y))
						rectLines[3] = approx[k];
					if ((approx[k].x > center.x) && (approx[k].y < center.y))
						rectLines[1] = approx[k];
					if ((approx[k].x > center.x) && (approx[k].y > center.y))
						rectLines[2] = approx[k];
				}

				Rect rect((int)rectLines[0].x, (int)rectLines[0].y,
					(int)(rectLines[1].x - rectLines[0].x),
					(int)(rectLines[2].y - rectLines[0].y));

				// �簢���� ���� ���� �ȿ� ����� ��..
				if (((rect.width * rect.height) > minArea) &&
					((rect.width * rect.height) < maxArea))
				{
					// ���� ū Rect�� ã�Ƽ� �����Ѵ�.
					if ((rect.width * rect.height) > (bigRect.width * bigRect.height))
					{
						rectIndex = i;
						bigRect = rect;
					}
				}
			}
		}
	}

	if (rectIndex < 0)
		return false;

	out = rectLines;
	return true;
}
