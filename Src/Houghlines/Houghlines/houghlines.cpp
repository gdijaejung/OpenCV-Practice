
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#define PI 3.1415926

#pragma comment(lib, "opencv_core2411d.lib")
#pragma comment(lib, "opencv_highgui2411d.lib")
#pragma comment(lib, "opencv_imgproc2411d.lib")


int main()
{
	cv::Mat image = cv::imread("road.jpg", 0);
	cv::namedWindow("Original Image");
	cv::imshow("Original Image", image);

	// ĳ�� �˰��� ����
	cv::Mat contours;
	cv::Canny(image, contours, 150, 200);

	// �� ���� ���� ���� ��ȯ
	std::vector<cv::Vec2f> lines;
	cv::HoughLines(contours, lines,
		1, PI / 180, // �ܰ躰 ũ��
		300);
		//80);  // ��ǥ(vote) �ִ� ����

			  // �� �׸���
	cv::Mat result(contours.rows, contours.cols, CV_8U, cv::Scalar(255));
	std::cout << "Lines detected: " << lines.size() << std::endl;

	// �� ���͸� �ݺ��� �� �׸���
	std::vector<cv::Vec2f>::const_iterator it = lines.begin();
	while (it != lines.end()) {
		float rho = (*it)[0];   // ù ��° ��Ҵ� rho �Ÿ�
		float theta = (*it)[1]; // �� ��° ��Ҵ� ��Ÿ ����
		if (theta < PI / 4. || theta > 3.*PI / 4.) { // ���� ��
			cv::Point pt1(rho / cos(theta), 0); // ù �࿡�� �ش� ���� ������   
			cv::Point pt2((rho - result.rows*sin(theta)) / cos(theta), result.rows);
			// ������ �࿡�� �ش� ���� ������
			cv::line(image, pt1, pt2, cv::Scalar(255), 1); // �Ͼ� ������ �׸���

		}
		else { // ���� ��
			cv::Point pt1(0, rho / sin(theta)); // ù ��° ������ �ش� ���� ������  
			cv::Point pt2(result.cols, (rho - result.cols*cos(theta)) / sin(theta));
			// ������ ������ �ش� ���� ������
			cv::line(image, pt1, pt2, cv::Scalar(255), 1); // �Ͼ� ������ �׸���
		}
		std::cout << "line: (" << rho << "," << theta << ")\n";
		++it;
	}

	cv::namedWindow("Detected Lines with Hough");
	cv::imshow("Detected Lines with Hough", image);

	cv::waitKey(0);

	return 0;
}

