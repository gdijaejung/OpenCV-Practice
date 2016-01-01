//
// 2015-11-29, jjuiddong
//
// ��Ŷ�� �����Ѵ�. Thread Safe 
// �ִ��� �����ϰ� �������.
//
// ť�� ��������, ���� ������ ��Ŷ�� �����ϰ�, �߰��Ѵ�.
// �� ���ϸ��� packetSize ũ�⸸ŭ ä�� ������ �������� �Ѿ�� �ʴ´�.
//
// ��Ŷ�� ť�� ������ ��, ���(sHeader)�� �߰��ȴ�.
//
#pragma once


namespace network
{
	struct sPacket
	{
		SOCKET sock; // ���� ����
		BYTE *buffer;
		int totalLen;
		bool full; // ���۰� �� ä������ true�� �ȴ�.
		int readLen;
		int actualLen; // ���� ��Ŷ�� ũ�⸦ ��Ÿ����. totalLen - sizeof(sHeader)
	};

	struct sSession;
	class cPacketQueue
	{
	public:
		cPacketQueue();
		virtual ~cPacketQueue();

		struct sHeader
		{
			BYTE head[2]; // $@
			BYTE protocol; // protocol id
			int length;	// packet length (byte)
		};

		bool Init(const int packetSize, const int maxPacketCount);
		void Push(const SOCKET sock, const BYTE *data, const int len, const bool fromNetwork=false);
		bool Front(OUT sPacket &out);
		void Pop();
		void SendAll();
		void SendBroadcast(vector<sSession> &sessions, const bool exceptOwner = true);
		void Lock();
		void Unlock();

		vector<sPacket> m_queue;


	protected:
		//---------------------------------------------------------------------
		// Simple Queue Memory Pool
		BYTE* Alloc();
		void Free(BYTE*ptr);
		void ClearMemPool();
		void Clear();

		struct sChunk
		{
			bool used;
			BYTE *p;
		};
		vector<sChunk> m_memPool;
		BYTE *m_memPoolPtr;
		int m_packetBytes;	// ������ ��Ŷ ũ��
		int m_chunkBytes;	// sHeader ����� ������ ��Ŷ ũ��
		int m_totalChunkCount;
		CRITICAL_SECTION m_criticalSection;
	};
}
