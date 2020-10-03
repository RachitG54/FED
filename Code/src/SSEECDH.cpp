
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
#include "SSEECDH.h"

SSEECDH::SSEECDH() {
    ctxbn = BN_CTX_new(); if(!ctxbn) handleErrors();
}
void SSEECDH::setDH() {
  if(NULL == (curve = EC_GROUP_new_by_curve_name(NID_secp224r1))) handleErrors();
    EC_GROUP_set_point_conversion_form(curve, POINT_CONVERSION_UNCOMPRESSED);
    q = BN_new();
    EC_GROUP_get_order(curve, q, ctxbn);
    g = EC_POINT_dup(EC_GROUP_get0_generator(curve),curve);
}
uint32_t SSEECDH::getDHsize() {
	return 2*size+8;
}
uint32_t SSEECDH::getDHsubgroupsize() {
    return BN_num_bytes(q);
}

BIGNUM* SSEECDH::bin2bn(uchar *str, uint32_t len) {
    BIGNUM* bn = BN_bin2bn(str, len, NULL); if(!bn) handleErrors();
    return bn;
}
uchar* SSEECDH::bn2bin(BIGNUM* bn) {
    uint32_t len = BN_num_bytes(bn);
    uchar* str = (uchar*) malloc(sizeof(uchar)*len);
    if(0 == BN_bn2bin(bn, str)) handleErrors();
    return str;
}

uint32_t SSEECDH::num_bytes(BIGNUM* bn) {
    return BN_num_bytes(bn);
}

BIGNUM* SSEECDH::point2bn(EC_POINT *p) {
    BIGNUM* bn = EC_POINT_point2bn(curve, p, POINT_CONVERSION_UNCOMPRESSED, NULL,ctxbn);
    return bn;
}
EC_POINT* SSEECDH::bn2point(BIGNUM* bn) {
    EC_POINT *p = EC_POINT_bn2point(curve, bn, NULL, ctxbn);
    return p;
}

void SSEECDH::free(BIGNUM** bn) {
    BN_free(*bn);
}

void SSEECDH::free(EC_POINT** p) {
    EC_POINT_free(*p);
}
BIGNUM* SSEECDH::modq(BIGNUM* bn) {
    BIGNUM *bnmod = BN_new();
    if(BN_mod(bnmod, bn, q, ctxbn)!=1) handleErrors();
    return bnmod;
}

BIGNUM* SSEECDH::modmulq(BIGNUM* bn1, BIGNUM* bn2) {
    BIGNUM *bnmod = BN_new();
    if(1 != BN_mod_mul(bnmod, bn1, bn2, q, ctxbn)) handleErrors();
    return bnmod;
}

BIGNUM* SSEECDH::modinvq(BIGNUM* bn) {
    BIGNUM *bnmod = BN_mod_inverse(NULL, bn, q, ctxbn);
    return bnmod;
}

EC_POINT* SSEECDH::modexpp(EC_POINT* p, BIGNUM* bn) {
    EC_POINT* pnew = EC_POINT_new(curve);
    if(1!= EC_POINT_mul(curve, pnew, NULL, p, bn, ctxbn)) handleErrors();
    return pnew;
}
EC_POINT* SSEECDH::modexppgen(BIGNUM* bn) {
    EC_POINT* pnew = EC_POINT_new(curve);
    if(1!= EC_POINT_mul(curve, pnew, bn, NULL, NULL, ctxbn)) handleErrors();
    return pnew;
}

SSEECDH::~SSEECDH() {
    BN_free(q);
    EC_POINT_free(g);
    BN_CTX_free(ctxbn);         
    EC_GROUP_free(curve);
}


