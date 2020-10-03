
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
#ifndef Xaddpaillier_H
#define Xaddpaillier_H

#include "SSEincludes.h"
#include "PRF.h"
#include "SSEpaillier.h"
#include "RSAencrypt.h"
#include "SSEtime.h"

class Xaddpaillier {
private:
	uint32_t KEYLENGTH;

public:
	int messagedim = 30;
	Xaddpaillier();
    void initialize();
    void multiDOinitialize(vector< vector<string> > &MDdata, int DOindex);
    void setkeysauth();
	string compserve(vi &T, uint32_t &y, int attr, int rowNum, int relcols, int clen, uchar** x);
    uint32_t compauth(string &sumstr);
    uint32_t compclient(uint32_t z, uint32_t y);
};


#endif // Xaddpaillier_H


