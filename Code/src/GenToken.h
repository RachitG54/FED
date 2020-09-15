#ifndef SSEGenToken
#define SSEGenToken

#include "SSEincludes.h"
#include "SSEDH.h"
#include "SSEECDH.h"
#include "TSet.h"
#include "PRF.h"
#include "PRF_P.h"
#include "cliquery.h"
#include "authtok.h"
#include "envelope.h"
#include "CPAencrypt.h"

class GenToken {
private:
	SSEECDH group;

public:
    GenToken();
    string authorize(string Qstr,map<string,int> &wordcount);
    ~GenToken();
};

#endif // SSEGenToken