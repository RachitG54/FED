#ifndef SCEXvalret
#define SCEXvalret

#include "SSEincludes.h"
#include "SSEDH.h"
#include "SSEECDH.h"
#include "TSet.h"
#include "PRF.h"
#include "PRF_P.h"
#include "CPAencrypt.h"
#include "RSAencrypt.h"
#include "SSEtime.h"

uchar* FKid(PRF &F,int id, int noofchars);

class Xvalret {
private:
	int numberofbits = 2;
public:
	uint32_t KEYLENGTH;
    Xvalret();
    void initialize();
    string compserve(vi &T, string &key, int noofchars, vi positions, int rowNum, uchar** theta/*, vi newpositions*/);
    string compauth(vi &T, string &keytemp, int noofchars/*, vi positions**/);
    string compclient(string &a, string &b, int ans, int noofchars, vi positions);
    void multiDOinitialize(vector< vector<string> > &MDdata, int DOindex);
};

#endif // SCEXvalret