#ifndef SSEMDDO_H
#define SSEMDDO_H

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
#include "MDSSE.h"
#include "SSEtime.h"


class SSEMDDO {
private:
	SSEECDH group;
	tcpclient serv;
public:
    SSEMDDO();
    ~SSEMDDO();
    void request(char* host, char* port);
    void initializeEQ();
};

#endif // SSEMDDO_H