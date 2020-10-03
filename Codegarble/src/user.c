
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
#include "../include/user.h"
#include "../include/garble.h"
#include "../include/common.h"
#include "../include/circuits.h"
#include "../include/gates.h"
#include "../include/util.h"
#include "../include/justGarble.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"
#include "time.h"
	
int checkfn(int *a, int *outputs, int n) {
	outputs[0] = a[0];
	int i = 0;
	for (i = 0; i < n - 1; i++) {
		if (i % 3 == 2)
			outputs[0] = outputs[0] | a[i + 1];
		if (i % 3 == 1)
			outputs[0] = outputs[0] & a[i + 1];
		if (i % 3 == 0)
			outputs[0] = outputs[0] ^ a[i + 1];
	}
	return outputs[0];
}
int checkadd(int *a, int *outputs, int n) {
    int i;
    int tempOut[3];
    int split = n / 2;
    int tempIn[3];

    tempIn[0] = a[0];
    tempIn[1] = a[split];

    tempOut[0] = tempIn[0] ^ tempIn[1];
    tempOut[1] = tempIn[0] & tempIn[1];

    outputs[0] = tempOut[0];

    for (i = 1; i < split; i++) {
            tempIn[2] = tempOut[1];
            tempIn[1] = a[split + i];
            tempIn[0] = a[i];
            int wire1 = tempIn[0] ^ tempIn[2];
            int wire2 = tempIn[0] ^ tempIn[1];
            int wire3 = wire2 ^ tempIn[2];
            int wire4 = wire1 & wire2;
            int wire5 = wire4 ^ tempIn[0];
            tempOut[0] = wire3;
            tempOut[1] = wire5;
            outputs[i] = tempOut[0];
    }

    return outputs[0];
}
int checkandB_k(int *a, int *outputs, int n, int k) {
	// int k = khere;
	int i;
	int S_k = 1 << k;
	for(i =0;i<n;i++) {
		int j = i & ( (S_k<<1) - 1);
		if(j < S_k) outputs[i] = a[i];
		else {
			outputs[i] = 0;
		}
	}
	return 0;
}
int checkxor(int *a, int *outputs, int n) {
	int i;
    int split = n / 2;
	for(i =0;i<split;i++) {
		// printf("a[i] is %d::a[split+i] is %d\n",a[i],a[split+i]);
		outputs[i] = a[i] ^ a[split+i];
	}
	return 0;
}
int checkshift(int *a, int *outputs, int n) {
	outputs[n-1] = 0;
	memcpy(outputs, a + 1, sizeof(int) * (n - 1));
}

