#include "PRF.h"
PRF::PRF() {
  if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();
  key = NULL;
}
void PRF::setkeylength(uint32_t keylength) {
    KEYLENGTH = keylength;
}
void PRF::setoutputlength(uint32_t outlength) {
    OUTPUTLENGTH = outlength;
}
void PRF::keyGen() {
    uint32_t len = KEYLENGTH/8;
    key = (uchar*)malloc(sizeof(uchar)*len);
    int rc = RAND_bytes(key, len);
    if(rc != 1) {
        exit(1);
    }
}
void PRF::setkey(uchar* input) {
    uint32_t len = KEYLENGTH/8;
    key = (uchar*)malloc(sizeof(uchar)*len);
    memcpy(key, input, len);
}
uchar* PRF::getkey() {
    return key;
}
void PRF::getkey(string &keyret) {
    keyret.resize(KEYLENGTH/8);
    REP(i,0,KEYLENGTH/8-1) keyret[i] = key[i];
}
uint32_t PRF::getkeylength() {
    return KEYLENGTH;
}

uchar* PRF::evaluate(const uchar* input,uint32_t sz_in, uint32_t &sz_out) {
    Hash H;
    uchar* midinp = H.evaluate(input,sz_in,sz_out);

    uint32_t olen = OUTPUTLENGTH/8;
    if(sz_out/8 < 14) {
        cout<<"Choose a bigger hash for PRF\n";
        exit(1);
    }
    unsigned char iv[16];
    memset(iv,0,16);
    memcpy(iv,midinp,14);

    AES_KEY wctx;
    AES_set_encrypt_key(key, 256, &wctx);
    int noofcounter = (olen+15)/16;
    int sz = noofcounter*16;

    uchar* ciphertext = (uchar*)malloc(sizeof(uchar)*sz);
    REP(i,0,noofcounter-1) {
        uchar* cntstr = tochar(i,4);
        iv[15] = cntstr[2];
        iv[15] = cntstr[3];
        AES_encrypt(iv, ciphertext+(i*16), &wctx);
    }
    sz_out = olen*8;
    free(midinp);
    return ciphertext;
}
uchar* PRF::evaluatePRP(const uchar* input,uint32_t sz_in, uint32_t &sz_out) {
    unsigned char *iv = (unsigned char *)"0123456789012345";
    int ivlen = 128;
    int len;
    int ciphertext_len;
    uchar* ciphertext = (uchar*)malloc(sizeof(uchar)*(sz_in+(ivlen/8)));
    //AES 256 CBC mode
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)) handleErrors();
    EVP_CIPHER_CTX_set_padding(ctx, 0);
    if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, input, sz_in)) handleErrors();
    ciphertext_len = len;
    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) handleErrors();
    ciphertext_len += len;
    sz_out = ciphertext_len*8;
    return ciphertext;
}
void PRF::storekey(string keyname) {
    FILE* fp;
    string fname = "./src/keys/"+keyname;
    fp = fopen(fname.c_str(),"w");
    fprintf(fp,"%u\n",KEYLENGTH);
    REP(i,0,(KEYLENGTH/8)-1) {
        fprintf(fp, "%02x", key[i]);
    }
    fclose(fp);
}
PRF::~PRF() {
    EVP_CIPHER_CTX_free(ctx);
    free(key);
}
