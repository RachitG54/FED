
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
#include "SSEMDAuxServer.h"

socket_ptr SSEMDAuxServer::request(char* host, char* port) {
	socket_ptr sockserv = authserv.initialize(host,port);
	return sockserv;
}
void SSEMDAuxServer::initialize() {
	set<string> Combiningdata;

	string filename = "./src/EDB/CombinedW_data";
	ifstream inp(filename);
	inp.seekg (0, inp.end);
	int filelen = inp.tellg();
	cout<<"[Final Data]: Size of data before running init on AuxServer is "<<filelen<<" bytes.\n";

	MDAuxSrvrbytes += filelen;
	MDAuxSrvrbytesSE += filelen;

	inp.seekg (0, inp.beg);
	char* data = (char*)malloc(sizeof(char)*filelen);
	inp.read(data,filelen);
	inp.close();

    set<string> W_Data;

	{
		msgpack::object_handle oh = msgpack::unpack(data, filelen);
		oh.get().convert(W_Data);
	}
	free(data);
	cout<<"Size of W_data is "<<W_Data.size()<<"\n";
	MDSSE Wobj;
	vector< pair<string,string> > W_datasep;
	Wobj.initializeAuth(W_Data, W_datasep);
	Wobj.makeintoSE(W_datasep);
}

