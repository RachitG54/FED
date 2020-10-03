
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
#include "Xaddpaillier.h"
Xaddpaillier::Xaddpaillier() {
    KEYLENGTH = 1024;
}

void Xaddpaillier::initialize() {

    SSEpaillier AHE(KEYLENGTH);
    AHE.keygen();
    AHE.storesecret("pubkey.txt","seckey.txt");
    int clen = AHE.getpublickeysize()*2;

	vector<string> allLines;
	getAllLinesFromFile(pathGenome, allLines);
	int allLinesIdx = 0;

	SSEtime timer;
	string line;
    line = allLines[allLinesIdx++];
    vector<string> xx;
    boost::split(xx, line, boost::is_any_of(" \t"), boost::token_compress_on);
    assert(xx.size() == 3);
    int rowNum = atoi(xx[0].c_str());
    int colNum = atoi(xx[1].c_str());
    int searchablefilter = atoi(xx[2].c_str());

    int relcols = colNum - searchablefilter;
    uchar** x = (uchar**)malloc(sizeof(uchar*)*rowNum);

    REP(i,0,rowNum-1) {
        xx.clear();
		line = allLines[allLinesIdx++];
        boost::split(xx, line, boost::is_any_of(" \t"), boost::token_compress_on);
        x[i] = (uchar*)malloc(sizeof(uchar)*relcols*clen);;
        REP(attr,searchablefilter,colNum-1) {
            int ind = attr-searchablefilter;
            string datastr = xx[attr];
            char* AHEencrypt = AHE.encrypt(datastr);
            memcpy((char*)(x[i]+(clen*ind)),AHEencrypt,clen);
            free(AHEencrypt);
        }
    }

	timer.donetime();
	cout<<"[Final IP]: XSetIP generation takes ";
	timer.printtime(0);

    ofstream fout;
    fout.open("./src/EDB/Xaddpaillier",ios::out| ios::binary);
    stringstream ss;
    ss << rowNum <<"\n"<<relcols<<"\n"<<clen<<"\n";
    REP(i,0,rowNum-1) {
        ss.write((char*)x[i],relcols*clen);
        free(x[i]);
    }
    free(x);

    string str = ss.str();
    fout.write(str.data(),str.size());
    fout.close();
	cout<<"[Final IP]: XSetIP size = "<<str.size()<<endl;

    cout<<"Xaddpaillier Dataset is setup\n";
}
void Xaddpaillier::setkeysauth() {
    SSEpaillier AHE(KEYLENGTH);
    AHE.keygen();
    AHE.storesecret("pubkey.txt","seckey.txt");
}
void Xaddpaillier::multiDOinitialize(vector< vector<string> > &MDdata, int DOindex) {

    timeDO.starttime();
    SSEpaillier AHE;
    AHE.setsecretpublic("pubkey.txt");
    int clen = AHE.getpublickeysize()*2;

    RSAencrypt PubCryptoSystem;
    PubCryptoSystem.readpubkey("pubkeyS.pem");

    int rowNum = MDdata.size();
    int colNum = 0;
    if(rowNum>0) colNum = MDdata[0].size();
    int relcols = colNum;

    uchar** x = (uchar**)malloc(sizeof(uchar*)*rowNum);

    REP(i,0,rowNum-1) {

        x[i] = (uchar*)malloc(sizeof(uchar)*relcols*clen);;
        REP(attr,0,colNum-1) {
            string datastr = MDdata[i][attr];
            char* AHEencrypt = AHE.encrypt(datastr);
            memcpy((char*)(x[i]+(clen*attr)),AHEencrypt,clen);
            free(AHEencrypt);
        }
    }
    vector<string> encpaillier(rowNum);
    REP(i,0,rowNum-1) {
        PubCryptoSystem.envelope_seal((uchar*)x[i],relcols*clen,encpaillier[i]);
        free(x[i]);
    }
    free(x);
    timeDO.donetime();
    {
        int sz = writetomsgpackfile(encpaillier, "./src/EDB/Xaddpaillier_"+to_string(DOindex));
        cout<<"Size of Additive Paillier is "<<sz<<" bytes.\n";
        MDDObytes += sz;
    }

    {
        ofstream fout;
        fout.open("./src/EDB/Xaddpaillierparams",ios::out| ios::binary);
        stringstream ss;
        ss << rowNum <<"\n"<<relcols<<"\n"<<clen<<"\n";
        string str = ss.str();
        fout.write(str.data(),str.size());
        fout.close();
    }

    cout<<"Xaddpaillier Dataset is setup\n";
}
string Xaddpaillier::compserve(vi &T, uint32_t &y, int attr, int rowNum, int relcols, int clen, uchar** x) {

    unsigned long N = 1 << messagedim;
    int len = (messagedim+7)/8;
    uchar* tmp = (uchar*)malloc(sizeof(uchar)*len);
    int rc = RAND_bytes(tmp, len); if(rc != 1) exit(1);

    BIGNUM* bn = BN_bin2bn(tmp, len, NULL); if(!bn) handleErrors();
    uint32_t rho = BN_mod_word(bn,N);
    BN_free(bn);
    free(tmp);

    SSEpaillier AHE;
    AHE.setsecretpublic("pubkey.txt");
    AHE.setsum();

    char* AHEencrypt = AHE.encrypt(rho);
    AHE.add(AHEencrypt);

    free(AHEencrypt);

    REP(i,0,T.size()-1) {
        char* ctxt = (char*)malloc(sizeof(char)*clen);
        memcpy(ctxt,(char*)(x[T[i]]+(clen*attr)),clen);

        AHE.add(ctxt);
        free(ctxt);
    }
    char* bytesum = AHE.returnsum();
    string sumstr(clen,'\0');
    REP(s,0,clen-1) sumstr[s] = bytesum[s];
    y = rho;
    free(bytesum);
    return sumstr;
}
uint32_t Xaddpaillier::compauth(string &sumstr) {
    SSEpaillier AHE;
    AHE.setsecret("pubkey.txt","seckey.txt");
    int clen = AHE.getpublickeysize()*2;

    char* ctxt = (char*)malloc(sizeof(char)*clen);
    memcpy(ctxt,sumstr.data(),clen);
    char* zstr = AHE.decrypt(ctxt);
    free(ctxt);
    
    uint32_t z = atoi(zstr);
    free(zstr);
    return z;
}

uint32_t Xaddpaillier::compclient(uint32_t z, uint32_t y) {
    unsigned long N = 1 << messagedim;
    unsigned long suby = N-y;
    return (z+suby)%N;
}
