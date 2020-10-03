
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
#ifndef SSEECDH_H
#define SSEECDH_H

#include "SSEincludes.h"

class SSEECDH{
private:
	uint32_t size = 224;
    EC_GROUP *curve;
    BN_CTX *ctxbn; 

public:

    BIGNUM* q;
    EC_POINT* g;

	SSEECDH();
	~SSEECDH();


    void setDH();
    
    uint32_t getDHsize();
    uint32_t getDHsubgroupsize();
    
    BIGNUM* bin2bn(uchar *str, uint32_t len);
    uchar* bn2bin(BIGNUM* bn);
    uint32_t num_bytes(BIGNUM* bn);
    BIGNUM* point2bn(EC_POINT *p);
    EC_POINT* bn2point(BIGNUM* bn);
    BIGNUM* modq(BIGNUM* bn);
    BIGNUM* modmulq(BIGNUM* bn1, BIGNUM* bn2);
    BIGNUM* modinvq(BIGNUM* bn);
    EC_POINT* modexppgen(BIGNUM* bn);
    EC_POINT* modexpp(EC_POINT* p, BIGNUM* bn);
    void free(BIGNUM** bn);
    void free(EC_POINT** p);
    void storeDH();
};

#endif // SSEECDH_H