#include "MDSSE.h"
MDSSE::MDSSE() {
	group.setDH();
	KEYLENGTH = 8*group.getDHsubgroupsize();
}
void MDSSE::setkeysServer() {
	OPRF OPRFK_MDSSE;
	OPRFK_MDSSE.setkeylength(KEYLENGTH);
	OPRFK_MDSSE.keyGen();
	OPRFK_MDSSE.storekey("K_mdsse");
	{
		RSAencrypt PubCryptoSystem;
		PubCryptoSystem.generate_key_tofile("privkeyS.pem","pubkeyS.pem");
	}
}
void MDSSE::setkeysAuxServer() {
	RSAencrypt PubCryptoSystemA;
	PubCryptoSystemA.generate_key_tofile("privkeyA.pem","pubkeyA.pem");
}

int MDSSE::initializeDB(db2 &database, vlli &idlist, vector< vector<string>> &XData, map<ll,int> &idmap) {

	vector<string> allLines;
	getAllLinesFromFile(pathGenome, allLines);
	string line;
	int allLinesIdx = 0;
	line = allLines[allLinesIdx++];


	vector<string> xx;
	boost::split(xx, line, boost::is_any_of(" \t"), boost::token_compress_on);
	assert(xx.size() == 3);
	int rowNum = atoi(xx[0].c_str());
	int colNum = atoi(xx[1].c_str());
	int searchablefilter = atoi(xx[2].c_str());

  uchar* id = (uchar*)malloc(sizeof(uchar)*8);
  int relcols = colNum - searchablefilter;


  idlist.resize(rowNum);
  XData.resize(rowNum);


	REP(i,0,rowNum-1) {
    XData[i].resize(relcols);

    int rc = RAND_bytes(id, 8);
    if(rc!=1) {
    	cout<<"Error in generating random number\n";
    	exit(1);
    }
    uint64_t randid = toBigInt(id,8);
    ll signedid = (ll)randid;
    if(signedid<0) signedid = -signedid;

		idlist[i] = signedid;

		if(present(idmap,signedid)) {
			cout<<i<<"\n";
			cout<<"Collision at id\n";
			exit(1);
		}

		idmap[signedid] = i;
		xx.clear();
		line = allLines[allLinesIdx++];
		boost::split(xx, line, boost::is_any_of(" \t"), boost::token_compress_on);

		REP(j,0,searchablefilter-1) {
			string kw = xx[j];
			database.DB_inv[kw].insert(idlist[i]);
			database.W.insert(kw);
		}
		REP(attr,searchablefilter,colNum-1) {
			XData[i][attr - searchablefilter] = xx[attr];
		}
	}
	cout<<"Read finished\n";

	if(idlist.size() != idmap.size()) {
		cout<<"Error in construction of random id's at multi-do\n";
		exit(1);
	}

	free(id);
	return rowNum;
}

