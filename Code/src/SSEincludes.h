
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
#ifndef SSEincludes
#define SSEincludes

#include <iostream>
#include <fstream>
#include <cstdio>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <bitset>
#include <random>
#include <chrono>
#include <cmath>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/bn.h>
#include <openssl/sha.h>
#include <openssl/err.h>
#include <openssl/hmac.h>
#include <openssl/cmac.h>
#include <openssl/conf.h>
#include <openssl/engine.h>
#include <openssl/dh.h>
#include <openssl/obj_mac.h>
#include <openssl/ec.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/bio.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <iomanip>
#include <vector>
#include <msgpack.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <zlib.h>
#include <gmp.h>
#include <errno.h>
#include <boost/algorithm/string.hpp>
#include <NTL/ZZ_pXFactoring.h>
#include <NTL/ZZ_pEX.h>
#include <cassert>

using namespace std;
using namespace std::chrono;

typedef vector<int> vi;
typedef vector<vi> vvi;
typedef pair<int,int> ii;
typedef vector<ii> vii;
typedef long long ll;
typedef vector<ll> vlli;
typedef unsigned char uchar;

//#define DEBUG

#ifdef DEBUG
#define NETWORKTEST
#endif

#define sz(a) int((a).size())
#define pb push_back
#define all(c) (c).begin(),(c).end()
#define tr(c,i) for(typeof((c).begin()) i = (c).begin(); i!=(c).end(); i++)
#define present(c,e) ((c).find(e) != (c).end())
#define cpresent(c,e) (find(all(c),e) != (c).end())
#define REP(i,a,b) for(int i=int(a); i<=int(b); i++)
#define constlen 20

extern string pathjustgarble;
extern string pathSSE;
extern string pathoblivc;
extern string pathGenome;
extern string pathQuery;
extern string pathtest;
extern int protocol;
extern int DO;
extern int DO_id;
extern int networkports;

extern ll MDDObytes;
extern ll MDClntbytes;
extern ll MDSrvrbytes;
extern ll MDAuxSrvrbytes;

extern ll MDDObytesSE;
extern ll MDClntbytesSE;
extern ll MDSrvrbytesSE;
extern ll MDAuxSrvrbytesSE;
extern int compressornot;

void handleErrors();
uchar* readkeys(uint32_t &keylen, string keyname);
set<string> readset(string name);
void storeset(set<BIGNUM*> s, string name);
set<string> readset2(string name);
void storeset2(set<BIGNUM*> s, string name, uint32_t size);
msgpack::object_handle readobject(string name);
uchar** readlabels(int &n,int &labelsize);
uint64_t toBigInt(uchar* bytes, uint32_t len);
uint64_t toBigInt(vector<uchar> bytes, uint32_t len);
uint32_t toInt(uchar* bytes);
uchar* tochar(uint32_t value, uint32_t len);
void getAllLinesFromFile(string fileName, vector<string>& output);
void get_file_contents(const char *filename, string &contents);

// Unpacking two message pack items one after the other, using templates for variable datatyoe
template <class T, class U> void readobjstore (T &obj, U &obj2, string name) {
	ifstream fin;
	msgpack::unpacker pac;
    fin.open("./src/EDB/"+name,ios::in);
    stringstream ss;
    ss << fin.rdbuf();
    string buffer = ss.str();
    // feeds the buffer.
    pac.reserve_buffer(buffer.size());
    memcpy(pac.buffer(), buffer.data(), buffer.size());
    pac.buffer_consumed(buffer.size());

    // now starts streaming deserialization.
    msgpack::object_handle oh;
    pac.next(oh);
    msgpack::object msgobj = oh.get();
    msgobj.convert(obj);
    pac.next(oh);
    msgpack::object msgobj2 = oh.get();
    msgobj2.convert(obj2);
    fin.close();
}
// Reading file from any datatype and returning file size
template <class T> int readfrommsgpackfile(T &obj, string filename) {
    ifstream inp(filename,ios::binary);

    std::stringstream buffer;
    buffer << inp.rdbuf();
    string str = buffer.str();
    inp.close();
    {
        msgpack::object_handle oh = msgpack::unpack(str.data(),str.size());
        oh.get().convert(obj);
    }
    int filelen = str.size();
    return filelen;
}
// Writing any datatype into file using message pack
template <class T> int writetomsgpackfile(T &obj, string filename) {
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, obj);
    ofstream out(filename,ios::binary);
    out.write(sbuf.data(),sbuf.size());
    out.close();
    return sbuf.size();
}
int filelength(string filename);
int compress_vector(string &message);
int decompress_vector(string &message);

#endif // Includes
