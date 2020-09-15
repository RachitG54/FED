#include "TSet.h"

void printstr(uchar* result, uint32_t len) {
	for (unsigned int i = 0; i != len; i++)
        printf("%d ", (unsigned int)result[i]);
    printf("\n");
}

hashelements splitintoBLK(uint32_t len_B, uint32_t len_K, uint32_t len_L, uchar* hashin, uint32_t len, uint32_t B) {
	if(len_B+len_L+len_K > len) {
		cout<<len_B<<"\t"<<len_L<<"\t"<<len_K<<"\n";
		cout<<"len is "<<len<<"\n";
		std::cout<<"Division of hash function not possible\n";
		exit(1);
	}
	hashelements ele;
	uchar* strB = (uchar*)malloc(sizeof(uchar)*(len_B)); for(uint32_t i=0;i<len_B;i++) strB[i] = hashin[i];
	ele.K = (uchar*)malloc(sizeof(uchar)*(len_K)); for(uint32_t i=0;i<len_K;i++) ele.K[i] = hashin[len_B+i];
	ele.L = (uchar*)malloc(sizeof(uchar)*(len_L)); for(uint32_t i=0;i<len_L;i++) ele.L[i] = hashin[len_B+len_K+i];
	ele.B = toInt(strB)%B;
	free(strB);
	return ele;
}

TSet::TSet() {
	RAND_poll();					//Seed random number
	KEYLENGTH = 256;
	totaldocs = 5200000;
	lambda = 90;
	B = 8000;//5000;//1000;//10000;
	S = 3000;//1000;//1000;//1000;
	double val = log (256);
	len_B = 4;//((uint32_t)(log(B)/val))+1;
	len_L = (lambda/8)+1;

	len_K = ((uint32_t)(log(2*totaldocs)/val))+1;

	if(len_K<4) len_K = 4;
	cout<<len_B<<" "<<len_K<<" "<<len_L<<"\n";
}
db TSet::getdata(string name) {

	cout<<"Data read starting\n";
    auto start = high_resolution_clock::now();


	db database;
	ifstream fin;
	msgpack::unpacker pac;
    fin.open("./src/DB/"+name,ios::in);
    stringstream ss;
    ss << fin.rdbuf();
    string buffer = ss.str();
    pac.reserve_buffer(buffer.size());
    memcpy(pac.buffer(), buffer.data(), buffer.size());
    pac.buffer_consumed(buffer.size());

    msgpack::object_handle oh;
    pac.next(oh);
    msgpack::object msgobj = oh.get();
    msgobj.convert(database.DB);
    pac.next(oh);
    msgobj = oh.get();
    msgobj.convert(database.DB_inv);
    pac.next(oh);
    msgobj = oh.get();
    msgobj.convert(database.W);
    fin.close();

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    cout << "Time taken by function: " << duration.count() << " milliseconds" << endl;
    cout<<"Data read\n";
    return database;
}

db TSet::genomeinitialize(string name) {
	cout<<"Data read starting\n";
	SSEtime timer;
	map<string, int> wordcount;
	db database;
	ifstream inp(pathGenome);
	string line;
	getline(inp, line);
	vector<string> xx;
	boost::split(xx, line, boost::is_any_of(" \t"), boost::token_compress_on);
	assert(xx.size() == 3);
	int rowNum = atoi(xx[0].c_str());
	int colNum = atoi(xx[1].c_str());
	int searchablefilter = atoi(xx[2].c_str());

	REP(i,0,rowNum-1) {
		xx.clear();
		getline(inp, line);
		boost::split(xx, line, boost::is_any_of(" \t"), boost::token_compress_on);
		REP(attr,0,searchablefilter-1) {
			string w = xx[attr];
	    	if(present(database.DB[i],w)) continue;
	    	if(present(wordcount,w)) wordcount[w]++;
	    	else wordcount[w] = 1;
	        database.DB[i].insert(w);
	        database.DB_inv[w].insert(i);
	        database.W.insert(w);
		}
	}

    inp.seekg (0, inp.end);
    int filelen = inp.tellg();
	inp.close();

  	int wordsize;
    {
    	msgpack::sbuffer sbuf;
        ofstream fout;
        msgpack::pack(sbuf, wordcount);
        fout.open("./src/DB/wordcount",ios::out|ios::trunc);
        fout<<sbuf.size()<<"\n";
        fout.write(sbuf.data(),sbuf.size());
        fout.close();
        wordsize = sbuf.size();
    }
    int dbsize;

    {
	    ofstream fout;
	    fout.open("./src/DB/database",ios::out);
	    msgpack::sbuffer sbuf;
	    msgpack::packer<msgpack::sbuffer> pk(&sbuf);
	    pk.pack(database.DB); pk.pack(database.DB_inv); pk.pack(database.W);
	    fout.write(sbuf.data(),sbuf.size());
	    fout.close();
	    dbsize = sbuf.size();
	}
	timer.donetime();
	cout<<"[SE:] Read takes ";
	timer.printtime(0);
	printf("[Final SE]: Size of unencrypted input file is filelen = (%d) bytes\n",filelen);
	return database;
}