void MDSSE::initializeEQ(int DOindex, db2 &database, int rowNum, vlli &idlist, map<ll,int> &idmap) {
		timeDO.starttime();
		timeDOSE.starttime();

    RSAencrypt PubCryptoSystem;
    RSAencrypt PubCryptoSystemA;

    PubCryptoSystem.readpubkey("pubkeyS.pem");

    PubCryptoSystemA.readpubkey("pubkeyA.pem");


    RSAencrypt PubCryptoSystem2;
    RSAencrypt PubCryptoSystemA2;

    PubCryptoSystem2.readpubkey("pubkeyS2.pem");
    PubCryptoSystemA2.readpubkey("pubkeyA2.pem");

    vector<string> istrdata(rowNum);
    REP(i,0,rowNum-1) {
    	string istr = to_string(idlist[i]);
    	istrdata[i] = PubCryptoSystem.encrypt(istr.size(),(const uchar*)istr.data());
    }
    cout<<"Encrypted id's\n";

    SSEtime sealenctime1;
    SSEtime sealenctime2;

    vector< pair<string,string> > vpss(database.W.size());
    int itercountout = 0;
	tr(database.W,it) {
		string tau = *it;
  	uchar* alpha = (uchar*)malloc(sizeof(uchar)*kwsize);
  	uchar* phi = (uchar*)malloc(sizeof(uchar)*kwsize);
    {
    	int rc = RAND_bytes(alpha, kwsize);
    	if(rc!=1) { exit(1); }
    }
    {
    	int rc = RAND_bytes(phi, kwsize);
    	if(rc!=1) { exit(1); }
		}
  	string beta(kwsize,'\0');
  	string theta(kwsize,'\0');
  	string tau2(kwsize,'\0');

  	REP(i,0,tau.size()-1) {
  		tau2[i] = tau[i];
  	}

    REP(i,0,kwsize-1) {
    	uchar cdash = alpha[i] ^ ((uchar)tau2[i]);
    	beta[i] = (char)cdash;
    	uchar c = phi[i] ^ ((uchar)tau2[i]);
    	theta[i] = (char)c;
    }
  	string encphi = PubCryptoSystem.encrypt(kwsize,phi);

  	free(phi);
  	vector< vector<string> > vvs(database.DB_inv[*it].size());
  	int itercount = 0;
		tr(database.DB_inv[*it],it2) {
			vvs[itercount].resize(2);

			int ind = idmap[*it2];
			uchar* gamma = (uchar*)malloc(sizeof(uchar)*kwsize);
	    	uchar* eta = (uchar*)malloc(sizeof(uchar)*kwsize);


			pair<string,string> data;
			data.first.resize(kwsize);
			vvs[itercount][1].resize(kwsize);
			{
			    int rc = RAND_bytes(gamma, kwsize);
			    if(rc!=1) { exit(1); }
			}
			{
			    int rc = RAND_bytes(eta, kwsize);
			    if(rc!=1) { exit(1); }
			}

	    string mu(kwsize,'\0');
	    REP(s,0,kwsize-1) {
	    	mu[s] = (char)(eta[s] ^ gamma[s]);
	    	data.first[s] = eta[s];
	    	vvs[itercount][1][s] = gamma[s];
	    }

    	{
			pair<string,string> pss;
			pss.first = istrdata[ind];
			pss.second = mu;

			msgpack::sbuffer sbuf;
			msgpack::pack(sbuf, pss);

			PubCryptoSystemA.envelope_seal((uchar*)sbuf.data(),sbuf.size(),data.second);
    	}


			{
				msgpack::sbuffer sbuf;
				msgpack::pack(sbuf, data);
				sealenctime2.starttime();
				PubCryptoSystem.envelope_seal((uchar*)sbuf.data(),sbuf.size(),vvs[itercount][0]);
				sealenctime2.donetime();
			}


			free(gamma);
			free(eta);
			itercount++;
		}

		{
			msgpack::sbuffer sbuf;
			msgpack::pack(sbuf, beta);
			msgpack::pack(sbuf, vvs);
			msgpack::pack(sbuf, theta);
			msgpack::pack(sbuf, encphi);
			PubCryptoSystemA.envelope_seal((uchar*)sbuf.data(),sbuf.size(),vpss[itercountout].second);
		}

  	vpss[itercountout].first.resize(kwsize);
  	REP(i,0,kwsize-1) {
  		vpss[itercountout].first[i] = (char)alpha[i];
  	}
  	free(alpha);
		itercountout++;
	}
	timeDO.donetime();
	timeDOSE.donetime();
	cout<<"Time taken for 1st envelope is ";
	sealenctime1.printtime(0);

	cout<<"Time taken for 2nd envelope is ";
	sealenctime2.printtime(0);
  {
    ofstream fout;
    fout.open("./src/EDB/idlist_"+to_string(DOindex),ios::out| ios::binary);
    stringstream ss;
    ss << rowNum<<"\n";
    REP(i,0,rowNum-1) {
        ss<<idlist[i]<<"\n";
    }
    string str = ss.str();
    fout.write(str.data(),str.size());
    fout.close();

    MDDObytes += str.size();
    MDDObytesSE += str.size();
  }

	{
		msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, vpss);
    ofstream out("./src/EDB/W_data"+to_string(DOindex));
    out.write(sbuf.data(),sbuf.size());
		cout<<"[Final Data]: Data to merge has size "<<sbuf.size()<<" bytes.\n";
    MDDObytes += sbuf.size();
    MDDObytesSE += sbuf.size();
    out.close();
	}
}
void MDSSE::initializeDO_1(stringstream &ss, BIGNUM** &rinverses, db2 &database) {
	SSEtime ttlTimer;
	OPRF F;
	vector<string> OPRFpart1(database.W.size());
	rinverses = (BIGNUM**)malloc(sizeof(BIGNUM*)*database.W.size());
	int count = 0;
	timeDO.starttime();
	timeDOSE.starttime();
	tr(database.W,it) {
		F.evaluateP1((const uchar*)(*it).c_str(),(*it).size(),&rinverses[count],OPRFpart1[count]);
		count++;
	}
	timeDO.donetime();
	timeDOSE.donetime();

	ttlTimer.donetime();
	cout<<"[Final Data]: Collect OPRF: Init_DO1 takes ";
	ttlTimer.printtime(0);

	{
		msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, OPRFpart1);
    ss.write(sbuf.data(),sbuf.size());
	}

}
void MDSSE::initializeServe(string &s,stringstream &ss) {
	timeSrvr.starttime();
	timeSrvrSE.starttime();
	OPRF F;
	uint32_t keylen;
	uchar* key = NULL;
	key = readkeys(keylen, "K_mdsse");
	F.setkeylength(keylen); F.setkey(key);
	free(key);

	msgpack::object_handle oh = msgpack::unpack(s.data(), s.size());
  vector< string > OPRFpart1;
  oh.get().convert(OPRFpart1);

	SSEtime ttlTimer;
  vector< string > OPRFpart2(OPRFpart1.size());
  REP(i,0,OPRFpart1.size()-1) {
	F.evaluateP2(OPRFpart1[i],OPRFpart2[i]);
  }

  {
		msgpack::sbuffer sbuf;
	  msgpack::pack(sbuf, OPRFpart2);
	  ss.write(sbuf.data(),sbuf.size());
	}
	timeSrvr.donetime();
	timeSrvrSE.donetime();

	ttlTimer.donetime();
	cout<<"[Final Data]: Collect OPRF: Server takes ";
	ttlTimer.printtime(0);
}

