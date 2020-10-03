
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