
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
#ifndef SSEEDBSetup
#define SSEEDBSetup

#include "SSEincludes.h"
#include "SSEDH.h"
#include "SSEECDH.h"
#include "TSet.h"
#include "PRF.h"
#include "PRF_P.h"
#include "CPAencrypt.h"
#include "RSAencrypt.h"

class EDBSetup {
private:
	SSEECDH group;
	uint32_t indlen = 4;
	uint32_t KEYLENGTH;

public:
	uint32_t enclen = 32;
	TSet TSetobj;
    EDBSetup();
    void initialize();
    uint32_t getindlen();
    ~EDBSetup();
};

#endif // SSEEDBSetup