#include "XfuncGC.h"
XfuncGC::XfuncGC() {
	KEYLENGTH = 256;
}

void printtemp(char* result,int len) {
	for (int i = 0; i != len; i++)
        printf("%02x", (unsigned int)result[i]);
    printf("\n");
}

uchar* XfuncGC::evaluate(PRF &F, int attribute, int ind, int data, uint32_t &clen) {
    uchar* attstr = tochar(attribute,attrlen);
    uchar* indstr = tochar(ind,indlen);
    int encsize = attrlen+indlen+1;
    uchar* enctext = (uchar*) malloc(sizeof(uchar)*(encsize));
    memset(enctext,0,encsize);
    memcpy(enctext,indstr,indlen);
    memcpy(enctext+indlen,attstr,attrlen);
    enctext[encsize-1] = data;

    uchar* ctxt = F.evaluate(enctext,encsize,clen);

    free(indstr);
	free(attstr);
    free(enctext);
	return ctxt;
}

void XfuncGC::initialize() {

	PRF PRF_GC;
	PRF_GC.setkeylength(KEYLENGTH);
	PRF_GC.keyGen();

	PRF PRF_PP;
	PRF_PP.setkeylength(KEYLENGTH);
	PRF_PP.keyGen();

	vector<string> allLines;
	getAllLinesFromFile(pathGenome, allLines);
	int allLinesIdx = 0;
	string line;
	line = allLines[allLinesIdx++];

	SSEtime timer;

	vector<string> xx;
	boost::split(xx, line, boost::is_any_of(" \t"), boost::token_compress_on);
	assert(xx.size() == 3);
	int rowNum = atoi(xx[0].c_str());
	int colNum = atoi(xx[1].c_str());
	int searchablefilter = atoi(xx[2].c_str());

	int relcols = (colNum - searchablefilter);

	uchar** x = (uchar**)malloc(sizeof(uchar*)*rowNum);
	uchar** randbitS = (uchar**)malloc(sizeof(uchar*)*rowNum);

	uint32_t templen;
	string temp = "0";
	uchar* out = PRF_GC.evaluate((const uchar*)temp.c_str(),1,templen);
	free(out);

	uint32_t PRFlen = templen/8;

	int randlen = (relcols*numberofbits+7)/8;

	REP(i,0,rowNum-1) {
		xx.clear();
		line = allLines[allLinesIdx++];
		boost::split(xx, line, boost::is_any_of(" \t"), boost::token_compress_on);
		x[i] = (uchar*)malloc(sizeof(uchar)*PRFlen*relcols*numberofbits);
		memset(x[i],0,PRFlen*relcols*numberofbits);

	    randbitS[i] = (uchar*)malloc(sizeof(uchar)*randlen);
		memset(randbitS[i],0,randlen);

		uchar* FKidstr = FKidGC(PRF_PP,i,randlen);
		REP(attr,searchablefilter,colNum-1) {
			string datastr = xx[attr];
			REP(j,0,numberofbits-1) {
				int data;
				if(datastr[j] == '1') data = 1; else data = 0;
				int ind = (attr - searchablefilter)*numberofbits+j;
				uchar* datakey = evaluate(PRF_GC,ind,i,data,templen);
				memcpy((char*)(x[i]+(PRFlen*ind)),datakey,PRFlen);

				int pos = ind%8;

				int bitA = (FKidstr[ind/8] >> pos) & 0x1;
				int bitS = bitA^data;
				randbitS[i][ind/8] += bitS<<pos;

				free(datakey);
			}
		}

		free(FKidstr);
	}

	timer.donetime();
	cout<<"[Final Hamm]: XfuncGC generation takes ";
	timer.printtime(0);

	cout<<"XfuncGC is setup\n";

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

		cout<<"[Final Hamm]: XfuncGC size = "<<str.size()<<" bytes."<<endl;
	}


	PRF_GC.storekey("K_GC");
	PRF_PP.storekey("K_PP");
}

