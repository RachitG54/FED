#ifndef SSEClient_H
#define SSEClient_H

#include "SSEincludes.h"
#include "Xvalret.h"
#include "Xaddpaillier.h"
#include "IP.h"
#include "EDBSetup.h"
#include "GenToken.h"
#include "tcpclient.h"
#include "tcpserver.h"
#include "cliquery.h"
#include "authtok.h"
#include "OPRF.h"
#include "MDSSE.h"
#include <cassert>

class SSEClient {
private:
	SSEECDH group;
public:
    SSEClient();
    ~SSEClient();
    void request(char* host, char* port1,char* port2);
};

#endif // SSEClient_H