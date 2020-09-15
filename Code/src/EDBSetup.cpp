#include "EDBSetup.h"

EDBSetup::EDBSetup() {
	KEYLENGTH = 256;
    indlen = 4;						//depends on doc size
    enclen = 32;
    if(DO) {
        RSAencrypt inst;
        inst.readpubkey("pubkeyA.pem");
        indlen = inst.getencsize();
        enclen = indlen+16;
    }
	group.setDH();
}
uint32_t EDBSetup::getindlen() {
    return indlen;
}
void EDBSetup::initialize() {
    set<BIGNUM*> XSet;
    set<BIGNUM*> XSetAll;
    map<string, vector< vector<uchar> > > Tlist;


    vector<string> id_inv;
    db database;
    if(DO) {
        {
            ifstream inp;
            inp.open("./src/EDB/kw_idlist",ios::in);
            stringstream ss;
            ss << inp.rdbuf();
            string str = ss.str();
            msgpack::object_handle oh = msgpack::unpack(str.data(), str.size());
            oh.get().convert(database.DB_inv);
        }

        {
            ifstream inp;
            inp.open("./src/EDB/OPRFkw",ios::in);
            stringstream ss;
            ss << inp.rdbuf();
            string str = ss.str();
            msgpack::object_handle oh = msgpack::unpack(str.data(), str.size());
            oh.get().convert(database.W);
        }

        {
            ifstream inp;
            inp.open("./src/EDB/id_invDO",ios::in);
            stringstream ss;
            ss << inp.rdbuf();
            string str = ss.str();
            msgpack::object_handle oh = msgpack::unpack(str.data(), str.size());
            oh.get().convert(id_inv);
        }
        cout<<"Read multi-DO's data\n";
    }
    else {
        database = TSetobj.genomeinitialize("null");
    }

	PRF PRFK_s;
	PRFK_s.setkeylength(KEYLENGTH);
	PRFK_s.keyGen();

	PRF_P PRFK_x;
	PRFK_x.setkeylength(KEYLENGTH);
	PRFK_x.keyGen();
	PRFK_x.setprime(group.q);

	PRF_P PRFK_i;
	PRFK_i.setkeylength(KEYLENGTH);
	PRFK_i.keyGen();
	PRFK_i.setprime(group.q);

    CPAencrypt enc_dec2;
    enc_dec2.keygen();
    enc_dec2.storekey("K_m");

    CPAencrypt ip;
    ip.keygen();
    ip.storekey("K_ip");

    CPAencrypt enc_dec;
    enc_dec.keygen();

    enclen = ((indlen+enc_dec.blocksize()-1)/enc_dec.blocksize())*enc_dec.blocksize() + enc_dec.ivsize();
    uint32_t newlen_k = group.getDHsubgroupsize() + enclen;

    cout<<"Starting EDBSetup\n";

    SSEtime timer;

	tr(database.W,it) {
    	const uchar* w = (const uchar*)((*it).c_str());
    	vector< vector<uchar> > t;

    	uint32_t strapbitlen;
        uchar* strap = PRFK_s.evaluate(w,it->size(),strapbitlen);

    	PRF strapPRF;
    	strapPRF.setkeylength(strapbitlen);
    	strapPRF.setkey(strap);


    	uint32_t K_zbitlen;

    	string str = to_string(1);
        uchar* K_z = strapPRF.evaluate((const uchar*)str.c_str(),str.size(),K_zbitlen);
        PRF_P PRFK_z;
        PRFK_z.setkeylength(K_zbitlen); PRFK_z.setkey(K_z,group.q);

        BIGNUM *kxw = PRFK_x.evaluate(w,it->size());
        EC_POINT *gkxw = group.modexppgen(kxw);
        group.free(&kxw);

        vector<int> randomorderer(database.DB_inv[*it].size());
        int tempi = 0;
        tr(database.DB_inv[*it],it2) {
            randomorderer[tempi++] = *it2;
        }
        random_shuffle(all(randomorderer));

        uint32_t c = 0;

        REP(ind,0,randomorderer.size()-1) {
        	string str = to_string(randomorderer[ind]);
            BIGNUM* xind = PRFK_i.evaluate((const uchar*)str.c_str(),str.size());
            c += 1;
            string concatstr = /**it + */to_string(c);
            BIGNUM* z_c = PRFK_z.evaluate((const uchar*)concatstr.c_str(),concatstr.size());
            BIGNUM *z_c_inv = group.modinvq(z_c);
            BIGNUM* y = group.modmulq(xind,z_c_inv);

            uchar* strno;
            if(DO) {
                if(randomorderer[ind]>=(int)id_inv.size()) {
                    cout<<"Error creating multi-DO stuff";
                    exit(1);
                }
                string PKstr = id_inv[randomorderer[ind]];
                strno = (uchar*)malloc(sizeof(int)*PKstr.size());
                memcpy(strno,PKstr.data(),PKstr.size());
            }
            else {
                strno = tochar(randomorderer[ind],indlen);
            }

            uchar* enctext = (uchar*) malloc(sizeof(uchar)*(enclen-enc_dec.ivsize()+1));

            memset(enctext,0,enclen-enc_dec.ivsize());
            memcpy(enctext,strno,indlen);

            enc_dec.ivgen();

            int clen;
            uchar* ciphertext = enc_dec.encrypt(enctext,enclen-enc_dec.ivsize(),clen,0);
            free(enctext);

            uint32_t ivlen;
            uchar* iv = enc_dec.getiv(ivlen);

            if(enclen != clen + (ivlen/8)) { cout<<"Fix this thing, stretch it, not needed for now\n"; exit(1); }

            uint32_t sizey = group.num_bytes(y);
            uint32_t expectedsz = newlen_k - enclen;

            if(sizey > expectedsz) {cout<<"Size mismatch, check arguments\n";exit(1);}

            uchar* stry = group.bn2bin(y);

            uchar* stretchedstry = (uchar*) malloc(sizeof(uchar)*expectedsz);
            memset(stretchedstry,0,expectedsz);
            memcpy(stretchedstry+expectedsz-sizey,stry,sizey);

            uchar* tmemc = (uchar*) malloc(sizeof(uchar)*(clen+(ivlen/8)+expectedsz));
            memcpy(tmemc,ciphertext,clen);
            memcpy(tmemc + clen,iv,ivlen/8);
            memcpy(tmemc + clen + (ivlen/8),stretchedstry,expectedsz);
            t.pb(vector<uchar>(tmemc,tmemc+clen+(ivlen/8)+expectedsz));
            free(tmemc);

            EC_POINT *xtagp = group.modexpp(gkxw, xind);
            BIGNUM* xtag = group.point2bn(xtagp);
            group.free(&xtagp);
            XSet.insert(xtag);


            free(iv);
            free(ciphertext);
            free(strno);
            free(stry);
            free(stretchedstry);
            group.free(&y);
            group.free(&xind);
            group.free(&z_c);
            group.free(&z_c_inv);
        }

        Tlist[*it] = t;
        free(strap);
        free(K_z);
        group.free(&gkxw);

	}

    timer.donetime();
    cout<<"[Final SE]: XSet generation takes ";
    timer.printtime(0);
    cout<<"Completed EDBSetup\n";

	PRFK_s.storekey("K_s");
	PRFK_x.storekey("K_x");
	PRFK_i.storekey("K_i");

  enc_dec.storekey("K_e");

	storeset2(XSet,"XSet",group.getDHsize()/8);
  ifstream inp("src/EDB/XSet");

  inp.seekg (0, inp.end);
  int filelen = inp.tellg();
  inp.close();

  cout<<"[Final SE]: Size of XSet data produced is "<<filelen<<" bytes.\n";
  tr(XSet,it) {BIGNUM* temp = *it; group.free(&temp);}
  tr(XSetAll,it) {BIGNUM* temp = *it; group.free(&temp);}

  TSetobj.TSetSetup(Tlist,newlen_k,"K_t");
  cout<<"TSet is setup::"<<newlen_k<<"\n";

}

EDBSetup::~EDBSetup() {
}