void MDSSE::initializeDO_2(string &s, BIGNUM** &rinverses, int DOindex, db2 &database, int rowNum, vlli &idlist, map<ll,int> &idmap) {

	OPRF F;
	timeDO.starttime();
	timeDOSE.starttime();

	msgpack::object_handle oh = msgpack::unpack(s.data(), s.size());
  vector< string > OPRFpart2;
  oh.get().convert(OPRFpart2);
  int totsize = 0;
  tr(database.DB_inv,it) {
  	totsize += (it->second).size();
  }

  vector< string > OPRF_WtoS(totsize);

  RSAencrypt PubCryptoSystem;
  RSAencrypt PubCryptoSystemA;

  PubCryptoSystem.readpubkey("pubkeyS.pem");
  PubCryptoSystemA.readpubkey("pubkeyA.pem");

  SSEtime timerOPRF;
  vector<string> finOPRF(OPRFpart2.size());
  {
  	int count = 0;
  	tr(database.W,it) {
		F.fin_evaluateP1(OPRFpart2[count],rinverses[count],finOPRF[count]);
		free(rinverses[count]);
		count++;
	}
	free(rinverses);

	}
  	timerOPRF.donetime();
	cout<<"[Final Data]: OPRF initialisation2 for SE takes ";
	timerOPRF.printtime(0);

	SSEtime ttltimerSE;
    vector<string> istrdata(rowNum);
    REP(i,0,rowNum-1) {
    	string istr = to_string(idlist[i]);
    	istrdata[i] = PubCryptoSystem.encrypt(istr.size(),(const uchar*)istr.data());
    }

    int count = 0;
    int itercount = 0;
	tr(database.W,it) {
		tr(database.DB_inv[*it],it2) {
			int ind = idmap[*it2];
			pair<string,string> data = make_pair(finOPRF[count],istrdata[ind]);

			msgpack::sbuffer sbuf;
		    msgpack::pack(sbuf, data);

			PubCryptoSystemA.envelope_seal((uchar*)sbuf.data(),sbuf.size(),OPRF_WtoS[itercount]);
			itercount++;
		}
		count++;
	}

	timeDO.donetime();
	timeDOSE.donetime();

	ttltimerSE.donetime();
	cout<<"[Final Data]: Init_DO2 for SE takes ";
	ttltimerSE.printtime(0);

  {
    ofstream fout;
    fout.open("./src/EDB/idlist_"+to_string(DOindex),ios::out| ios::binary);
    stringstream ss;
    ss << rowNum<<"\n";
    REP(i,0,rowNum-1) {
        ss<<idlist[i]<<"\n";
    }
    string str = ss.str();
    fout.write(str.data(),str.size());
    fout.close();

    MDDObytes += str.size();
    MDDObytesSE += str.size();
  }

	{
		msgpack::sbuffer sbuf;
		msgpack::pack(sbuf, OPRF_WtoS);
		ofstream out("./src/EDB/W_data"+to_string(DOindex));
		out.write(sbuf.data(),sbuf.size());
		MDDObytes += sbuf.size();
		MDDObytesSE += sbuf.size();

		cout<<"[Final Data]: Data to merge has size "<<sbuf.size()<<" bytes.\n";
		out.close();
	}
}
void MDSSE::initializeDCFE(vector< vector<string> > &XData, int DOindex) {
	if(protocol == 0) {
		Xvalret MDXdata;
		MDXdata.multiDOinitialize(XData,DOindex);
	}
	else if(protocol == 1 || protocol == 2) {
		IP MDXdata;
		MDXdata.multiDOinitialize(XData,DOindex);
	}
	else if(protocol == 3) {
		XfuncGC MDXdata;
		MDXdata.multiDOinitialize(XData,DOindex);
	}
	else if(protocol == 4) {
		Xaddpaillier MDXdata;
		MDXdata.multiDOinitialize(XData,DOindex);
	}
}

