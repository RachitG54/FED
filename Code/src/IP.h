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


