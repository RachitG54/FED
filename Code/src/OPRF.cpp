
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
#include "OPRF.h"
OPRF::OPRF() {
  if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();
  key = NULL;
  keybn = NULL;
  group.setDH();
}
void OPRF::setkeylength(uint32_t keylength) {
    KEYLENGTH = keylength;
}
void OPRF::setoutputlength(uint32_t outlength) {
    OUTPUTLENGTH = outlength;
}
void OPRF::keyGen() {
    uint32_t len = KEYLENGTH/8;
    key = (uchar*)malloc(sizeof(uchar)*len);
    int rc = RAND_bytes(key, len);
    if(rc != 1) {
        exit(1);
    }
    BIGNUM* tmpbn = group.bin2bn(key,len);
    keybn = group.modq(tmpbn);
    BN_print_fp(stdout,tmpbn);
    cout<<"\n";
    group.free(&tmpbn);
}
void OPRF::setkey(uchar* input) {
    uint32_t len = KEYLENGTH/8;
    key = (uchar*)malloc(sizeof(uchar)*len);
    memcpy(key, input, len);
    BIGNUM* tmpbn = group.bin2bn(key,len);
    keybn = group.modq(tmpbn);
    group.free(&tmpbn);
}
uchar* OPRF::getkey() {
    return key;
}
void OPRF::getkey(string &keyret) {
    keyret.resize(KEYLENGTH/8);
    REP(i,0,KEYLENGTH/8-1) keyret[i] = key[i];
}
uint32_t OPRF::getkeylength() {
    return KEYLENGTH;
}


void OPRF::evaluateP1(const uchar* input,uint32_t sz_in, BIGNUM** rinv, string &OPRFstr) {

    uint32_t hasholen;
    Hash H;
    uchar* tmpinp = H.evaluate(input,sz_in,hasholen);
    hasholen = hasholen/8;
    uchar* midinp = (uchar*)malloc(sizeof(uchar)*(hasholen+1));
    midinp[0] = 0;
    memcpy(midinp+1,tmpinp,hasholen);
    free(tmpinp);
    hasholen++;
    BIGNUM* midinpbn = group.bin2bn(midinp,hasholen);
    BIGNUM* hashmod = group.modq(midinpbn);

    group.free(&midinpbn);

    uint32_t len = group.getDHsubgroupsize();
    uchar* tmp = (uchar*)malloc(sizeof(uchar)*len);
    int rc = RAND_bytes(tmp, len);
    if(rc != 1) exit(1);
    BIGNUM* tmpbn = group.bin2bn(tmp,len);
    BIGNUM* r = group.modq(tmpbn);
    *rinv = group.modinvq(r);

    BIGNUM* modmul = group.modmulq(r,hashmod);

    EC_POINT* hx_r_p = group.modexppgen(modmul);


    group.free(&hashmod);
    group.free(&tmpbn);
    group.free(&r);
    group.free(&modmul);
    
    free(tmp);
    free(midinp);


    BIGNUM *bn = group.point2bn(hx_r_p);

    uint32_t lenbn = group.num_bytes(bn);
    uchar* str = group.bn2bin(bn);

    OPRFstr.resize(lenbn);
    REP(s,0,lenbn-1) OPRFstr[s] = str[s];

    free(str);
    group.free(&hx_r_p);
    group.free(&bn);
}
void OPRF::evaluateP2(string &s, string &OPRFstr) {
    uchar* str = (uchar*)malloc(sizeof(uchar)*s.size());
    memcpy(str,s.data(),s.size());
    BIGNUM* hx_r = group.bin2bn(str,s.size());
    EC_POINT* hx_r_p = group.bn2point(hx_r);

    EC_POINT* hx_r_k_p = group.modexpp(hx_r_p,keybn);

    BIGNUM *bn = group.point2bn(hx_r_k_p);
    uint32_t lenbn = group.num_bytes(bn);

    uchar* str2 = group.bn2bin(bn);
    OPRFstr.resize(lenbn);
    REP(s,0,lenbn-1) OPRFstr[s] = str2[s];

    free(str);
    group.free(&hx_r);
    group.free(&hx_r_p);
    group.free(&hx_r_k_p);
    group.free(&bn);
    free(str2);
}
void OPRF::fin_evaluateP1(string &s, BIGNUM* rinv, string &OPRFstr) {
    uchar* str = (uchar*)malloc(sizeof(uchar)*s.size());
    memcpy(str,s.data(),s.size());
    BIGNUM* hx_r_k = group.bin2bn(str,s.size());
    EC_POINT* hx_r_k_p = group.bn2point(hx_r_k);
    EC_POINT* hx_k_p = group.modexpp(hx_r_k_p,rinv);

    BIGNUM *bn = group.point2bn(hx_k_p);
    uint32_t lenbn = group.num_bytes(bn);

    uchar* str2 = group.bn2bin(bn);

    uint32_t hasholen;
    Hash H;
    uchar* midinp = (uchar*)malloc(sizeof(uchar)*(lenbn+1));
    midinp[0] = 1;
    memcpy(midinp+1,str2,lenbn);
    uchar* tmpinp = H.evaluate(midinp,lenbn+1,hasholen);
    hasholen = hasholen/8;
    free(midinp);

    OPRFstr.resize(hasholen);
    REP(s,0,hasholen-1) OPRFstr[s] = tmpinp[s];
    free(str);
    free(tmpinp);
    group.free(&hx_r_k);
    group.free(&hx_r_k_p);
    group.free(&hx_k_p);
    group.free(&bn);
    free(str2);

}
void OPRF::storekey(string keyname) {
    FILE* fp;
    string fname = "./src/keys/"+keyname;
    fp = fopen(fname.c_str(),"w");
    fprintf(fp,"%u\n",KEYLENGTH);
    REP(i,0,(KEYLENGTH/8)-1) {
        fprintf(fp, "%02x", key[i]);
    }
    fclose(fp);
}
OPRF::~OPRF() {
    EVP_CIPHER_CTX_free(ctx);
    free(key);
    group.free(&keybn);
}
