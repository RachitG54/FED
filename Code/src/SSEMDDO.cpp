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