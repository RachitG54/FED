
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
#ifndef SSEGenToken
#define SSEGenToken

#include "SSEincludes.h"
#include "SSEDH.h"
#include "SSEECDH.h"
#include "TSet.h"
#include "PRF.h"
#include "PRF_P.h"
#include "cliquery.h"
#include "authtok.h"
#include "envelope.h"
#include "CPAencrypt.h"

class GenToken {
private:
	SSEECDH group;

public:
    GenToken();
    string authorize(string Qstr,map<string,int> &wordcount);
    ~GenToken();
};

#endif // SSEGenToken