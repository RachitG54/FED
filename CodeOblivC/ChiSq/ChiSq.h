// #pragma once
#define precision 100;

typedef struct protocolIO {
	long N;
	long numberCase;
	long totalNumCase;
	long numberControl;
	long totalNumControl;
	long long output1;
	long long output2;
} protocolIO;

void ChiSq(void* args);
