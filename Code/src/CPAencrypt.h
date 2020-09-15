#ifndef SSECPAencrypt
#define SSECPAencrypt

#include "SSEincludes.h"

class CPAencrypt {
private:
  	EVP_CIPHER_CTX *ctx = NULL;
	uchar* key = NULL;
	uint32_t keylen = 256;
	uchar* iv = NULL;
	uint32_t ivlen = 128;
public:
	CPAencrypt();
	CPAencrypt(uchar* k, uint32_t klen);
	~CPAencrypt();
	void storekey(string keyname);
	void readkey(string keyname);
	void setiv(uchar* IV, uint32_t IVlen);
	void ivgen();
	uchar* getiv(uint32_t &IVlen);
	void setkey(uchar* KEY, uint32_t KEYlen);
	void keygen();
	uchar* getkey(uint32_t &KEYlen);
	uchar* encrypt(unsigned char *plaintext, int plaintext_len, int &ciphertext_len, int pad);
	uchar* decrypt(unsigned char *ciphertext, int ciphertext_len, int &plaintext_len, int pad);
	int ivsize();
	int blocksize();
};


#endif // SSECPAencrypt


