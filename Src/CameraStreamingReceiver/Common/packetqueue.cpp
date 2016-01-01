#include "stdafx.h"
#include "packetqueue.h"
#include "session.h"

using namespace network;


cPacketQueue::cPacketQueue()
	: m_memPoolPtr(NULL)
	, m_chunkBytes(0)
	, m_totalChunkCount(0)
{
	InitializeCriticalSectionAndSpinCount(&m_criticalSection, 0x00000400);
}

cPacketQueue::~cPacketQueue()
{
	Clear();

	DeleteCriticalSection(&m_criticalSection);
}


bool cPacketQueue::Init(const int packetSize, const int maxPacketCount)
{
	Clear();

	// Init Memory pool
	m_chunkBytes = packetSize;
	m_totalChunkCount = maxPacketCount;
	m_memPoolPtr = new BYTE[(packetSize+sizeof(sHeader)) * maxPacketCount];
	m_memPool.reserve(maxPacketCount);
	for (int i = 0; i < maxPacketCount; ++i)
		m_memPool.push_back({ false, m_memPoolPtr + (i*(packetSize + sizeof(sHeader))) });
	//

	m_queue.reserve(maxPacketCount);

	return true;
}


void cPacketQueue::Push(const SOCKET sock, const BYTE *data, const int len,
	const bool fromNetwork) // fromNetwork = false
{
	cAutoCS cs(m_criticalSection);

	bool isCopyBuffer = false;

	// find specific packet by socket
	for (u_int i = 0; i < m_queue.size(); ++i)
	{
		if (sock == m_queue[i].sock)
		{
			// �ش� ������ ä������ ���� ���۶��,
			if (m_queue[i].readLen < m_queue[i].totalLen)
			{
				const int copyLen = min(m_queue[i].totalLen - m_queue[i].readLen, len);
				memcpy(m_queue[i].buffer + m_queue[i].readLen, data, copyLen);
				m_queue[i].readLen += copyLen;

				if (m_queue[i].readLen == m_queue[i].totalLen)
					m_queue[i].full = true;

				isCopyBuffer = true;
				break;
			}
		}
	}

	// ���� �߰��� ������ ��Ŷ�̶��
	if (!isCopyBuffer)
	{
		sPacket packet;
		packet.sock = sock;
		packet.totalLen = 0;
		packet.readLen = len;

		int offset = 0;
		if (fromNetwork)
		{
			// ��Ʈ��ũ�� ���� �� ��Ŷ�̶��, �̹� ��Ŷ����� ���Ե� ���´�.
			// ��ü ��Ŷ ũ�⸦ �����ؼ�, �и��� ��Ŷ������ �Ǵ��Ѵ�.
			sHeader *pheader = (sHeader*)data;
			if ((pheader->head[0] == '$') && (pheader->head[1] == '@'))
			{
				packet.totalLen = pheader->length;
				packet.actualLen = pheader->length - sizeof(sHeader);

				if (len == packet.totalLen)
					packet.full = true;
			}
			else
			{
				// error occur!!
				// ��Ŷ�� ���ۺΰ� �ƴѵ�, ���ۺη� ������.
				// ����ΰ� �����ų�, ���� ���۰� Pop �Ǿ���.
				// �����ϰ� �����Ѵ�.
				return;
			}
		}

		packet.buffer = Alloc();

		if (!packet.buffer)
		{
			cs.Leave();
			Pop();		
			cs.Enter();
			packet.buffer = Alloc();
		}

		if (!packet.buffer)
			return; // Error!! 

		if (!fromNetwork)
		{
			// �۽��� ��Ŷ�� �߰��� ���, 
			// ��Ŷ ����� �߰��Ѵ�.
			packet.totalLen = len + sizeof(sHeader);
			packet.actualLen = len;
			packet.full = true;

			sHeader header;
			header.head[0] = '$';
			header.head[1] = '@';
			header.length = len + sizeof(sHeader);

			memcpy(packet.buffer, &header, sizeof(sHeader));
			offset += sizeof(header);
		}

		// ��Ŷ ���� ����
		memcpy(packet.buffer + offset, data, len);

		m_queue.push_back(packet);
	}
}


