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