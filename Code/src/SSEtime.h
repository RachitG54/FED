
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
#ifndef SSEtime_H
#define SSEtime_H

#include "SSEincludes.h"

class SSEtime {
private:
    high_resolution_clock::time_point start = high_resolution_clock::now();
    high_resolution_clock::time_point stop = high_resolution_clock::now();
    microseconds duration = duration_cast<microseconds>(stop - start);
public:
    SSEtime();
    void starttime();
    void donetime();
    void printtime(int cont);
	uint64_t getTime();
    uint64_t getTimefromstart(); 
	void resetTime();
	uint64_t getAndResetTime();
};
extern SSEtime timeDO;
extern SSEtime timeSrvr; 
extern SSEtime timeAuxSrvr;


extern SSEtime timeDOSE;
extern SSEtime timeSrvrSE; 
extern SSEtime timeAuxSrvrSE;

extern SSEtime envseal; 
extern SSEtime envopen;
  
#endif // SSEtime_H