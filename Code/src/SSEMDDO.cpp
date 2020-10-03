
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
#include "SSEMDDO.h"
SSEMDDO::SSEMDDO() {
	group.setDH();
	cout<<"group is setup\n";
}

void SSEMDDO::request(char* host, char* port) {
	socket_ptr sockserv = serv.initialize(host,port);
	MDSSE Wdata;
	stringstream ss;
	BIGNUM** rinverses;
	db2 database;
	vlli idlist;
	vector< vector<string> > MDData;
	map<ll,int> idmap;
	int rowNum = Wdata.initializeDB(database,idlist,MDData,idmap);
	Wdata.initializeDO_1(ss,rinverses,database);
	string s = ss.str();
	serv.writewithsize(sockserv,s);
	MDDObytes += serv.getcurrBytesWritten();
	MDDObytesSE += serv.getcurrBytesWritten();

	cout<<"[Final Data]: Collect OPRF: Init_DO1 sending "<<s.size()<<" bytes.\n";
	string str; serv.readstructure(sockserv,str);
	MDDObytes += serv.getcurrBytesRead();
	MDDObytesSE += serv.getcurrBytesRead();

	cout<<"[Final Data]: Collect OPRF: DO received "<<str.size()<<" bytes from server.\n";
	Wdata.initializeDO_2(str,rinverses,DO_id,database,rowNum,idlist,idmap);
	Wdata.initializeDCFE(MDData,DO_id);
}

void SSEMDDO::initializeEQ() {
	MDSSE Wdata;

	db2 database;
	vlli idlist;
	vector< vector<string> > MDData;
	map<ll,int> idmap;
	int rowNum = Wdata.initializeDB(database,idlist,MDData,idmap);

	Wdata.initializeEQ(DO_id, database,rowNum, idlist, idmap);
	Wdata.initializeDCFE(MDData,DO_id);
}

SSEMDDO::~SSEMDDO() {
}