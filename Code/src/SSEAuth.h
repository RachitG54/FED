#ifndef SSEAuth_H
#define SSEAuth_H

#include "SSEincludes.h"
#include "Xvalret.h"
#include "Xaddpaillier.h"
#include "XfuncGC.h"
#include "EDBSetup.h"
#include "GenToken.h"
#include "tcpclient.h"
#include "tcpserver.h"
#include "cliquery.h"
#include "authtok.h"
#include "SSEtime.h"
#include "SSEpaillier.h"
#include "IP.h"
#include "MDSSE.h"
#include "EQCheck.h"
#include <cassert>

typedef pair<socket_ptr,socket_ptr> socketptrs;

class SSEAuth {
	private:
		tcpserver authcli;
		tcpclient authserv;
		SSEECDH group;
	public:
		SSEAuth();
	    socketptrs setupAuth(char* hostname, char* port, char* port2);
	    void authorize(socketptrs &socks);
};

#endif // SSEAuth_H