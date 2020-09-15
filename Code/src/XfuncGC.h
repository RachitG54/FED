#ifndef SCEXfuncGC_H
#define SCEXfuncGC_H

#include "SSEincludes.h"
#include "PRF.h"
#include "CPAencrypt.h"
#include "RSAencrypt.h"
#include "SSEtime.h"

uchar* FKidGC(PRF &F,int id, int noofchars);

class XfuncGC {
private:
	int indlen =  4;
	int attrlen = 4;

public:
	int numberofbits = 3;
	uint32_t KEYLENGTH;
	XfuncGC();
	
	uchar* evaluate(PRF &F, int attribute, int ind, int data, uint32_t &clen);
    void initialize();
    void multiDOinitialize(vector< vector<string> > &MDdata, int DOindex);
	void compserve(vi &T, string &datafromA, int PRFlen, int relcols, int randlen, uchar** x, uchar** pandp);
	string compauth(vi &T, vector<string> &query, vector<string> &pandp, vector<string> &keyslab, int tempPRFlen);
    unsigned long compclient(unsigned long gamma, unsigned long delta);
};


#endif // SCEXfuncGC_H


