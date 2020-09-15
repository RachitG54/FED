#ifndef SSEelgamal_H
#define SSEelgamal_H

#include "SSEincludes.h"

class elgamal{
private:
    BN_CTX *ctxbn;
    uint32_t size = 1024;
    uint32_t maxmsgspace = 1000;
    BIGNUM* s;              //secret
    BIGNUM* h;              //public
public:
    BIGNUM *p;
    BIGNUM *q;                  
    BIGNUM *g;
    elgamal();
    elgamal(uint32_t s, uint32_t m);
    ~elgamal();
    void groupgen(); 
    void setgroup(BIGNUM* a,BIGNUM* b, BIGNUM* c, uint32_t newsize, uint32_t newmaxspace);
    void storegroup(string name);
    void setgroup(string name);
    BIGNUM* randomnogen();
    void keygen();
    void storesecret(string name);
    void setsecret(string name);
    void encrypt(BIGNUM* m, BIGNUM** x, BIGNUM** y);
    int  decrypt(BIGNUM* x,BIGNUM* y);
};

#endif // SSEelgamal_H