
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
#include "EQCheck.h"
void EQCheck::initiate(int iter) {
	cout<<"$$$$$ Starting EQCheck.initiate "<<iter<<endl;
	NTL::ZZ_p::init(NTL::ZZ(2));
	NTL::ZZ_pX P;
	NTL::BuildIrred(P, GFsize); // generate an irreducible polynomial P

	NTL::ZZ_pE::init(P); // define GF(2^64)

	{
		ofstream fout("./src/keys/GF");
		fout << P <<"\n";
		fout.close();
	}

	vector< pair<string,string> > val1(iter);
	vector< pair<string,string> > val2(iter);
	uint64_t size1 = 0;
	uint64_t size2 = 0;
	REP(i,0,iter-1) {
		NTL::ZZ_pE a = NTL::random_ZZ_pE();
		NTL::ZZ_pE p = NTL::random_ZZ_pE();
		NTL::ZZ_pE q = NTL::random_ZZ_pE();
		NTL::ZZ_pE b = p*q-a;
		{ stringstream ss; ss << a; val1[i].first = ss.str(); size1 += (ss.str().size()); }
		{ stringstream ss; ss << p; val1[i].second = ss.str(); size1 += (ss.str().size());}
		{ stringstream ss; ss << b; val2[i].first = ss.str(); size2 += (ss.str().size());}
		{ stringstream ss; ss << q; val2[i].second = ss.str(); size2 += (ss.str().size());}
	}

	cout<<"$$$$$$$$$$$$$$$$$$$ Almost done EQCheck.initiate "<<endl;
	cout<<"Sizes of val1, val2 = "<<val1.size()<<" "<<val2.size()<<endl;
	cout<<"size1, size2 = "<<size1<<" "<<size2<<endl;

	{
		int sz = writetomsgpackfile(val1, "./src/EDB/EQCheckpreshareS");
		cout<<"Share S is "<<sz<<" bytes.\n";
	}
	{
		int sz = writetomsgpackfile(val2, "./src/EDB/EQCheckpreshareA");
		cout<<"Share A is "<<sz<<" bytes.\n";
	}
}
void EQCheck::hashintoelement(string &s, NTL::ZZ_pE &x) {

	Hash H;
	uint32_t sz_out;
	uchar* streval = H.evaluate((const uchar*)s.data(),s.size(),sz_out);
	stringstream ss;
	ss << "[";

	REP(s,0,GFsize- 1) {
		int val = s/8;
		int pos = s%8;
		int quant = (uchar)streval[val];
		int bit = (quant>>pos)&0x1;
		if(bit == 1) {
			ss << "1";
		}
		else {
			ss << "0";
		}
		if(s!=GFsize-1) ss<<" ";
	}
	ss << "]";

	free(streval);
	ss >> x;

}
void EQCheck::readEQshares(string filename) {
	int sz = readfrommsgpackfile(val, filename);
	cout <<"Size of read dataset is "<<sz<<"\n";
}
void EQCheck::AtoS(vector< vector<string> > &rMAP, string &s, int query) {
	NTL::ZZ_p::init(NTL::ZZ(2));
	NTL::ZZ_pX P;
	{
		ifstream fin("./src/keys/GF");
		fin >> P;
		fin.close();
	}
	NTL::ZZ_pE::init(P);
	int n = rMAP.size();

	vector<string> eqNTLshares;
	int iter = 0;
	REP(i,0,n-1) {
		int m = rMAP[i].size();
		int l;
		if(query == 1) l = 0;
		else l = i+1;
		REP(j,l,m-1) {
			NTL::ZZ_pE x;
			hashintoelement(rMAP[i][j],x);

			stringstream ss(val[iter].second);

			NTL::ZZ_pE p;
			ss >> p;

			NTL::ZZ_pE u = x-p;
			ss.clear();
			ss << u;
			eqNTLshares.pb(ss.str());
			iter++;
		}
	}

	{
		msgpack::sbuffer sbuf;
	    msgpack::pack(sbuf, eqNTLshares);
	    stringstream ss;
	    ss.write(sbuf.data(),sbuf.size());
	    s = ss.str();
	}
}
void EQCheck::StoA(vector< vector<string> > &uMAP, string &str, string &s, int query) {

	vector<string> eqNTLsharesA;
	{
		msgpack::object_handle oh = msgpack::unpack(str.data(), str.size());
		oh.get().convert(eqNTLsharesA);
	}

	NTL::ZZ_p::init(NTL::ZZ(2));
	NTL::ZZ_pX P;
	{
		ifstream fin("./src/keys/GF");
		fin >> P;
		fin.close();
	}

	NTL::ZZ_pE::init(P);
	int n = uMAP.size();


	vector<string> eqNTLshares;
	int iter = 0;
	REP(i,0,n-1) {
		int m = uMAP[i].size();
		int l;
		if(query == 1) l = 0;
		else l = i+1;
		REP(j,l,m-1) {
			NTL::ZZ_pE y;
			hashintoelement(uMAP[i][j],y);

			NTL::ZZ_pE q,b,u;
			{
				stringstream ss(val[iter].second);
				ss >> q;
			}

			{
				stringstream ss(val[iter].first);
				ss >> b;
			}

			{
				stringstream ss(eqNTLsharesA[iter]);
				ss >> u;
			}

			NTL::ZZ_pE v = q*(u-y)+b;
			stringstream ss;
			ss << v;
			eqNTLshares.pb(ss.str());
			iter++;
		}
	}

	{
		msgpack::sbuffer sbuf;
	    msgpack::pack(sbuf, eqNTLshares);
	    stringstream ss;
	    ss.write(sbuf.data(),sbuf.size());
	    s = ss.str();
	}

}
void EQCheck::compA(string &s, UnionFind &UF, int nkw) {
	vector<string> eqNTLsharesS;
	{
		msgpack::object_handle oh = msgpack::unpack(s.data(), s.size());
		oh.get().convert(eqNTLsharesS);
	}

	UF.initSet(nkw);
	int iter = 0;
	REP(i,0,nkw-1) {
		REP(j,i+1,nkw-1) {
			NTL::ZZ_pE a,v;

			{
				stringstream ss(val[iter].first);
				ss >> a;
			}

			{
				stringstream ss(eqNTLsharesS[iter]);
				ss >> v;
			}

			if(a == v) {
				if(!UF.isSameSet(i,j))
					UF.unionSet(i,j);
			}
			iter++;
		}
	}
}

