
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