bool cPacketQueue::Front(OUT sPacket &out)
{
	cAutoCS cs(m_criticalSection);
	RETV(m_queue.empty(), false);
	RETV(!m_queue[0].full, false);

	out.sock = m_queue[0].sock;
	out.buffer = m_queue[0].buffer + sizeof(sHeader); // ����θ� ������ ��Ŷ ����Ÿ�� �����Ѵ�.
	out.readLen = m_queue[0].readLen;
	out.totalLen = m_queue[0].totalLen;
	out.actualLen = m_queue[0].actualLen;

	return true;
}


void cPacketQueue::Pop()
{
	cAutoCS cs(m_criticalSection);
	RET(m_queue.empty());

	Free(m_queue.front().buffer);
	common::rotatepopvector(m_queue, 0);
}


void cPacketQueue::SendAll()
{
	RET(m_queue.empty());

	cAutoCS cs(m_criticalSection);
	for (u_int i = 0; i < m_queue.size(); ++i)
	{
		send(m_queue[i].sock, (const char*)m_queue[i].buffer, m_queue[i].totalLen, 0);
		Free(m_queue[i].buffer);
	}
	m_queue.clear();
}


// exceptOwner �� true �϶�, ��Ŷ�� ���� Ŭ���̾�Ʈ�� ������ ������ Ŭ���̾�Ʈ�鿡�� ���
// ��Ŷ�� ������.
void cPacketQueue::SendBroadcast(vector<sSession> &sessions, const bool exceptOwner)
{
	cAutoCS cs(m_criticalSection);

	for (u_int i = 0; i < m_queue.size(); ++i)
	{
		if (!m_queue[i].full)
			continue; // �� ä������ ���� ��Ŷ�� ����

		for (u_int k = 0; k < sessions.size(); ++k)
		{
			// exceptOwner�� true�� ��, ��Ŷ�� �� Ŭ���̾�Ʈ���Դ� ������ �ʴ´�.
			const bool isSend = !exceptOwner || (exceptOwner && (m_queue[i].sock != sessions[k].socket));
			if (isSend)
				send(sessions[k].socket, (const char*)m_queue[i].buffer, m_queue[i].totalLen, 0);
		}
	}

	// ��� ������ �� ť�� ����.
	for (u_int i = 0; i < m_queue.size(); ++i)
		Free(m_queue[i].buffer);
	m_queue.clear();

	ClearMemPool();
}


void cPacketQueue::Lock()
{
	EnterCriticalSection(&m_criticalSection);
}


void cPacketQueue::Unlock()
{
	LeaveCriticalSection(&m_criticalSection);
}


BYTE* cPacketQueue::Alloc()
{
	for (u_int i = 0; i < m_memPool.size(); ++i)
	{
		if (!m_memPool[i].used)
		{
			m_memPool[i].used = true;
			return m_memPool[i].p;
		}
	}
	return NULL;
}


void cPacketQueue::Free(BYTE*ptr)
{
	for (u_int i = 0; i < m_memPool.size(); ++i)
	{
		if (ptr == m_memPool[i].p)
		{
			m_memPool[i].used = false;
			break;
		}
	}
}


void cPacketQueue::Clear()
{
	SAFE_DELETEA(m_memPoolPtr);
	m_memPool.clear();
	m_queue.clear();
}


// �޸� Ǯ�� �ʱ�ȭ�ؼ�, ��¼�� ������ �� ���׸� �����Ѵ�.
void cPacketQueue::ClearMemPool()
{
	for (u_int i = 0; i < m_memPool.size(); ++i)
		m_memPool[i].used = false;
}
