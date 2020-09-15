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