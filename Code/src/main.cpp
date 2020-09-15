#include "Xvalret.h"
#include "XfuncGC.h"
#include "Xaddpaillier.h"
#include "EDBSetup.h"
#include "GenToken.h"
#include "tcpclient.h"
#include "tcpserver.h"
#include "cliquery.h"
#include "authtok.h"
#include "SSEServer.h"
#include "SSEClient.h"
#include "SSEAuth.h"
#include "SSEpaillier.h"
#include "SSEtime.h"
#include "IP.h"
#include "SSEMDDO.h"
#include "SSEMDServer.h"
#include "SSEMDAuxServer.h"
#include "SSEtime.h"
#include <cassert>

void Auth() {
	char* hostname = (char*)"localhost";
	char port[10]; sprintf(port,"%d",networkports);
	char port2[10]; sprintf(port2,"%d",networkports+1);

	SSEAuth Auth;
	socketptrs socks = Auth.setupAuth(hostname, port,port2);
	Auth.authorize(socks);
}

void Server(EDBSetup &edbobj) {
	SSEServer server;
	char port[10]; sprintf(port,"%d",networkports+2);
	char port2[10]; sprintf(port2,"%d",networkports+1);

	server.serve(port,port2,edbobj);
}
void Client() {

	char* hostname = (char*)"localhost";

	char port1[10]; sprintf(port1,"%d",networkports);
	char port2[10]; sprintf(port2,"%d",networkports+2);

	SSEClient client;
	client.request(hostname,port1,port2);
}

void MDDO(string prot) {
	char* hostname = (char*)"localhost";
	char port1[10]; sprintf(port1,"%d",networkports+3);

	SSEMDDO DO;
	if(prot=="OPRF"){
		DO.request(hostname,port1);
	}
	else if(prot=="EQCheck"){
		DO.initializeEQ();
	}

	cout<<"[Output Comm:] DO has send/recieve bytes "<< MDDObytes<<"\n";
	cout<<"[Output Time:] DO takes ";
	timeDO.printtime(0);

	cout<<"[Output Comm:] DO SE has send/recieve bytes "<< MDDObytesSE<<"\n";
	cout<<"[Output Time:] DO SE takes ";
	timeDOSE.printtime(0);

}
void MDServer(string prot, string mergeornot, int noofDO) {
	SSEMDServer Server;
	if(prot=="OPRF") {
		if(mergeornot == "collect") {
			char port1[10]; sprintf(port1,"%d",networkports+3);

			socket_ptr sock = Server.setupserver(port1);
			Server.serve(sock);
		}
		else if(mergeornot == "merge") {
			char port1[10]; sprintf(port1,"%d",networkports+4);
			socket_ptr sock = Server.setupserver(port1);
			Server.merge(noofDO);
			Server.merge3(noofDO);
			Server.serv.writemessage(sock,"1");
			Server.serv.readlenbytes(sock);
			Server.initialize();
		}
		else {
			cout<<"Select either merge, collect for server\n";
			exit(1);
		}
	}
	else if(prot=="EQCheck") {
		if(mergeornot == "merge") {
			char port1[10]; sprintf(port1,"%d",networkports+5);

			socket_ptr sock = Server.setupserver(port1);
			Server.merge2(sock,noofDO);
			Server.merge3(noofDO);
			Server.serv.writemessage(sock,"1");
			Server.serv.readlenbytes(sock);
			Server.initialize();
		}
		else {
			cout<<"Select either merge, collect for server\n";
			exit(1);
		}
	}

	cout<<"[Output Comm:] Server has send/recieve bytes "<< MDSrvrbytes<<"\n";
	cout<<"[Output Time:] Server takes ";
	timeSrvr.printtime(0);

	cout<<"[Output Comm:] Server SE has send/recieve bytes "<< MDSrvrbytesSE<<"\n";
	cout<<"[Output Time:] Server SE takes ";
	timeSrvrSE.printtime(0);
}

