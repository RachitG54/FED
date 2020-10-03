
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