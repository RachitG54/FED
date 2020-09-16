/*
 This file is part of JustGarble.

    JustGarble is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    JustGarble is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with JustGarble.  If not, see <http://www.gnu.org/licenses/>.

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "../include/justGarble.h"
#include "../include/user.h"
#include "math.h"

void readparams() {
	#ifdef FREE_XOR
		printf("FREE_XOR is switched on!\n");
	#endif
	
	srand(time(NULL));
	GarbledCircuit garbledCircuit;
	GarblingContext garblingContext;

	readCircuitFromFile(&garbledCircuit, "SSE.scd");

	// int n = atoi(argv[1]) * 1024;
	// int m = 1;
	// int q = n;
	// int r = q+n;

	// //Actually garble the circuit created from the file.
    int n = garbledCircuit.n;
    int m = garbledCircuit.m;
    int q = garbledCircuit.q;
    int r = garbledCircuit.r;
    printf("%d %d %d %d\n",n,m,q,r);

}
int main(int argc, char **argv) {
	srand(time(NULL));
	GarbledCircuit garbledCircuit;
	GarblingContext garblingContext;

	int n = atoi(argv[1]);
	int split = n/2;
	int m = ceil(log2(split+1));
	int q = 7*n*log2(n);
	int r = 7*n*log2(n);
	printf("%d %d %d %d\n",n,m,q,r);
	// {

	// 	int n = atoi(argv[1]);
	// 	int split = n/2;
	// 	int m = ceil(log2(split+1));
	// 	// printf("m is %d\n",m);

	// 	int i, j;

	// 	int outputReal[m];
	// 	int inputs[n];

	// 	int count = 0;
	//     for (i = 0; i < 2; i++) {
	// 		printf("Test %d\n",i);
	// 		// long long val1 = 0;
	// 		for (j = 0; j < n/2; j++) {
	// 			inputs[j] = rand() % 2;
	// 			printf("%d ",inputs[j]);
	// 		}
	// 		// for (j = n/2-1; j >= 0; j--) {
	// 		// 	inputs[j] = rand() % 2;
	// 		// 	printf("%d ",inputs[j]);
	// 		// }
	// 		// printf("%lld ", val1);
	// 		// printf("\t");
	// 		// long long val2 = 0;
	// 		for (j = n/2; j < n; j++) {
	// 			inputs[j] = rand() % 2;
	// 			printf("%d ",inputs[j]);
	// 		}
	// 		// for (j = n-1; j >= n/2; j--) {
	// 		// 	inputs[j] = rand() % 2;
	// 		// 	printf("%d ",inputs[j]);
	// 		// }
	// 		// printf("%lld ", val2);
	// 		printf("\n");
	// 		// long long mod = 1<<m;
	// 		// printf("%lld %lld ", ansval,(val1+val2)%mod);
	// 		// printf("\n");

	// 		// checkham(inputs, outputReal, n);

	// 		checkham2(inputs, outputReal, n);
			
	// 		// for (j = m-1; j >= 0; j--) {
	// 		// 	printf("%d ",outputReal[j]);
	// 		// }

	// 		for (j = 0; j < m; j++) {
	// 			printf("%d ",outputReal[j]);
	// 		}
	// 		// long long ansval = 0;
	// 		// for (j = m-1; j >= 0; j--) {
	// 		// 	ansval = 2*ansval + outputVals[j];
	// 		// 	// printf("%lld  ",ansval);
	// 		// }
	// 		printf("\n");
	// 	}
	// 	printf("Total count is %d, failed count is %d\n",TIMES,count);
	// }
	// Setup input and output tokens/labels.
	block *labels = (block*) malloc(sizeof(block) * 2 * n);
	block *outputbs = (block*) malloc(sizeof(block) * 2 * m);
	int *inp = (int *) malloc(sizeof(int) * n);
	countToN(inp, n);
	int outputs[m];
	int i;

	OutputMap outputMap = outputbs;
	InputLabels inputLabels = labels;

	//Create a circuit.
	createInputLabels(labels, n);
	
	createEmptyGarbledCircuit(&garbledCircuit, n, m, q, r, inputLabels);
	startBuilding(&garbledCircuit, &garblingContext);
    HammingCircuit2(&garbledCircuit, &garblingContext, n, inp, outputs);
	// MIXEDCircuit(&garbledCircuit, &garblingContext, n, inp, outputs);
	finishBuilding(&garbledCircuit, &garblingContext, outputMap, outputs);

	// // Write the created circuit to a file
	// writeCircuitToFile(&garbledCircuit, "SSE.scd");

	// // //Actually garble the circuit created from the file.
	garbleCircuit(&garbledCircuit, inputLabels, outputMap);

	// FILE*fp;
	// fp = fopen("labels","w");
	// fprintf(fp, "%d\n16\n", n);
	// for(i = 0; i < 2*n; i++) {
	// 	unsigned char* labelchar = convert128_num(inputLabels[i]);
	// 	int j;
	// 	for(j = 0; j < 16;j++) {
	// 		fprintf(fp, "%c", labelchar[j]);
	// 	}
	// 	free(labelchar);
	// }
	// fclose(fp);

	// fp = fopen("omap","w");
	// fprintf(fp, "%d\n16\n", m);
	// for(i = 0; i < 2*m; i++) {
	// 	unsigned char* mapchar = convert128_num(outputMap[i]);
	// 	int j;
	// 	for(j = 0; j < 16;j++) {
	// 		fprintf(fp, "%c", mapchar[j]);
	// 	}
	// 	free(mapchar);
	// }
	// fclose(fp);

	// // for(i = 0; i < m; i++) {
	// // 	// convert128_num(outputMap[i]);
	// // }

	// fp = fopen("garbledtable","w");
	// unsigned char* key = convert128_num(garbledCircuit.globalKey);
	// fprintf(fp, "16\n%d\n16\n",garbledCircuit.q);
	// for(i = 0; i < 16;i++) {
	// 	fprintf(fp, "%c",key[i]);
	// }
	// free(key);

	// for(i=0;i<garbledCircuit.q;i++) {
	// 	GarbledTable temptable = garbledCircuit.garbledTable[i];
	// 	int j;
	// 	for(j = 0; j < 4; j++) {
	// 		unsigned char* tstr = convert128_num(temptable.table[j]);
	// 		int k;
	// 		for(k = 0; k < 16;k++) {
	// 			fprintf(fp, "%c", tstr[k]);
	// 		}
	// 		free(tstr);
	// 	}
	// }
	// fclose(fp);
	//Evaluate the circuit with random values and check the computed
	//values match the outputs of the desired function.
	checkCircuit(&garbledCircuit, inputLabels, outputMap, &(checkham2));
	printf("Completed circuit generation\n");
	readparams();
	return 0;

}

/*
srand(time(NULL));
        GarbledCircuit garbledCircuit;
        GarblingContext garblingContext;
        if (argc != 2) {
            printf("Usage: <prog_name> number_of_gates (in thousands)");
        }

        //Set up circuit parameters
        int n = atoi(argv[1]);
        int m = n/2;
        int q = 5*n;
        int r = 5*n;

        //Set up input and output tokens/labels.
        block *labels = (block*) malloc(sizeof(block) * 2 * n);
        block *outputbs = (block*) malloc(sizeof(block) * 2*m);
        int *inp = (int *) malloc(sizeof(int) * n);
        countToN(inp, n);
        int outputs[m];

        OutputMap outputMap = outputbs;
        InputLabels inputLabels = labels;

        //Actually build a circuit. Alternatively, this circuit could be read
        //from a file.
        createInputLabels(labels, n);
        createEmptyGarbledCircuit(&garbledCircuit, n, m, q, r, inputLabels);
        startBuilding(&garbledCircuit, &garblingContext);
        ADDCircuit(&garbledCircuit, &garblingContext, n, inp, outputs);
        finishBuilding(&garbledCircuit, &garblingContext, outputMap, outputs);

        //Garble the built circuit.
        garbleCircuit(&garbledCircuit, inputLabels, outputMap);

        //Evaluate the circuit with random values and check the computed
        //values match the outputs of the desired function.
        checkCircuit(&garbledCircuit, inputLabels, outputMap, &(checkfn));

        return 0;
*/