void MDAuxServer(string prot, string mergeorinit, int noofDO) {
	SSEMDAuxServer AuxServer;
	if(prot=="OPRF") {
		char* hostname = (char*)"localhost";
		char port1[10]; sprintf(port1,"%d",networkports+4);

		socket_ptr sock = AuxServer.request(hostname,port1);
		AuxServer.authserv.readlenbytes(sock);
		AuxServer.initialize();
		AuxServer.merge(noofDO);
		AuxServer.authserv.writemessage(sock,"1");
	}
	else if(prot=="EQCheck") {
		char* hostname = (char*)"localhost";
		char port1[10]; sprintf(port1,"%d",networkports+5);

		socket_ptr sock = AuxServer.request(hostname,port1);
		AuxServer.initializeEQ(sock);
		AuxServer.authserv.readlenbytes(sock);
		AuxServer.merge(noofDO);
		AuxServer.authserv.writemessage(sock,"1");
	}


	cout<<"[Output Comm:] AuxServer has send/recieve bytes "<< MDAuxSrvrbytes<<"\n";
	cout<<"[Output Time:] AuxServer takes ";
	timeAuxSrvr.printtime(0);

	cout<<"[Output Comm:] AuxServer SE has send/recieve bytes "<< MDAuxSrvrbytesSE<<"\n";
	cout<<"[Output Time:] AuxServer SE takes ";
	timeAuxSrvrSE.printtime(0);
}

void ParseConfigFileStore_Line(string key, string value)
{
	if (key == "pathjustgarble")
		pathjustgarble = value;
	else if (key == "pathSSE")
		pathSSE = value;
	else if (key == "pathoblivc")
		pathoblivc = value;
}

void ParseConfigFile(){
	const string globalConfigFileName = "paths.config";
	ifstream config_file(globalConfigFileName);
	if (!config_file.is_open()){
		cerr<<"Couldn't open config file for reading. Exiting."<<endl;
		exit(1);
	}

	string line;
	while(getline(config_file, line))
	{
		istringstream cur_line(line);
		string key;
		if(getline(cur_line, key, '='))
		{
			string value;
			if(getline(cur_line, value)){
				ParseConfigFileStore_Line(key, value);
			}
		}
	}
}

#ifdef NETWORKTEST

#define MAX 600000010
char buff[MAX];

int mainNetworkTest(int argc, char** argv){
	if (argc <= 1){
		cerr<<"Error more args needed."<<endl;
		exit(1);
	}

	string num = argv[1];
	char* port = (char*)"7854";
	char* host = (char*)"localhost";
	if (num == "server"){
		tcpserver serv;
		socket_ptr sock = serv.setupserver(port);
		cout<<"Server : connected"<<endl;
		string readstr; serv.readstructure(sock,readstr);
		cout<<"Done reading."<<endl;
		cout<<serv.getTotalBytesRead()<<" "<<serv.getTotalTimeSpentInMicroSec()<<endl;
		cout<<(serv.getTotalBytesRead()*1.0)/serv.getTotalTimeSpentInMicroSec()<<" MB/sec\n";
	}
	else if (num == "client"){
		tcpclient cli;
		socket_ptr sock = cli.initialize(host, port);
		cout<<"Client: connected"<<endl;
		memset(buff, 1, MAX*sizeof(buff[0]));
		string str(buff);
		cli.writewithsize(sock, str);
		cout<<"Done writing"<<endl;
		cout<<cli.getTotalBytesWritten()<<" "<<cli.getTotalTimeSpentInMicroSec()<<endl;
		cout<<(cli.getTotalBytesWritten()*1.0)/cli.getTotalTimeSpentInMicroSec()<<" MB/sec\n";
	}
	else{
		cerr<<"Unknown arg."<<endl;
		exit(1);
	}
}

#endif

