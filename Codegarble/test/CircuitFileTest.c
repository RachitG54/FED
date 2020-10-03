
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
#include "time.h"

int main(int argc, char **argv) {
	srand(time(NULL));
	GarbledCircuit garbledCircuit;
	GarblingContext garblingContext;
	// int n = atoi(argv[1]);
	// int m = 1;
	// int q = n;
	// int r = q+n;

    // int n = atoi(argv[1]);
    // int m = n/2;
    // int q = 5*n;
    // int r = 5*n;

	// int n = atoi(argv[1]);
	// int m = n/2;
	// int q = 7*n*log2(n);
	// int r = 7*n*log2(n);


	// int Tsize = atoi(argv[2]);
	// int colbits = atoi(argv[3]);
	// int n = atoi(argv[1]);

	// int noofbits = atoi(argv[4]); 
	// int noofcols = colbits/noofbits;

	// int m = Tsize*ceil(log2(colbits+1));
	// // int m = ceil(log2(2*noofcols*Tsize+1));
	// //m = n;
	// // int m = split;
	// int q = 7*n*log2(n);
	// int r = 7*n*log2(n);

	int Tsize = atoi(argv[2]);
	int colbits = atoi(argv[3]);
	int n = atoi(argv[1]);

	int noofbits = atoi(argv[4]); 
	int noofcols = colbits/noofbits;

	// int m = Tsize*ceil(log2(colbits+1));
	int m = ceil(log2(2*noofcols*Tsize+1));
	//m = n;
	// int m = split;
	int q = 7*n*log2(n);
	int r = 7*n*log2(n);
	
	// int n = atoi(argv[1]);
	// int split = n/2;
	// int m = ceil(log2(split+1));
	// m = n;
	// // int m = split;
	// int q = 7*n*log2(n);
	// int r = 7*n*log2(n);

	clock_t begin = clock();
	//Setup input and output tokens/labels.
	block *labels = (block*) malloc(sizeof(block) * 2 * n);
	block *outputbs = (block*) malloc(sizeof(block) * 2 * m);
	int *inp = (int *) malloc(sizeof(int) * n);
	countToN(inp, n);
	int outputs[m];
	int i;


	//Create a circuit.
	for(i=0;i<2*n;i++) {
		unsigned char labelchar[16];
		int j;
		for(j = 0; j < 16;j++) {
			int t;
			t = rand()%256;
			labelchar[j] = (unsigned char)t;
		}
		memcpy(&labels[i],labelchar,16);
	}
	// createInputLabels(labels, n);

	// OutputMap outputMap = outputbs;
	// InputLabels inputLabels = labels;

	
	createEmptyGarbledCircuit(&garbledCircuit, n, m, q, r, labels);
	startBuilding(&garbledCircuit, &garblingContext);
    GenomeCircuit2(&garbledCircuit, &garblingContext, n, colbits,Tsize, noofcols, inp, outputs);
    // GenomeCircuit(&garbledCircuit, &garblingContext, n, colbits,inp, outputs);
	// MIXEDCircuit(&garbledCircuit, &garblingContext, n, inp, outputs);
	finishBuilding(&garbledCircuit, &garblingContext, outputbs, outputs);

	// Write the created circuit to a file
	// writeCircuitToFile(&garbledCircuit, "SSE.scd");

	// //Actually garble the circuit created from the file.
	garbleCircuit(&garbledCircuit, labels, outputbs);
	clock_t end = clock();
	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("[Final Hamm]: Circuit Generation time is %lf\nBeginning to write\n",time_spent);

	FILE*fp;
	fp = fopen("labels","w");
	// fprintf(fp, "%d\n16\n", n);
	for(i = 0; i < 2*n; i++) {
		unsigned char* labelchar = convert128_num(labels[i]);
		int j;
		for(j = 0; j < 16;j++) {
			fprintf(fp, "%c", labelchar[j]);
		}
		free(labelchar);
	}
	fclose(fp);
	long long no = 2*n*16;
	printf("Size of labels is %lld\n",no);

	fp = fopen("omap","w");
	// fprintf(fp, "%d\n16\n", m);
	for(i = 0; i < 2*m; i++) {
		unsigned char* mapchar = convert128_num(outputbs[i]);
		int j;
		for(j = 0; j < 16;j++) {
			fprintf(fp, "%c", mapchar[j]);
		}
		free(mapchar);
	}
	fclose(fp);
	no = 2*m*16;
	printf("Size of output map is %lld\n",no);

	// for(i = 0; i < m; i++) {
	// 	// convert128_num(outputMap[i]);
	// }

	fp = fopen("garbledkey","w");
	unsigned char* key = convert128_num(garbledCircuit.globalKey);
	// fprintf(fp, "16\n%d\n16\n",garbledCircuit.q);
	for(i = 0; i < 16;i++) {
		fprintf(fp, "%c",key[i]);
	}
	free(key);

	fclose(fp);

	fp = fopen("garbledtable","w");
	for(i=0;i<garbledCircuit.q;i++) {
		// GarbledTable temptable = garbledCircuit.garbledTable[i];
		int j;
		for(j = 0; j < 4; j++) {
			unsigned char* tstr = convert128_num(garbledCircuit.garbledTable[i].table[j]);
			int k;
			for(k = 0; k < 16;k++) {
				fprintf(fp, "%c", tstr[k]);
			}
			free(tstr);
		}
	}
	fclose(fp);
	no = ((long long)garbledCircuit.q)*4*16;
	printf("[Final Hamm] : Size of garbled table is %lld, Number of gates =%d\n",no, garbledCircuit.q);

	// {
	// 	for(i=0;i<garbledCircuit.q;i++) {
	// 		// GarbledTable temptable = garbledCircuit.garbledTable[i];
	// 		int j;
	// 		for(j = 0; j < 4; j++) {
	// 			unsigned char* tstr = convert128_num(garbledCircuit.garbledTable[i].table[j]);
	// 			int k;
	// 			for(k = 0; k < 16;k++) {
	// 				fprintf(stdout, "%d ", (int)tstr[k]);
	// 			}
	// 			free(tstr);
	// 		}
	// 		fprintf(stdout, "%d:: ",i);
	// 	}
	// 	fprintf(stdout, "\n");
	// }
	printf("Done printing here\n");
	//Evaluate the circuit with random values and check the computed
	//values match the outputs of the desired function.

	// garbleCircuit(&garbledCircuit, inputLabels, outputMap);
	// checkCircuit(&garbledCircuit, inputLabels, outputMap, &(checkinc));
	printf("Completed circuit generation\n");

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
