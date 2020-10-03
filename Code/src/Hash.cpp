
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
#include "Hash.h"
uchar* Hash::evaluate(const uchar* input, uint32_t sz_in, uint32_t &sz_out) {
    sz_out = 256;
    uint32_t len = sz_out/8;
    unsigned char* result;
    result = (unsigned char*)malloc(sizeof(char) * len);

    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, input, sz_in);
    SHA256_Final(result, &sha256);
    return result;
}
void Hash::evaluate(const uchar* input, uint32_t sz_in, uint32_t &sz_out, unsigned char** result) {
    sz_out = OUTPUTLENGTH;
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, input, sz_in);
    SHA256_Final(*result, &sha256);
}
