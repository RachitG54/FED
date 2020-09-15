#include "SSEDH.h"

SSEDH::SSEDH() {
    ctxbn = BN_CTX_new(); if(!ctxbn) handleErrors();
}
void SSEDH::setDH(BIGNUM* a,BIGNUM* b, BIGNUM* c, uint32_t newsize) {
	p = a; q = b; g = c;
	size = newsize;
}
void SSEDH::generateDH(uint32_t newsize) {
    size = newsize;
    DSA* dsa;
    if(NULL == (dsa = DSA_new())) handleErrors();
    if(1 != DSA_generate_parameters_ex(dsa,size,NULL,0,NULL,NULL,NULL)) handleErrors();
    p = BN_dup(dsa->p);
    q = BN_dup(dsa->q);
    g = BN_dup(dsa->g);
}

uint32_t SSEDH::getDHsize() {
	return size;
}
uint32_t SSEDH::getDHsubgroupsize() {
    return BN_num_bytes(q);
}

BIGNUM* SSEDH::getp() {
	return p;
}

BIGNUM* SSEDH::getq() {
	return q;
}

BIGNUM* SSEDH::getg() {
	return g;
}

void SSEDH::storeDH() {
	FILE* fp;
	string fname = "./src/keys/DH.txt";
    fp = fopen(fname.c_str(),"w");
    fprintf(fp,"%u\n",size);
    char *strp = BN_bn2hex(p);
    char *strq = BN_bn2hex(q);
    char *strg = BN_bn2hex(g);
    fprintf(fp, "%s\n",strp);
    fprintf(fp, "%s\n",strq);
    fprintf(fp, "%s\n",strg);
    fclose(fp);
}

void SSEDH::setDH() {
	ifstream fp;
	string fname = "./src/keys/DH.txt";
    fp.open(fname,ios::in);
    p = BN_new();q = BN_new();g = BN_new();
    string strp,strq,strg;
    fp >> size;
    fp >> strp >> strq >> strg;
 	if(0==BN_hex2bn(&p, strp.c_str())) handleErrors();
 	if(0==BN_hex2bn(&q, strq.c_str())) handleErrors();
 	if(0==BN_hex2bn(&g, strg.c_str())) handleErrors();
    fp.close();
}
BIGNUM* SSEDH::bin2bn(uchar *str, uint32_t len) {
    BIGNUM* bn = BN_bin2bn(str, len, NULL); if(!bn) handleErrors();
    return bn;
}
uchar* SSEDH::bn2bin(BIGNUM* bn) {
    uint32_t len = BN_num_bytes(bn);
    uchar* str = (uchar*) malloc(sizeof(uchar)*len);
    if(0 == BN_bn2bin(bn, str)) handleErrors();
    return str;
}
BIGNUM* SSEDH::modq(BIGNUM* bn) {
    BIGNUM *bnmod = BN_new();
    if(BN_mod(bnmod, bn, q, ctxbn)!=1) handleErrors();
    return bnmod;
}
BIGNUM* SSEDH::modp(BIGNUM* bn) {
    BIGNUM *bnmod = BN_new();
    if(BN_mod(bnmod, bn, p, ctxbn)!=1) handleErrors();
    return bnmod;
}
BIGNUM* SSEDH::modmulq(BIGNUM* bn1, BIGNUM* bn2) {
    BIGNUM *bnmod = BN_new();
    if(1 != BN_mod_mul(bnmod, bn1, bn2, q, ctxbn)) handleErrors();
    return bnmod;
}
BIGNUM* SSEDH::modmulp(BIGNUM* bn1, BIGNUM* bn2) {
    BIGNUM *bnmod = BN_new();
    if(1 != BN_mod_mul(bnmod, bn1, bn2, p, ctxbn)) handleErrors();
    return bnmod;
}
BIGNUM* SSEDH::modexpq(BIGNUM* bn1, BIGNUM* bn2) {
    BIGNUM *bnmod = BN_new();
    if(1 != BN_mod_exp(bnmod, bn1, bn2, q, ctxbn)) handleErrors();
    return bnmod;
}
BIGNUM* SSEDH::modexpp(BIGNUM* bn1, BIGNUM* bn2) {
    BIGNUM *bnmod = BN_new();
    if(1 != BN_mod_exp(bnmod, bn1, bn2, p, ctxbn)) handleErrors();
    return bnmod;
}
BIGNUM* SSEDH::modinvq(BIGNUM* bn) {
    BIGNUM *bnmod = BN_mod_inverse(NULL, bn, q, ctxbn);
    return bnmod;
}
BIGNUM* SSEDH::modinvp(BIGNUM* bn) {
    BIGNUM *bnmod = BN_mod_inverse(NULL, bn, p, ctxbn);
    return bnmod;
}
uint32_t SSEDH::num_bytes(BIGNUM* bn) {
    return BN_num_bytes(bn);
}
void SSEDH::free(BIGNUM** bn) {
    BN_free(*bn);
}
SSEDH::~SSEDH() {
  BN_CTX_free(ctxbn);
	BN_free(p);
	BN_free(q);
	BN_free(g);
}
