
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
