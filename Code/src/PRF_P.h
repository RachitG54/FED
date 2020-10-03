
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
#ifndef SSEPRF_P
#define SSEPRF_P

#include "SSEincludes.h"
#include "PRF.h"

class PRF_P {
private:
    uint32_t KEYLENGTH;
    uint32_t OUTPUTLENGTH = 256;
    BIGNUM* p;
    uchar* key;
    
public:
    PRF_P();
    ~PRF_P();
    void setkeylength(uint32_t keylength);
    void setkey(uchar* input, BIGNUM* p);
    void setprime(BIGNUM* prime);
    void keyGen();
    uchar* getkey();
    uint32_t getkeylength();
    BIGNUM* getprime();
    BIGNUM* evaluate(const uchar* input, uint32_t sz_in);
    void storekey(string keyname);
};

#endif // SSEPRF_P
