#ifndef SSEMDServer_H
#define SSEMDServer_H

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
#include "MDSSE.h"
#include "RSAencrypt.h"
#include "EQCheck.h"
#include "UnionFind.h"

typedef pair<socket_ptr,socket_ptr> socketptrs;

class SSEMDServer {
private:
	SSEECDH group;
	tcpserver serv2;
public:

    tcpserver serv;
    SSEMDServer();
    ~SSEMDServer();
    socket_ptr setupserver(char* port);
    void serve(socket_ptr &sock);
    void merge(int numberofDOs);
    void merge2(socket_ptr &sock, int numberofDOs);
    void merge3(int numberofDOs); 
    void initialize();
};

#endif // SSEMDServer_H