void EQCheck::compAquery(vector< vector<string> > &uMAP,string &s,vector<int> &corrshares) {
	vector< vector<string> > rMAP;
	{
		msgpack::object_handle oh = msgpack::unpack(s.data(), s.size());
		oh.get().convert(rMAP);
	}
	int n = uMAP.size();
	corrshares.resize(n);
	REP(i,0,n-1) {
		int m = uMAP[i].size();
		int flag = -1;
		REP(j,0,m-1) {
			if(memcmp(rMAP[i][j].data(),uMAP[i][j].data(),rMAP[i][j].size())==0) {
				flag = 1;
				corrshares[i] = j;
				break;
			}
		}
		if(flag==-1) {
			cout<<"Error in creating keyword shares in EQCheck\n";
			exit(1);
		}
	}
}
void EQCheck::compAquery2(string &s,vector<int> &corrshares, int n, int m) {
	vector<string> eqNTLsharesS;
	{
		msgpack::object_handle oh = msgpack::unpack(s.data(), s.size());
		oh.get().convert(eqNTLsharesS);
	}

	int iter = 0;
	corrshares.resize(n);
	REP(i,0,n-1) {
		int flag = -1;
		iter = m*i;
		REP(j,0,m-1) {
			NTL::ZZ_pE a,v;

			{
				stringstream ss(val[iter].first);
				ss >> a;
			}

			{
				stringstream ss(eqNTLsharesS[iter]);
				ss >> v;
			}
			iter++;
			if(a == v) {
				flag = 1;
				corrshares[i] = j;
				break;
			}
		}
		if(flag==-1) {
			cout<<"Error in creating keyword shares in EQCheck 2nd one\n";
		}
	}
}
