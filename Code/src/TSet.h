#ifndef SSETSet
#define SSETSet

#include "SSEincludes.h"
// #include "Hash.h"
#include "PRF.h"
#include "SSEtime.h"
/// Structure to hold information about the TSet's

typedef pair< vector<uchar> , vector<uchar> > record;

struct recordchar
{
    char* first = NULL;
    char* second = NULL;
};

struct hashelements{
	uint32_t B;
	uchar* K;
	uchar* L; 
};

uchar* tochar(uint32_t value, uint32_t len);

struct db{
    set<string> W;
    map<uint32_t, set<string> > DB;   
    map<string, set<int> > DB_inv;
};

struct db2{
    set<string> W;
    map<ll, set<string> > DB;   
    map<string, set<ll> > DB_inv;
};

class TSet {
private:
	uint32_t B;
	uint32_t S;
	uint32_t lambda;
    uint32_t totaldocs;
	uint32_t len_B;
	uint32_t len_L;
    uint32_t len_K; 
    uint32_t KEYLENGTH;
    vector< vector <record> > T;
    recordchar** Tnew = NULL;
    int *countind = NULL;

public:
    TSet();
    ~TSet();
    void TSetSetup();
    void TSetSetup(map<string, vector< vector<uchar> > > &Tlist, uint32_t newlen_K, string name);
    db getdata(string name);
    db datainitialize_basic();
    db genomeinitialize(string name);
    void TSetGetTag(uchar* key, uint32_t keylen, const uchar* w, uint32_t wlen, uchar* &stag, uint32_t &stagbitlen);
    // vector< vector<uchar> > TSetRetrieve(uchar* stag, uint32_t stagbitlen);
    // void TSetstore(string name);
    // void readTSetstore(string name);
    void tempstore(string name);
    void tempstore2(string name);
    // void tempreadstore(string name);
    void optimizeTset();
    void tempreadstore2(string name);
    vector< vector<uchar> > TSetRetrieve2(uchar* stag, uint32_t stagbitlen);
};

#endif // SSETSet