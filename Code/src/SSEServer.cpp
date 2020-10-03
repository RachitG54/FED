
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
#include "SSEServer.h"
#include <unistd.h>

SSEServer::SSEServer() {
	group.setDH();
}

socketptrs SSEServer::setupserver(char* port, char* port2) {
	cout<<"Setting up\n";
	socket_ptr sock2 = serv2.setupserver(port2);
	socket_ptr sock = serv.setupserver(port);
	return make_pair(sock,sock2);
}

void SSEServer::serve(char* port, char* port2, EDBSetup &edbobj) {

	SSEtime timer;
    set<string> XSet = readset2("XSet");
    timer.donetime();timer.printtime(0);

    SSEtime timertottime;
    cout<<"XSet read\n";
    cout<<"Reading other Databases\n";

	CPAencrypt enc_dec;
	enc_dec.readkey("K_m");

	bool controlvar = true;
	unsigned long gamma1,gamma2;
	uint32_t size1,size2;

    map<ll,int> invlist;

    RSAencrypt PubCryptoSystem;
	if(DO) {
	    cout<<"Multi DO reading from disk\n";
	    PubCryptoSystem.readprivkey("privkeyS.pem");
		string filename = "./src/EDB/Combinedidlist";
		ifstream inp(filename);
		inp.seekg (0, inp.end);
		int filelen = inp.tellg();
		inp.seekg (0, inp.beg);
		char* data = (char*)malloc(sizeof(char)*filelen);
		inp.read(data,filelen);
		inp.close();
	    vector<ll> Combinedidlist;
		{
			msgpack::object_handle oh = msgpack::unpack(data, filelen);
			oh.get().convert(Combinedidlist);
		}
		free(data);

		REP(i,0,Combinedidlist.size()-1) {
			invlist[Combinedidlist[i]] = i;
		}
	}

	vector<string> EQshares;
	EQCheck Checkobj;

	int kwsize;
	if(DO==2) {
		int sz = readfrommsgpackfile(EQshares,"./src/keys/EQWdata");
		cout<<"Loading EQChecks keyword shares of size "<<sz<<" bytes.\n";

		MDSSE MD;
		kwsize = MD.kwsize;
		Checkobj.readEQshares("./src/EDB/EQCheckpreshareS");
	}

    cout<<"Single DO reading from disk\n";

	int rowNum;
	int noofchars;
	uchar** theta;

    vector< vector< unsigned long> > XData;

    int PRFlen,relcols,randlen;
    uchar** x;
    uchar** pandp;
    vector<string> enckeyslab;

    int clenpaillier;

	if(protocol == 0) {

		FILE* fp;
		fp = fopen(string("./src/EDB/Xvalret").c_str(),"rb");
		fscanf(fp,"%d\n%d\n",&noofchars,&rowNum);

		theta = (uchar**)malloc(sizeof(uchar*)*rowNum);
		REP(i,0,rowNum-1) {
			theta[i] = (uchar*)malloc(sizeof(uchar)*noofchars);
			REP(k,0,noofchars-1) {fscanf(fp,"%c",&theta[i][k]);}
		}
		fclose(fp);
	}
	else if(protocol == 1 || protocol == 2) {
		int filelen = readfrommsgpackfile(XData,"./src/EDB/XsumIP");
	    cout<<"[Debug] Size is "<<filelen<<"\n";
	}
	else if(protocol == 3) {
		XfuncGC inst;
		int numberofbits = inst.numberofbits;

		FILE* fp;
		fp = fopen(string("./src/EDB/XfuncGC").c_str(),"rb");
		fscanf(fp,"%d\n%d\n%d\n%d\n",&rowNum,&PRFlen,&relcols,&randlen);

		x = (uchar**)malloc(sizeof(uchar*)*rowNum);
		REP(i,0,rowNum-1) {
			x[i] = (uchar*)malloc(sizeof(uchar)*PRFlen*relcols*numberofbits);
			REP(j,0,PRFlen*relcols*numberofbits-1) fscanf(fp,"%c",&x[i][j]);
		}

		pandp = (uchar**)malloc(sizeof(uchar*)*rowNum);
		REP(i,0,rowNum-1) {
			pandp[i] = (uchar*)malloc(sizeof(uchar)*randlen);
			REP(k,0,randlen-1) {fscanf(fp,"%c",&pandp[i][k]);}
		}
		fclose(fp);

		if(DO) {
			readfrommsgpackfile(enckeyslab, "./src/EDB/XfuncGCkeys");
		}
	}
	else if(protocol == 4) {
		Xaddpaillier inst;

	    FILE* fp;
	    fp = fopen(string("./src/EDB/Xaddpaillier").c_str(),"rb");
	    fscanf(fp,"%d\n%d\n%d\n",&rowNum,&relcols,&clenpaillier);
	    cout<<rowNum<<" "<<relcols<<" "<<clenpaillier<<"\n";
	    x = (uchar**)malloc(sizeof(uchar*)*rowNum);
	    REP(i,0,rowNum-1) {
	        x[i] = (uchar*)malloc(sizeof(uchar)*relcols*clenpaillier);
	        REP(k,0,relcols*clenpaillier-1) {fscanf(fp,"%c",&x[i][k]);}
	    }
	    fclose(fp);

	}
	cout<<"Single DO read\n";

	cout<<"Serving\n";

	socketptrs socks = setupserver(port,port2);

	socket_ptr sock  = socks.first;
	socket_ptr sock2 = socks.second;

    for (;;) {
		cout<<"\n****************Moving to next query****************\n"<<endl;
		// cout<<"Sleeping for 2 seconds before next query."<<endl;
		// usleep(2*1000*1000);

		serv.getAndResetTotalBytesRead();
		serv.getAndResetTotalBytesWritten();
		serv.getAndResetTotalTimeSpentInMicroSec();
		serv2.getAndResetTotalBytesRead();
		serv2.getAndResetTotalBytesWritten();
		serv2.getAndResetTotalTimeSpentInMicroSec();

		cout<<"Done with reset of read and written bytes and time taken vars."<<endl;

		SSEtime myTtlCtTimer;
		if(DO == 1) {
			cout<<"Helping client with the queries(OPRF)\n";
			OPRF F;
			uint32_t keylen;
			uchar* key = NULL;
			key = readkeys(keylen, "K_mdsse");
			F.setkeylength(keylen); F.setkey(key);
			free(key);

			vector<string> tmpkw;
			{
				string str; serv.readstructure(sock,str);
				if (str == ""){
					cout<<"Received emtpy str from client. Exiting."<<endl;
					exit(0);
				}
	            msgpack::object_handle oh = msgpack::unpack(str.data(), str.size());
	            oh.get().convert(tmpkw);
			}
			vector<string> kw(tmpkw);
			REP(i,0,tmpkw.size()-1) {
    			F.evaluateP2(tmpkw[i],kw[i]);
			}

			{
				stringstream ss;
				msgpack::sbuffer sbuf;
			    msgpack::pack(sbuf, kw);
			    ss.write(sbuf.data(),sbuf.size());
			    string s = ss.str();
			    serv.writewithsize(sock,s);
			}

		}
		else if(DO == 2) {
			cout<<"Helping AuxServer with the queries(EQCheck)\n";
			vector<string> gamma;
			{
				string str; serv.readstructure(sock,str);
				if (str == ""){
					cout<<"Received empty string. Exiting."<<endl;
					exit(0);
				}
	            msgpack::object_handle oh = msgpack::unpack(str.data(), str.size());
	            oh.get().convert(gamma);
			}
			vector< vector<string> > uMAP(gamma.size(),vector<string>(EQshares.size()));
			REP(i,0,gamma.size()-1) {
				REP(j,0,EQshares.size()-1) {
					uMAP[i][j].resize(kwsize);
					REP(k,0,kwsize-1) {
						uchar t1 = (uchar)gamma[i][k];
						uchar t2 = (uchar)EQshares[j][k];
						uMAP[i][j][k] = (char)(t1^t2);
					}
				}
			}

			{
				string str; serv2.readstructure(sock2,str);
				string Sshare;
				Checkobj.StoA(uMAP,str,Sshare,1);
				serv2.writewithsize(sock2,Sshare);
			}

		}

		string Qstr; serv2.readstructure(sock2,Qstr);
		if (Qstr == ""){
			cout<<"Received empty string. Exiting."<<endl;
			exit(0);
		}

    	SSEtime totquerytime;
		SSEtok::authtok tok2 = SSEtok::receiveclass(Qstr);

		enc_dec.setiv((uchar*)tok2.rand_iv.c_str(),tok2.rand_iv.size()*8);
		int plen;
		uchar* ptext = enc_dec.decrypt((uchar*)tok2.enc_env.c_str(),tok2.enc_env.size(),plen,1);
		string envstr(plen, ' '); REP(i,0,plen-1) envstr[i] = ptext[i];
		SSEenv::envelope env = SSEenv::receiveclass(envstr);
    uint32_t stagbitlen = env.stag.size()*8;
		uchar* stag = (uchar*)malloc(sizeof(uchar)*env.stag.size());
		memcpy(stag,env.stag.data(),env.stag.size());

		timer.starttime();
		timertottime.starttime();
		vector< vector<uchar> > t = edbobj.TSetobj.TSetRetrieve2(stag,stagbitlen);
    timer.donetime();timer.printtime(0);
   	timertottime.donetime(); timertottime.starttime();
    cout<<"Done retrieving\n";
    cout<<"T.size() of first keyword is " << t.size()<<"\n";

    SSEtime timercomp;
    SSEtime timercomp2;
		vector< vector<uchar> > tfinal;
		uint32_t enclen = edbobj.enclen;
		uint32_t keywordno;
		keywordno = env.rhos.size()+1;
		int flag = 1;

		string itersize = to_string(t.size());

		serv2.writemessage(sock2,itersize);
		if(keywordno>1 && t.size()>0) {
			REP(i,0,t.size()-1) {
				uchar* ystr = (uchar*)malloc(sizeof(uchar)*(t[i].size()-enclen));
				REP(j,enclen,t[i].size()-1) ystr[j-enclen] = t[i][j];
				BIGNUM *y = group.bin2bn(ystr, t[i].size()-enclen);
				timertottime.donetime();

				string strtoken; serv2.readstructure(sock2,strtoken);
				SSEtok::authtok toks = SSEtok::receiveclass(strtoken);

				timertottime.starttime();
				flag = 1;
				if(toks.tokens.size()!=env.rhos.size()) {
					cout<<"Error in construction and exchange of token blindings\n";
					exit(1);
				}
				REP(j,0,toks.tokens.size()-1) {

					timercomp.starttime();
    				BIGNUM *bxtrap = group.bin2bn(toks.tokens[j].data(), toks.tokens[j].size());
    				EC_POINT *bxtrapp = group.bn2point(bxtrap);
    				BIGNUM* exponent = group.modq(y);
    				EC_POINT *bxtrapservp = group.modexpp(bxtrapp,exponent);
    				group.free(&exponent);


						BIGNUM* bxtrapserv = group.point2bn(bxtrapservp);
						uint32_t len = group.num_bytes(bxtrapserv);
						uchar* compxset = group.bn2bin(bxtrapserv);
						timercomp.donetime();
						timercomp2.starttime();
						string comp; comp.resize(len);
						REP(k,0,len-1) {comp[k] = compxset[k];}

    				group.free(&bxtrap);
      			group.free(&bxtrapservp);
      			group.free(&bxtrapserv);
      			group.free(&bxtrapp);

          	if(present(XSet,comp)) {
          	}
          	else {
          		flag = 0;
          	}
          	timercomp2.donetime();
		  		}
				group.free(&y);
				timertottime.donetime();

				if(flag==1) tfinal.pb(t[i]);
				timertottime.starttime();
			}
			timertottime.donetime();
		}
		else {
			REP(i,0,t.size()-1) tfinal.pb(t[i]);
		}

    cout<<"[Final Data]: T.size() is " << tfinal.size()<<"\n";


		timertottime.donetime();
		cout<<"[SE:] Computation of Query at server takes(w/o communication time) ";
		timertottime.printtime(0);

		cout<<"Total time spent in group is: "; timercomp.printtime(0);
		cout<<"Total time spent in comparison is(possibly reduced by bloom filter): "; timercomp2.printtime(0);
		vector< vector<uchar> > reply;
		REP(i,0,tfinal.size()-1) {
			vector<uchar> ttemp;
			REP(s,0,enclen-1) ttemp.pb(tfinal[i][s]);
			reply.pb(ttemp);
		}
		stringstream ss;
		msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, reply);
		cout << "[SE:] [Comm:] "<<sbuf.size()<<" bytes to auxilliary server"<<"\n";
    ss.write(sbuf.data(),sbuf.size());
		string message = ss.str();
    serv2.writewithsize(sock2,message);


  	vector<string> ans;
		vi ind;

		{
      string tstr; serv2.readstructure(sock2,tstr);
	    cout << "[SE:] [Comm:] "<<tstr.size()<<" bytes from auxilliary server"<<"\n";
      msgpack::object_handle oh = msgpack::unpack(tstr.data(), tstr.size());
      msgpack::object obj = oh.get();
			obj.convert(ans);
		}

    if(DO) {
	    cout<<"Outputting id's\n";
    	REP(i,0,ans.size()-1) {
				string output = PubCryptoSystem.decrypt(ans[i].size(),(const uchar*)ans[i].data());
				ll index = stoll(output,NULL,10);
				int finindex = invlist[index];
				ind.pb(finindex);
			}
			cout<<"\n";
    }
    else {
			REP(i,0,ans.size()-1) {
				uchar* ptext = (uchar*)malloc(sizeof(uchar)*ans[i].size());
				memcpy(ptext,ans[i].data(),ans[i].size());
				uint32_t index = toInt(ptext);
				int intind = (int)index;
				ind.pb(intind);
				free(ptext);
			}
		}

		{
			myTtlCtTimer.donetime();
			uint64_t seTotalTimeTaken = myTtlCtTimer.getTime();
			uint64_t seNetworkTimeAuth = serv2.getTotalTimeSpentInMicroSec();
			uint64_t seNetworkTimeClient = serv.getTotalTimeSpentInMicroSec();
			uint64_t seActualTime = seTotalTimeTaken - seNetworkTimeClient - seNetworkTimeAuth;
			assert(seActualTime > 0);
			cout<<"[Server SE Time]: Total time = "<<seTotalTimeTaken<<", network cli = "<<seNetworkTimeClient<<", network auth = "<<seNetworkTimeAuth<<endl;
			cout<<"[Server SE Time]: Actual time = "<<seActualTime<<endl;
		}

		totquerytime.donetime();
		cout<<"[SE:] Query at server takes ";
		totquerytime.printtime(0);

		cout<<"[Server SE comm.] : With auth, read = "<<serv2.getTotalBytesRead()<<", written = "<<serv2.getTotalBytesWritten()<<endl;
		cout<<"[Server SE comm.] : With client, read = "<<serv.getTotalBytesRead()<<", written = "<<serv.getTotalBytesWritten()<<endl;

		cout<<"Starting DCFE part\n";

		serv.getAndResetTotalBytesRead();
		serv.getAndResetTotalBytesWritten();
		serv.getAndResetTotalTimeSpentInMicroSec();
		serv2.getAndResetTotalBytesRead();
		serv2.getAndResetTotalBytesWritten();
		serv2.getAndResetTotalTimeSpentInMicroSec();

		myTtlCtTimer.resetTime();
		myTtlCtTimer.starttime();

		vi shuffind(ind);

		random_shuffle(all(shuffind));
		sort(all(shuffind));
		cout<<"[Final Data]: Size of documents retrieved is "<<shuffind.size()<<"\n";

		if(protocol == 0) {
			int attr;
			{
				string attrstr; serv.readstructure(sock,attrstr);
				attr = stoi(attrstr);
			}

			Xvalret XData;
			string keyval;
			vi positions;
			positions.pb(attr);

			string DatatoC = XData.compserve(shuffind,keyval,noofchars,positions,rowNum,theta);

			{
				stringstream stream;
				msgpack::sbuffer buffer;
        msgpack::packer<msgpack::sbuffer> pk(&buffer);
        pk.pack(keyval);
        pk.pack(noofchars);
        pk.pack(shuffind);
				cout << "[ValueRet:] [Comm:] "<<buffer.size()<<" bytes to auxilliary server"<<"\n";
				stream.write(buffer.data(),buffer.size());

				SSEtime timer;
				string message = stream.str();
				serv2.writewithsize(sock2,message);
				timer.donetime();
				cout << "[ValueRet:] [Comm:] Server-AuxServer takes ";
				timer.printtime(0);
			}

			{
				stringstream stream;
				msgpack::sbuffer buffer;
				msgpack::packer<msgpack::sbuffer> pk(&buffer);
				pk.pack(ind.size());
				pk.pack(noofchars);
				pk.pack(DatatoC);
				pk.pack(positions);
				cout << "[ValueRet:] [Comm:] "<<buffer.size()<<" bytes to client"<<"\n";
				stream.write(buffer.data(),buffer.size());


				SSEtime timer;
				string message = stream.str();
        serv.writewithsize(sock,message);
        timer.donetime();
		    cout << "[ValueRet:] [Comm:] Server-Client takes ";
		    timer.printtime(0);
			}
		}
		else if(protocol == 1) {
			int attr;
			{
				string attrstr; serv.readstructure(sock,attrstr);
				attr = stoi(attrstr);
			}
			IP XDataIP;
			unsigned long rholong;
			unsigned long gamma = XDataIP.compserve(shuffind, rholong, attr,XData);
			cout<<rholong<<"\n";


			{
				stringstream stream;
				msgpack::sbuffer buffer;
        msgpack::packer<msgpack::sbuffer> pk(&buffer);
        pk.pack(attr);
        pk.pack(rholong);
        pk.pack(shuffind);
        stream.write(buffer.data(),buffer.size());
        SSEtime timer;
				string message = stream.str();
        serv2.writewithsize(sock2,message);
				timer.donetime();
		    cout << "[MAF:] [Comm:] Server-AuxServer takes ";
		    timer.printtime(0);
		    cout << "[MAF:] [Comm:] "<<buffer.size()<<" bytes to AuxServer"<<"\n";
			}


			SSEtime timer;
			string command = pathoblivc + "/MAF/a.out 1 " + to_string(networkports+6) +  " -- "+to_string(gamma)+" "+ to_string(XDataIP.messagedim)+" "+to_string(shuffind.size());
			system(command.c_str());
			timer.donetime();
	    cout << "[MAF:] OblivC takes ";
	    timer.printtime(0);
	    cout<<command<<"\n";

			{
				long long share1;
				FILE* fp;
				fp = fopen("./src/EDB/P1Out","r");
				fscanf(fp,"%lld\n",&share1);
				fclose(fp);
				cout<<"Share 1 is "<<share1<<"\n";

			 	stringstream stream;
			 	msgpack::sbuffer buffer;
        msgpack::packer<msgpack::sbuffer> pk(&buffer);
        pk.pack(share1);
        cout<<"Sending "<<buffer.size()<<" bytes for summation(IP) to client\n";
	      stream.write(buffer.data(),buffer.size());
				string message = stream.str();
        serv.writewithsize(sock,message);
			}

		}
		else if(protocol == 2) {
			int attr;
			{
				string attrstr; serv.readstructure(sock,attrstr);
				attr = stoi(attrstr);
			}
			IP XDataIP;
			unsigned long rholong;
			unsigned long gamma = XDataIP.compserve(shuffind, rholong, attr,XData);


			{
				stringstream stream;
				msgpack::sbuffer buffer;
		        msgpack::packer<msgpack::sbuffer> pk(&buffer);
		        pk.pack(attr);
		        pk.pack(rholong);
		        pk.pack(shuffind);
		        stream.write(buffer.data(),buffer.size());
		        SSEtime timer;
			string message = stream.str();
		        serv2.writewithsize(sock2,message);
				timer.donetime();
			    cout << "[ChiSq:] [Comm:] Server-AuxServer takes ";
			    timer.printtime(0);
			    cout << "[ChiSq:] [Comm:] "<<buffer.size()<<" bytes to AuxServer"<<"\n";
			}


			if(controlvar) {
				controlvar = false;
			}
			else {
				controlvar = true;
			}
			if(controlvar) {
				gamma2 = gamma;
				size2 = shuffind.size();
				SSEtime timer;
				string command = pathoblivc + "/ChiSq/a.out 1 " + to_string(networkports+7) +  " -- "+to_string(gamma1)+" "+ to_string(gamma2)+" "+ to_string(size1) + " " + to_string(size2) + " "+to_string(XDataIP.messagedim);

				system(command.c_str());
				timer.donetime();
			    cout << "[ChiSq:] OblivC takes ";
			    timer.printtime(0);

				{
					long long share1;
					FILE* fp;
					fp = fopen("./src/EDB/P1Out","r");
					fscanf(fp,"%lld\n",&share1);
					fclose(fp);

					cout<<"Share 1 is "<<share1<<"\n";

					stringstream stream;
					msgpack::sbuffer buffer;
	        msgpack::packer<msgpack::sbuffer> pk(&buffer);
	        pk.pack(share1);
					stream.write(buffer.data(),buffer.size());
	        SSEtime timer;
					string message = stream.str();
	        serv.writewithsize(sock,message);
					timer.donetime();
			    cout << "[ChiSq:] [Comm:] Server-Client takes ";
			    timer.printtime(0);
			    cout << "[ChiSq:] [Comm:] "<<buffer.size()<<" bytes to Client"<<"\n";
				}

			}
			else {
				gamma1 = gamma;
				size1 = shuffind.size();
			}
		}
		else if(protocol == 3) {

			if(!DO)
			{
				stringstream ss;
		    msgpack::sbuffer sbuf;
		    msgpack::pack(sbuf, shuffind);
        ss.write(sbuf.data(),sbuf.size());
				string message = ss.str();
        serv2.writewithsize(sock2,message);
			}
			else
			{
				vector<string> relkeys;
				REP(i,0,shuffind.size()-1) {
					relkeys.pb(enckeyslab[shuffind[i]]);
				}

				stringstream ss;
		    msgpack::sbuffer sbuf;
		    msgpack::pack(sbuf, shuffind);
		    msgpack::pack(sbuf, relkeys);
		    msgpack::pack(sbuf, PRFlen);
				ss.write(sbuf.data(),sbuf.size());
				string message = ss.str();
        serv2.writewithsize(sock2,message);
			}

			cout<<"Sent Data to AuxServer\n";
			XfuncGC XDataGC;
			string garbvals;
			{
	        serv2.readstructure(sock2,garbvals);
	        cout<<"Recieved "<<garbvals.size()<<" bytes from authority\n";
			}
			XDataGC.compserve(shuffind,garbvals,PRFlen,relcols,randlen,x,pandp);
		}
		else if(protocol == 4) {

			int attr;
			{
				string attrstr; serv.readstructure(sock,attrstr);
				attr = stoi(attrstr);
			}

			Xaddpaillier XDataPaillier;
			uint32_t y;
			string c = XDataPaillier.compserve(shuffind, y, attr,rowNum,relcols,clenpaillier,x);
			if(c.size()==0) c = "1";

			{
				cout<<"Sending "<<c.size()<<" bytes for summation(Additive Paillier) to authority\n";
				serv2.writewithsize(sock2,c);
			}

			{
				stringstream stream;
				msgpack::sbuffer buffer;
        msgpack::packer<msgpack::sbuffer> pk(&buffer);
        pk.pack(y);
        cout<<"Sending "<<buffer.size()<<" bytes for summation(Additive Paillier) to client\n";
        stream.write(buffer.data(),buffer.size());
				string message = stream.str();
        serv.writewithsize(sock,message);
			}
		}
		{
			myTtlCtTimer.donetime();
			uint64_t netTotalTimeTaken = myTtlCtTimer.getTime();
			uint64_t netNetworkTimeAuth = serv2.getTotalTimeSpentInMicroSec();
			uint64_t netNetworkTimeClient = serv.getTotalTimeSpentInMicroSec();
			uint64_t netActualTime = netTotalTimeTaken - netNetworkTimeClient - netNetworkTimeAuth;
			assert(netActualTime > 0);
			cout<<"[Server DCFE Time]: Total time = "<<netTotalTimeTaken<<", network cli = "<<netNetworkTimeClient<<", network auth = "<<netNetworkTimeAuth<<endl;
			cout<<"[Server DCFE Time]: Actual time = "<<netActualTime<<endl;
		}

		cout<<"[Server DCFE comm.] : With auth, read = "<<serv2.getTotalBytesRead()<<", written = "<<serv2.getTotalBytesWritten()<<endl;
		cout<<"[Server DCFE comm.] : With client, read = "<<serv.getTotalBytesRead()<<", written = "<<serv.getTotalBytesWritten()<<endl;

    }
}
SSEServer::~SSEServer() {
}
