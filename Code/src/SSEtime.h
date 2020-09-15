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