#ifndef MDSSE_H
#define MDSSE_H

#include "SSEincludes.h"
#include "SSEDH.h"
#include "SSEECDH.h"
#include "PRF.h"
#include "PRF_P.h"
#include "OPRF.h"
#include "CPAencrypt.h"
#include "SSEtime.h"
#include "RSAencrypt.h"
#include "TSet.h"
#include "Xvalret.h"
#include "IP.h"
#include "Xaddpaillier.h"
#include "XfuncGC.h"

class MDSSE {
private:
	SSEECDH group;
	uint32_t KEYLENGTH;
public:
    int kwsize = 20;
    MDSSE();
    void setkeysServer();
    void setkeysAuxServer();
    void initializeEQ(int DOindex, db2 &database, int rowNum, vlli &idlist, map<ll,int> &idmap);
    int initializeDB(db2 &database, vlli &idlist, vector< vector<string>> &XData, map<ll,int> &idmap);
    void initializeDO_1(stringstream &ss, BIGNUM** &rinverses, db2 &database);
    void initializeServe(string &s, stringstream &ss);
    void initializeDO_2(string &s, BIGNUM** &rinverses, int DOindex, db2 &database, int rowNum, vlli &idlist, map<ll,int> &idmap);
    void makeintoSE(vector< pair<string,string> > &W_datasep);
    void initializeAuth(set<string> &W_data, vector< pair<string,string> > &W_datasep);
    void initializeDCFE(vector< vector<string> > &XData, int DOindex); 
};

#endif // MDSSE_H
