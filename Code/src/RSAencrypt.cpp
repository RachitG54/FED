#include "RSAencrypt.h"
RSAencrypt::RSAencrypt() {
	if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();
	KEYLENGTH = 2048;
}
void RSAencrypt::generate_key_tofile(string name1,string name2) {
	string command = "openssl genpkey -algorithm RSA -out ./src/keys/"+name1+" -pkeyopt rsa_keygen_bits:" + to_string(KEYLENGTH);
    system(command.c_str());
    command = "openssl rsa -pubout -in ./src/keys/"+name1+" -out ./src/keys/"+name2;
    system(command.c_str());
}
void RSAencrypt::readpubkey(string name) {
    FILE *pubkeyfile;
    string loc = "./src/keys/"+name;
    if ((pubkeyfile = fopen(loc.c_str(), "r")) == NULL) {
        printf("Failed to open public key for reading\n");
        handleErrors();
    }
    if ((pubkey = PEM_read_PUBKEY(pubkeyfile, &pubkey, NULL, NULL)) == NULL) {
        fclose(pubkeyfile);
        handleErrors();
    }
    fclose(pubkeyfile);
}
void RSAencrypt::readprivkey(string name) {
    FILE *privkeyfile;
    string loc = "./src/keys/"+name;
    if ((privkeyfile = fopen(loc.c_str(), "r")) == NULL) {
        printf("Failed to open private key for reading\n");
        handleErrors();
    }
    if ((privkey = PEM_read_PrivateKey(privkeyfile, &privkey, NULL, NULL)) == NULL) {
        fclose(privkeyfile);
        handleErrors();
    }
    fclose(privkeyfile);
}

string RSAencrypt::encrypt(int flen, const unsigned char *from) {
	if(rsaenc == NULL) rsaenc = EVP_PKEY_get1_RSA(pubkey);
	unsigned char* to = (unsigned char*) malloc(sizeof(unsigned char)*RSA_size(rsaenc));
	int clen = RSA_public_encrypt(flen,from,to,rsaenc,RSA_PKCS1_OAEP_PADDING);
	string s(clen,'\0');
	REP(i,0,clen-1)s[i] = to[i];
	free(to);
	return s;
}


string RSAencrypt::decrypt(int flen, const unsigned char *from) {
	if(rsadec == NULL) rsadec = EVP_PKEY_get1_RSA(privkey);
	unsigned char* to = (unsigned char*) malloc(sizeof(unsigned char)*RSA_size(rsadec));
	int plen = RSA_private_decrypt(flen,from,to,rsadec,RSA_PKCS1_OAEP_PADDING);

	string s(plen,'\0');
	REP(i,0,plen-1)s[i] = to[i];

	free(to);
	return s;
}

string RSAencrypt::encryptNOPAD(int flen, const unsigned char *from) {
    if(rsaenc == NULL) rsaenc = EVP_PKEY_get1_RSA(pubkey);
    if(RSA_size(rsaenc) != flen) {
        cout<<"Size should match to use with nopadding mode\n";
        exit(1);
    }
    unsigned char* to = (unsigned char*) malloc(sizeof(unsigned char)*RSA_size(rsaenc));
    int clen = RSA_public_encrypt(flen,from,to,rsaenc,RSA_NO_PADDING);

    string s(clen,'\0');
    REP(i,0,clen-1)s[i] = to[i];
    free(to);
    return s;
}

string RSAencrypt::decryptNOPAD(int flen, const unsigned char *from) {
    if(rsadec == NULL) rsadec = EVP_PKEY_get1_RSA(privkey);
    if(RSA_size(rsadec) != flen) {
        cout<<"Size should match to use with nopadding mode\n";
        exit(1);
    }
    unsigned char* to = (unsigned char*) malloc(sizeof(unsigned char)*RSA_size(rsadec));
    int plen = RSA_private_decrypt(flen,from,to,rsadec,RSA_NO_PADDING);

    string s(plen,'\0');
    REP(i,0,plen-1)s[i] = to[i];

    free(to);
    return s;
}
string RSAencrypt::encrypt(int flen, const unsigned char *from, BIGNUM* mod) {

    if(rsaenc == NULL) rsaenc = EVP_PKEY_get1_RSA(pubkey);

    unsigned char* to = (unsigned char*) malloc(sizeof(unsigned char)*RSA_size(rsaenc));
    int clen;
    int count = 0;
    BIGNUM* f = BN_new();
    while(1) {
        clen = RSA_public_encrypt(flen,from,to,rsaenc,RSA_PKCS1_OAEP_PADDING);
        if(BN_bin2bn(to,clen,f) == NULL) handleErrors();
        count++;
        if(BN_ucmp(f, mod) >= 0) {
            continue;
        }
        else {
            break;
        }
    }
    if(count!=1)cout<<count<<"\t";
    BN_free(f);

    string s(clen,'\0');
    REP(i,0,clen-1)s[i] = to[i];
    free(to);
    return s;
}
string RSAencrypt::encryptNOPAD(int flen, const unsigned char *from, BIGNUM* mod) {
    if(rsaenc == NULL) rsaenc = EVP_PKEY_get1_RSA(pubkey);
    if(RSA_size(rsaenc) != flen) {
        cout<<"Size should match to use with nopadding mode\n";
        exit(1);
    }

    unsigned char* to = (unsigned char*) malloc(sizeof(unsigned char)*RSA_size(rsaenc));
    int clen;
    int count = 0;
    BIGNUM* f = BN_new();
    while(1) {
        clen = RSA_public_encrypt(flen,from,to,rsaenc,RSA_NO_PADDING);
        if(BN_bin2bn(to,clen,f) == NULL) handleErrors();
        count++;
        if(BN_ucmp(f, mod) >= 0) {
            continue;
        }
        else {
            break;
        }
    }
    if(count!=1)cout<<count<<"::";
    BN_free(f);

    string s(clen,'\0');
    REP(i,0,clen-1)s[i] = to[i];
    free(to);
    return s;
}

