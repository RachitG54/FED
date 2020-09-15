#ifndef SSEPRF
#define SSEPRF

#include "SSEincludes.h"
#include "Hash.h"

class PRF {
private:
    uint32_t KEYLENGTH;
    uint32_t OUTPUTLENGTH = 256;
    uchar* key;
    EVP_CIPHER_CTX *ctx = NULL;
public:
    PRF();
    ~PRF();
    void setkeylength(uint32_t keylength);
    void setoutputlength(uint32_t outlength);
    void setkey(uchar* input);
    void keyGen();
    uchar* getkey();
    void getkey(string &keyret);
    uint32_t getkeylength();
    uchar* evaluate(const uchar* input, uint32_t sz_in, uint32_t &sz_out);
    uchar* evaluatePRP(const uchar* input, uint32_t sz_in, uint32_t &sz_out);
    void storekey(string keyname);
};

#endif // SSEPRF