db TSet::datainitialize_basic() {
	db database;
	int d = 1000;

	ifstream wordsf;
	wordsf.open("/media/rachit/rachit/words_alpha.txt",ios::in);
	set<string> words;
	string tempw;
	while(wordsf >> tempw) {
		words.insert(tempw);
	}
	wordsf.close();

	ifstream fin;

	ifstream filename;
	filename.open("./src/Docs/filenames.txt",ios::in);
	cout<<"Data read starting\n";
	map<string, int> wordcount;
  auto start = high_resolution_clock::now();
    REP(i,1,d) {
    	string fname;
    	filename>>fname;

			fin.open(fname,ios::in);
	    string w;
	    while(fin>>w) {
	    	if(!present(words,w)) continue;
	    	if(present(database.DB[i],w)) continue;
	    	if(present(wordcount,w)) wordcount[w]++;
	    	else wordcount[w] = 1;
	        database.DB[i].insert(w);
	        database.DB_inv[w].insert(i);
	        database.W.insert(w);
	    }
	    fin.close();
    }
    filename.close();

    msgpack::sbuffer sbuf;
    ofstream fout;
    msgpack::pack(sbuf, wordcount);
    fout.open("./src/DB/wordcount",ios::out|ios::trunc);
    fout<<sbuf.size()<<"\n";
    fout.write(sbuf.data(),sbuf.size());
    fout.close();
    if(1) {
	    ofstream fout;
	    fout.open("./src/DB/database",ios::out);
	    msgpack::sbuffer sbuf;
	    msgpack::packer<msgpack::sbuffer> pk(&sbuf);
	    pk.pack(database.DB); pk.pack(database.DB_inv); pk.pack(database.W);
	    fout.write(sbuf.data(),sbuf.size());
	    fout.close();
		}
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    cout << "Time taken by function: " << duration.count() << " milliseconds" << endl;
    cout<<"Data read\n";

    return database;
}

void TSet::TSetGetTag(uchar* key, uint32_t keylen, const uchar* w, uint32_t wlen, uchar* &stag, uint32_t &stagbitlen) {
	PRF PRF_obj;
	PRF_obj.setkeylength(keylen);
	PRF_obj.setkey(key);
    stag = PRF_obj.evaluate(w,wlen,stagbitlen);
}