int RSAencrypt::getencsize() {
	if(rsaenc == NULL) rsaenc = EVP_PKEY_get1_RSA(pubkey);
	return RSA_size(rsaenc);
}
void RSAencrypt::envelope_seal(uchar* plaintext, int plaintext_len, string &ctxt)
{
		unsigned char *encrypted_key;
    int encrypted_key_len;
    unsigned char *iv;
    int iv_len;
    unsigned char *ciphertext;
    int ciphertext_len;

    envseal.starttime();
    int len;
    const EVP_CIPHER *type = EVP_aes_256_cbc();
    unsigned char *tmpiv = NULL, *tmpenc_key = NULL, *tmpctxt = NULL;

    iv_len = EVP_CIPHER_iv_length(type);
    if ((tmpiv = (unsigned char*)malloc(iv_len)) == NULL) handleErrors();

    if ((tmpenc_key = (unsigned char*)malloc(EVP_PKEY_size(pubkey))) == NULL) handleErrors();

    if ((tmpctxt = (unsigned char*)malloc(plaintext_len + EVP_CIPHER_block_size(type))) == NULL)
        handleErrors();

    if(EVP_SealInit(ctx, type, &tmpenc_key, &encrypted_key_len, tmpiv, &pubkey,1) != 1)
        handleErrors();

    if(EVP_SealUpdate(ctx, tmpctxt, &len, plaintext, plaintext_len) != 1)
        handleErrors();
    ciphertext_len = len;

    if(EVP_SealFinal(ctx, tmpctxt + len, &len) != 1)
        handleErrors();
    ciphertext_len += len;

    iv = tmpiv;
    encrypted_key = tmpenc_key;
    ciphertext = tmpctxt;

    tmpiv = NULL;
    tmpenc_key = NULL;
    tmpctxt = NULL;

    {
    	stringstream ss;
    	uchar* temp = tochar(ciphertext_len,4);
    	ss.write((char*)temp,4);free(temp);

    	temp = tochar(iv_len,4);
    	ss.write((char*)temp,4);free(temp);

    	temp = tochar(encrypted_key_len,4);
    	ss.write((char*)temp,4);free(temp);

			ss.write((char*)ciphertext,ciphertext_len);
			ss.write((char*)iv,iv_len);
			ss.write((char*)encrypted_key,encrypted_key_len);
			ctxt = ss.str();
    }
    envseal.donetime();
    free(ciphertext);
    free(iv);
    free(encrypted_key);
}
void RSAencrypt::envelope_open(string &ctxt, string &ptext)
{
	unsigned char *ciphertext;
	int ciphertext_len;
	unsigned char *encrypted_key, *iv, *plaintext;
	int encrypted_key_len;
	int plaintext_len;

	int bytesconsumed, iv_len;
	const char* cdata = ctxt.c_str();
	ciphertext_len = toInt((uchar*)cdata);
	iv_len = toInt((uchar*)(cdata+4));
	encrypted_key_len = toInt((uchar*)(cdata+8));
	bytesconsumed = 12;

  ciphertext = (uchar*)(cdata+bytesconsumed);
  iv = (uchar*)(cdata+bytesconsumed+ciphertext_len);
  encrypted_key = (uchar*)(cdata+bytesconsumed+ciphertext_len+iv_len);

  int len;
  unsigned char *tmpptxt = NULL;

  if ((tmpptxt = (unsigned char*)malloc(ciphertext_len)) == NULL) handleErrors();

  envopen.starttime();
  if(EVP_OpenInit(ctx, EVP_aes_256_cbc(), encrypted_key, encrypted_key_len, iv, privkey) != 1)
      return;

  envopen.donetime();
  if(EVP_OpenUpdate(ctx, tmpptxt, &len, ciphertext, ciphertext_len) != 1)
      return;
  plaintext_len = len;

  if(EVP_OpenFinal(ctx, tmpptxt + len, &len) != 1)
      return;
  plaintext_len += len;

  plaintext = tmpptxt;

  tmpptxt = NULL;
  ptext.resize(plaintext_len);

  REP(i,0,plaintext_len-1) {
  	ptext[i] = plaintext[i];
  }
  free(plaintext);
}
BIGNUM* RSAencrypt::retmod() {
    if(rsaenc == NULL) rsaenc = EVP_PKEY_get1_RSA(pubkey);
    return (rsaenc->n);
}
RSAencrypt::~RSAencrypt() {
    EVP_CIPHER_CTX_free(ctx);
    EVP_PKEY_free(pubkey);
    EVP_PKEY_free(privkey);
    RSA_free(rsaenc);
    RSA_free(rsadec);
}
