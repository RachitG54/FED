
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
#include "Xvalret.h"

Xvalret::Xvalret() {
	KEYLENGTH = 256;
}

void Xvalret::initialize() {

	vector<string> allLines;
	int allLinesIdx = 0;
	getAllLinesFromFile(pathGenome, allLines);

	SSEtime timer;
	PRF PRFKval;
	PRFKval.setkeylength(KEYLENGTH);
	PRFKval.keyGen();

	string line;
	line = allLines[allLinesIdx++];

	vector<string> xx;
	boost::split(xx, line, boost::is_any_of(" \t"), boost::token_compress_on);
	assert(xx.size() == 3);
	int rowNum = atoi(xx[0].c_str());
	int colNum = atoi(xx[1].c_str());
	int searchablefilter = atoi(xx[2].c_str());

	int possibilities = 1 << numberofbits;

	int relcols = colNum - searchablefilter;
	int noofchars = (relcols*numberofbits+7)>>3;

	uchar** x = (uchar**)malloc(sizeof(uchar*)*rowNum);
	REP(i,0,rowNum-1) {
		xx.clear();
		line = allLines[allLinesIdx++];
		boost::split(xx, line, boost::is_any_of(" \t"), boost::token_compress_on);
		x[i] = (uchar*)malloc(sizeof(uchar)*noofchars);
		memset(x[i],0,noofchars);
		REP(attr,searchablefilter,colNum-1) {
			string datastr = xx[attr];
			stringstream ss(datastr);
			int data; ss >> data;
			int ind = attr - searchablefilter;
			int t = x[i][ind/possibilities];
			x[i][ind/possibilities] = (uchar)(data + (t*possibilities));

			if(attr==colNum-1) {
				int rem = possibilities-1-(ind%possibilities);
				x[i][ind/possibilities] = (uchar)(x[i][ind/possibilities]<<(rem*numberofbits));
			}
		}
		uchar* FKidstr = FKid(PRFKval,i,noofchars);

		REP(j,0,noofchars-1) {
			x[i][j] = (uchar)(FKidstr[j] ^ x[i][j]);
		}
		free(FKidstr);
	}
	cout<<"Xdata is setup. No of chars is "<<noofchars<<"\n";
	cout<<"[Final ValueRet]: Dataset generation takes ";
	timer.donetime();
	timer.printtime(0);

	ofstream fout;
	fout.open("./src/EDB/Xvalret",ios::out| ios::binary);
	stringstream ss;
	ss << noofchars<<"\n"<<rowNum<<"\n";
	REP(i,0,rowNum-1) {
		ss.write((char*)x[i],noofchars);
		free(x[i]);
	}
	free(x);
	string str = ss.str();
	fout.write(str.data(),str.size());
	fout.close();

	cout<<"[Final ValueRet]: Size of encrypted data is "<<str.size()<<" bytes\n";

	PRFKval.storekey("Kval");
}

void Xvalret::multiDOinitialize(vector< vector<string> > &MDdata, int DOindex) {
	cout<<"Combining ValueRet for multiple DO\n";

	timeDO.starttime();

  RSAencrypt PubCryptoSystem;
  PubCryptoSystem.readpubkey("pubkeyS.pem");

  RSAencrypt PubCryptoSystemA;
  PubCryptoSystemA.readpubkey("pubkeyA.pem");

	SSEtime timer;

	int rowNum = MDdata.size();

	int colNum = 0;
	if(rowNum>0) colNum = MDdata[0].size();
	int possibilities = 1 << numberofbits;

	int relcols = colNum;
	int noofchars = (relcols*numberofbits+7)>>3;
	uchar** x = (uchar**)malloc(sizeof(uchar*)*rowNum);

  vector< pair<string,int> > enckeys(rowNum);

  vector<string> encData(rowNum);
	REP(i,0,rowNum-1) {

		PRF PRFKval;
		PRFKval.setkeylength(KEYLENGTH);
		PRFKval.keyGen();

		x[i] = (uchar*)malloc(sizeof(uchar)*noofchars);
		memset(x[i],0,noofchars);

		uchar* id = (uchar*)malloc(sizeof(uchar)*4);
    RAND_bytes(id, 4);

    uint32_t randid = toInt(id);
    int signedid = (int)randid;

    if(signedid<0) signedid = -signedid;
    free(id);

		REP(attr,0,colNum-1) {
			string datastr = MDdata[i][attr];

			stringstream ss(datastr);
			int data; ss >> data;
			int ind = attr;
			int t = x[i][ind/possibilities];
			x[i][ind/possibilities] = (uchar)(data + (t*possibilities));
			if(attr==colNum-1) {
				int rem = possibilities-1-(ind%possibilities);
				x[i][ind/possibilities] = (uchar)(x[i][ind/possibilities]<<(rem*numberofbits));
			}
		}
		uchar* FKidstr = FKid(PRFKval,signedid,noofchars);

		REP(j,0,noofchars-1) {
			x[i][j] = (uchar)(FKidstr[j] ^ x[i][j]);
		}
		free(FKidstr);

		string plainkey;
		PRFKval.getkey(plainkey);
    enckeys[i].first = PubCryptoSystem.encrypt(plainkey.size(),(const uchar*)plainkey.data());
    enckeys[i].second = signedid;

    {
        stringstream ss;
        msgpack::sbuffer sbuf;
        string xstr(noofchars,'\0');
        REP(s,0,noofchars-1) xstr[s] = x[i][s];

        msgpack::pack(sbuf, xstr);
        msgpack::pack(sbuf, enckeys[i]);
        ss.write(sbuf.data(),sbuf.size());
        string str = ss.str();
        PubCryptoSystemA.envelope_seal((uchar*)str.data(),str.size(),encData[i]);
    }
	}
	cout<<"Xdata is setup. No of chars is "<<noofchars<<"\n";

	cout<<"[Final Data]: ValueRet Dataset generation takes ";
	timer.donetime();
	timer.printtime(0);

	timeDO.donetime();

	{
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, encData);
    ofstream out("./src/EDB/Xvalret_" + to_string(DOindex));
    out.write(sbuf.data(),sbuf.size());
    out.close();
    cout<<"[Final Data]: Size of encrypted data is "<<sbuf.size()<<endl;

    MDDObytes += sbuf.size();
  }
}

