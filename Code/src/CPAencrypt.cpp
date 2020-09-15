#include "CPAencrypt.h"
CPAencrypt::CPAencrypt() {
  if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();
}
CPAencrypt::CPAencrypt(uchar* k, uint32_t klen) :
    key(k), keylen(klen)
{
  if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();
}
CPAencrypt::~CPAencrypt() {
  EVP_CIPHER_CTX_free(ctx);
  free(key);
  free(iv);
}
void CPAencrypt::storekey(string keyname) {
    FILE* fp;
    string fname = "./src/keys/"+keyname;
    fp = fopen(fname.c_str(),"w");
    fprintf(fp,"%u\n",keylen);
    REP(i,0,(keylen/8)-1) {
        fprintf(fp, "%02x", key[i]);
    }
    fclose(fp);
}
void CPAencrypt::readkey(string keyname) {
	FILE* fp;
	string fname = "./src/keys/"+keyname;
  fp = fopen(fname.c_str(),"r");
  fscanf(fp, "%u\n",&keylen);
  key = (uchar*)malloc((keylen/8)*sizeof(uchar));
  REP(i,0,(keylen/8)-1) {
    int temp;
    fscanf(fp, "%02x", &temp);
    key[i] = (uchar)temp;
  }
  fclose(fp);
}
void CPAencrypt::setiv(uchar* IV, uint32_t IVlen) {
    uint32_t len = IVlen/8;
    iv = (uchar*)malloc(sizeof(uchar)*len);
    memcpy(iv, IV, len);
}
void CPAencrypt::ivgen() {
    uint32_t len = ivlen/8;
    iv = (uchar*)malloc(sizeof(uchar)*len);
    int rc = RAND_bytes(iv, len);
    if(rc != 1) {
        exit(1);
    }
}
uchar* CPAencrypt::getiv(uint32_t &IVlen) {
	IVlen = ivlen;
	uchar* IV = (uchar*)malloc(sizeof(uchar)*(IVlen/8));
	memcpy(IV,iv,(IVlen/8));
	return IV;
}

void CPAencrypt::setkey(uchar* KEY, uint32_t KEYlen) {
    uint32_t len = KEYlen/8;
    key = (uchar*)malloc(sizeof(uchar)*len);
    memcpy(key, KEY, len);
}
void CPAencrypt::keygen() {
    uint32_t len = keylen/8;
    key = (uchar*)malloc(sizeof(uchar)*len);
    int rc = RAND_bytes(key, len);
    if(rc != 1) {
        exit(1);
    }
}
uchar* CPAencrypt::getkey(uint32_t &KEYlen) {
	KEYlen = keylen;
	uchar* KEY = (uchar*)malloc(sizeof(uchar)*(KEYlen/8));
	memcpy(KEY,key,KEYlen/8);
	return KEY;
}

uchar* CPAencrypt::encrypt(unsigned char *plaintext, int plaintext_len, int &ciphertext_len, int pad) {
  int len;
  uchar* ciphertext = (uchar*)malloc(sizeof(uchar)*(plaintext_len+(ivlen/8)));
  //AES 256 CBC mode
  if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)) handleErrors();
  if(!pad) EVP_CIPHER_CTX_set_padding(ctx, 0);

  if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
    handleErrors();
  ciphertext_len = len;
  if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) handleErrors();
  ciphertext_len += len;
  return ciphertext;
}

uchar* CPAencrypt::decrypt(unsigned char *ciphertext, int ciphertext_len, int &plaintext_len, int pad) {
  int len;
  int len2 = ciphertext_len+(ivlen/8);
  uchar* plaintext = (uchar*)malloc(sizeof(uchar)*(len2));
  if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)) handleErrors();
  if(!pad) EVP_CIPHER_CTX_set_padding(ctx, 0);

  if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
    handleErrors();
  plaintext_len = len;

  if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) handleErrors();
  plaintext_len += len;
  return plaintext;
}

int CPAencrypt::ivsize() {
  return 16;
}

int CPAencrypt::blocksize() {
  return 16;
}
