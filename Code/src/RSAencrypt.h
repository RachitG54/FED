
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
