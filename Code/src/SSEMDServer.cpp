#include "SSEMDServer.h"

SSEMDServer::SSEMDServer() {
	group.setDH();
}

socket_ptr SSEMDServer::setupserver(char* port) {
	socket_ptr sock = serv.setupserver(port);
	return sock;
}
void SSEMDServer::serve(socket_ptr &sock) {
	socket_ptr sockDO  = sock;
	string s;
	serv.readstructure(sockDO,s);
	cout<<"[Final Data]: Collect OPRF: Server recieved "<<s.size()<<" bytes\n";
	MDSrvrbytes += serv.getcurrBytesRead();
	MDSrvrbytesSE += serv.getcurrBytesRead();
	MDSSE Wdata;
	stringstream ss;
	Wdata.initializeServe(s,ss);
	string s2 = ss.str();
	serv.writewithsize(sockDO,s2);
	MDSrvrbytes += serv.getcurrBytesWritten();
	MDSrvrbytesSE += serv.getcurrBytesWritten();
	cout<<"[Final Data]: Collect OPRF: Server sending "<<s2.size()<<" bytes.\n";
}

void SSEMDServer::merge(int numberofDOs) {
	set<string> Combiningdata;

	SSEtime ttlTimer;
	REP(i,0,numberofDOs-1) {
		string filename = "./src/EDB/W_data"+to_string(i);
		ifstream inp(filename);

		inp.seekg (0, inp.end);
		int filelen = inp.tellg();
		if (filelen == -1){
			cerr<<"Error in opening file "<<filename<<endl;
			exit(1);
		}

		inp.seekg (0, inp.beg);
		cout<<filelen<<"\n";
	    MDSrvrbytes += filelen;
	    MDSrvrbytesSE += filelen;

		char* data = (char*)malloc(sizeof(char)*filelen);
		inp.read(data,filelen);
		inp.close();

		timeSrvr.starttime();
		timeSrvrSE.starttime();

		vector<string> W_Data;

		{
			msgpack::object_handle oh = msgpack::unpack(data, filelen);
			oh.get().convert(W_Data);
		}
		free(data);
		ttlTimer.starttime();
	    REP(j,0,W_Data.size()-1) {
	    	Combiningdata.insert(W_Data[j]);
	    }
	    timeSrvr.donetime();
	    timeSrvrSE.donetime();

	    ttlTimer.donetime();

	}

	cout<<"[Final Data]: SE Server merge step takes ";
	ttlTimer.printtime(0);

	{
		msgpack::sbuffer sbuf;
	    msgpack::pack(sbuf, Combiningdata);
	    ofstream out("./src/EDB/CombinedW_data");
	    out.write(sbuf.data(),sbuf.size());
	    out.close();

	    MDSrvrbytes += sbuf.size();
	    MDSrvrbytesSE += sbuf.size();

		cout<<"[Final Data]: SE Server merged dataset size is "<<sbuf.size()<<" bytes.\n";
	}

}
void SSEMDServer::merge3(int numberofDOs) {
	if(protocol == 0 ) {
		vector< string > encDATA;
		SSEtime timer;
		REP(i,0,numberofDOs-1) {
			vector< string > tempobj;
			int sz = readfrommsgpackfile(tempobj,"./src/EDB/Xvalret_" + to_string(i));
		    MDSrvrbytes += sz;
			timer.starttime();
			timeSrvr.starttime();
			REP(j,0,tempobj.size()-1) {
				encDATA.pb(tempobj[j]);
			}
			timeSrvr.donetime();
			timer.donetime();
		}
		int sz = writetomsgpackfile(encDATA,"./src/EDB/Xvalret");
		cout<<"Merged size is "<<sz<<" bytes.\n";

	}
	else if(protocol == 1 || protocol == 2) {
		vector< string > encDATA;
		SSEtime timer;
		REP(i,0,numberofDOs-1) {
			vector< string > tempobj;
			int sz = readfrommsgpackfile(tempobj,"./src/EDB/XsumIP_" + to_string(i));
		    MDSrvrbytes += sz;
			timer.starttime();
			timeSrvr.starttime();
			REP(j,0,tempobj.size()-1) {
				encDATA.pb(tempobj[j]);
			}
			timeSrvr.donetime();
			timer.donetime();
		}

		int sz = writetomsgpackfile(encDATA,"./src/EDB/XsumIP");
		cout<<"Merged size is "<<sz<<" bytes.\n";
	}
}
void SSEMDServer::merge2(socket_ptr &sock, int numberofDOs) {
	socket_ptr sockSA  = sock;
	set< pair<string,string> > Combiningdata;


    RSAencrypt PubCryptoSystem;
    PubCryptoSystem.readprivkey("privkeyS.pem");

	SSEtime ttlTimer;
	REP(i,0,numberofDOs-1) {
		string filename = "./src/EDB/W_data"+to_string(i);
		ifstream inp(filename);

		inp.seekg (0, inp.end);
		int filelen = inp.tellg();
		inp.seekg (0, inp.beg);
		cout<<"Reading file: "<<filename<<" of filelen = "<<filelen<<".\n";

	    MDSrvrbytes += filelen;
	    MDSrvrbytesSE += filelen;

		char* data = (char*)malloc(sizeof(char)*filelen);
		inp.read(data,filelen);
		inp.close();

		timeSrvr.starttime();
		timeSrvrSE.starttime();

		vector< pair<string,string> > W_Data;

		{
			msgpack::object_handle oh = msgpack::unpack(data, filelen);
			oh.get().convert(W_Data);
		}
		free(data);
		ttlTimer.starttime();
	    REP(j,0,W_Data.size()-1) {
	    	Combiningdata.insert(W_Data[j]);
	    }
	    ttlTimer.donetime();
	    timeSrvr.donetime();
	    timeSrvrSE.donetime();
	}

	ttlTimer.starttime();
	timeSrvr.starttime();
	timeSrvrSE.starttime();

	vector<string> alpha(Combiningdata.size());
	vector<string> pidata(Combiningdata.size());

	int itercount = 0;
	tr(Combiningdata,it) {
		alpha[itercount] = (*it).first;
		pidata[itercount] = (*it).second;
		itercount++;
	}
    ttlTimer.donetime();
    timeSrvr.donetime();
    timeSrvrSE.donetime();

	cout<<"[Final Data]: SE Server merge step takes ";
	ttlTimer.printtime(0);

	{
		msgpack::sbuffer sbuf;
	    msgpack::pack(sbuf, pidata);
	    stringstream ss;
	    ss.write(sbuf.data(),sbuf.size());
	    string str = ss.str();
		serv.writewithsize(sockSA,str);

	    MDSrvrbytes += serv.getcurrBytesWritten();
	    MDSrvrbytesSE += serv.getcurrBytesWritten();

		cout<<"[Final Data]: SE Server merged dataset size is "<<sbuf.size()<<" bytes.\n";
	}

	timeSrvr.starttime();
	timeSrvrSE.starttime();

	MDSSE MD;
	int kwsize = MD.kwsize;
	vector< vector<string> > uMAP(alpha.size(),vector<string>(alpha.size()));
	REP(i,0,alpha.size()-1) {
		REP(j,i+1,alpha.size()-1) {
			if(j==i) continue;
			uMAP[i][j].resize(kwsize);
			REP(k,0,kwsize-1) {
				uchar t1 = (uchar)alpha[i][k];
				uchar t2 = (uchar)alpha[j][k];
				uMAP[i][j][k] = (char)(t1^t2);
			}
		}
	}

	timeSrvr.donetime();
	timeSrvrSE.donetime();

	cout<<"****Starting equality check phase"<<endl;
	EQCheck Checkobj;

	string EQserv;
	string EQAuxServ;

	{
		cout<<"Starting read from A for eqcheck message"<<endl;
		serv.readstructure(sockSA,EQAuxServ);
		cout<<"Done read from A for eqcheck message"<<endl;
		MDSrvrbytes += serv.getcurrBytesRead();
		MDSrvrbytesSE += serv.getcurrBytesRead();

		cout<<"EQAuxServ.size() = "<<EQAuxServ.size()<<"\n";
	}

	Checkobj.readEQshares("./src/EDB/EQCheckpreshareS");

	timeSrvr.starttime();
	timeSrvrSE.starttime();
	cout<<"Starting StoA "<<endl;
	Checkobj.StoA(uMAP,EQAuxServ,EQserv);
	cout<<"Done StoA"<<endl;
	timeSrvr.donetime();
	timeSrvrSE.donetime();

	cout<<"Sending message to A"<<endl;
	serv.writewithsize(sockSA,EQserv);
	cout<<"Done sending message to A"<<endl;

    MDSrvrbytes += serv.getcurrBytesWritten();
    MDSrvrbytesSE += serv.getcurrBytesWritten();

	vector<string> separatingdata;
	{
		cout<<"Receving from A"<<endl;
		string str; serv.readstructure(sockSA,str);
		cout<<"Done"<<endl;
		MDSrvrbytes += serv.getcurrBytesRead();
		MDSrvrbytesSE += serv.getcurrBytesRead();

		msgpack::object_handle oh = msgpack::unpack(str.data(), str.size());
		oh.get().convert(separatingdata);
	}
	timeSrvr.starttime();
	timeSrvrSE.starttime();

	cout<<"Received dataset, starting culling\n";
	cout<<separatingdata.size()<<"\n";
	SSEtime timer;

	SSEtime dectime1;
	SSEtime dectime2;

	vector<string> cullingdata;
	REP(i,0,separatingdata.size()-1) {
		string encgamma;
		string encetamuxi;
		string etamuxi;
		{
		    msgpack::unpacker pac;
	        pac.reserve_buffer(separatingdata[i].size());
	        memcpy(pac.buffer(), separatingdata[i].data(), separatingdata[i].size());
	        pac.buffer_consumed(separatingdata[i].size());
	        msgpack::object_handle oh;
	        pac.next(oh);
	        oh.get().convert(encgamma);
	        pac.next(oh);
	        oh.get().convert(encetamuxi);
		}

		{

	        timer.starttime();

	        dectime2.starttime();
	        PubCryptoSystem.envelope_open(encetamuxi,etamuxi);
	        dectime2.donetime();
	        timer.donetime();
		}

		pair<string,string> pss;
		{
			msgpack::object_handle oh = msgpack::unpack(etamuxi.data(), etamuxi.size());
			oh.get().convert(pss);
		}

		string encgammamu(kwsize,'\0');
		REP(s,0,kwsize-1) {
			encgammamu[s] = (char)((uchar)encgamma[s] ^ (uchar)pss.first[s]);
		}

		{
			msgpack::sbuffer sbuf;
			msgpack::pack(sbuf, encgammamu);
			msgpack::pack(sbuf, pss.second);
			stringstream ss;
			ss.write(sbuf.data(),sbuf.size());
			string s = ss.str();
			cullingdata.pb(s);
		}
	}
	timer.printtime(0);
	timeSrvr.donetime();
	timeSrvrSE.donetime();

	cout<<"Time taken for 1st envelope is ";
	dectime1.printtime(0);

	cout<<"Time taken for 2nd envelope is ";
	dectime2.printtime(0);

	{
		msgpack::sbuffer sbuf;
		msgpack::pack(sbuf, cullingdata);
		stringstream ss;
		ss.write(sbuf.data(),sbuf.size());
		string s = ss.str();
		serv.writewithsize(sockSA,s);

		MDSrvrbytes += serv.getcurrBytesWritten();
		MDSrvrbytesSE += serv.getcurrBytesWritten();
		cout<<"Finish culling step, sending "<<sbuf.size()<<" bytes.\n";
	}

	vector< pair<string,string> > deltaencphi;
	{
		string str;
		serv.readstructure(sockSA,str);

		MDSrvrbytes += serv.getcurrBytesRead();
		MDSrvrbytesSE += serv.getcurrBytesRead();

		msgpack::object_handle oh = msgpack::unpack(str.data(), str.size());
		oh.get().convert(deltaencphi);
	}
	timeSrvr.starttime();
	timeSrvrSE.starttime();
	vector<string> findata(deltaencphi.size());
	REP(i,0,deltaencphi.size()-1) {
		findata[i] = PubCryptoSystem.decrypt(deltaencphi[i].second.size(),(const uchar*)deltaencphi[i].second.data());
		REP(s,0,kwsize-1) {
			findata[i][s] = (char)((uchar)findata[i][s] ^ (uchar)deltaencphi[i].first[s]);
		}
	}
	timeSrvr.donetime();
	timeSrvrSE.donetime();
	int sz = writetomsgpackfile(findata,"./src/keys/EQWdata");
	cout<<"For Equality Check storing "<<sz<<" bytes.\n";

}


