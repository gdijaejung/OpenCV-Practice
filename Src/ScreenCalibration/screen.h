//
// ��ũ���� ���� �� �۾��� ó���Ѵ�.
// �ν��� ��ũ���� ũ���, ���� �������� ��µǴ� ��ũ���� ũ�� ������ �����Ѵ�.
// 
// ī�޶��� ���� Ư���� ���� ��ũ���� ������� �����ϴ� ������ �Ѵ�.
//
#pragma once

struct sCellMapping
{
	cv::Rect screenCell; // ȭ�鿡�� ����ϴ� cell ��ġ
	vector<cv::Point> screenCell2; // ȭ�鿡�� ����ϴ� cell ��ġ
	vector<cv::Point> cameraCell; // ī�޶󿡼� �ν��� cell ��ġ
	cv::Point2f errorLength;  // ī�޶󿡼� �ν��� ��ġ�� �����ϱ� ���� ��.
	int errorCount; // errorLength ���� ����ϱ� ���� ī��Ʈ�� (�ο��н�����)
	cv::Mat tm;
};


class cCapture;
class cScreen
{
public:
	cScreen();
	virtual ~cScreen();

	void Init(const cv::Rect &screenResolution, const vector<cv::Point> &screenContour, 
		const int increaseScreenSize = 25);

	void InitResolution(const cv::Rect &screenResolution);

	cv::Mat& GetWarpScreen(cCapture &capture);

	int GetWidth();
	int GetHeight();
	int GetRecogWidth();
	int GetRecogHeight();
	pair<float,float> GetResolutionRecognitionRate();

	const cv::Rect& GetScreenResolution() const;
	const vector<cv::Point>& GetScreenContour() const;

	void CreateCellMappingTable(const int cols, const int rows);
	void SetCellMappingTable(const int index, const cv::Rect &screenCell,
		const vector<cv::Point> &cameraCell);
	const vector<sCellMapping>& GetCellMappingTable() const;
	void CalculateCellMapping();
	int GetCellIndex(const cv::KeyPoint &point);
	cv::Point GetPointPos(const cv::KeyPoint &point);
	cv::Point GetPointPos(const cv::KeyPoint &point, const int index);
	void SetMappingError(const cv::KeyPoint &point, const cv::Point &errorPos);

	bool IsInContour(const vector<cv::Point> &contour, const cv::Point2f &pos, OUT float &out);


protected:
	cv::Rect m_screenResolution; // {0,0,w,h}
	vector<cv::Point> m_screenContour;	// �νĵ� ��ũ���� ũ�� (�ε��� ����)
										// 0 ---------- 1
										// |            |
										// |            |
										// |            |
										// 2 ---------- 3
										//

	int m_increaseScreenLength; // �νĵ� ��ũ������ ����,���� ������ų ���� �����Ѵ�.


	int m_cellCols;
	int m_cellRows;
	vector<sCellMapping> m_cellMapping; // ��ũ���� ����ϴ� �������� ī�޶󿡼� �ν��ϴ� �������� �����ϰ� �����Ѵ�.
	cv::Mat m_warpScreen;

};


inline const cv::Rect& cScreen::GetScreenResolution() const { return m_screenResolution; }
inline const vector<cv::Point>& cScreen::GetScreenContour() const { return m_screenContour; }
inline const vector<sCellMapping>& cScreen::GetCellMappingTable() const { return m_cellMapping; }