string Xvalret::compserve(vi &T, string &key, int noofchars, vi positions, int rowNum, uchar** theta/*, vi newpositions*/) {
	PRF PRFKvaltemp;
	PRFKvaltemp.setkeylength(KEYLENGTH);
	PRFKvaltemp.keyGen();

	PRFKvaltemp.getkey(key);

	int possibilities = 1 << numberofbits;

	SSEtime timer;
	int poschars = ((positions.size()*numberofbits+7)>>3);
	string str;
	str.resize(T.size()*poschars);
	REP(i,0,T.size()-1) {
		uchar* FKidstr = FKid(PRFKvaltemp,T[i],noofchars);
		REP(j,0,positions.size()-1) {

			int cat = positions[j]>>numberofbits; int ind = possibilities-1-(positions[j]%possibilities);
			int data1 = (uchar)theta[T[i]][cat];
			data1 = (data1 >> (ind*numberofbits))&(possibilities-1);
			int data2 = FKidstr[cat];
			data2 = (data2 >> (ind*numberofbits))&(possibilities-1);

			int data = data1 ^ data2;

			cat = j>>numberofbits; ind = possibilities-1-(j%possibilities);

			int prevdata = (uchar)str[i*poschars+cat];
			str[i*poschars+cat] = (char)(data + (prevdata*possibilities));

			if(j == (int)positions.size()-1) {
				prevdata = (uchar)str[i*poschars+cat];
				prevdata = prevdata<<(numberofbits*ind);
				str[i*poschars+cat] = (char)prevdata;
			}
		}
		free(FKidstr);
	}
	cout<<"[ValueRet:] Server takes ";
	timer.donetime();
	timer.printtime(0);
	return str;

}
string Xvalret::compauth(vi &T, string &keytemp, int noofchars/*, vi newpositions**/) {
	cout<<noofchars<<"\n";
	SSEtime timer;
	PRF PRFKval;
	uint32_t keylen;
	uchar* key = NULL;
	key = readkeys(keylen, "Kval");
	PRFKval.setkeylength(keylen); PRFKval.setkey(key);

	PRF PRFKvaltemp;

	memcpy(key,keytemp.data(),keylen/8);
	PRFKvaltemp.setkeylength(keylen); PRFKvaltemp.setkey(key);
	free(key);

	string str;
	str.resize(T.size()*noofchars);

	REP(i,0,T.size()-1) {
		uchar* FKidstr1 = FKid(PRFKvaltemp,T[i],noofchars);
		uchar* FKidstr2 = FKid(PRFKval,T[i],noofchars);
		REP(j,0,noofchars-1) {
			int data1 = (uchar)FKidstr1[j];
			int data2 = (uchar)FKidstr2[j];
			int data = data1^data2;
			str[i*noofchars+j] = (char)(data);
		}
		free(FKidstr1);
		free(FKidstr2);
	}
	cout<<"[ValueRet:] Auxilliary server takes ";
	timer.donetime();
	timer.printtime(0);
	return str;

}

string Xvalret::compclient(string &a, string &b, int ans, int noofchars, vi positions) {
	SSEtime timer;
	string c;
	int possibilities = 1 << numberofbits;
	int poschars = ((positions.size()*numberofbits+7)>>3);
	c.resize(ans*poschars);
	REP(i,0,ans-1) {
		REP(j,0,positions.size()-1) {

			int cat = positions[j]>>numberofbits; int ind = possibilities-1-(positions[j]%possibilities);
			int data1 = (uchar)a[i*noofchars+cat];

			data1 = (data1 >> (ind*numberofbits))&(possibilities-1);

			cat = j>>numberofbits; ind = possibilities-1-(j%possibilities);
			int data2 = (uchar)b[i*poschars+cat];

			data2 = (data2 >> (ind*numberofbits))&(possibilities-1);

			int data = data1^data2;

			int prevdata = (uchar)c[i*poschars+cat];
			c[i*poschars+cat] = (char)(data + (prevdata*possibilities));

			if(j == (int)positions.size()-1) {
				prevdata = (uchar)c[i*poschars+cat];
				prevdata = prevdata<<(numberofbits*ind);
				c[i*poschars+cat] = (char)prevdata;
			}

			printf("[Ans]: i: %d, Position: %d, data %d\n", i,positions[j],data);
		}
	}
	cout<<"[ValueRet:] Client takes ";
	timer.donetime();
	timer.printtime(0);
	return c;
}

uchar* FKid(PRF &F,int id, int noofchars) {
	uint32_t outlength;
	string idstr = to_string(id);
	uint32_t idlen = ((noofchars + 15)>>4)<<4;
	F.setoutputlength(idlen*8);
	uchar* out = F.evaluate((uchar*)idstr.c_str(),idstr.size(),outlength);
	uchar* FKidstr = (uchar*)malloc(sizeof(uchar)*noofchars);
	memcpy(FKidstr,out,noofchars);
	free(out);
	return FKidstr;
}