void XfuncGC::multiDOinitialize(vector< vector<string> > &MDdata, int DOindex) {
	cout<<"Hamming for multiple DO\n";

    timeDO.starttime();

    RSAencrypt PubCryptoSystem;
    PubCryptoSystem.readpubkey("pubkeyS.pem");


    RSAencrypt PubCryptoSystemA;
    PubCryptoSystemA.readpubkey("pubkeyA.pem");


	PRF PRF_GC;

	{
		PRF_GC.setkeylength(KEYLENGTH);
		PRF_GC.keyGen();
	}

	SSEtime timer;

	int rowNum = MDdata.size();

	int colNum = 0;
	if(rowNum>0) colNum = MDdata[0].size();

	int relcols = colNum;

	uchar** x = (uchar**)malloc(sizeof(uchar*)*rowNum);
	uchar** randbitS = (uchar**)malloc(sizeof(uchar*)*rowNum);
	uchar** randbitA = (uchar**)malloc(sizeof(uchar*)*rowNum);

	uint32_t templen;
	string temp = "0";
	uchar* out = PRF_GC.evaluate((const uchar*)temp.c_str(),1,templen);
	free(out);

	uint32_t PRFlen = templen/8;

	int randlen = (relcols*numberofbits+7)/8;

	vector<string> pubPRF(rowNum);

	REP(i,0,rowNum-1) {

		x[i] = (uchar*)malloc(sizeof(uchar)*PRFlen*relcols*numberofbits);
		memset(x[i],0,PRFlen*relcols*numberofbits);


		uchar* PRFkeys = (uchar*)malloc(sizeof(uchar)*2*PRFlen*relcols*numberofbits);
		memset(PRFkeys,0,2*PRFlen*relcols*numberofbits);

	    randbitS[i] = (uchar*)malloc(sizeof(uchar)*randlen);
		memset(randbitS[i],0,randlen);

	    randbitA[i] = (uchar*)malloc(sizeof(uchar)*randlen);

		memset(randbitA[i],0,randlen);

	    int rc = RAND_bytes(randbitA[i], randlen); if(rc != 1) exit(1);

		REP(attr,0,colNum-1) {
			string datastr = MDdata[i][attr];
			REP(j,0,numberofbits-1) {
				int data;

				if(datastr[j] == '1') data = 1; else data = 0;
				int ind = attr*numberofbits+j;

				uchar* datakey = evaluate(PRF_GC,ind,i,data,templen);
				memcpy((char*)(x[i]+(PRFlen*ind)),datakey,PRFlen);

				int pos = ind%8;

				int bitA = (randbitA[i][ind/8] >> pos) & 0x1;
				int bitS = bitA^data;
				randbitS[i][ind/8] += bitS<<pos;

				if(data==0) {
					memcpy((char*)(PRFkeys+(PRFlen*ind)),datakey,PRFlen);
					uchar* datakey2 = evaluate(PRF_GC,ind,i,1,templen);
					memcpy((char*)(PRFkeys+(PRFlen*ind)+PRFlen*relcols*numberofbits),datakey2,PRFlen);
					free(datakey2);
				}
				else {
					memcpy((char*)(PRFkeys+(PRFlen*ind)+PRFlen*relcols*numberofbits),datakey,PRFlen);
					uchar* datakey2 = evaluate(PRF_GC,ind,i,0,templen);
					memcpy((char*)(PRFkeys+(PRFlen*ind)),datakey2,PRFlen);
					free(datakey2);
				}

				free(datakey);
			}
		}

		{
			stringstream ss;
			ss.write((char*)PRFkeys,2*PRFlen*relcols*numberofbits);
			ss.write((char*)randbitA[i],randlen);
			string str = ss.str();
			PubCryptoSystemA.envelope_seal((uchar*)str.data(),str.size(),pubPRF[i]);
		}

		free(PRFkeys);
		free(randbitA[i]);
	}
	free(randbitA);

	cout<<"Size is "<<2*PRFlen*relcols*numberofbits<<"\n";
	cout<<"XfuncGC is setup\n";

	vector<string> pvs(rowNum);
	{
		REP(i,0,rowNum-1) {
			stringstream ss;
			ss.write((char*)x[i],PRFlen*relcols*numberofbits);
			ss.write((char*)randbitS[i],randlen);
			ss.write(pubPRF[i].data(),pubPRF[i].size());

			string str = ss.str();


			PubCryptoSystem.envelope_seal((uchar*)str.data(),str.size(),pvs[i]);
			free(x[i]);
			free(randbitS[i]);
		}
		free(x);
		free(randbitS);

		timer.donetime();
		cout<<"[Final Data]: XfuncGC generation takes ";
		timer.printtime(0);
	}
	timeDO.donetime();

	{
		int sz = writetomsgpackfile(pvs,"./src/EDB/XfuncGC_"+to_string(DOindex));
		cout<<"[Final Data]: XfuncGC size = "<<sz<<" bytes."<<endl;

	    MDDObytes += sz;
	}

	{
		ofstream fout;
		fout.open("./src/EDB/XfuncGCparams",ios::out| ios::binary);
		stringstream ss;
		ss << PRFlen<<"\n"<<relcols<<"\n"<<randlen<<"\n";
		string str = ss.str();
		fout.write(str.data(),str.size());
		fout.close();
	}
}

