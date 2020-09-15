#ifndef SSEpaillier_H
#define SSEpaillier_H

#include "SSEincludes.h"
#include "paillier.h"
#include <assert.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <gmp.h>

class SSEpaillier{
private:
    long size = 1024;
    paillier_pubkey_t* pubKey;
    paillier_prvkey_t* secKey;
    paillier_ciphertext_t* sumct = NULL;

public:
    SSEpaillier();
    SSEpaillier(long s);
    ~SSEpaillier();
    void  keygen();
    void  storesecret(string name1, string name2);
    void  setsecret(string name1, string name2);
    void  setsecretpublic(string name1);
    int   getpublickeysize();
    void  setsum();
    char* returnsum();
    char* encrypt(string message);
    char* encrypt(uint32_t message);
    char* sum(char* byteCtxt1, char* byteCtxt2);
    void add(char* byteCtxt);
    void add(uint32_t val);
    char* decrypt(char* byteCtxt);
};

#endif // SSEpaillier_H