void TSet::TSetSetup(  map<string, vector< vector<uchar> > > &Tlist , uint32_t newlen_K, string name) {
	len_K = newlen_K+1;
	bool repeat = false;
	vector< set<int> > Free(B);
	T.clear();
	T.resize(B);
	cout<<"Starting setup\n";

	SSEtime timer;
	do {
	    REP(i,0,B-1) {
	    	Free[i].clear();
	    	T[i].clear();
	    	T[i].resize(S);
	    }
	    cout<<"Setup done\n";
		PRF PRF_obj;
		PRF_obj.setkeylength(KEYLENGTH);
		PRF_obj.keyGen();
	    repeat = false;
	    int totalsize = 0;
	    tr(Tlist,it) {
	    	const uchar* w = (const uchar*)((it->first).c_str());
	    	uint32_t stagbitlen;
	        uchar* stag = PRF_obj.evaluate(w,(it->first).size(),stagbitlen);
	        uint32_t cnt = 0;
	        totalsize += Tlist[it->first].size();
	        if(it->first == "not" || it->first == "yes" || it->first == "no" || it->first == "rare" || it->first == "extremely" || it->first == "extreme" || it->first == "is" || it->first == "was" || it->first == "rarely" || it->first == "Case" || it->first == "Control")
	        	cout<<"Size : "<<it->first<<" "<<Tlist[it->first].size()<<"\n";
	        REP(k,0,Tlist[it->first].size()-1) {
	        	PRF hashPRF;
	        	hashPRF.setkeylength(stagbitlen);
	        	hashPRF.setkey(stag);
	        	uint32_t hashinbitlen;
	        	string str = to_string(cnt);
	            unsigned char* hashin = hashPRF.evaluate((const uchar*)str.c_str(),str.size(),hashinbitlen);

	            Hash H;
	            int numberofhashes = (len_B+len_K+len_L+(H.OUTPUTLENGTH/8)-1)/(H.OUTPUTLENGTH/8);
	            uint32_t hashoutlen = numberofhashes*(H.OUTPUTLENGTH/8);
	            uchar* hashout = (uchar*)malloc(hashoutlen*sizeof(uchar));
	            REP(i,0,numberofhashes-1) {
	            	uint32_t sizetemp;
		        	string no = to_string(i);
		        	uint32_t newinsz = hashinbitlen/8+no.size();
		        	uchar* hashintemp = (uchar*)malloc(newinsz*sizeof(uchar));
		        	memcpy(hashintemp,hashin,hashinbitlen/8);
		        	memcpy(hashintemp+(hashinbitlen/8),no.data(),no.size());

	            	uchar *ptr; ptr = &hashout[i*(H.OUTPUTLENGTH/8)];
	            	H.evaluate(hashintemp,newinsz,sizetemp,&(ptr));
	            	free(hashintemp);
	            }
	            hashelements ele = splitintoBLK(len_B,len_K,len_L,hashout,hashoutlen,B);
	            free(hashout);
	            free(hashin);

	            if(Free[ele.B].size()==S) {
	                repeat = true;
	                cout<<"Free is completely filled\n";
	                break;
	            }
	            else {
	            	uint32_t r;
	            	int count = 0;
	            	do {
	            		if(count>1000) {
	            			cout<<"\nError: Increase size of S\n";
	            			cout<<Free[ele.B].size()<<"\n";
	            			exit(1);
	            		}
		            	uchar* no = (uchar*)malloc(sizeof(uchar)*4);
							    int rc = RAND_bytes(no, 4); if(rc != 1) exit(1);
							    r = toInt(no)%(S);
							    if(r < 0) r += S;
							    count++;
	                free(no);
	            	}while(present(Free[ele.B],r));

                int j = r;//*(it3);
                if(present(Free[ele.B],j)) {
                	cout<<"Error in construction\n";
                	exit(1);
                }
                Free[ele.B].insert(j);
		        		cnt++;
                uint32_t beta;
                if(cnt == Tlist[it->first].size()) beta = 0;
                else beta = 1;

                vector< uchar> si = Tlist[it->first][k];

                T[ele.B][j].second.resize(len_K);
                T[ele.B][j].second[0] = beta ^ ele.K[0];

                REP(i,1,len_K-1) {
                	T[ele.B][j].second[i] = si[i-1] ^ ele.K[i];
                }
                T[ele.B][j].first.resize(len_L);
                REP(i,0,len_L-1) T[ele.B][j].first[i] = ele.L[i];

	            }

		        free(ele.K);
		        free(ele.L);
	        }
	        if(repeat) break;
	        free(stag);
	    }
	    if(!repeat) {
	    	cout<<"Total Size of documents is "<<totalsize<<"\n";
	    	PRF_obj.storekey(name);
	    }

	} while(repeat);
	timer.donetime();
	cout<<"[Final SE]: TSet generation takes ";
	timer.printtime(0);

	cout<<"Parameters are ::"<<len_B<<"\t"<<len_L<<"\t"<<len_K<<"\n";
}

