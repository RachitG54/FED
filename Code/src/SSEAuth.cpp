#include "SSEAuth.h"
#include <unistd.h>

SSEAuth::SSEAuth() {
	group.setDH();
	cout<<"group is setup\n";
}

socketptrs SSEAuth::setupAuth(char* hostname, char* port, char* port2) {
	socket_ptr sock2 = authserv.initialize(hostname,port2);
	cout<<"Done setting up auth-client connection\n";
	socket_ptr sock = authcli.setupserver(port);
	return make_pair(sock,sock2);
}
void SSEAuth::authorize(socketptrs &socks) {

	socket_ptr sock  = socks.first;
	socket_ptr sock2 = socks.second;

    msgpack::object_handle oh = readobject("./src/DB/wordcount");
    msgpack::object obj = oh.get();
    map<string,int> wordcount;
    obj.convert(wordcount);
    cout<<"Read database for verification\n";

	CPAencrypt enc_dec;
	enc_dec.readkey("K_e");

  RSAencrypt PubCryptoSystemA;
	int PRFlen,relcols,randlen,numberofbits;
	if(DO) {
		if(protocol==3) {
			XfuncGC inst;
			numberofbits = inst.numberofbits;
			{
				FILE* fp;
				string fname = "./src/EDB/XfuncGCparams";
				fp = fopen(fname.c_str(),"rb");
				fscanf(fp,"%d\n%d\n%d\n",&PRFlen,&relcols,&randlen);
				fclose(fp);
			}
			PubCryptoSystemA.readprivkey("privkeyA.pem");
		}
	}

	int kwsize;
	vector<string> htau;
	EQCheck Checkobj;
	if(DO==2) {
		PRF PRFtau;
		uint32_t keylen;
		uchar* key = NULL;
		key = readkeys(keylen, "KtauEQ");
		PRFtau.setkeylength(keylen); PRFtau.setkey(key);

		int htausize;
		readfrommsgpackfile(htausize,"./src/keys/L");

		MDSSE MDinst;
		kwsize = MDinst.kwsize;

		PRFtau.setoutputlength(8*kwsize);

		htau.resize(htausize);

		REP(i,0,htausize-1) {
			string str = to_string(i);
			uint32_t sz;
			uchar* htaustr = PRFtau.evaluate((const uchar*)str.data(),str.size(),sz);
			htau[i].resize(sz/8);
			REP(s,0,sz/8 - 1) {
				htau[i][s] = htaustr[s];
			}
			free(htaustr);
		}
		Checkobj.readEQshares("./src/EDB/EQCheckpreshareA");
	}

	bool controlvar = true;
	unsigned long delta1,delta2;

    for (;;) {
		cout<<"\n****************Moving to next query****************\n"<<endl;

		authserv.getAndResetTotalBytesRead();
		authserv.getAndResetTotalBytesWritten();
		authserv.getAndResetTotalTimeSpentInMicroSec();
		authcli.getAndResetTotalBytesRead();
		authcli.getAndResetTotalBytesWritten();
		authcli.getAndResetTotalTimeSpentInMicroSec();

		cout<<"Done with reset of read and written bytes and time taken vars."<<endl;

		SSEtime myTtlCtTimer;
		if(DO==2) {
			cout<<"Helping Client Realize the EQ Check keywords.\n";
			vector<string> delta;
			{
				string str; authcli.readstructure(sock,str);
				if (str == ""){
					cout<<"Received empty string. Exiting."<<endl;
					exit(0);
				}
	            msgpack::object_handle oh = msgpack::unpack(str.data(), str.size());
	            oh.get().convert(delta);
			}

			vector< vector<string> > rMAP(delta.size(),vector<string>(htau.size()));
			REP(i,0,delta.size()-1) {
				REP(j,0,htau.size()-1) {
					rMAP[i][j].resize(kwsize);
					REP(k,0,kwsize-1) {
						uchar t1 = (uchar)delta[i][k];
						uchar t2 = (uchar)htau[j][k];
						rMAP[i][j][k] = (char)(t1^t2);
					}
				}
			}


			vector<int> corrshares;

			{
				string Ashare;
				Checkobj.AtoS(rMAP, Ashare, 1);
			    authserv.writewithsize(sock2,Ashare);

				string Sshare; authserv.readstructure(sock2,Sshare);
				Checkobj.compAquery2(Sshare,corrshares,delta.size(),htau.size());
			}

			vector<string> keywords;
			REP(i,0,corrshares.size()-1) {
				keywords.pb(htau[corrshares[i]]);
			}

			cout<<"Sending the final keywords back to client for successful starting of SE\n";
			{
				stringstream ss;
				msgpack::sbuffer sbuf;
			    msgpack::pack(sbuf, keywords);
			    ss.write(sbuf.data(),sbuf.size());
			    string s = ss.str();
			    authcli.writewithsize(sock,s);
			}

		}

		SSEtime timer;
		string Qstr; authcli.readstructure(sock,Qstr);
		if (Qstr == ""){
			cout<<"Received empty string from client. Exiting."<<endl;
			exit(0);
		}

    cout << "[SE:] [Comm:] Client-AuxServer takes ";
    timer.printtime(0);
    cout << "[SE:] [Comm:] "<<Qstr.size()<<" receving bytes from Client"<<"\n";

		GenToken authprotocol;
		string tokensstr = authprotocol.authorize(Qstr,wordcount);

		cout<<"Authorization completed\n";

		//Reading tokens from authority
		SSEtok::authtok tok = SSEtok::receiveclass(tokensstr);

		vector<std::vector<uchar> > tokens = tok.gettokens();
		uint32_t presentattr = tok.attribute;

		uint32_t strapbitlen = tokens[0].size()*8;
		uchar* strap = (uchar*)malloc(sizeof(uchar)*tokens[0].size());
		memcpy(strap,tokens[0].data(),tokens[0].size());

	  	PRF strapPRF; strapPRF.setkeylength(strapbitlen); strapPRF.setkey(strap);
    	uint32_t K_zbitlen;
    	string str = to_string(1);
      	uchar* K_z = strapPRF.evaluate((const uchar*)str.c_str(),str.size(),K_zbitlen);
      	PRF_P PRFK_z; PRFK_z.setkeylength(K_zbitlen); PRFK_z.setkey(K_z,group.q);
	  	uint32_t c = 0;

	  	tok.tokens.clear();
	  	tok.attribute = 0;

	  	string tokmsg = SSEtok::sendstr(tok);

	    cout << "[SE:] [Comm:] "<<tokmsg.size()<<" bytes to server"<<"\n";
	  	authserv.writewithsize(sock2,tokmsg);
	  	uint32_t tokensumsize = 0;

    	int flag = authserv.readlenbytes(sock2);
    	if(tokens.size()>1 && flag > 0) {
		  	REP(z,0,flag-1) {
		  		SSEtok::authtok tokc;
       		c += 1;
        	string concatstr = /*keywords[0] + */to_string(c);
        	BIGNUM* z_c = PRFK_z.evaluate((const uchar*)concatstr.c_str(),concatstr.size());
	        REP(i,1,tokens.size()-1) {
        		uchar* str = (uchar*)malloc(sizeof(uchar)*tokens[i].size());
        		memcpy(str,tokens[i].data(),tokens[i].size());

	    			BIGNUM *bxtrap = group.bin2bn(str, tokens[i].size());
	    			EC_POINT *bxtrapp = group.bn2point(bxtrap);
	    			EC_POINT *bxtrapclip = group.modexpp(bxtrapp,z_c);
	    			BIGNUM *bxtrapcli = group.point2bn(bxtrapclip);

        		uint32_t len = group.num_bytes(bxtrapcli);
        		uchar* strtoken = group.bn2bin(bxtrapcli);
        		tokc.tokens.pb(vector<uchar>(strtoken,strtoken+len));
        		free(strtoken);
        		group.free(&bxtrap);
        		group.free(&bxtrapp);
        		group.free(&bxtrapcli);
          		group.free(&bxtrapclip);
        		free(str);
	       	}
       		string tokmid = SSEtok::sendstr(tokc);
       		tokensumsize += tokmid.size();
        	authserv.writewithsize(sock2,tokmid);
        	group.free(&z_c);
		  	}
		}


	  cout << "[SE:] [Comm:] "<<tokensumsize<<" bytes to server"<<"\n";
		free(strap);
		free(K_z);


		string tstr; authserv.readstructure(sock2,tstr);
		cout<<tstr.size()<<" worth of bytes is received from server\n";
    msgpack::object_handle oh2 = msgpack::unpack(tstr.data(), tstr.size());
    msgpack::object obj2 = oh2.get();
		vector< vector<uchar> > tfinal;
		obj2.convert(tfinal);

		vector<string> tanswer;

		REP(i,0,tfinal.size()-1) {
			int clen = tfinal[i].size() - enc_dec.ivsize();

			int ivlen = 8*enc_dec.ivsize();

			uchar* iv = (uchar*)malloc(sizeof(uchar)*(enc_dec.ivsize()));
			REP(s,0,enc_dec.ivsize()-1)iv[s] = tfinal[i][s+clen];

			enc_dec.setiv(iv,ivlen);

			int plen2;
			uchar* ptext2 = enc_dec.decrypt(tfinal[i].data(),clen,plen2,0);
			string str(plen2,'\0');
			REP(s,0,plen2-1)str[s] = ptext2[s];
			tanswer.pb(str);

			free(iv);
			free(ptext2);
		}



		{
			stringstream ss;
			msgpack::sbuffer sbuf;
		    msgpack::pack(sbuf, tanswer);
		    ss.write(sbuf.data(),sbuf.size());
		    string s = ss.str();
		    cout<<"Sending final size in SE of "<<s.size()<<"\n";
		    authserv.writewithsize(sock2,s);
		}

		{
			myTtlCtTimer.donetime();
			uint64_t seTotalTimeTaken = myTtlCtTimer.getTime();
			uint64_t seNetworkTimeClient = authcli.getTotalTimeSpentInMicroSec();
			uint64_t seNetworkTimeServer = authserv.getTotalTimeSpentInMicroSec();
			uint64_t seActualTime = seTotalTimeTaken - seNetworkTimeClient - seNetworkTimeServer;
			assert(seActualTime > 0);
			cout<<"[Authority SE Time]: Total time = "<<seTotalTimeTaken<<", network cli = "<<seNetworkTimeClient<<", network serv = "<<seNetworkTimeServer<<endl;
			cout<<"[Authority SE Time]: Actual time = "<<seActualTime<<endl;
		}
		cout<<"[Authority SE comm]: With server, read = "<<authserv.getTotalBytesRead()<<", Written = "<<authserv.getTotalBytesWritten()<<endl;
		cout<<"[Authority SE comm]: With client, read = "<<authcli.getTotalBytesRead()<<", Written = "<<authcli.getTotalBytesWritten()<<endl;

		cout<<"Starting DCFE part\n";

		authserv.getAndResetTotalBytesRead();
		authserv.getAndResetTotalBytesWritten();
		authserv.getAndResetTotalTimeSpentInMicroSec();
		authcli.getAndResetTotalBytesRead();
		authcli.getAndResetTotalBytesWritten();
		authcli.getAndResetTotalTimeSpentInMicroSec();

		myTtlCtTimer.resetTime();
		myTtlCtTimer.starttime();

		if(protocol == 0) {

			string keyval;
			int noofchars;
			vi shuffind;

	    {
	        SSEtime timer;
		     	string str; authserv.readstructure(sock2,str);
		      timer.donetime();
			    cout << "[ValueRet:] [Comm:] Server-AuxServer takes ";
			    timer.printtime(0);
			    cout << "[ValueRet:] [Comm:] "<<str.size()<<" bytes from server"<<"\n";
	        msgpack::unpacker pac;
	        pac.reserve_buffer(str.size());
	        memcpy(pac.buffer(), str.data(), str.size());
	        pac.buffer_consumed(str.size());
	        msgpack::object_handle oh;
	        pac.next(oh);
	        oh.get().convert(keyval);
	        pac.next(oh);
	        oh.get().convert(noofchars);
	        pac.next(oh);
	        oh.get().convert(shuffind);
	    	}
	    	Xvalret XData;
	    	string  xoredstr = XData.compauth(shuffind,keyval,noofchars);

		    cout<<"Sending "<<xoredstr.size()<<" bytes for value retrieval from authority to client\n";

        timer.starttime();
				string tempstr = "1";
        if(xoredstr.size()!=0)
        	authcli.writewithsize(sock,xoredstr);
        else
        	authcli.writewithsize(sock,tempstr);

        timer.donetime();
		    cout << "[ValueRet:] [Comm:] AuxServer-Cli takes ";
		    timer.printtime(0);
		    cout << "[ValueRet:] [Comm:] "<<xoredstr.size()<<" bytes to client"<<"\n";
		}
		else if(protocol == 1) {
			vi shuffind2;
			unsigned long rho;
			int attr;

	    {
        SSEtime timer;
	     	string str; authserv.readstructure(sock2,str);
	     	timer.donetime();
		    cout << "[MAF:] [Comm:] Server-AuxServer takes ";
		    timer.printtime(0);
		    cout << "[MAF:] [Comm:] "<<str.size()<<" bytes from Server"<<"\n";
        msgpack::unpacker pac;
        pac.reserve_buffer(str.size());
        memcpy(pac.buffer(), str.data(), str.size());
        pac.buffer_consumed(str.size());
        // now starts streaming deserialization.
        msgpack::object_handle oh;
        pac.next(oh);
        oh.get().convert(attr);
        pac.next(oh);
        oh.get().convert(rho);
        pac.next(oh);
        oh.get().convert(shuffind2);
	    }

    	IP XDataIP;
    	unsigned long delta = XDataIP.compauth(shuffind2,rho,attr);

			SSEtime timer;
			string command2 = pathoblivc+"/MAF/a.out 2 " + to_string(networkports+6) +  " localhost "+to_string(delta);

			int a = system(command2.c_str());
			int count = 0;
			while(a != 0 && count < 10000) {
				count++;
				a = system(command2.c_str());
			}

			timer.donetime();
			cout<<"[MAF:] OblivC takes ";
			timer.printtime(0);

	    cout<<command2<<"\n";

			{
				long long share2;
				FILE* fp;
				fp = fopen("./src/EDB/P2Out","r");
				fscanf(fp,"%lld\n",&share2);
				fclose(fp);
				cout<<"Share 2 is "<<share2<<"\n";

				stringstream stream;
				msgpack::sbuffer buffer;
        msgpack::packer<msgpack::sbuffer> pk(&buffer);
        pk.pack(share2);
        stream.write(buffer.data(),buffer.size());
        SSEtime timer;
				string message = stream.str();
        authcli.writewithsize(sock,message);
				timer.donetime();
		    cout << "[MAF:] [Comm:] AuxServer-Client takes ";
		    timer.printtime(0);
		    cout << "[MAF:] [Comm:] "<<buffer.size()<<" bytes to Client"<<"\n";
			}
		}
		else if(protocol == 2)  {
			vi shuffind2;
			unsigned long rho;
			int attr;

	    {
	        SSEtime timer;
		     	string str; authserv.readstructure(sock2,str);
					timer.donetime();
			    cout << "[ChiSq:] [Comm:] Server-AuxServer takes ";
			    timer.printtime(0);
			    cout << "[ChiSq:] [Comm:] "<<str.size()<<" bytes from Server"<<"\n";
	        msgpack::unpacker pac;
	        pac.reserve_buffer(str.size());
	        memcpy(pac.buffer(), str.data(), str.size());
	        pac.buffer_consumed(str.size());
	        // now starts streaming deserialization.
	        msgpack::object_handle oh;
	        pac.next(oh);
	        oh.get().convert(attr);
	        pac.next(oh);
	        oh.get().convert(rho);
	        pac.next(oh);
	        oh.get().convert(shuffind2);
	    }

	    	IP XDataIP;
	    	unsigned long delta = XDataIP.compauth(shuffind2,rho,attr);

	    	if(controlvar) {
				controlvar = false;
			}
			else {
				controlvar = true;
			}

    	if(controlvar) {
				delta2 = delta;
				SSEtime timer;
				string command2 = pathoblivc+"/ChiSq/a.out 2 " + to_string(networkports+7) +  " localhost "+to_string(delta1)+" "+to_string(delta2);

				cout<<"Synchronising client for oblivc\n";
				int a = system(command2.c_str());
				int count = 0;
				while(a != 0 && count < 10000) {
					count++;
					a = system(command2.c_str());
				}
				cout<<"Count of iterations of waiting for client is "<<count<<"\n";
				timer.donetime();
				cout<<"[ChiSq:] OblivC takes ";
				timer.printtime(0);

				{
					long long share2;
					FILE* fp;
					fp = fopen("./src/EDB/P2Out","r");
					fscanf(fp,"%lld\n",&share2);
					fclose(fp);
					cout<<"Share 2 is "<<share2<<"\n";
					stringstream stream;
					msgpack::sbuffer buffer;
	        msgpack::packer<msgpack::sbuffer> pk(&buffer);
	        pk.pack(share2);
					stream.write(buffer.data(),buffer.size());
	        SSEtime timer;
					string message = stream.str();
			    authcli.writewithsize(sock,message);
					timer.donetime();
			    cout << "[ChiSq:] [Comm:] AuxServer-Client takes ";
			    timer.printtime(0);
			    cout << "[ChiSq:] [Comm:] "<<buffer.size()<<" bytes to Client"<<"\n";
				 }
			}
			else {
				delta1 = delta;
			}

		}
		else if(protocol == 3) {
			vi shuffind2;
			vector<string> quer;
			{
        SSEtime timer;
	     	string str; authcli .readstructure(sock,str);
				timer.donetime();
		    cout << "[Hamm:] [Comm:] Client-AuxServer takes ";
		    timer.printtime(0);
		    cout << "[Hamm:] [Comm:] "<<str.size()<<" bytes from Client"<<"\n";

		    msgpack::object_handle oh = msgpack::unpack(str.data(), str.size());
        oh.get().convert(quer);
			}
			vector<string> keyslab;
			vector<string> pandp;

			int tempPRFlen = 0;
			if(!DO) {
      	SSEtime timer;
	     	string str; authserv.readstructure(sock2,str);
				timer.donetime();
		    cout << "[Hamm:] [Comm:] Server-AuxServer takes ";
		    timer.printtime(0);
		    cout << "[Hamm:] [Comm:] "<<str.size()<<" bytes from Server"<<"\n";

		    msgpack::object_handle oh = msgpack::unpack(str.data(), str.size());
	      oh.get().convert(shuffind2);
	    }
	    else {
      	SSEtime timer;
	     	string str; authserv.readstructure(sock2,str);
				timer.donetime();
		    cout << "[Hamm:] [Comm:] Server-AuxServer takes ";
		    timer.printtime(0);
		    cout << "[Hamm:] [Comm:] "<<str.size()<<" bytes from Server"<<"\n";

				msgpack::unpacker pac;
        pac.reserve_buffer(str.size());
        memcpy(pac.buffer(), str.data(), str.size());
        pac.buffer_consumed(str.size());

        msgpack::object_handle oh;
        pac.next(oh);
        oh.get().convert(shuffind2);
        pac.next(oh);

        vector<string> enckeyslab;
        oh.get().convert(enckeyslab);
        pac.next(oh);
        oh.get().convert(tempPRFlen);

        keyslab.resize(enckeyslab.size());
        pandp.resize(enckeyslab.size());

        REP(i,0,enckeyslab.size()-1) {
        	string strplain;
					PubCryptoSystemA.envelope_open(enckeyslab[i],strplain);
					keyslab[i].resize(2*PRFlen*relcols*numberofbits);
					pandp[i].resize(randlen);
					int size1 = 2*PRFlen*relcols*numberofbits;
					int size2 = randlen;
					REP(s,0,size1-1) {
						keyslab[i][s] = strplain[s];
					}

					REP(s,0,size2-1) {
						pandp[i][s] = strplain[s+size1];
					}
		    }

	    }

			XfuncGC XDataGC;
			string garbvals = XDataGC.compauth(shuffind2,quer,pandp,keyslab,tempPRFlen);

    	cout<<"Sending "<<garbvals.size()<<" bytes for gen function from authority to server\n";
    	authserv.writewithsize(sock2,garbvals);
		}
		else if(protocol == 4) {
			Xaddpaillier XDatapaillier;
	     	string rhoencstr; authserv.readstructure(sock2,rhoencstr);
	     	cout<<"Received "<<rhoencstr.size()<<" bytes for summation(Additive Paillier) from server to authority";
	    	uint32_t z = XDatapaillier.compauth(rhoencstr);
	    	{
				stringstream stream;
				msgpack::sbuffer buffer;
		        msgpack::packer<msgpack::sbuffer> pk(&buffer);
		        pk.pack(z);
	            cout<<"Sending "<<buffer.size()<<" bytes for summation(Additive Paillier) from authority to client\n";
		        stream.write(buffer.data(),buffer.size());
			string message = stream.str();
		        authcli.writewithsize(sock,message);
			}
		}
		{
			myTtlCtTimer.donetime();
			uint64_t netTotalTimeTaken = myTtlCtTimer.getTime();
			uint64_t netNetworkTimeClient = authcli.getTotalTimeSpentInMicroSec();
			uint64_t netNetworkTimeServer = authserv.getTotalTimeSpentInMicroSec();
			uint64_t netActualTime = netTotalTimeTaken - netNetworkTimeClient - netNetworkTimeServer;
			assert(netActualTime > 0);
			cout<<"[Authority DCFE Time]: Total time = "<<netTotalTimeTaken<<", network cli = "<<netNetworkTimeClient<<", network serv = "<<netNetworkTimeServer<<endl;
			cout<<"[Authority DCFE Time]: Actual time = "<<netActualTime<<endl;
		}

		cout<<"[Authority DCFE comm] : With server, read = "<<authserv.getTotalBytesRead()<<", Written = "<<authserv.getTotalBytesWritten()<<endl;
		cout<<"[Authority DCFE comm]: With client, read = "<<authcli.getTotalBytesRead()<<", Written = "<<authcli.getTotalBytesWritten()<<endl;
    }
}
