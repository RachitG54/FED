#ifndef SSEHash
#define SSEHash

#include "SSEincludes.h"

class Hash {
private:
public:
    uint32_t OUTPUTLENGTH = 256;
    uchar* evaluate(const uchar* input, uint32_t sz_in, uint32_t &sz_out);
	void evaluate(const uchar* input, uint32_t sz_in, uint32_t &sz_out, unsigned char** result);
};

#endif // SSEHash
