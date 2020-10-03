
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
#ifndef SCEXfuncGC_H
#define SCEXfuncGC_H

#include "SSEincludes.h"
#include "PRF.h"
#include "CPAencrypt.h"
#include "RSAencrypt.h"
#include "SSEtime.h"

uchar* FKidGC(PRF &F,int id, int noofchars);

class XfuncGC {
private:
	int indlen =  4;
	int attrlen = 4;

public:
	int numberofbits = 3;
	uint32_t KEYLENGTH;
	XfuncGC();
	
	uchar* evaluate(PRF &F, int attribute, int ind, int data, uint32_t &clen);
    void initialize();
    void multiDOinitialize(vector< vector<string> > &MDdata, int DOindex);
	void compserve(vi &T, string &datafromA, int PRFlen, int relcols, int randlen, uchar** x, uchar** pandp);
	string compauth(vi &T, vector<string> &query, vector<string> &pandp, vector<string> &keyslab, int tempPRFlen);
    unsigned long compclient(unsigned long gamma, unsigned long delta);
};


#endif // SCEXfuncGC_H


