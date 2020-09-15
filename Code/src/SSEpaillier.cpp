#include "SSEpaillier.h"
SSEpaillier::SSEpaillier() = default;
SSEpaillier::SSEpaillier(long s) :
  size(s)
{
  cout<<"Size is "<<size<<"\n";
}

SSEpaillier::~SSEpaillier() {
  if(sumct!=NULL) paillier_freeciphertext(sumct);
  paillier_freepubkey(pubKey);
  if(secKey!=NULL)paillier_freeprvkey(secKey);
}

void SSEpaillier::keygen() {
  paillier_keygen(size, &pubKey, &secKey, paillier_get_rand_devurandom);

}
void SSEpaillier::storesecret(string name1, string name2) {
  string fname = "./src/keys/";
  std::fstream pubKeyFile(fname + name1, std::fstream::out|std::fstream::trunc);
  std::fstream secKeyFile(fname + name2, std::fstream::out|std::fstream::trunc);
  char* hexPubKey = paillier_pubkey_to_hex(pubKey);
  char* hexSecKey = paillier_prvkey_to_hex(secKey);
  pubKeyFile << hexPubKey;
  secKeyFile << hexSecKey;
  pubKeyFile.close();
  secKeyFile.close();

  free(hexPubKey);
  free(hexSecKey);
}
void SSEpaillier::setsecret(string name1, string name2) {
  string fname = "./src/keys/";
  std::fstream pubKeyFile(fname + name1, std::fstream::in);
  std::fstream secKeyFile(fname + name2, std::fstream::in);

  std::string hexPubKey;
  std::string hexSecKey;
  std::getline(pubKeyFile, hexPubKey);
  std::getline(secKeyFile, hexSecKey);

  pubKeyFile.close();
  secKeyFile.close();

  pubKey = paillier_pubkey_from_hex(&hexPubKey[0]);
  secKey = paillier_prvkey_from_hex(&hexSecKey[0], pubKey);
}

void SSEpaillier::setsecretpublic(string name1) {
  string fname = "./src/keys/";
  std::fstream pubKeyFile(fname + name1, std::fstream::in);

  std::string hexPubKey;
  std::getline(pubKeyFile, hexPubKey);

  pubKeyFile.close();

  pubKey = paillier_pubkey_from_hex(&hexPubKey[0]);
  secKey = NULL;
}

int SSEpaillier::getpublickeysize() {
  return PAILLIER_BITS_TO_BYTES(pubKey->bits);
}

void SSEpaillier::setsum() {
  sumct = paillier_create_enc_zero();
}

char* SSEpaillier::returnsum() {
  char* byteCtxt = (char*)paillier_ciphertext_to_bytes(getpublickeysize()*2, sumct);
  return byteCtxt;
}

char* SSEpaillier::encrypt(string message) {
  paillier_plaintext_t* m = paillier_plaintext_from_ui(std::atoi(message.c_str()));
  paillier_ciphertext_t* ctxt;
  ctxt = paillier_enc(NULL, pubKey, m, paillier_get_rand_devurandom);
  char* byteCtxt = (char*)paillier_ciphertext_to_bytes(getpublickeysize()*2, ctxt);

  paillier_freeplaintext(m);
  paillier_freeciphertext(ctxt);
  return byteCtxt;
}
char* SSEpaillier::encrypt(uint32_t message) {
  paillier_plaintext_t* m = paillier_plaintext_from_ui(message);
  paillier_ciphertext_t* ctxt;
  ctxt = paillier_enc(NULL, pubKey, m, paillier_get_rand_devurandom);
  char* byteCtxt = (char*)paillier_ciphertext_to_bytes(getpublickeysize()*2, ctxt);

  paillier_freeplaintext(m);
  paillier_freeciphertext(ctxt);
  return byteCtxt;
}
char* SSEpaillier::sum(char* byteCtxt1, char* byteCtxt2) {
  paillier_ciphertext_t* ctxt1 = paillier_ciphertext_from_bytes((void*)byteCtxt1, getpublickeysize()*2);
  paillier_ciphertext_t* ctxt2 = paillier_ciphertext_from_bytes((void*)byteCtxt2, getpublickeysize()*2);

  paillier_ciphertext_t* encryptedSum = paillier_create_enc_zero();

  paillier_mul(pubKey, encryptedSum, ctxt1, ctxt2);
  char* byteCtxt = (char*)paillier_ciphertext_to_bytes(getpublickeysize()*2, encryptedSum);

  paillier_freeciphertext(ctxt1);
  paillier_freeciphertext(ctxt2);
  paillier_freeciphertext(encryptedSum);
  return byteCtxt;
}

void SSEpaillier::add(uint32_t val) {
  paillier_plaintext_t* m = paillier_plaintext_from_ui(val);
  paillier_ciphertext_t* ctxt;
  ctxt = paillier_enc(NULL, pubKey, m, paillier_get_rand_devurandom);
  paillier_ciphertext_t* encryptedSum = paillier_create_enc_zero();
  paillier_mul(pubKey, encryptedSum, ctxt, sumct);
  paillier_ciphertext_t* temp = encryptedSum;
  encryptedSum = sumct; sumct = temp;
  paillier_freeciphertext(encryptedSum);
  paillier_freeciphertext(ctxt);
  paillier_freeplaintext(m);
}

void SSEpaillier::add(char* byteCtxt) {
  paillier_ciphertext_t* ctxt = paillier_ciphertext_from_bytes((void*)byteCtxt, getpublickeysize()*2);
  paillier_ciphertext_t* encryptedSum = paillier_create_enc_zero();
  paillier_mul(pubKey, encryptedSum, ctxt, sumct);
  paillier_ciphertext_t* temp = encryptedSum;
  encryptedSum = sumct; sumct = temp;
  paillier_freeciphertext(encryptedSum);
  paillier_freeciphertext(ctxt);
}

char* SSEpaillier::decrypt(char* byteCtxt) {
  paillier_ciphertext_t* ctxt = paillier_ciphertext_from_bytes((void*)byteCtxt, getpublickeysize()*2);
  paillier_plaintext_t* dec;
  dec = paillier_dec(NULL, pubKey, secKey, ctxt);
  char* bytePtxt = (char*)malloc(sizeof(char)*getpublickeysize()*2);
  gmp_sprintf(bytePtxt,"%Zd", dec);
  paillier_freeplaintext(dec);
  paillier_freeciphertext(ctxt);
  return bytePtxt;
}