int checkshift_k(int *a, int *outputs, int n, int k) {
	int i;
	for(i = 0; i < k; i ++) 
		outputs[n-1-k] = 0;
	memcpy(outputs, a + k, sizeof(int) * (n - k));
}
int checknot(int *a, int *outputs, int n) {
	int i;
	for(i=0;i<n;i++) {
		outputs[i] = !a[i];	
	}
	return 0;
}
int checkinc(int *a, int *outputs, int n) {
	outputs[0] = !a[0];
	int carry = a[0];
	int newCarry;
	int i;
	for (i = 1; i < n; i++) {
		outputs[i] = carry ^ a[i];
		newCarry = carry & a[i];
		carry = newCarry;
	}
	return 0;
}
int checksub(int *a, int *outputs, int n) {
	int split = n/2;

	int* tempWires2 = (int*)malloc(sizeof(int)*n);
	int* tempWires = (int*)malloc(sizeof(int)*split);

	// int tempWires[split];
	// int tempWires2[n];

	checknot(a+split,tempWires,split);
	checkinc(tempWires,tempWires2+split,split);
	memcpy(tempWires2, a, sizeof(int) * split);
	checkadd(tempWires2,outputs,n);
	free(tempWires2);
	free(tempWires);
	return 0;
}
int checksetzero(int bit, int* outputs, int n) {
	outputs[0] = bit;
	int i;
	for(i = 1; i < n; i++) {
		outputs[i] = 0;
	}
}
int checkaddN1(int* a, int *outputs, int n, int bit) {
	int i;
    // tempOut[0] = tempIn[0] ^ tempIn[1];
    // tempOut[1] = tempIn[0] & tempIn[1];

    outputs[0] = a[0] ^ bit;
    int carry = a[0] & bit;
    int newCarry;

	for (i = 1; i < n; i++) {
		outputs[i] = carry ^ a[i];
		newCarry = carry & a[i];
		carry = newCarry;
	}
	return 0;
}
void print(int*outputs, int n) {
	int j;
	printf("Printing in middle\t");
	// for (j = n-1; j >= 0; j--) {
	// 	printf("%d ",outputs[j]);
	// }

	for (j = 0; j < n; j++) {
		printf("%d ",outputs[j]);
	}
	printf("\n");
}
int checkham2(int *a, int *outputs, int n) {
	if(n==2) {
		checkxor(a,outputs,n);
		return 0;
    }

    int i;
	int split = n/2;
	int m = ceil(log2(split+1));
	

	int* tempone = (int*)malloc(sizeof(int)*split);


	int* tempthree = (int*)malloc(sizeof(int)*m);

	// int tempone[n];
	checkxor(a,tempone,n);
	
	// print(tempone,split);
	
	int* temptwo = (int*)malloc(sizeof(int)*m);

	checksetzero(tempone[0],temptwo,m);
	// print(temptwo,m);
	for(i = 1; i < split; i++) {
		// printf("Adding i %d\t",i);
		checkaddN1(temptwo,tempthree,m,tempone[i]);
		// print(tempthree,m);
		int *swap = tempthree;
		tempthree = temptwo;
		temptwo = swap;
	}
	memcpy(outputs,temptwo,sizeof(int)*m);

	free(tempone);
	free(temptwo);
	free(tempthree);
	return 0;
}

int checkham(int *a, int *outputs, int n) {
	if(n==2) {
		checkxor(a,outputs,n);
		return 0;
    }

    
    int i;
	int split = n/2;
	int lenk = ceil(log(split));
	// printf("%d\n",lenk);
	

	int* tempone = (int*)malloc(sizeof(int)*n);

	int* temptwo = (int*)malloc(sizeof(int)*split);

	int* tempthree = (int*)malloc(sizeof(int)*split);

	// int tempone[n];
	checkxor(a,tempone,n);
	
	print(tempone,split);
	
	// int temptwo[split];

	checkshift(tempone,temptwo,split);
	print(temptwo,split);

	// int tempthree[split];

	checkandB_k(temptwo,tempthree,split,0);
	print(tempthree,split);

	memcpy(tempone+split,tempthree,sizeof(int)*split);
	
	checksub(tempone,temptwo,n);
	print(temptwo,split);

	printf("Going into loop\n");

	for(i = 1; i < lenk; i++) {
		int S_k = 1 << i;
		checkshift_k(temptwo,tempthree,split,S_k);
		print(tempthree,split);
		checkandB_k(tempthree,tempone,split,i);
		print(tempone,split);
		checkandB_k(temptwo,tempthree,split,i);
		print(tempthree,split);

		memcpy(tempone+split,tempthree,sizeof(int)*split);
		checkadd(tempone,temptwo,n);
		print(temptwo,split);
	}
	for(i = 0; i < split; i++) {
		outputs[i] = temptwo[i];
	}
	free(tempone);
	free(temptwo);
	free(tempthree);
	return 0;
}
// void print128_num(__m128i var) {
//     uint16_t *val = (uint16_t*) &var;
//     printf("Numerical: %i %i %i %i %i %i %i %i \n", 
//            val[0], val[1], val[2], val[3], val[4], val[5], 
//            val[6], val[7]);
// }