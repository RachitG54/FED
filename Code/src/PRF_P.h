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