void TSet::tempstore(string name) {
	ofstream fout;
	fout.open("./src/EDB/"+name,ios::out| ios::binary);
	stringstream ss;
	ss << len_B <<"\n" << len_L << "\n" << len_K << "\n";
	ss << B << "\n" << S <<"\n";
	REP(i,0,T.size()-1) {
		int count = 0;
		REP(j,0,T[i].size()-1) {
			if(!T[i][j].first.empty()) count++;
		}
		ss<<count<<"\n";
	}

	REP(i,0,T.size()-1) {
		REP(j,0,T[i].size()-1) {
			if(T[i][j].first.empty()) continue;
			ss.write((const char*)T[i][j].first.data(),T[i][j].first.size());
			ss.write((const char*)T[i][j].second.data(),T[i][j].second.size());
		}
	}
	string str = ss.str();
	fout.write(str.data(),str.size());
	fout.close();
	cout<<"[SE:] Size of encrypted data(inverted index) is "<<str.size()<<" bytes\n";
}

void TSet::tempstore2(string name) {
	ofstream fout;
	fout.open("./src/EDB/"+name,ios::out| ios::binary);
	stringstream ss;
	ss << len_B <<"\n" << len_L << "\n" << len_K << "\n";
	ss << B << "\n" << S <<"\n";
	REP(i,0,B-1) {
		ss<<countind[i]<<"\n";
	}
	REP(i,0,B-1) {
		REP(j,0,countind[i]-1) {
			ss.write(Tnew[i][j].first,len_L);
			ss.write(Tnew[i][j].second,len_K);
		}
	}
	string str = ss.str();
	fout.write(str.data(),str.size());
	fout.close();
}

void TSet::tempreadstore2(string name) {
	FILE* fp;
	fp = fopen(("./src/EDB/" + name).c_str(),"rb");
	fscanf(fp,"%d\n%d\n%d\n%d\n%d\n",&len_B,&len_L,&len_K,&B,&S);
	countind = (int*)malloc(sizeof(int)*B);

	Tnew = (recordchar**)malloc(sizeof(recordchar*)*B);

	REP(i,0,B-1) {
		fscanf(fp,"%d\n",&countind[i]);
		if(countind[i] == 0) Tnew[i] = NULL;
		else Tnew[i] = (recordchar*) malloc(sizeof(recordchar)*countind[i]);
	}

	REP(i,0,B-1) {
		REP(j,0,countind[i]-1) {
			Tnew[i][j].first = (char*)malloc(sizeof(char)*len_L);
			REP(k,0,len_L-1) {fscanf(fp,"%c",&Tnew[i][j].first[k]);}

			Tnew[i][j].second = (char*)malloc(sizeof(char)*len_K);
			REP(k,0,len_K-1) {fscanf(fp,"%c",&Tnew[i][j].second[k]);}
		}
	}
	fclose(fp);
}

void TSet::optimizeTset() {
	Tnew = (recordchar**)malloc(sizeof(recordchar*)*B);
	countind = (int*)malloc(sizeof(int)*B);

	REP(i,0,B-1) {
		int count = 0;
		REP(j,0,T[i].size()-1) {
			if(!T[i][j].first.empty()) count++;
		}
		countind[i] = count;

		if(count == 0) Tnew[i] = NULL;
		else Tnew[i] = (recordchar*) malloc(sizeof(recordchar)*count);

		int jind = 0;
		REP(j,0,T[i].size()-1) {
			if(T[i][j].first.empty()) continue;

			Tnew[i][jind].first = (char*)malloc(sizeof(char)*len_L);
			memcpy(Tnew[i][jind].first,T[i][j].first.data(),len_L);

			Tnew[i][jind].second = (char*)malloc(sizeof(char)*len_K);
			memcpy(Tnew[i][jind].second,T[i][j].second.data(),len_K);
			jind++;
		}
	}
}

