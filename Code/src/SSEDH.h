
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
#ifndef SSEDH_H
#define SSEDH_H

#include "SSEincludes.h"

class SSEDH{
private:
	uint32_t size = 2048;
    BN_CTX *ctxbn; 

public:
    
    BIGNUM* p;
    BIGNUM* q;
    BIGNUM* g;
	SSEDH();
	~SSEDH();
    void setDH(BIGNUM* a,BIGNUM* b, BIGNUM* c, uint32_t newsize);
    void setDH();
    void generateDH(uint32_t newsize);
    uint32_t getDHsize();
    uint32_t getDHsubgroupsize();
    BIGNUM* getp();
    BIGNUM* getq();
    BIGNUM* getg();
    BIGNUM* bin2bn(uchar *str, uint32_t len);
    uchar* bn2bin(BIGNUM* bn);
    BIGNUM* modq(BIGNUM* bn);
    BIGNUM* modp(BIGNUM* bn);
    BIGNUM* modmulq(BIGNUM* bn1, BIGNUM* bn2);
    BIGNUM* modmulp(BIGNUM* bn1, BIGNUM* bn2);
    BIGNUM* modexpq(BIGNUM* bn1, BIGNUM* bn2);
    BIGNUM* modexpp(BIGNUM* bn1, BIGNUM* bn2);
    BIGNUM* modinvq(BIGNUM* bn);
    BIGNUM* modinvp(BIGNUM* bn);
    uint32_t num_bytes(BIGNUM* bn);
    void free(BIGNUM** bn);
    void storeDH();
};

#endif // SSEDH_H