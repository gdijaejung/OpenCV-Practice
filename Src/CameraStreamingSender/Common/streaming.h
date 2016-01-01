//
// ��Ʈ���ֿ� ���õ� ������ �����Ѵ�.
//
#pragma once


namespace cvproc
{
	struct sStreamingData
	{
		unsigned char id;			// streaming id (���� ���̵𳢸� ����Ÿ�� ��ģ �Ŀ� ����Ѵ�.)
		unsigned char chunkSize;	// chunk size (�̹��� �뷮 ������ ������ ����Ÿ ûũ�� ����)
		unsigned char chunkIndex;	// chunk index
		unsigned char isGray;		// gray 0 : 1
		unsigned char isCompressed;	// jpeg compressed 0 : 1
		int imageBytes;				// image size (byte unit)
		char *data;					// image data
	};


	const static int g_maxStreamSize = (int)pow(2, 15) - 1;
}
