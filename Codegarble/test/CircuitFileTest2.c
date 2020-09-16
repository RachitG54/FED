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

int main(int argc, char **argv) {
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
	int i;

	//Setup input and output tokens/labels.
	// block *labels = (block*) malloc(sizeof(block) * 2 * n);
	// block *outputbs = (block*) malloc(sizeof(block) * 2 * m);
	// int *inp = (int *) malloc(sizeof(int) * n);
	// countToN(inp, n);
	// int outputs[m];
	// int i;

	// OutputMap outputMap = outputbs;
	// InputLabels inputLabels = labels;

	// //Create a circuit.
	// createInputLabels(labels, n);
	
	// createEmptyGarbledCircuit(&garbledCircuit, n, m, q, r, inputLabels);
	// startBuilding(&garbledCircuit, &garblingContext);
 //    ADDCircuit(&garbledCircuit, &garblingContext, n, inp, outputs);
	// // MIXEDCircuit(&garbledCircuit, &garblingContext, n, inp, outputs);
	// finishBuilding(&garbledCircuit, &garblingContext, outputMap, outputs);


	
	
	FILE*fp;

	fp = fopen("labels","r");
	int nlabel,labelsize;
	fscanf(fp,"%d\n%d\n",&nlabel,&labelsize);

    block *labels = (block*) malloc(sizeof(block) * 2 * nlabel);
	for(i = 0; i < 2*nlabel; i++) {	
		unsigned char labelchar[labelsize];
		int j;
		for(j = 0; j < labelsize;j++) {
			char tempchar;
			fscanf(fp, "%c", &tempchar);
			labelchar[j] = (unsigned char)tempchar;
		}
		memcpy(&labels[i],labelchar,labelsize);
	}
	fclose(fp);

	InputLabels inputLabels = labels;

	int correctnlabel;
	int correctlabelsize;
	fp = fopen("correctlabels","r");
	fscanf(fp,"%d\n%d\n",&correctnlabel,&correctlabelsize);

    block *labelscorrect = (block*) malloc(sizeof(block) * correctnlabel);
	for(i = 0; i < correctnlabel; i++) {	
		unsigned char labelchar[correctlabelsize];
		int j;
		for(j = 0; j < correctlabelsize;j++) {
			char tempchar;
			fscanf(fp, "%c", &tempchar);
			labelchar[j] = (unsigned char)tempchar;
		}
		memcpy(&labelscorrect[i],labelchar,correctlabelsize);
		if (memcmp(&labelscorrect[i],&labels[2*i],16) == 0) {
			printf("0::");
		}	
		else if (memcmp(&labelscorrect[i],&labels[2*i+1],16) == 0) {
			printf("1::");
		}
	}
	printf("\n");

	fclose(fp);
	// for(i=0;i<n;i++) {
	// 	labelscorrect[i] = labels[2*i+1];
	// }

	int mmap,mapsize;
	fp = fopen("omap","r");
	fscanf(fp,"%d\n%d\n",&mmap,&mapsize);
	block *outputbs = (block*) malloc(sizeof(block) * 2 * mmap);
	for(i = 0; i < 2*mmap; i++) {
		unsigned char mapchar[mapsize];
		int j;
		for(j = 0; j < mapsize;j++) {
			char tempchar;
			fscanf(fp, "%c", &tempchar);
			mapchar[j] = (unsigned char)tempchar;
		}
		memcpy(&outputbs[i],mapchar,mapsize);
	}
	fclose(fp);

	OutputMap outputMap = outputbs;
	
	int keysize,qsize,tablecol;
	fp = fopen("garbledkey","r");
	fscanf(fp,"%d\n%d\n%d\n",&keysize,&qsize,&tablecol);
	unsigned char keychar[keysize];
	for(i = 0; i < keysize;i++) {
		char tempchar;
		fscanf(fp, "%c", &tempchar);
		keychar[i] = (unsigned char)tempchar;
	}
	memcpy(&garbledCircuit.globalKey,keychar,keysize);
	
	fclose(fp);	
	fp = fopen("garbledtable","r");
	for(i=0;i<garbledCircuit.q;i++) {
		int j;
		for(j = 0; j < 4; j++) {
			unsigned char tablechar[tablecol];
			int k;
			for(k = 0; k < tablecol;k++) {
				char tempchar;
				fscanf(fp, "%c", &tempchar);
				tablechar[k] = (unsigned char)tempchar;
				// int tempint;
				// fscanf(fp, "%d ", &tempint);
				// tablechar[k] = (unsigned char)tempint;
			}
			// if(i==87) printf("Here\n");
			block instr; 
			memcpy(&instr,tablechar,tablecol);
			garbledCircuit.garbledTable[i].table[j] = instr;
		} 
	}
	fclose(fp);

	// garbleCircuit(&garbledCircuit, inputLabels, outputMap);
	// checkCircuit(&garbledCircuit, inputLabels, outputMap, &(checkfn2));
	
	ExtractedLabels extractedLabels = labelscorrect;
	block computedOutputMap[mmap];
	// printf("mmap is %d\n\n\n",mmap);
	evaluate(&garbledCircuit, extractedLabels, computedOutputMap);
	int outputVals[mmap];
	mapOutputs(outputMap, computedOutputMap, outputVals, mmap);

	// long long ansval = 0;
	for (i = m-1; i >= 0; i--) {
		// ansval = 2*ansval + outputVals[i];
		printf("%d",outputVals[i]);
		// printf("%lld  ",ansval);
	}
	printf("\n");
	// printf("%lld\n", ansval);

	// for(i = 0; i < m; i++) {
	// 	// convert128_num(outputMap[i]);
	// }

	// Write the created circuit to a file
	// writeCircuitToFile(&garbledCircuit, "SSE.scd");

	// Create an empty garbled circuit data structure
	// GarbledCircuit garbledCircuit2;

	// //Read a circuit from a file and initialize the garbled circuit data
	// //structure with it. Note that this step does not perform actual
	// //garbling. It just creates empty garbled tables and space for wires and gates.
	// readCircuitFromFile(&garbledCircuit2, "SSE.scd");

	// //Actually garble the circuit created from the file.
	// garbleCircuit(&garbledCircuit2, inputLabels, outputMap);

	// writeCircuitToFile(&garbledCircuit2, "tst2.scd");

	//Create an empty garbled circuit data structure
	// GarbledCircuit garbledCircuit3;
	// GarblingContext garblingContext3;

	// block *outputbs3 = (block*) malloc(sizeof(block) * 2 * m);

	// OutputMap outputMap3 = outputbs3;
	// // readCircuitFromFile(&garbledCircuit3, "tst2.scd");

	// // garbleCircuit(&garbledCircuit3, inputLabels, outputMap);

	// createEmptyGarbledCircuit(&garbledCircuit3, n, m, q, r, inputLabels);
	// startBuilding(&garbledCircuit3, &garblingContext3);	
	// // MIXEDCircuit(&garbledCircuit3, &garblingContext3, n, inp, outputs);
	// finishBuilding(&garbledCircuit3, &garblingContext3, outputMap3, outputs);

	// printf("%d\n",outputs[0]);


	// garbleCircuit(&garbledCircuit3, inputLabels, outputMap3);

	// for(i = 0; i < 2*m; i++) {
	// 	convert128_num(outputMap[i]);
	// 	convert128_num(outputMap3[i]);
	// }
	// for(i=0;i<)
	// garbledCircuit3.garbledTable = garbledCircuit2.garbledTable;

	//Evaluate the circuit with random values and check the computed
	//values match the outputs of the desired function.
	// checkCircuit(&garbledCircuit2, inputLabels, outputMap, &(checksub));

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