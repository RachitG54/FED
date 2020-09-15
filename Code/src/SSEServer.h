#ifndef SSEServer_H
#define SSEServer_H

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
#include "IP.h"
#include "OPRF.h"
#include "MDSSE.h"
#include "EQCheck.h"
#include <cassert>


typedef pair<socket_ptr,socket_ptr> socketptrs;

class SSEServer {
private:
	SSEECDH group;
	tcpserver serv;
	tcpserver serv2;
public:
    SSEServer();
    ~SSEServer();
    socketptrs setupserver(char* port, char* port2);
    void serve(char* port, char* port2, EDBSetup &edbobj);

};

#endif // SSEServer_H