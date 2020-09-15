#ifndef SSEEQCheck
#define SSEEQCheck

#include "SSEincludes.h"
#include "UnionFind.h"
#include "PRF.h"


class EQCheck {
private:
	int GFsize = 64;
	vector< pair<string,string> > val;
public:
	void initiate(int iter);
	void hashintoelement(string &s, NTL::ZZ_pE &x);
    void AtoS(vector< vector<string> > &rMAP, string &s, int query = 0);
    void StoA(vector< vector<string> > &uMAP, string &str, string &s, int query = 0);
    void compA(string &s, UnionFind &UF, int nkw);
    void compAquery(vector< vector<string> > &uMAP,string &s,vector<int> &corrshares);
    void compAquery2(string &s,vector<int> &corrshares, int n, int m);
    void readEQshares(string filename);
};

#endif // SSEEQCheck