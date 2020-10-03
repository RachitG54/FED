
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
#include <math.h>
#include "../include/justGarble.h"
#include "../include/user.h"

int main(int argc, char **argv) {
	srand(time(NULL));
	GarbledCircuit garbledCircuit;
	GarblingContext garblingContext;
        if (argc != 2) {
            printf("Usage: <prog_name> number_of_gates (in thousands)");
        }

	//Set up circuit parameters
	int n = atoi(argv[1]);// * 1024;
	int m = 1;
	int q = 100* log2(n) * n;
	int r = 100* log2(n) * n;

	// int Tsize = 8;
	// int colbits = 24;
	// int n = 216;

	// int noofbits = 3; 
	// int noofcols = colbits/noofbits;

	// // int m = Tsize*ceil(log2(colbits+1));
	// int m = ceil(log2(2*noofcols*Tsize+1));
	// //m = n;
	// // int m = split;
	// int q = 7*n*log2(n);
	// int r = 7*n*log2(n);	

	//Set up input and output tokens/labels.
	block *labels = (block*) malloc(sizeof(block) * 2 * n);
	// block *exlabels = (block*) malloc(sizeof(block) * n);
	// block *outputbs2 = (block*) malloc(sizeof(block) * 2 * m);
	block *outputbs = (block*) malloc(sizeof(block) * 2 * m);
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

    // GenomeCircuit2(&garbledCircuit, &garblingContext, n, colbits,Tsize, noofcols, inp, outputs);
	LEQCircuit2(&garbledCircuit, &garblingContext, n, inp, outputs);
	finishBuilding(&garbledCircuit, &garblingContext, outputMap, outputs);

	//Garble the built circuit.
	garbleCircuit(&garbledCircuit, inputLabels, outputMap);

	//Evaluate the circuit with random values and check the computed
	//values match the outputs of the desired function.
	checkCircuit(&garbledCircuit, inputLabels, outputMap, &(checkadd));

	return 0;

}

