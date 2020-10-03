
/* ====================================================================
* Functionally Encrypted Datatores - Implementation for project eprint.iacr.org/2019/1262
* Copyright (C) 2019  Rachit Garg Nishant Kumar

* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <https://www.gnu.org/licenses/>.

* ====================================================================
*
*/
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