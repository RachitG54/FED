
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
#ifndef SSEOPRF
#define SSEOPRF

#include "SSEincludes.h"
#include "PRF.h"
#include "SSEECDH.h"

class OPRF {
private:
    uint32_t KEYLENGTH;
    uint32_t OUTPUTLENGTH = 256;
    uchar* key;
    BIGNUM* keybn;
    EVP_CIPHER_CTX *ctx = NULL;
public:
    SSEECDH group;
    OPRF();
    ~OPRF();
    void setkeylength(uint32_t keylength);
    void setoutputlength(uint32_t outlength);
    void setkey(uchar* input);
    void keyGen();
    uchar* getkey();
    void getkey(string &keyret);
    uint32_t getkeylength();
    void evaluateP1(const uchar* input,uint32_t sz_in, BIGNUM** rinv, string &OPRFstr);
    void evaluateP2(string &s, string &OPRFstr);
    void fin_evaluateP1(string &s, BIGNUM* rinv, string &OPRFstr);
    void storekey(string keyname);
};

#endif // SSEOPRF