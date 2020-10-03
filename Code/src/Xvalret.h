
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
#ifndef SCEXvalret
#define SCEXvalret

#include "SSEincludes.h"
#include "SSEDH.h"
#include "SSEECDH.h"
#include "TSet.h"
#include "PRF.h"
#include "PRF_P.h"
#include "CPAencrypt.h"
#include "RSAencrypt.h"
#include "SSEtime.h"

uchar* FKid(PRF &F,int id, int noofchars);

class Xvalret {
private:
	int numberofbits = 2;
public:
	uint32_t KEYLENGTH;
    Xvalret();
    void initialize();
    string compserve(vi &T, string &key, int noofchars, vi positions, int rowNum, uchar** theta/*, vi newpositions*/);
    string compauth(vi &T, string &keytemp, int noofchars/*, vi positions**/);
    string compclient(string &a, string &b, int ans, int noofchars, vi positions);
    void multiDOinitialize(vector< vector<string> > &MDdata, int DOindex);
};

#endif // SCEXvalret