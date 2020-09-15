#ifndef SSEEDBSetup
#define SSEEDBSetup

#include "SSEincludes.h"
#include "SSEDH.h"
#include "SSEECDH.h"
#include "TSet.h"
#include "PRF.h"
#include "PRF_P.h"
#include "CPAencrypt.h"
#include "RSAencrypt.h"

class EDBSetup {
private:
	SSEECDH group;
	uint32_t indlen = 4;
	uint32_t KEYLENGTH;

public:
	uint32_t enclen = 32;
	TSet TSetobj;
    EDBSetup();
    void initialize();
    uint32_t getindlen();
    ~EDBSetup();
};

#endif // SSEEDBSetup