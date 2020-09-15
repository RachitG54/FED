#include "GenToken.h"

GenToken::GenToken() {
	group.setDH();
}
string GenToken::authorize(string Qstr,map<string,int> &wordcount) {
	SSEquery::cliquery Q = SSEquery::receiveclass(Qstr);
	uint32_t keylen;
	uchar* key = NULL;

	key = readkeys(keylen, "K_s");
	PRF PRFK_s; PRFK_s.setkeylength(keylen); PRFK_s.setkey(key);
	free(key);

	key = readkeys(keylen, "K_x");
	PRF_P PRFK_x; PRFK_x.setkeylength(keylen); PRFK_x.setkey(key,group.q);
	free(key);

	key = readkeys(keylen, "K_i");
	PRF_P PRFK_i; PRFK_i.setkeylength(keylen); PRFK_i.setkey(key,group.q);
	free(key);


	uint32_t keylen_TSet;
	uchar* key_TSet = readkeys(keylen_TSet,"K_t");

	uchar* stag = NULL;
	uint32_t stagbitlen;

	int minval = wordcount[Q.keywords[0]];
	int minindex = 0;
	REP(i,1,Q.keywords.size()-1) {
		int val = wordcount[Q.keywords[i]];
		if(val < minval) {
			minindex = i;
			minval = val;
		}
	}
	string tempstr = Q.keywords[minindex]; Q.keywords[minindex] = Q.keywords[0];
	Q.keywords[0] = tempstr;

	string word = Q.keywords[0];
	const uchar* w = (const uchar*)(word.c_str());

	TSet TSetobj;
	TSetobj.TSetGetTag(key_TSet,keylen_TSet,w,word.size(),stag,stagbitlen);

	uint32_t strapbitlen;
    uchar* strap = PRFK_s.evaluate(w,word.size(),strapbitlen);

    SSEtok::authtok tok;

    tok.tokens.pb(vector<uchar>(strap,strap+(strapbitlen/8)));
    tok.attribute = 1;

    SSEenv::envelope e;
    e.attribute = 1;

    e.stag.clear();
    e.stag.resize((stagbitlen/8));

    REP(i,0,(stagbitlen/8)-1) e.stag[i] = stag[i];

    REP(i,1,Q.keywords.size()-1) {
    	vector<uchar> trying;
    	e.rhos.pb(trying);
    }

    REP(i,1,Q.keywords.size()-1) {
	    BIGNUM* kxw_prf = PRFK_x.evaluate((const uchar*)Q.keywords[i].c_str(),Q.keywords[i].size());
	    BIGNUM* kxw = group.modq(kxw_prf);
	    group.free(&kxw_prf);
	    EC_POINT* gkxwp = group.modexppgen(kxw);
	    group.free(&kxw);
	    BIGNUM* gkxw = group.point2bn(gkxwp);
	    group.free(&gkxwp);
	    uint32_t len = group.num_bytes(gkxw);
      uchar* xtrap = group.bn2bin(gkxw);
      tok.tokens.pb(vector<uchar>(xtrap,xtrap+len));
      group.free(&gkxw);
      free(xtrap);
	}
	CPAencrypt enc_dec;
	enc_dec.readkey("K_m");
	string env = SSEenv::sendstr(e);
	enc_dec.ivgen();
	int clen;
	uchar* ciphertext = enc_dec.encrypt((uchar*)env.c_str(),env.size(),clen,1);
	tok.enc_env.resize(clen);
	REP(i,0,clen-1) tok.enc_env[i] = ciphertext[i];

	uint32_t ivlen; uchar* iv = enc_dec.getiv(ivlen);
	tok.rand_iv.resize(ivlen/8);
	REP(i,0,(ivlen/8)-1) tok.rand_iv[i] = iv[i];


	string sendtoclient = SSEtok::sendstr(tok);
	cout << "Sending "<<sendtoclient.size()<<" bytes of token from authortiy"<<"\n";

	free(key_TSet);
	free(strap);
	free(stag);
	free(iv);
	free(ciphertext);
	return sendtoclient;
}
GenToken::~GenToken() {
}
