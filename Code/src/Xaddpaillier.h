#ifndef Xaddpaillier_H
#define Xaddpaillier_H

#include "SSEincludes.h"
#include "PRF.h"
#include "SSEpaillier.h"
#include "RSAencrypt.h"
#include "SSEtime.h"

class Xaddpaillier {
private:
	uint32_t KEYLENGTH;

public:
	int messagedim = 30;
	Xaddpaillier();
    void initialize();
    void multiDOinitialize(vector< vector<string> > &MDdata, int DOindex);
    void setkeysauth();
	string compserve(vi &T, uint32_t &y, int attr, int rowNum, int relcols, int clen, uchar** x);
    uint32_t compauth(string &sumstr);
    uint32_t compclient(uint32_t z, uint32_t y);
};


#endif // Xaddpaillier_H


