
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
#ifndef SSETSet
#define SSETSet

#include "SSEincludes.h"
#include "PRF.h"
#include "SSEtime.h"

typedef pair< vector<uchar> , vector<uchar> > record;

struct recordchar
{
    char* first = NULL;
    char* second = NULL;
};

struct hashelements{
	uint32_t B;
	uchar* K;
	uchar* L;
};

uchar* tochar(uint32_t value, uint32_t len);

struct db{
    set<string> W;
    map<uint32_t, set<string> > DB;
    map<string, set<int> > DB_inv;
};

struct db2{
    set<string> W;
    map<ll, set<string> > DB;
    map<string, set<ll> > DB_inv;
};

class TSet {
private:
	uint32_t B;
	uint32_t S;
	uint32_t lambda;
    uint32_t totaldocs;
	uint32_t len_B;
	uint32_t len_L;
    uint32_t len_K;
    uint32_t KEYLENGTH;
    vector< vector <record> > T;
    recordchar** Tnew = NULL;
    int *countind = NULL;

public:
    TSet();
    ~TSet();
    void TSetSetup();
    void TSetSetup(map<string, vector< vector<uchar> > > &Tlist, uint32_t newlen_K, string name);
    db getdata(string name);
    db datainitialize_basic();
    db genomeinitialize(string name);
    void TSetGetTag(uchar* key, uint32_t keylen, const uchar* w, uint32_t wlen, uchar* &stag, uint32_t &stagbitlen);
    void tempstore(string name);
    void tempstore2(string name);
    void optimizeTset();
    void tempreadstore2(string name);
    vector< vector<uchar> > TSetRetrieve2(uchar* stag, uint32_t stagbitlen);
};

#endif // SSETSet
