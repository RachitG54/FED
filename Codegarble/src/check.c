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


#include "../include/common.h"
#include "../include/garble.h"
#include "../include/check.h"
#include "../include/util.h"
#include "../include/justGarble.h"


int checkCircuit(GarbledCircuit *garbledCircuit, InputLabels inputLabels,
		OutputMap outputMap, int check(int *a, int *out, int s)) {

	int i, j;
	int n = garbledCircuit->n;
	int m = garbledCircuit->m;
	block extractedLabels[n];
	block computedOutputMap[m];
	int outputVals[m];
	int outputReal[m];
	int inputs[n];
	int count = 0;
	int nooftimes = TIMES;
	for (i = 0; i < nooftimes; i++) {
		printf("Test %d\n",i);
		// long long val1 = 0;
		for (j = 0; j < n/2; j++) {
			inputs[j] = rand() % 2;
			// if(j==7 || j == 9) inputs[j] = 1;else inputs[j] = 0;
			printf("%d ",inputs[j]);
		}
		// for (j = n/2-1; j >= 0; j--) {
		// 	inputs[j] = rand() % 2;
		// 	printf("%d ",inputs[j]);
		// }
		// printf("%lld ", val1);
		printf("\t");
		// long long val2 = 0;
		for (j = n/2; j < n; j++) {
			inputs[j] = rand() % 2;
			// inputs[j] = 0;
			printf("%d ",inputs[j]);
		}
		// for (j = n-1; j >= n/2; j--) {
		// 	inputs[j] = rand() % 2;
		// 	printf("%d ",inputs[j]);
		// }
		// printf("%lld ", val2);
		printf("\n");
		extractLabels(extractedLabels, inputLabels, inputs, n);
		evaluate(garbledCircuit, extractedLabels, computedOutputMap);
		mapOutputs(outputMap, computedOutputMap, outputVals, m);
		printf("Outout size is %d\n",m);
		for(j = 0; j < m; j++) {
			printf("%d ",outputVals[j]);
		}
		// for (j = m-1; j >= 0; j--) {
		// 	printf("%d ",outputVals[j]);
		// }
		// long long ansval = 0;
		// for (j = m-1; j >= 0; j--) {
		// 	ansval = 2*ansval + outputVals[j];
		// 	// printf("%lld  ",ansval);
		// }
		printf("\n");
		// long long mod = 1<<m;
		// printf("%lld %lld ", ansval,(val1+val2)%mod);
		// printf("\n");

		// check(inputs, outputReal, n);

		// for (j = 0; j < m; j++) {
		// 	printf("%d ",outputReal[j]);
		// }
		// printf("\n");
		// for (j = m-1; j >= 0; j--) {
		// 	printf("%d ",outputReal[j]);
		// }
		// long long ansval = 0;
		// for (j = m-1; j >= 0; j--) {
		// 	ansval = 2*ansval + outputVals[j];
		// 	// printf("%lld  ",ansval);
		// }
		// printf("\n");
		for (j = 0; j < m; j++)
			if (outputVals[j] != outputReal[j]) {
				count++;
				fprintf(stderr, "Check failed %u \n", j);
			}
	}
	printf("Total count is %d, failed count is %d\n",nooftimes,count);
	return 0;
}

unsigned long timedEval(GarbledCircuit *garbledCircuit, InputLabels inputLabels) {

	int n = garbledCircuit->n;
	int m = garbledCircuit->m;
	block extractedLabels[n];
	block outputs[m];
	int j;
	int inputs[n];
	unsigned long startTime, endTime;
	unsigned long sum = 0;
	for (j = 0; j < n; j++) {
		inputs[j] = rand() % 2;
	}
	extractLabels(extractedLabels, inputLabels, inputs, n);
	startTime = RDTSC;
	evaluate(garbledCircuit, extractedLabels, outputs);
	endTime = RDTSC;
	sum = endTime - startTime;
	return sum;

}

