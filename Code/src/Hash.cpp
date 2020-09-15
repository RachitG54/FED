#include "Hash.h"
uchar* Hash::evaluate(const uchar* input, uint32_t sz_in, uint32_t &sz_out) {
    sz_out = 256;
    uint32_t len = sz_out/8;
    unsigned char* result;
    result = (unsigned char*)malloc(sizeof(char) * len);

    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, input, sz_in);
    SHA256_Final(result, &sha256);
    return result;
}
void Hash::evaluate(const uchar* input, uint32_t sz_in, uint32_t &sz_out, unsigned char** result) {
    sz_out = OUTPUTLENGTH;
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, input, sz_in);
    SHA256_Final(*result, &sha256);
}