void SSEMDServer::initialize() {

	if(protocol==0) {

	    RSAencrypt PubCryptoSystem;
	    PubCryptoSystem.readprivkey("privkeyS.pem");

		vector< pair<string,int> > enckeys;
		int fileLenXKeys = readfrommsgpackfile(enckeys,"./src/EDB/Xkeys");
		cout<<"[Final Data]: Server Init stage input size: Xkeys size = "<<fileLenXKeys<<" bytes.\n";

	    vector<string> pkeys(enckeys.size());

		int rowNum,noofchars;
		FILE* fp;
		string fname = "./src/EDB/Xvalret";
		fp = fopen(fname.c_str(),"rb");
		fscanf(fp,"%d\n%d\n",&noofchars,&rowNum);
		uchar** theta = (uchar**)malloc(sizeof(uchar*)*rowNum);
		REP(i,0,rowNum-1) {
			theta[i] = (uchar*)malloc(sizeof(uchar)*noofchars);
			REP(k,0,noofchars-1) {
				fscanf(fp,"%c",&theta[i][k]);
			}
		}
		fclose(fp);

		cout<<"[Final Data]: Server Init stage input size: XValRet size = "<<(rowNum*noofchars)<<" bytes.\n";

		SSEtime timer;
		Xvalret inst;
		timeSrvr.starttime();

	    REP(i,0,enckeys.size()-1) {
			pkeys[i] = PubCryptoSystem.decrypt(enckeys[i].first.size(),(const uchar*)enckeys[i].first.data());
	    }


		REP(i,0,rowNum-1) {
			PRF PRFKval;
			PRFKval.setkeylength(inst.KEYLENGTH);
			PRFKval.setkey((uchar*)pkeys[i].data());

			uchar* FKidstr = FKid(PRFKval,enckeys[i].second,noofchars);

			REP(k,0,noofchars-1) {
				int data1 = (uchar)theta[i][k];
				int data2 = (uchar)FKidstr[k];
				int data = data1 ^ data2;
				theta[i][k] = (uchar)data;
			}
			free(FKidstr);
		}
		timer.donetime();
		cout<<"[Final Data]: DCFE Server init step takes ";
		timer.printtime(0);
		timeSrvr.donetime();

		{
			ofstream fout;
			fout.open("./src/EDB/Xvalret",ios::out| ios::binary);
			stringstream ss;
			ss << noofchars<<"\n"<<rowNum<<"\n";

			REP(i,0,rowNum-1) {
				ss.write((char*)theta[i],noofchars);
				free(theta[i]);
			}
			free(theta);
			string str = ss.str();
			fout.write(str.data(),str.size());
			fout.close();
			cout<<"[Final Data]: Server init output: Size of encrypted data is "<<str.size()<<" bytes\n";

		}
	}
	else if(protocol == 1 || protocol == 2) {

	    RSAencrypt PubCryptoSystem;
	    PubCryptoSystem.readprivkey("privkeyS.pem");

		vector< pair<string,int> > enckeys;
		int fileLenXKeys = readfrommsgpackfile(enckeys,"./src/EDB/Xkeys");

		cout<<"[Final Data]: Server Init stage input size: Size of input XKeys = "<<fileLenXKeys<<" bytes.\n";

	    vector< vector< unsigned long> > XData;
		int fileLenXSumIP = readfrommsgpackfile(XData,"./src/EDB/XsumIP");

		cout<<"[Final Data]: Server Init stage input size: Size of input XSumIP = "<<fileLenXSumIP<<" bytes.\n";

		timeSrvr.starttime();
	    vector<string> pkeys(enckeys.size());
		SSEtime timer;
	    REP(i,0,enckeys.size()-1) {
			pkeys[i] = PubCryptoSystem.decrypt(enckeys[i].first.size(),(const uchar*)enckeys[i].first.data());
	    }


		IP inst;
	    unsigned long N = 1 << inst.messagedim;

		REP(i,0,XData.size()-1) {
			PRF PRFK_ip;
			PRFK_ip.setkeylength(inst.KEYLENGTH);
			PRFK_ip.setkey((uchar*)pkeys[i].data());

	    	REP(j,0,XData[i].size()-1) {
	            unsigned long val = inst.evaluate(PRFK_ip,j,enckeys[i].second)%N;
	            unsigned long tempval = (N-val);
	            XData[i][j] = (XData[i][j]+tempval)%N;
	    	}
	    }

		timer.donetime();
		cout<<"[Final Data]: DCFE Server init step takes ";
		timer.printtime(0);
		timeSrvr.donetime();

		int sz = writetomsgpackfile(XData,"./src/EDB/XsumIP");

		cout<<"[Final Data]: Server init output: DCFE Server init step final size is "<<sz<<" bytes.\n";
		cout<<"Initialised MAF multi-DO\n";
	}
	else if(protocol == 3) {

	    RSAencrypt PubCryptoSystem;
	    PubCryptoSystem.readprivkey("privkeyS.pem");

	    XfuncGC inst;
	    int numberofbits = inst.numberofbits;

		vector<string> pvs;
		int fileLenXFuncGC = readfrommsgpackfile(pvs, "./src/EDB/XfuncGC");

	    MDSrvrbytes += fileLenXFuncGC;
		cout<<"[Final Data]: Server Init stage input size: Size of input XFuncGC = "<<fileLenXFuncGC<<" bytes.\n";



		int PRFlen,relcols,randlen;
		{
			FILE* fp;
			string fname = "./src/EDB/XfuncGCparams";
			fp = fopen(fname.c_str(),"rb");
			fscanf(fp,"%d\n%d\n%d\n",&PRFlen,&relcols,&randlen);
			fclose(fp);
		}

		SSEtime timer;
		timeSrvr.starttime();

		int rowNum = pvs.size();
		vector<string> pvsplain(rowNum);


		uchar** x = (uchar**)malloc(sizeof(uchar*)*rowNum);
		uchar** randbitS = (uchar**)malloc(sizeof(uchar*)*rowNum);

		vector<string> PRFkeys(rowNum);


		REP(i,0,rowNum-1) {
			PubCryptoSystem.envelope_open(pvs[i],pvsplain[i]);

			x[i] = (uchar*)malloc(sizeof(uchar)*PRFlen*relcols*numberofbits);
			memset(x[i],0,PRFlen*relcols*numberofbits);
		    randbitS[i] = (uchar*)malloc(sizeof(uchar)*randlen);
			memset(randbitS[i],0,randlen);

			char* strdata = (char*)pvsplain[i].data();

			int size1 = PRFlen*relcols*numberofbits;
			int size2 = randlen;
			int size3 = pvsplain[i].size()-size1-size2;

			memcpy(x[i],strdata,size1);
			memcpy(randbitS[i],strdata+size1,size2);

			PRFkeys[i].resize(size3);
			REP(s,0,size3-1) PRFkeys[i][s] = pvsplain[i][size1+size2+s];
		}
		timer.donetime();
		cout<<"[Final Data]: DCFE Server init step takes ";
		timer.printtime(0);
		timeSrvr.donetime();

		{
			ofstream fout;
			fout.open("./src/EDB/XfuncGC",ios::out| ios::binary);
			stringstream ss;
			ss << rowNum <<"\n"<<PRFlen<<"\n"<<relcols<<"\n"<<randlen<<"\n";
			REP(i,0,rowNum-1) {
				ss.write((char*)x[i],PRFlen*relcols*numberofbits);
				free(x[i]);
			}
			free(x);

			REP(i,0,rowNum-1) {
				ss.write((char*)randbitS[i],randlen);
				free(randbitS[i]);
			}
			free(randbitS);

			string str = ss.str();
			fout.write(str.data(),str.size());
			fout.close();

			cout<<"[Final Data]: Server init output: XfuncGC size = "<<str.size()<<" bytes."<<endl;
		}
		{
			int sz = writetomsgpackfile(PRFkeys,"./src/EDB/XfuncGCkeys");
			cout<<"[Final Data]: Server init output: XfuncGC size (both keys enc) = "<<sz<<" bytes."<<endl;
		}
	}
	else if(protocol == 4) {
		vector<string> encpaillier;
		int fileLenpaillier = readfrommsgpackfile(encpaillier, "./src/EDB/Xaddpaillier");
		cout<<"[Final Data]: Server Init stage input size: Size of input Xaddpaillier = "<<fileLenpaillier<<" bytes.\n";

	    MDSrvrbytes += fileLenpaillier;

	    RSAencrypt PubCryptoSystem;
	    PubCryptoSystem.readprivkey("privkeyS.pem");
	    int rowNum,relcols,clen;
		{
			FILE* fp;
			string fname = "./src/EDB/Xaddpaillierparams";
			fp = fopen(fname.c_str(),"rb");
			fscanf(fp,"%d\n%d\n%d\n",&rowNum,&relcols,&clen);
			fclose(fp);
		}
		cout<<rowNum<<" "<<relcols<<" "<<clen<<"\n";
		timeSrvr.starttime();
		vector<string> paillierplain(encpaillier.size());
	    REP(i,0,encpaillier.size()-1) {
			PubCryptoSystem.envelope_open(encpaillier[i],paillierplain[i]);
	    }
		timeSrvr.donetime();
	    {
	    	ofstream fout;
		    fout.open("./src/EDB/Xaddpaillier",ios::out| ios::binary);
		    stringstream ss;
		    ss << rowNum <<"\n"<<relcols<<"\n"<<clen<<"\n";
		    REP(i,0,rowNum-1) {
		    	if((int)paillierplain[i].size()!=relcols*clen) {
		    		cout<<"Error in combining Multi-DO paillier\n";
		    		exit(1);
		    	}
		        ss.write(paillierplain[i].data(),paillierplain[i].size());
		    }

		    string str = ss.str();
		    fout.write(str.data(),str.size());
		    fout.close();
	    }
	}

}
SSEMDServer::~SSEMDServer() {
}