void XfuncGC::compserve(vi &T, string &datafromA, int PRFlen, int relcols, int randlen, uchar** x, uchar** pandp) {

	uint32_t relcols2;

	const char* dataA = datafromA.c_str();
	uint32_t TSize, clen, ivlenbytes;
	TSize = toInt((uchar*)dataA);
	relcols2 = toInt((uchar*)(dataA+4));
	clen = toInt((uchar*)(dataA+8));
	ivlenbytes = toInt((uchar*)(dataA+12));

	cout<<TSize<<"::"<<relcols2<<"::"<<clen<<"::"<<ivlenbytes<<"::"<<relcols<<"\n";
	int bytesconsumed = 16;
	if((int)relcols2 != relcols) {
		cout<<relcols2<<" "<<relcols<<"\n";
		printf("Discrepancy in generating data.\n");
		exit(1);
	}

	int totalencsize = clen+ivlenbytes;
	uchar** encryptions = (uchar**)malloc(sizeof(uchar*)*TSize*relcols2*numberofbits);
	REP(i,0,TSize-1) {
		REP(attr,0,numberofbits*relcols2-1) {
			int combinedi = i*numberofbits*relcols2+attr;
			encryptions[combinedi] = (uchar*)malloc(sizeof(uchar)*totalencsize);
			uchar store = pandp[T[i]][attr/8];
			int bit = (store >> (attr%8)) & 0x1;
			if(bit==0) {
				memcpy(encryptions[combinedi], &dataA[bytesconsumed+combinedi*2*totalencsize],totalencsize);
			}
			else {
				memcpy(encryptions[combinedi], &dataA[bytesconsumed+combinedi*2*totalencsize+totalencsize],totalencsize);
			}
		}
	}
	int n = (T.size()+1)*relcols*numberofbits;

	SSEtime timerdecrypt;
	uchar **correctlabel = (uchar**) malloc(sizeof(uchar*)*n);
	int correctlabellen;

	REP(i,0,T.size()-1) {
		REP(attr,0,relcols*numberofbits-1) {
			int combinedi = i*numberofbits*relcols+attr;

			CPAencrypt GSKenc;
			GSKenc.setkey(x[T[i]]+(attr*PRFlen),PRFlen*8);
			GSKenc.setiv(encryptions[combinedi]+clen,ivlenbytes*8);

			int plen;
			uchar* ptext = GSKenc.decrypt(encryptions[combinedi],clen,plen,0);
			correctlabel[combinedi] = (uchar*)malloc(sizeof(uchar)*plen);
			memcpy(correctlabel[combinedi],ptext,plen);
			correctlabellen = plen;
			free(ptext);
		}

	}

	timerdecrypt.donetime();
	cout << "Time taken to decrypt and find out labels is "; timerdecrypt.printtime(0);


	REP(i,0,T.size()*relcols*numberofbits-1) {
		free(encryptions[i]);
	}
	free(encryptions);

	SSEtime excludetime;

	string path = "correctlabels";

	int corrlen = n - (relcols*numberofbits);

	FILE* fp;
	fp = fopen(path.c_str(),"w");
	for(int i = 0; i < corrlen; i++) {
		for(int j = 0; j < correctlabellen;j++)  {
			fprintf(fp, "%c", correctlabel[i][j]);
		}
	}
	fclose(fp);

	excludetime.donetime();
	cout<<"[Final Hamm]: Exclude time: (server correct label write) = ";
	excludetime.printtime(0);

	for(int i = 0; i < corrlen; i++) {
		free(correctlabel[i]);
	}
	free(correctlabel);

	SSEtime timereval;
	path = pathjustgarble + "/bin/Test.out "+ to_string(n) + " " + to_string(TSize) + " " + to_string(relcols*numberofbits) + " " + to_string(numberofbits);
	cout<<path<<"\n";
	system(path.c_str());
	timereval.donetime();
	cout << "[Final Hamm]: Exclude time: (Time taken by the circuit to evaluate) = ";
	timereval.printtime(0);
}

