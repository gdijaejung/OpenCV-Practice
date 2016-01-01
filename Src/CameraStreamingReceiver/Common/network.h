#pragma once


#include <windows.h>
#include <winsock.h>
#include <string>
#include <algorithm>
using std::string;

#pragma comment( lib, "wsock32.lib" )



namespace network {
	enum {
		BUFFER_LENGTH = 512,
	};

// 	struct sFD_Set : fd_set {
// 		SOCKET socket_array[FD_SETSIZE];		// fd_array[] ���Ͽ� �ش��ϴ� netid ��
// 	};

}


#include "launcher.h"
#include "protocol.h"
#include "udpclient.h"
#include "udpserver.h"
#include "tcpserver.h"
#include "tcpclient.h"