
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
#ifndef IP_H
#define IP_H

#include "SSEincludes.h"
#include "PRF.h"
#include "RSAencrypt.h"
#include "SSEtime.h"


class IP {
private:
	int attrlen = 4;
	int indlen = 4;

public:
	uint32_t KEYLENGTH;
	int messagedim = 30;
	IP();
    void initialize();
    unsigned long compserve(vi &T, unsigned long &rho, int attr, vector< vector<unsigned long> > &XData);
    unsigned long compauth(vi &T, unsigned long rho, int attr);
    unsigned long compclient(unsigned long gamma, unsigned long delta);

	unsigned long evaluate(PRF &F, uint32_t attribute, uint32_t ind);
	void multiDOinitialize(vector< vector<string> > &MDdata, int DOindex);
};


#endif // IP_H


