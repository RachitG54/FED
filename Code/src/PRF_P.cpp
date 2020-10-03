
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
#include "PRF_P.h"
PRF_P::PRF_P() {
  key = NULL;
}
void PRF_P::setkeylength(uint32_t keylength) {
    KEYLENGTH = keylength;
}
void PRF_P::keyGen() {
    uint32_t len = KEYLENGTH/8;
    key = (uchar*)malloc(sizeof(uchar)*len);
    int rc = RAND_bytes(key, len);
    if(rc != 1) {
        exit(1);
    }
}
void PRF_P::setprime(BIGNUM* prime) {
    p = BN_dup((const BIGNUM*)prime);
}
void PRF_P::setkey(uchar* input, BIGNUM* prime) {
    uint32_t len = KEYLENGTH/8;
    key = (uchar*)malloc(sizeof(uchar)*len);
    memcpy(key, input, len);
    p = BN_dup((const BIGNUM*)prime);
}
uchar* PRF_P::getkey() {
    return key;
}
uint32_t PRF_P::getkeylength() {
    return KEYLENGTH;
}

BIGNUM* PRF_P::getprime() {
    return p;
}
BIGNUM* PRF_P::evaluate(const uchar* input,uint32_t sz_in) {
    uint32_t len = OUTPUTLENGTH/8;

    PRF F;
    F.setkeylength(KEYLENGTH); F.setkey(key);
    uchar* result = F.evaluate(input,sz_in,len);

    len = len/8;
    BN_CTX *ctxbn;
    ctxbn = BN_CTX_new(); if(!ctxbn) handleErrors();
    BIGNUM *a = BN_bin2bn(result, len, NULL); if(!a) handleErrors();
    BIGNUM *rem = BN_new();
    if(BN_mod(rem, a, (const BIGNUM *)p, ctxbn)!=1) handleErrors();
    
    free(result);
    BN_free(a);
    BN_CTX_free(ctxbn);
    return rem;
}
void PRF_P::storekey(string keyname) {
    FILE* fp;
    string fname = "./src/keys/"+keyname;
    fp = fopen(fname.c_str(),"w");
    fprintf(fp,"%u\n",KEYLENGTH);
    REP(i,0,(KEYLENGTH/8)-1) {
        fprintf(fp, "%02x", key[i]);
    }
    fclose(fp);
}
PRF_P::~PRF_P() {
    BN_free(p);
    free(key);
}