void SSEMDAuxServer::initializeEQ(socket_ptr sock) {

	vector<string> pidata;
	{
		string str; authserv.readstructure(sock,str);

		MDAuxSrvrbytes += authserv.getcurrBytesRead();
		MDAuxSrvrbytesSE += authserv.getcurrBytesRead();

		msgpack::object_handle oh = msgpack::unpack(str.data(), str.size());
		oh.get().convert(pidata);
	}

	timeAuxSrvr.starttime();
	timeAuxSrvrSE.starttime();

    RSAencrypt PubCryptoSystemA;

    PubCryptoSystemA.readprivkey("privkeyA.pem");
    vector<string> beta(pidata.size());
    vector<string> theta(pidata.size());
    vector<string> encphi(pidata.size());
    vector< vector< vector<string> > > vvvs(pidata.size());
 	REP(i,0,pidata.size()-1) {
		string str;
		PubCryptoSystemA.envelope_open(pidata[i],str);
		{
		    msgpack::unpacker pac;
	        pac.reserve_buffer(str.size());
	        memcpy(pac.buffer(), str.data(), str.size());
	        pac.buffer_consumed(str.size());
	        msgpack::object_handle oh;
	        pac.next(oh);
	        oh.get().convert(beta[i]);
	        pac.next(oh);
	        oh.get().convert(vvvs[i]);
	        pac.next(oh);
	        oh.get().convert(theta[i]);
	        pac.next(oh);
	        oh.get().convert(encphi[i]);
		}
	}
	MDSSE MD;
	int kwsize = MD.kwsize;
	vector< vector<string> > rMAP(pidata.size(),vector<string>(pidata.size()));
	REP(i,0,pidata.size()-1) {
		REP(j,i+1,pidata.size()-1) {
			if(j==i) continue;
			rMAP[i][j].resize(kwsize);
			REP(k,0,kwsize-1) {
				uchar t1 = (uchar)beta[i][k];
				uchar t2 = (uchar)beta[j][k];
				rMAP[i][j][k] = (char)(t1^t2);
			}
		}
	}

	cout<<"***Starting equality check phase"<<endl;

	EQCheck Checkobj;

	timeAuxSrvr.donetime();
	timeAuxSrvrSE.donetime();
	int totchecks = (pidata.size()*(pidata.size()-1))/2;

	cout<<"***Starting eqcheck.initiate."<<endl;
	Checkobj.initiate(totchecks);
	cout<<"***Done with eqcheck intitiate."<<endl;
	Checkobj.readEQshares("./src/EDB/EQCheckpreshareA");

	string sendshares;
	cout<<"Starting CheckObj.AtoS"<<endl;
	Checkobj.AtoS(rMAP, sendshares);
	cout<<"Done CheckObj.AtoS"<<endl;
	authserv.writewithsize(sock,sendshares);

	MDAuxSrvrbytes += authserv.getcurrBytesWritten();
	MDAuxSrvrbytesSE += authserv.getcurrBytesWritten();

	cout<<"Receving message from S"<<endl;
	string s; authserv.readstructure(sock,s);
	cout<<"Done receiving message from S"<<endl;

	MDAuxSrvrbytes += authserv.getcurrBytesRead();
	MDAuxSrvrbytesSE += authserv.getcurrBytesRead();

	timeAuxSrvr.starttime();
	timeAuxSrvrSE.starttime();
	UnionFind UF;

	cout<<"Starting CheckObj.compA"<<endl;
	Checkobj.compA(s, UF, rMAP.size());
	cout<<"Done CheckObj.compA"<<endl;

	map<int,int> kset;
	cout<<"Printinhg kset\n";
	int count = 0;
	REP(i,0,UF.pset.size()-1) {
		int setindex = UF.findSet(i);
		if(!present(kset,setindex)) {
			kset[setindex] = count++;
		}
	}
	cout<<kset.size()<<"\n";

	PRF PRFtau;
	PRFtau.setkeylength(KEYLENGTH);
	PRFtau.keyGen();
	PRFtau.setoutputlength(8*kwsize);

	vector<string> htau(kset.size());

	REP(i,0,kset.size()-1) {
		string str = to_string(i);
		uint32_t sz;
		uchar* htaustr = PRFtau.evaluate((const uchar*)str.data(),str.size(),sz);
		htau[i].resize(sz/8);
		REP(s,0,sz/8 - 1) {
			htau[i][s] = htaustr[s];
		}
		free(htaustr);
	}

	vector<string> seperatingdata;
	REP(i,0,vvvs.size()-1) {
		int kind = kset[UF.findSet(i)];
		REP(j,0,vvvs[i].size()-1) {
			string encgamma(kwsize,'\0');
			REP(s,0,kwsize-1) {
				encgamma[s] = (char)((uchar)vvvs[i][j][1][s]^(uchar)htau[kind][s]);
			}

			{
				msgpack::sbuffer sbuf;
				msgpack::pack(sbuf, encgamma);
				msgpack::pack(sbuf, vvvs[i][j][0]);
				stringstream ss;
				ss.write(sbuf.data(),sbuf.size());
				string s = ss.str();
				seperatingdata.pb(s);
			}
		}
	}

	timeAuxSrvr.donetime();
	timeAuxSrvrSE.donetime();

	{
		msgpack::sbuffer sbuf;
		msgpack::pack(sbuf, seperatingdata);
		stringstream ss;
		ss.write(sbuf.data(),sbuf.size());
		string s = ss.str();
		authserv.writewithsize(sock,s);

		MDAuxSrvrbytes += authserv.getcurrBytesWritten();
		MDAuxSrvrbytesSE += authserv.getcurrBytesWritten();

		cout<<"Data to be sent for culling entries is "<<sbuf.size()<<" bytes.\n";
	}

	vector<string> cullingdata;
	{
		string str; authserv.readstructure(sock,str);

		MDAuxSrvrbytes += authserv.getcurrBytesRead();
		MDAuxSrvrbytesSE += authserv.getcurrBytesRead();

		msgpack::object_handle oh = msgpack::unpack(str.data(), str.size());
		oh.get().convert(cullingdata);
	}
	cout<<"Received dataset, finishing culling\n";
	SSEtime timer;

	timeAuxSrvr.starttime();
	timeAuxSrvrSE.starttime();
	vector< pair<string,string>	> SEdata(cullingdata.size());

	REP(i,0,cullingdata.size()-1) {
		string encgammaeta;
		string encmuxi;

		string mu;

		{
		    msgpack::unpacker pac;
	        pac.reserve_buffer(cullingdata[i].size());
	        memcpy(pac.buffer(), cullingdata[i].data(), cullingdata[i].size());
	        pac.buffer_consumed(cullingdata[i].size());
	        msgpack::object_handle oh;
	        pac.next(oh);
	        oh.get().convert(encgammaeta);
	        pac.next(oh);
	        oh.get().convert(encmuxi);
		}

		{
			string temponion;
	        timer.starttime();
	        PubCryptoSystemA.envelope_open(encmuxi,temponion);

	        pair<string,string> pss;
	        {
				msgpack::object_handle oh = msgpack::unpack(temponion.data(), temponion.size());
				oh.get().convert(pss);
	        }
	        SEdata[i].second = pss.first;
	        mu = pss.second;
	        timer.donetime();
		}

		SEdata[i].first.resize(kwsize);
		REP(s,0,kwsize-1) {
			SEdata[i].first[s] = (char)((uchar)encgammaeta[s] ^ (uchar)mu[s]);
		}
	}

	cout<<"Time taken to calculate decryptions is: ";
	timer.printtime(0);

	timeAuxSrvr.donetime();
	timeAuxSrvrSE.donetime();
	MD.makeintoSE(SEdata);

	timeAuxSrvr.starttime();
	timeAuxSrvrSE.starttime();
	cout<<"Sharing the mapped keywords\n";
	vector< pair<string,string> > thetaencphi(kset.size());

	tr(kset,it) {
		int kind = it->second;
		int tkstar = it->first;
		thetaencphi[kind].first.resize(kwsize);
		REP(s,0,kwsize-1) {
			thetaencphi[kind].first[s] = (char)((uchar)htau[kind][s] ^ (uchar)theta[tkstar][s]);
		}
		thetaencphi[kind].second = encphi[tkstar];
	}

	timeAuxSrvr.donetime();
	timeAuxSrvrSE.donetime();

	{
		msgpack::sbuffer sbuf;
		msgpack::pack(sbuf, thetaencphi);
		stringstream ss;
		ss.write(sbuf.data(),sbuf.size());
		string s = ss.str();
		authserv.writewithsize(sock,s);

		MDAuxSrvrbytes += authserv.getcurrBytesWritten();
		MDAuxSrvrbytesSE += authserv.getcurrBytesWritten();

		cout<<"Data to be sent for mapping keywords is "<<sbuf.size()<<" bytes.\n";
	}
	int htausize = htau.size();
	writetomsgpackfile(htausize,"./src/keys/L");
	PRFtau.storekey("KtauEQ");
}

