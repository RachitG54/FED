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