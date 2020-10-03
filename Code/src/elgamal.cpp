
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
#include "elgamal.h"
elgamal::elgamal() {
  ctxbn = BN_CTX_new(); if(!ctxbn) handleErrors();
}
elgamal::elgamal(uint32_t s, uint32_t m) :
  size(s), maxmsgspace(m)
{
  ctxbn = BN_CTX_new(); if(!ctxbn) handleErrors();
}
elgamal::~elgamal() {
  BN_free(p);
  BN_free(q);
  BN_free(g);
  BN_CTX_free(ctxbn);
}
void elgamal::groupgen() {
  DSA* dsa;
  if(NULL == (dsa = DSA_new())) handleErrors();
  if(1 != DSA_generate_parameters_ex(dsa,size,NULL,0,NULL,NULL,NULL)) handleErrors();
  p = BN_dup(dsa->p);
  q = BN_dup(dsa->q);
  g = BN_dup(dsa->g);
}

void elgamal::setgroup(BIGNUM* a,BIGNUM* b, BIGNUM* c, uint32_t newsize, uint32_t newmaxspace) {
  p = a; q = b; g = c;
  size = newsize;
  maxmsgspace = newmaxspace;
}

void elgamal::storegroup(string name) {
  FILE* fp;
  string fname = "./src/keys/" + name;
  fp = fopen(fname.c_str(),"w");
  fprintf(fp,"%u\n",size);
  fprintf(fp,"%u\n",maxmsgspace);
  char *strp = BN_bn2hex(p);
  char *strq = BN_bn2hex(q);
  char *strg = BN_bn2hex(g);
  fprintf(fp, "%s\n",strp);
  fprintf(fp, "%s\n",strq);
  fprintf(fp, "%s\n",strg);
  fclose(fp);
}

void elgamal::setgroup(string name) {
  ifstream fp;
  string fname = "./src/keys/" + name;
  fp.open(fname,ios::in);
  p = BN_new();q = BN_new();g = BN_new();
  string strp,strq,strg;
  fp >> size;
  fp >> maxmsgspace;
  fp >> strp >> strq >> strg;
  if(0==BN_hex2bn(&p, strp.c_str())) handleErrors();
  if(0==BN_hex2bn(&q, strq.c_str())) handleErrors();
  if(0==BN_hex2bn(&g, strg.c_str())) handleErrors();
  fp.close();
}

BIGNUM* elgamal::randomnogen() {
  int len = size/8;
  uchar* key = (uchar*)malloc(sizeof(uchar)*len);
  int rc = RAND_bytes(key, len);
  if(rc != 1) exit(1);
  BN_CTX *ctxbn;
  BIGNUM* bn = BN_new();
  BIGNUM *a = BN_bin2bn(key, len, NULL); if(!a) handleErrors();
  if(BN_mod(bn, a, q, ctxbn)!=1) handleErrors();
  BN_free(a);
  return bn;
}

void elgamal::keygen() {
  s = randomnogen();
  h = BN_new();
  if(1 != BN_mod_exp(h, g, s, p, ctxbn)) handleErrors();
}
void elgamal::storesecret(string name) {
  FILE* fp;
  string fname = "./src/keys/" + name;
  fp = fopen(fname.c_str(),"w");
  char *strs = BN_bn2hex(s);
  char *strh = BN_bn2hex(h);
  fprintf(fp, "%s\n",strs);
  fprintf(fp, "%s\n",strh);
  fclose(fp);
}
void elgamal::setsecret(string name) {
  ifstream fp;
  string fname = "./src/keys/" + name;
  fp.open(fname,ios::in);
  s = BN_new();h = BN_new();
  string strs,strh;
  fp >> strs >> strh;
  if(0==BN_hex2bn(&s, strs.c_str())) handleErrors();
  if(0==BN_hex2bn(&h, strh.c_str())) handleErrors();
  fp.close();
}

void elgamal::encrypt(BIGNUM* m, BIGNUM** x, BIGNUM** y) {
  // BIGNUM* m = BN_new();
  // if(0 == BN_dec2bn(&m, message_str.c_str())) handleErrors();
  BIGNUM* alpha = randomnogen();
  if(1 != BN_mod_exp(*x,g,alpha,p,ctxbn)) handleErrors();

  BIGNUM* y1 = BN_new();
  if(1 != BN_mod_exp(y1,h,alpha,p,ctxbn)) handleErrors();

  BIGNUM* y2 = BN_new();
  if(1 != BN_mod_exp(y2,g,m,p,ctxbn)) handleErrors();

  if(1 != BN_mod_mul(*y, y1, y2, p, ctxbn)) handleErrors();
  BN_free(y1);
  BN_free(y2);
}
int elgamal::decrypt(BIGNUM* x,BIGNUM* y) {
  BIGNUM* x_s = BN_new();
  if(1 != BN_mod_exp(x_s,x,s,p,ctxbn)) handleErrors();
  BIGNUM *x__s = BN_mod_inverse(NULL, x_s, p, ctxbn);
  BIGNUM* g_m = BN_new();
  if(1 != BN_mod_mul(g_m, y, x__s, p, ctxbn)) handleErrors();
  int i;
  for(i = 0; i <= maxmsgspace; i++) {
    string message_str = to_string(i);
    BIGNUM* m = BN_new();
    if(0 == BN_dec2bn(&m, message_str.c_str())) handleErrors();

    BIGNUM* y2 = BN_new();
    if(1 != BN_mod_exp(y2,g,m,p,ctxbn)) handleErrors();
    if(1!= BN_sub(y2, y2, g_m)) handleErrors();
    BN_print_fp(stdout,y2);

    if(BN_is_zero(y2)) {BN_free(y2);BN_free(m);break;}
    BN_free(y2);
    BN_free(m);
  }
  BN_free(g_m);
  BN_free(x__s);
  BN_free(x_s);
  return i;
}
