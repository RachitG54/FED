
/* ====================================================================
* Functionally Encrypted Datatores - Implementation for project eprint.iacr.org/2019/1262
* Copyright (C) 2019  Rachit Garg Nishant Kumar

* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <https://www.gnu.org/licenses/>.

* ====================================================================
*
*/
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