
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