string XfuncGC::compauth(vi &T, vector<string> &query, vector<string> &pandp, vector<string> &keyslab, int tempPRFlen) {
	PRF PRFK_GC;
	PRF PRFK_PP;


	if(!DO)
	{

		{
			uint32_t keylen;
			uchar* key = NULL;
			key = readkeys(keylen, "K_GC");
			PRFK_GC.setkeylength(keylen); PRFK_GC.setkey(key);
			free(key);
		}

		{
			uint32_t keylen;
			uchar* key = NULL;
			key = readkeys(keylen, "K_PP");
			PRFK_PP.setkeylength(keylen); PRFK_PP.setkey(key);
			free(key);
		}
	}

	int relcols = query.size();

	int randlen = (relcols*numberofbits+7)/8;

	int totsize = (T.size()+1)*numberofbits*relcols;
	int hamsize = numberofbits*relcols;

	SSEtime timercircuit;
	string cmd = pathjustgarble + "/bin/CircuitFileTest.out " + to_string(totsize) + " " + to_string(T.size()) + " " + to_string(numberofbits*relcols) + " " + to_string(numberofbits);
	cout<<cmd<<"\n";
	system(cmd.c_str());
	timercircuit.donetime();
	cout << "[Final Hamm]: Exclude time: (Time taken by circuit to generate is) = ";
	timercircuit.printtime(0);

	SSEtime timerread;

	int n;
	int labelsize;

	n = totsize;
	labelsize = 16;

	uchar **labels = readlabels(n,labelsize);
	timerread.donetime();
	cout<<"[Final Hamm]: Exclude time: (Auth time taken to read labels) = ";
	timerread.printtime(0);

	CPAencrypt CPAtemp;
	uint32_t ivlenbytes = CPAtemp.ivsize();
	uint32_t tempclen = labelsize;

	SSEtime timerencrypt;

	uint32_t ctextsize = (ivlenbytes+tempclen);

	uchar** encryptions = (uchar**)malloc(sizeof(uchar*)*hamsize*T.size());
	REP(i,0,T.size()-1) {
		uchar* FKidstr = NULL;

		if(!DO) {
			FKidstr = FKidGC(PRFK_PP,T[i],randlen);
		}
		else {
			FKidstr = (uchar*)pandp[i].data();
		}

		uchar* fullkeytext = NULL;

		if(DO) {
			fullkeytext = (uchar*)keyslab[i].data();
		}

		REP(attrind,0,relcols*numberofbits-1) {
			int attr = attrind;
			encryptions[i*hamsize + attrind] = (uchar*)malloc(sizeof(uchar)*2*ctextsize);


			uchar store = FKidstr[attr/8];

			int bit = (store >> (attr%8)) & 0x1;

			REP(data,0,1) {
				uint32_t templen;
				CPAencrypt GSKenc;
				uchar* datakey = NULL;
				if(!DO) {
					datakey = evaluate(PRFK_GC,attr,T[i],data,templen);
				}
				else {
					datakey = (uchar*)&fullkeytext[attrind*tempPRFlen+data*tempPRFlen*relcols*numberofbits];
					templen = 8*tempPRFlen;
				}

				GSKenc.setkey(datakey,templen);
				GSKenc.ivgen();

				uint32_t ivlen;
				uchar* iv = GSKenc.getiv(ivlen);

				int clen;
				int position = bit ^ data;
				uchar* ciphertext = GSKenc.encrypt(labels[2*(i*hamsize + attrind)+data],labelsize,clen,0);
				memcpy((char*)(encryptions[i*hamsize + attrind]+(position*ctextsize)),ciphertext,clen);
				memcpy((char*)(encryptions[i*hamsize + attrind]+(position*ctextsize)+clen),iv,ivlen/8);

				free(iv);
				free(ciphertext);

				if(!DO) free(datakey);

				if(ctextsize!=(ivlen/8)+clen) {
					cout<<"Error in creating malloc array, will lead to problems, sort out block sizes.\n";
					exit(1);
				}
			}
		}
		if(!DO) free(FKidstr);
	}

	timerencrypt.donetime();
	cout << "Time taken by label encryption to execute is "; timerencrypt.printtime(0);

	vi quer;
	REP(i,0,query.size()-1) {
		REP(j,0,numberofbits-1) {
			if(query[i][j] == '0') quer.pb(0);
			else if(query[i][j] == '1') quer.pb(1);
			else {
				cout<<"Error in sending query to auxserver\n";
				exit(1);
			}
		}
	}

	if((int)quer.size()!=relcols*numberofbits) {
		cout<<"Check query sent by client\n";
		exit(1);
	}

	SSEtime excludetime;
	{
		ofstream fout;
		fout.open("labelsauth",ios::out| ios::binary);

		stringstream ss;
		REP(attrind,0,relcols*numberofbits-1) {
			int i = T.size();
			int ind = (i*hamsize+attrind);
			if(quer[attrind] == 0) {
				ss.write((char*)labels[2*ind],labelsize);
			}
			else {
				ss.write((char*)labels[2*ind+1],labelsize);
			}
		}

		string str = ss.str();
		fout.write(str.data(),str.size());
		fout.close();
	}

	excludetime.donetime();
	cout<<"[Final Hamm]: Exclude time: (Auth labels write time) = ";
	excludetime.printtime(0);

	for(int i = 0; i < 2*n; i++) {
		free(labels[i]);
	}
	free(labels);


	string datatosend;
	{
		stringstream ss;

		cout << T.size() <<"\n"<<relcols<<"\n"<<tempclen<<"\n"<<ivlenbytes<<"\n";
		uchar* temp = tochar(T.size(),4);
		ss.write((char*)temp,4);free(temp);

		temp = tochar(relcols,4);
		ss.write((char*)temp,4);free(temp);

		temp = tochar(tempclen,4);
		ss.write((char*)temp,4);free(temp);

		temp = tochar(ivlenbytes,4);
		ss.write((char*)temp,4);free(temp);

		REP(i,0,T.size()*relcols*numberofbits-1) {
			ss.write((char*)encryptions[i],2*ctextsize);
			free(encryptions[i]);
		}
		free(encryptions);

		datatosend = ss.str();
	}

    return datatosend;
}
unsigned long XfuncGC::compclient(unsigned long gamma, unsigned long delta) {
	int messagedim = 0;
    unsigned long N = 1<<messagedim;
    unsigned long subdelta = N-delta;
    unsigned long ans = (gamma+subdelta)%N;
    return ans;
}

uchar* FKidGC(PRF &F,int id, int noofchars) {
	uint32_t outlength;
	string idstr = to_string(id);
	uint32_t idlen = ((noofchars + 15)>>4)<<4;
	F.setoutputlength(idlen*8);
	uchar* out = F.evaluate((uchar*)idstr.c_str(),idstr.size(),outlength);
	uchar* FKidstr = (uchar*)malloc(sizeof(uchar)*noofchars);
	memcpy(FKidstr,out,noofchars);
	free(out);
	return FKidstr;
}
