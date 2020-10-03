
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
#include "SSEtime.h"

SSEtime timeDO;
SSEtime timeSrvr;
SSEtime timeAuxSrvr;

SSEtime timeDOSE;
SSEtime timeSrvrSE;
SSEtime timeAuxSrvrSE;


SSEtime envseal; 
SSEtime envopen;

SSEtime::SSEtime() {
	start = high_resolution_clock::now();
	duration = duration_cast<microseconds>(start - start);
}
void SSEtime::starttime() {
	start = high_resolution_clock::now();
}

void SSEtime::donetime() {
    stop = high_resolution_clock::now();
    duration += duration_cast<microseconds>(stop - start);
}
void SSEtime::printtime(int cont) {
    cout << (duration).count() << " microseconds" << endl;
	if(cont != 1)duration = duration_cast<microseconds>(start - start);
}

uint64_t SSEtime::getTime(){
	return (duration).count();
}

uint64_t SSEtime::getTimefromstart() {
	high_resolution_clock::time_point now = high_resolution_clock::now();
    return (duration_cast<seconds>(now - start)).count();
}

void SSEtime::resetTime(){
	duration = duration_cast<microseconds>(start - start);
}

uint64_t SSEtime::getAndResetTime(){
	microseconds temp = duration;
	duration = duration_cast<microseconds>(start - start);
	start = high_resolution_clock::now();
	return (temp).count();
}