#ifndef SSEMDAuxServer_H
#define SSEMDAuxServer_H

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
#include "UnionFind.h"


class SSEMDAuxServer {
	private:
	public:
		tcpclient authserv;
		uint32_t KEYLENGTH = 256;
	    socket_ptr request(char* hostname, char* port);
	    void initialize();
	    void initializeEQ(socket_ptr sock);
	    void merge(int numberofDOs);
};

#endif // SSEMDAuxServer_H
