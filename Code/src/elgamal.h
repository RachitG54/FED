
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
#ifndef SSEelgamal_H
#define SSEelgamal_H

#include "SSEincludes.h"

class elgamal{
private:
    BN_CTX *ctxbn;
    uint32_t size = 1024;
    uint32_t maxmsgspace = 1000;
    BIGNUM* s;              //secret
    BIGNUM* h;              //public
public:
    BIGNUM *p;
    BIGNUM *q;                  
    BIGNUM *g;
    elgamal();
    elgamal(uint32_t s, uint32_t m);
    ~elgamal();
    void groupgen(); 
    void setgroup(BIGNUM* a,BIGNUM* b, BIGNUM* c, uint32_t newsize, uint32_t newmaxspace);
    void storegroup(string name);
    void setgroup(string name);
    BIGNUM* randomnogen();
    void keygen();
    void storesecret(string name);
    void setsecret(string name);
    void encrypt(BIGNUM* m, BIGNUM** x, BIGNUM** y);
    int  decrypt(BIGNUM* x,BIGNUM* y);
};

#endif // SSEelgamal_H