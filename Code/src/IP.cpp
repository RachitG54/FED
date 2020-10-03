
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
#include "IP.h"
IP::IP() {
    KEYLENGTH = 256;
}
unsigned long IP::evaluate(PRF &F, uint32_t attribute, uint32_t ind) {
    uchar* attstr = tochar(attribute,attrlen);
    uchar* indstr = tochar(ind,indlen);
    int encsize = attrlen+indlen;
    unsigned long N = 1 << messagedim;
    uchar* enctext = (uchar*) malloc(sizeof(uchar)*(encsize));
    memset(enctext,0,encsize);
    memcpy(enctext,indstr,indlen);
    memcpy(enctext+indlen,attstr,attrlen);

    uint32_t clen;
    uchar* ctxt = F.evaluate(enctext,encsize,clen);
    BIGNUM* bn = BN_bin2bn(ctxt, clen/8, NULL); if(!bn) handleErrors();
    unsigned long val = BN_mod_word(bn,N);
    BN_free(bn);
    free(indstr);
    free(enctext);
    free(ctxt);
    free(attstr);
    return val;
}

void IP::initialize() {

	vector<string> allLines;
	getAllLinesFromFile(pathGenome, allLines);
	int allLinesIdx = 0;

    string line;
	line = allLines[allLinesIdx++];

	SSEtime timer;
    unsigned long N = 1 << messagedim;
    PRF PRFK_ip;
    PRFK_ip.setkeylength(KEYLENGTH);
    PRFK_ip.keyGen();

    vector<string> xx;
    boost::split(xx, line, boost::is_any_of(" \t"), boost::token_compress_on);
    assert(xx.size() == 3);
    int rowNum = atoi(xx[0].c_str());
    int colNum = atoi(xx[1].c_str());
    int searchablefilter = atoi(xx[2].c_str());

    vector< vector< unsigned long> > XData(rowNum, vector<unsigned long>(colNum - searchablefilter));

    REP(i,0,rowNum-1) {
        xx.clear();
        line = allLines[allLinesIdx++];
        boost::split(xx, line, boost::is_any_of(" \t"), boost::token_compress_on);
        REP(attr,searchablefilter,colNum-1) {
            int j = attr - searchablefilter;
            string datastr = xx[attr];
            stringstream ss(datastr);
            unsigned long data; ss >> data;
            unsigned long val = evaluate(PRFK_ip,j,i)%N;
            XData[i][j] = (val+data)%N;
        }
    }


    PRFK_ip.storekey("K_ip2");

	timer.donetime();
	cout<<"[Final IP]: XSetIP generation takes ";
	timer.printtime(0);

    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, XData);
    ofstream out("./src/EDB/XsumIP");
    out.write(sbuf.data(),sbuf.size());
    out.close();

  	cout<<"[Final IP]: XSetIP size = "<<sbuf.size()<<endl;

    cout<<"IP Dataset is setup\n";
}

void IP::multiDOinitialize(vector< vector<string> > &MDdata, int DOindex) {

    timeDO.starttime();
    RSAencrypt PubCryptoSystem;
    PubCryptoSystem.readpubkey("pubkeyS.pem");

    RSAencrypt PubCryptoSystemA;
    PubCryptoSystemA.readpubkey("pubkeyA.pem");

    SSEtime timer;
    unsigned long N = 1 << messagedim;

    int rowNum = MDdata.size();

    int colNum = 0;
    if(rowNum>0) colNum = MDdata[0].size();
    int relcols = colNum;

    vector< vector< unsigned long> > XData(rowNum, vector<unsigned long>(relcols));

    vector< pair<string,int> > enckeys(rowNum);

    vector<string> encData(rowNum);

    REP(i,0,rowNum-1) {
        PRF PRFK_ip;
        PRFK_ip.setkeylength(KEYLENGTH);
        PRFK_ip.keyGen();
        uchar* id = (uchar*)malloc(sizeof(uchar)*4);
        RAND_bytes(id, 4);

        uint32_t randid = toInt(id);
        int signedid = (int)randid;

        if(signedid<0) signedid = -signedid;
        free(id);

        REP(attr,0,colNum-1) {
            int j = attr;
            string datastr = MDdata[i][j];
            stringstream ss(datastr);
            unsigned long data; ss >> data;


            unsigned long val = evaluate(PRFK_ip,j,signedid)%N;

            XData[i][j] = (val+data)%N;
        }

        string plainkey;
        PRFK_ip.getkey(plainkey);
        enckeys[i].first = PubCryptoSystem.encrypt(plainkey.size(),(const uchar*)plainkey.data());
        enckeys[i].second = signedid;

        {

            stringstream ss;
            msgpack::sbuffer sbuf;
            msgpack::pack(sbuf, XData[i]);
            msgpack::pack(sbuf, enckeys[i]);

            ss.write(sbuf.data(),sbuf.size());
            string str = ss.str();
            PubCryptoSystemA.envelope_seal((uchar*)str.data(),str.size(),encData[i]);
        }
    }
    timeDO.donetime();

    timer.donetime();
    cout<<"[Final Data]: XSetIP generation takes ";
    timer.printtime(0);

    {
        msgpack::sbuffer sbuf;
        msgpack::pack(sbuf, encData);
        ofstream out("./src/EDB/XsumIP_" + to_string(DOindex));
        out.write(sbuf.data(),sbuf.size());
        out.close();
        cout<<"[Final Data]: XSetIP size = "<<sbuf.size()<<endl;
        MDDObytes += sbuf.size();
    }
    cout<<"IP Dataset is setup\n";
}

unsigned long IP::compserve(vi &T, unsigned long &rho, int attr, vector< vector<unsigned long> > &XData) {

    unsigned long N = 1 << messagedim;
    int len = (messagedim+7)/8;
    uchar* tmp = (uchar*)malloc(sizeof(uchar)*len);
    int rc = RAND_bytes(tmp, len); if(rc != 1) exit(1);

    BIGNUM* bn = BN_bin2bn(tmp, len, NULL); if(!bn) handleErrors();
    rho = BN_mod_word(bn,N);
    BN_free(bn);
    free(tmp);
    // rho = 0;
    unsigned long sum = rho;
    REP(i,0,T.size()-1) {
        sum = (sum+XData[T[i]][attr])%N;
    }
    return sum;
}
unsigned long IP::compauth(vi &T, unsigned long rho, int attr) {
    unsigned long N = 1 << messagedim;

    PRF PRFK_ip;
    uint32_t keylen;
    uchar* key = NULL;
    key = readkeys(keylen, "K_ip2");
    PRFK_ip.setkeylength(keylen); PRFK_ip.setkey(key);
    unsigned long sum = rho;
    REP(i,0,T.size()-1) {
        unsigned long val = evaluate(PRFK_ip,attr,T[i])%N;
        sum = (sum + val)%N;
    }
    free(key);
    return sum;
}
unsigned long IP::compclient(unsigned long gamma, unsigned long delta) {
    unsigned long N = 1<<messagedim;
    unsigned long subdelta = N-delta;
    unsigned long ans = (gamma+subdelta)%N;
    return ans;
}