bool ucharcmp(vector<uchar> &a, uchar* b, uint32_t len) {
	REP(i,0,len-1) {
		if(a[i]!=b[i]) return false;
	}
	return true;
}
vector< vector<uchar> > TSet::TSetRetrieve2(uchar* stag, uint32_t stagbitlen) {
	cout<<"Parameters here are ::"<<len_B<<"\t"<<len_L<<"\t"<<len_K<<"\n";
	uint32_t beta = 1;
	uint32_t cnt = 0;
	vector< vector<uchar> > t;
	SSEtime timerfinal;
	SSEtime timerhash;
	SSEtime timersplit;
	SSEtime timercompare;
	while(beta == 1) {
		SSEtime timer;
		PRF hashPRF;
  	hashPRF.setkeylength(stagbitlen);
  	hashPRF.setkey(stag);

  	uint32_t hashinbitlen;
  	string str = to_string(cnt);
    unsigned char* hashin = hashPRF.evaluate((const uchar*)str.c_str(),str.size(),hashinbitlen);

    Hash H;
    int numberofhashes = (len_B+len_K+len_L+(H.OUTPUTLENGTH/8)-1)/(H.OUTPUTLENGTH/8);
    uint32_t hashoutlen = numberofhashes*(H.OUTPUTLENGTH/8);
    uchar* hashout = (uchar*)malloc(hashoutlen*sizeof(uchar));
    REP(i,0,numberofhashes-1) {
    	uint32_t sizetemp;
    	string no = to_string(i);
    	uint32_t newinsz = hashinbitlen/8+no.size();
    	uchar* hashintemp = (uchar*)malloc(newinsz*sizeof(uchar));
    	memcpy(hashintemp,hashin,hashinbitlen/8);
    	memcpy(hashintemp+(hashinbitlen/8),no.data(),no.size());

    	uchar *ptr; ptr = &hashout[i*(H.OUTPUTLENGTH/8)];
    	H.evaluate(hashintemp,newinsz,sizetemp,&(ptr));
    	free(hashintemp);
    }
    hashelements ele = splitintoBLK(len_B,len_K,len_L,hashout,hashoutlen,B);
    free(hashout);
    free(hashin);

		char* Lstr = (char*) malloc(sizeof(char)*len_L);
		memcpy(Lstr,ele.L,len_L);


    if(Tnew[ele.B] == NULL) {
    	cout<<"Queried for something that is not in dataset! Error\n";
    	exit(1);
    }
    uint32_t flag = 0;
    REP(i,0,countind[ele.B]-1) {
    	if(memcmp(Tnew[ele.B][i].first,Lstr,len_L) == 0) {
    		t.pb(vector<uchar>());
    		int index = t.size()-1;
    		t[index].resize(len_K-1);
    		REP(j,1,len_K-1) t[index][j-1] = ((uchar)Tnew[ele.B][i].second[j]) ^ ele.K[j];
    		beta = ((uchar)Tnew[ele.B][i].second[0]) ^ ele.K[0];
    		cnt++;
    		flag = 1;
    		break;
    	}
    }


    free(Lstr);
    free(ele.K);
    free(ele.L);

    if(flag==0) {
    	cout<<"No document found\n";
    	break;
    }
	}
	// cout<<"Total hash time is "; timerhash.printtime(0);
	// cout<<"Total split time is "; timersplit.printtime(0);
	// cout<<"Total compare time is "; timercompare.printtime(0);
	timerfinal.donetime();
	cout<<"Total time taken is "; timerfinal.printtime(0);
	return t;
}

TSet::~TSet() {
	if(Tnew != NULL) {
		REP(i,0,B-1) {
			REP(j,0,countind[i]-1) {
				free(Tnew[i][j].first);
				free(Tnew[i][j].second);
			}
			free(Tnew[i]);
		}
		free(Tnew);
		free(countind);
	}
}