int main(int argc, char* argv[]) {

	#ifdef NETWORKTEST

	cout<<"Starting network test."<<endl;
	mainNetworkTest(argc, argv);
	cout<<"Done network test. Exiting."<<endl;
	exit(0);

	#endif

	srand(time(NULL));

	//Parse machine specific paths from config file
	ParseConfigFile();

	networkports = atoi(argv[1]);
	cout << networkports<<" is the network port. Choose more than 1024 as some are reserved for admin.\n";
	uint32_t id = atoi(argv[2]);
    ERR_load_crypto_strings();


	if(argc>=4) {
		string protocolstr = string(argv[3]);
		cout<<"Running protocol "+protocolstr<<"\n";
		if(protocolstr == "ValueRet") protocol = 0;
		else if(protocolstr == "MAF") protocol = 1;
		else if(protocolstr == "ChiSq") protocol = 2;
		else if(protocolstr == "Hamming") protocol = 3;
		else if(protocolstr == "AddHom") protocol = 4;
		else protocol = 0;
	}

  	if(id==0) {
		cout<<"Data Owner starting Setup phase\n";

  		if(argc>=5) {
  			pathGenome = argv[4];
  		}

  		if(argc>=6) {
  			DO=atoi(argv[5]);
  		}

		EDBSetup edbobj;
		SSEtime timer;
		edbobj.initialize();

		timer.starttime();
		edbobj.TSetobj.tempstore("TSet_manualstore");
		timer.donetime(); timer.printtime(0);

		ifstream inp("src/EDB/TSet_manualstore");
		inp.seekg (0, inp.end);
		int filelen = inp.tellg();
		inp.close();
		cout<<"[Final SE]: Size of TSet data produced is "<<filelen<<" bytes.\n";

		cout<<"Read completed\n";
		if(!DO) {
			if (protocol == 0){
				Xvalret XData;
				XData.initialize();
			}

			if (protocol == 1 || protocol == 2){
				IP XDataIP;
				cout<<"Initializing IP\n";
				XDataIP.initialize();
			}

			if (protocol == 3){
				XfuncGC XDataGC;
				XDataGC.initialize();
			}
			if(protocol == 4){
				Xaddpaillier XDatapaillier;
				XDatapaillier.initialize();
			}
		}

	}
	else if(id==1) {
		cout<<"Starting Server\n";
  		if(argc==5) {
  			DO=atoi(argv[4]);
  		}

		EDBSetup edbobj;
		SSEtime timer;

		timer.starttime();
		edbobj.TSetobj.tempreadstore2("TSet_manualstore");
		timer.donetime(); timer.printtime(0);

		Server(edbobj);
	}
	else if(id==2){
		cout<<"Starting Aux-Server\n";
		if(argc>=5) {
  			DO=atoi(argv[4]);
  		}
  		Auth();
	}
	else if(id==3) {
		cout<<"Starting Client\n";
  		if(argc>=5) {
  			pathQuery = argv[4];
  		}
  		if(argc>=6) {
  			DO=atoi(argv[5]);
  		}

		Client();
	}
	else if(id==4) {
		EDBSetup edbobj2;
		db database = edbobj2.TSetobj.getdata("database");
		while(1) {
			string s;
			cout<<"Enter word\n";
			cin>>s;
		    cout<<database.DB_inv[s].size()<<"\n";
		}
	}
	else if(id == 5) {
		SSEpaillier AHE(1024);
		AHE.keygen();
		AHE.storesecret("pubkey.txt","seckey.txt");
	}
	else if (id == 6) {
  		if(argc>=5) {
  			pathGenome = argv[4];
  		}

  		if(argc >= 6) {
  			DO_id = atoi(argv[5]);
  		}
		MDDO("OPRF");
	}
	else if(id==7) {
		string mergeornot = argv[4];
		int index = -1;
		if(argc==6) {
			index = atoi(argv[5]);
		}

		MDServer("OPRF",mergeornot,index);
	}
	else if(id==8) {
		string mergeorinit = argv[4];
		int index = -1;
		if(argc==6) {
			index = atoi(argv[5]);
		}
		MDAuxServer("OPRF",mergeorinit,index);
	}
	else if(id==9) {
		MDSSE Cryptsystem;
		Cryptsystem.setkeysServer();
		Cryptsystem.setkeysAuxServer();
	}
	else if(id==10) {

  		if(argc>=5) {
  			pathGenome = argv[4];
  		}

  		if(argc >= 6) {
  			DO_id = atoi(argv[5]);
  		}
		MDDO("EQCheck");
	}
	else if(id==11) {
		string mergeornot = argv[4];
		int index = -1;
		if(argc==6) {
			index = atoi(argv[5]);
		}

		MDServer("EQCheck",mergeornot,index);
	}
	else if(id==12) {
		string mergeorinit = argv[4];
		int index = -1;
		if(argc==6) {
			index = atoi(argv[5]);
		}
		MDAuxServer("EQCheck",mergeorinit,index);
	}
	cout<<"Sealing takes ";
	envseal.printtime(0);

	cout<<"Opening takes ";
	envopen.printtime(0);
}
