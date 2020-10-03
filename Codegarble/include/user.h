
// Modified by : Rachit Garg Nishant Kumar
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
#ifndef USER_H_
#define USER_H_
int checkfn(int *a, int *outputs, int n);
int checkadd(int *a, int *outputs, int n);
int checkandB_k(int *a, int *outputs, int n, int k);
int checkxor(int *a, int *outputs, int n);
int checkshift(int *a, int *outputs, int n);
int checkshift_k(int *a, int *outputs, int n, int k);
int checknot(int *a, int *outputs, int n);
int checkinc(int *a, int *outputs, int n);
int checksub(int *a, int *outputs, int n);
int checksetzero(int bit, int* outputs, int n);
int checkaddN1(int* a, int *outputs, int n, int bit);
void print(int*outputs, int n);
int checkham2(int *a, int *outputs, int n);
int checkham(int *a, int *outputs, int n);
#endif /* USER_H_ */
