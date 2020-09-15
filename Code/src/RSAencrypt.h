#ifndef SSERSAencrypt
#define SSERSAencrypt

#include "SSEincludes.h"
#include "SSEtime.h"

class RSAencrypt {
private:
    EVP_PKEY *privkey = NULL;
    EVP_CIPHER_CTX *ctx = NULL;
    RSA* rsadec = NULL;
public:
    RSA* rsaenc = NULL;
    EVP_PKEY *pubkey = NULL;
	uint32_t KEYLENGTH = 2048;
	RSAencrypt();
	void generate_key_tofile(string name1, string name2);
	void readpubkey(string name);
	void readprivkey(string name);
	string encrypt(int flen, const unsigned char *from);
	string decrypt(int flen, const unsigned char *from);

	string encryptNOPAD(int flen, const unsigned char *from);
	string decryptNOPAD(int flen, const unsigned char *from);

	string encrypt(int flen, const unsigned char *from, BIGNUM* mod);
	string encryptNOPAD(int flen, const unsigned char *fro, BIGNUM* mod);

	int getencsize();
	void envelope_seal(uchar* plaintext, int plaintext_len, string &ctxt);
	void envelope_open(string &ctext, string &ptxt);
	BIGNUM* retmod(); 
	~RSAencrypt();
};


#endif // SSECPAencrypt