void SSEMDAuxServer::merge(int numberofDOs) {

	int totcount = 0;
	REP(i,0,numberofDOs-1) {
		int rowNum;
		FILE* fp;
		string fname = "./src/EDB/idlist_" + to_string(i);
		fp = fopen(fname.c_str(),"rb");
		fscanf(fp,"%d\n",&rowNum);
		totcount += rowNum;
		fclose(fp);

	}

	vector<ll> combinedidlist(totcount);
	int prevcountid = 0;
	REP(i,0,numberofDOs-1) {
		int rowNum;
		FILE* fp;
		string fname = "./src/EDB/idlist_" + to_string(i);
		fp = fopen(fname.c_str(),"rb");
		fscanf(fp,"%d\n",&rowNum);

		MDAuxSrvrbytes += 4*rowNum;
		MDAuxSrvrbytesSE += 4*rowNum;

		REP(j,0,rowNum-1) {
			fscanf(fp,"%lld\n",&combinedidlist[prevcountid+j]);
		}
		fclose(fp);
		prevcountid += rowNum;

	}


	if(protocol == 0) {

		RSAencrypt PubCryptoSystemA;
	    PubCryptoSystemA.readprivkey("privkeyA.pem");

		vector< string > encDATA;
		SSEtime timer;
		int sz = readfrommsgpackfile(encDATA,"./src/EDB/Xvalret");
		cout<<"Read size is "<<sz<<" bytes.\n";

		MDAuxSrvrbytes += sz;

		int rowNum = encDATA.size();

		timeAuxSrvr.starttime();
		vector<string> thetastr(rowNum);
	    vector< pair<string,int> > enckeys(rowNum);

		REP(i,0,encDATA.size()-1) {

			{
				string str;
				PubCryptoSystemA.envelope_open(encDATA[i],str);
				msgpack::unpacker pac;
        pac.reserve_buffer(str.size());
        memcpy(pac.buffer(), str.data(), str.size());
        pac.buffer_consumed(str.size());
        msgpack::object_handle oh;
        pac.next(oh);
        oh.get().convert(thetastr[i]);
        pac.next(oh);
        oh.get().convert(enckeys[i]);
		  }
		}


		uchar** theta = (uchar**)malloc(sizeof(uchar*)*thetastr.size());

		int noofchars = 0;

		PRF PRFKval;
		Xvalret inst;
		PRFKval.setkeylength(inst.KEYLENGTH);
		PRFKval.keyGen();

		REP(j,0,thetastr.size()-1) {
			noofchars = thetastr[j].size();
			uchar* FKidstr = FKid(PRFKval,j,noofchars);
			theta[j] = (uchar*)malloc(sizeof(uchar)*noofchars);
			REP(k,0,noofchars-1) {
				theta[j][k] = (uchar)thetastr[j][k];
				timer.starttime();
				int data1 = (uchar)theta[j][k];
				int data2 = (uchar)FKidstr[k];
				int data = data1 ^ data2;
				theta[j][k] = (uchar)data;
				timer.donetime();
			}
			free(FKidstr);
		}

		cout<<noofchars<<" "<<thetastr.size()<<"\n";

		PRFKval.storekey("Kval");

		cout<<"[Final Data]: DCFE AuxServer merge step takes ";
		timer.printtime(0);

		timeAuxSrvr.donetime();

		{
			ofstream fout;
			fout.open("./src/EDB/Xvalret",ios::out| ios::binary);
			stringstream ss;
			ss << noofchars<<"\n"<<thetastr.size()<<"\n";
			REP(i,0,totcount-1) {
				ss.write((char*)theta[i],noofchars);
				free(theta[i]);
			}
			free(theta);
			string str = ss.str();
			fout.write(str.data(),str.size());
			fout.close();
			MDAuxSrvrbytes += str.size();
			cout<<"[Final Data]: Size of encrypted merged data by AuxServer is "<<str.size()<<" bytes\n";
		}

    cout<<"Merged Xvalret\n";
    {
			msgpack::sbuffer sbuf;
      msgpack::pack(sbuf, enckeys);
      ofstream out("./src/EDB/Xkeys");
      out.write(sbuf.data(),sbuf.size());
      out.close();

			MDAuxSrvrbytes += sbuf.size();
      cout<<"[Final Data]: Size of encrypted merged data (keys) by AuxServer is "<<sbuf.size()<<" bytes\n";
		}
	}
	else if(protocol == 1 || protocol == 2) {
		RSAencrypt PubCryptoSystemA;
    PubCryptoSystemA.readprivkey("privkeyA.pem");

		vector< string > encDATA;
		SSEtime timer;
		int sz = readfrommsgpackfile(encDATA,"./src/EDB/XsumIP");
		cout<<"Read size is "<<sz<<" bytes.\n";

		MDAuxSrvrbytes += sz;

		int rowNum = encDATA.size();

		timeAuxSrvr.starttime();

		vector< vector< unsigned long> > XData(rowNum);
    vector< pair<string,int> > enckeys(rowNum);

		REP(i,0,encDATA.size()-1) {

			{
				string str;
				PubCryptoSystemA.envelope_open(encDATA[i],str);
				msgpack::unpacker pac;
        pac.reserve_buffer(str.size());
        memcpy(pac.buffer(), str.data(), str.size());
        pac.buffer_consumed(str.size());
        msgpack::object_handle oh;
        pac.next(oh);
        oh.get().convert(XData[i]);
        pac.next(oh);
        oh.get().convert(enckeys[i]);
		  }
		}

		PRF PRFK_ip;
		IP inst;
    unsigned long N = 1 << inst.messagedim;
		PRFK_ip.setkeylength(inst.KEYLENGTH);
		PRFK_ip.keyGen();

    timer.starttime();
    REP(j,0,XData.size()-1) {
    	REP(k,0,XData[j].size()-1) {
        unsigned long val = inst.evaluate(PRFK_ip,k,j)%N;
        XData[j][k] = (val+XData[j][k])%N;
    	}
    }
    timer.donetime();

    PRFK_ip.storekey("K_ip2");


		timeAuxSrvr.donetime();

		cout<<"[Final Data]: DCFE AuxServer merge step takes ";
		timer.printtime(0);
		{
      msgpack::sbuffer sbuf;
      msgpack::pack(sbuf, XData);
      ofstream out("./src/EDB/XsumIP");
      out.write(sbuf.data(),sbuf.size());
      out.close();
      cout<<"[Final Data]: Size of merged data by Aux / XSetIP size = "<<sbuf.size()<<endl;
			MDAuxSrvrbytes += sbuf.size();
	  }
	  cout<<"Merged XsumIP\n";
	  {
			msgpack::sbuffer sbuf;
      msgpack::pack(sbuf, enckeys);
      ofstream out("./src/EDB/Xkeys");
      out.write(sbuf.data(),sbuf.size());
      out.close();
      cout<<"[Final Data]: Size of encrypted merged data (keys) by Aux is "<<sbuf.size()<<" bytes\n";

			MDAuxSrvrbytes += sbuf.size();
		}
	}
	else if(protocol == 3) {
		SSEtime timer;
		vector<string> pvs;

		{
			REP(i,0,numberofDOs-1) {
				vector<string> pvstemp;
				{
			    ifstream inp("./src/EDB/XfuncGC_"+to_string(i));
			    inp.seekg (0, inp.end);
			    int filelen = inp.tellg();
			    inp.seekg (0, inp.beg);
			    char* data = (char*)malloc(sizeof(char)*filelen);
			    inp.read(data,filelen);
			    cout<<filelen<<"\n";

					MDAuxSrvrbytes += filelen;
			    inp.close();
			    {
			        msgpack::object_handle oh = msgpack::unpack(data, filelen);
			        oh.get().convert(pvstemp);
			    }
			    free(data);
				}

				timeAuxSrvr.starttime();
				timer.starttime();
				REP(j,0,pvstemp.size()-1) {
					pvs.pb(pvstemp[j]);
				}
				timer.donetime();
				timeAuxSrvr.donetime();
			}
		}


		cout<<"[Final Data]: DCFE AuxServer merge step takes ";
		timer.printtime(0);

		{
			int sz = writetomsgpackfile(pvs,"./src/EDB/XfuncGC");

			MDAuxSrvrbytes += sz;
			cout<<"[Final Data]: Size of merged data by AuxSerer / XfuncGC size = "<<sz<<" bytes."<<endl;
		}
	  cout<<"Merged XfuncGC\n";
	}
	else if(protocol == 4) {

		SSEtime timer;
		vector<string> encpaillier;
		REP(i,0,numberofDOs-1) {
			vector< string > tempobj;
			int sz = readfrommsgpackfile(tempobj,"./src/EDB/Xaddpaillier_" + to_string(i));
			MDAuxSrvrbytes += sz;

			timeAuxSrvr.starttime();
			timer.starttime();
			REP(j,0,tempobj.size()-1) {
				encpaillier.pb(tempobj[j]);
			}
			timer.donetime();
			timeAuxSrvr.donetime();
		}
		int sz = writetomsgpackfile(encpaillier,"./src/EDB/Xaddpaillier");
		cout<<"Size of merged dataset is "<<sz<<" bytes.\n";

		MDAuxSrvrbytes += sz;

	}



	{
		msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, combinedidlist);
    ofstream out("./src/EDB/Combinedidlist");
    out.write(sbuf.data(),sbuf.size());
    out.close();

		MDAuxSrvrbytes += sbuf.size();
		MDAuxSrvrbytesSE += sbuf.size();
	}
	
}