void MDSSE::makeintoSE(vector< pair<string,string> > &W_datasep) {

  db database;
  map<string,int> wordcount;
  map<string,int> idstr;
  vector<string> id_inv;
  int count = 0;

	SSEtime ttlTimer;

	timeAuxSrvr.starttime();
	timeAuxSrvrSE.starttime();

	REP(itercount,0,W_datasep.size()-1) {
		int presindex = -1;
		if(!present(idstr,W_datasep[itercount].second)) {
			presindex = count;

			id_inv.pb(W_datasep[itercount].second);
			idstr[W_datasep[itercount].second] = count++;
		}
		else {
			presindex = idstr[W_datasep[itercount].second];
		}
		database.DB_inv[W_datasep[itercount].first].insert(presindex);

		database.W.insert(W_datasep[itercount].first);
		wordcount[W_datasep[itercount].first]++;
	}

	ttlTimer.donetime();
	cout<<"[Final Data]: Init time by AuxServer : ";
	ttlTimer.printtime(0);

	timeAuxSrvr.donetime();
	timeAuxSrvrSE.donetime();
  {
  	msgpack::sbuffer sbuf;
    ofstream fout;
    msgpack::pack(sbuf, wordcount);
    fout.open("./src/DB/wordcount",ios::out|ios::trunc);
    fout<<sbuf.size()<<"\n";
    fout.write(sbuf.data(),sbuf.size());
    fout.close();
		cout<<"[Final Data]: AuxServer Init output: AuxServer wordcount size is : "<<sbuf.size()<<"\n";
  }


  {
    ofstream fout;
    fout.open("./src/EDB/kw_idlist",ios::out);
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, database.DB_inv);
    fout.write(sbuf.data(),sbuf.size());
    fout.close();
		cout<<"[Final Data]: AuxServer Init output: AuxServer inverted index size is : "<<sbuf.size()<<"\n";
	}

	{
    ofstream fout;
    fout.open("./src/EDB/OPRFkw",ios::out);
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, database.W);
    fout.write(sbuf.data(),sbuf.size());
    fout.close();
		cout<<"[Final Data]: AuxServer Init output: AuxServer total keywords size is : "<<sbuf.size()<<"\n";
	}

	{
    ofstream fout;
    fout.open("./src/EDB/id_invDO",ios::out);
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, id_inv);
    fout.write(sbuf.data(),sbuf.size());
    fout.close();
		MDAuxSrvrbytes += sbuf.size();
		MDAuxSrvrbytesSE += sbuf.size();
		cout<<"[Final Data]: AuxServer Init output: AuxServer inverse DO size(map answer back to DCFE) is : "<<sbuf.size()<<"\n";
	}

}
void MDSSE::initializeAuth(set<string> &W_data, vector< pair<string,string> > &W_datasep) {

	timeAuxSrvr.starttime();
	timeAuxSrvrSE.starttime();
  RSAencrypt PubCryptoSystemA;

  PubCryptoSystemA.readprivkey("privkeyA.pem");

  W_datasep.resize(W_data.size());
  int itercount = 0;
  tr(W_data,it) {
		string t = *it;
		string s;
		PubCryptoSystemA.envelope_open(t,s);

		msgpack::object_handle oh = msgpack::unpack(s.data(), s.size());
		oh.get().convert(W_datasep[itercount]);
		itercount++;
	}

	timeAuxSrvr.donetime();
	timeAuxSrvrSE.donetime();
}
