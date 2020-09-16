
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "../include/justGarble.h"
#include "math.h"
#include "time.h"

int main(int argc, char **argv) {
	#ifdef FREE_XOR
		printf("FREE_XOR is switched on!\n");
	#endif
	
	// srand(time(NULL));
	GarbledCircuit garbledCircuit;
	GarblingContext garblingContext;

	// int Tsize = atoi(argv[2]);
	// int colbits = atoi(argv[3]);
	
	// int n = atoi(argv[1]);

	// int m = Tsize*ceil(log2(colbits+1));

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


	//Setup input and output tokens/labels.
	block *labels = (block*) malloc(sizeof(block) * 2 * n);
	block *outputbs = (block*) malloc(sizeof(block) * 2 * m);
	int *inp = (int *) malloc(sizeof(int) * n);
	countToN(inp, n);
	int outputs[m];
	int i;

	OutputMap outputMap = outputbs;
	InputLabels inputLabels = labels;
	
	FILE*fp;
	fp = fopen("labels","r");
	int nlabel,labelsize;
	nlabel = n;
	labelsize = 16;
	// fscanf(fp,"%d\n%d\n",&nlabel,&labelsize);
    // block *labels = (block*) malloc(sizeof(block) * 2 * nlabel);
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

	//Create a circuit.
	// createInputLabels(labels, n);
	
	createEmptyGarbledCircuit(&garbledCircuit, n, m, q, r, inputLabels);
	startBuilding(&garbledCircuit, &garblingContext);
    GenomeCircuit2(&garbledCircuit, &garblingContext, n, colbits,Tsize, noofcols, inp, outputs);
    // GenomeCircuit(&garbledCircuit, &garblingContext, n, colbits,inp, outputs);
    // NOTCircuit(&garbledCircuit, &garblingContext, n, inp, outputs);
	// MIXEDCircuit(&garbledCircuit, &garblingContext, n, inp, outputs);
	finishBuilding(&garbledCircuit, &garblingContext, outputMap, outputs);

	int correctnlabel;
	int correctlabelsize;

	correctnlabel = n - colbits;
	correctlabelsize = 16;

	fp = fopen("correctlabels","r");
	// fscanf(fp,"%d\n%d\n",&correctnlabel,&correctlabelsize);

	int totcount = 0;
    block *labelscorrect = (block*) malloc(sizeof(block) * nlabel);
	// int count = -1;	

	for(i = 0; i < correctnlabel; i++) {
		unsigned char labelchar[correctlabelsize];
		int j;
		for(j = 0; j < correctlabelsize;j++) {
			char tempchar;
			fscanf(fp, "%c", &tempchar);
			labelchar[j] = (unsigned char)tempchar;
		}
		memcpy(&labelscorrect[i],labelchar,correctlabelsize);
		totcount++;
		if (memcmp(&labelscorrect[i],&labels[2*i],16) == 0) {
			// count = (i%colbits)%3;
		}	
		else if (memcmp(&labelscorrect[i],&labels[2*i+1],16) == 0) {
			// count = i%3;
		}
		else {
			printf("\nPut correct labels first_%d\n",i);
			// int k = rand()%2;
			// memcpy(&labelscorrect[i],&labels[2*i+k],labelsize);
			exit(1);
		}
		// if(i%3==2) {printf("%d::",count);count=-1;}
		// if(i%colbits == colbits-1) printf("\n");
	}
	
	int correctnlabel2;
	int correctlabelsize2;

	correctnlabel2 = colbits;
	correctlabelsize2 = 16;

	fp = fopen("labelsauth","r");
	// fscanf(fp,"%d\n%d\n",&correctnlabel2,&correctlabelsize2);
	if(nlabel != correctnlabel + correctnlabel2) {
		printf("Error in joining separate labels\n");
		exit(1);
	}
	long long tempno = correctnlabel2*correctlabelsize2;
	printf("[Final Hamm]: Label size sent from authority to server : %lld\n", tempno);
	// printf("correctnlabel2 is %d, correctlabelsize2 is %d\n",correctnlabel2, correctlabelsize2);
	for(i = 0; i < correctnlabel2; i++) {	
		totcount++;	
		unsigned char labelchar[correctlabelsize2];
		int j;
		for(j = 0; j < correctlabelsize2;j++) {
			char tempchar;
			fscanf(fp, "%c", &tempchar);
			labelchar[j] = (unsigned char)tempchar;
		}

		memcpy(&labelscorrect[correctnlabel + i],labelchar,correctlabelsize2);
		if (memcmp(&labelscorrect[correctnlabel + i],&labels[2*(correctnlabel+i)],16) == 0) {
			// printf("0::");
		}	
		else if (memcmp(&labelscorrect[correctnlabel + i],&labels[2*(correctnlabel+i)+1],16) == 0) {
			// printf("1::");
		}
		else {
			printf("Put correct labels first::%d\n",i);
			// printf("%d\n",memcmp(&labelscorrect[correctnlabel + i],&labels[2*(correctnlabel+i)],16));
			// printf("%d\n",memcmp(&labelscorrect[correctnlabel + i],&labels[2*(correctnlabel+i)+1],16));
			// int t = 0;
			// unsigned char* ptr = (unsigned char*)&labelscorrect[correctnlabel+i];
			// for(t=0;t<16;t++) {
			// 	printf("%d ",ptr[t]);
			// }
			// printf("\n");

			// ptr = (unsigned char*)&labels[2*(correctnlabel+i)];
			// for(t=0;t<16;t++) {
			// 	printf("%d ",ptr[t]);
			// }
			// printf("\n");


			// ptr = (unsigned char*)&labels[2*(correctnlabel+i)+1];
			// for(t=0;t<16;t++) {
			// 	printf("%d ",ptr[t]);
			// }
			// printf("\n");

			// int k = rand()%2;
			// memcpy(&labelscorrect[i],&labels[2*i+k],labelsize);
			exit(1);
		}
	}
	fclose(fp);
	printf("All labels are correct, Total correct label count is %d\n",totcount);
	// for(i=0;i<n;i++) {
	// 	labelscorrect[i] = labels[2*i+1];
	// }

	int mmap,mapsize;
	mmap = m;
	mapsize = 16;
	fp = fopen("omap","r");
	// fscanf(fp,"%d\n%d\n",&mmap,&mapsize);
	// block *outputbs = (block*) malloc(sizeof(block) * 2 * mmap);
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

	// OutputMap outputMap = outputbs;
	
	int keysize,qsize,tablecol;
	keysize = 16;
	qsize = garbledCircuit.q;
	tablecol = 16;
	fp = fopen("garbledkey","r");
	// fscanf(fp,"%d\n%d\n%d\n",&keysize,&qsize,&tablecol);
	unsigned char keychar[keysize];
	for(i = 0; i < keysize;i++) {
		char tempchar;
		fscanf(fp, "%c", &tempchar);
		keychar[i] = (unsigned char)tempchar;
	}
	memcpy(&garbledCircuit.globalKey,keychar,keysize);
	
	fclose(fp);	
	fp = fopen("garbledtable","r");
	for(i=0;i<qsize;i++) {
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
	
	clock_t begin = clock();
	ExtractedLabels extractedLabels = labelscorrect;
	block computedOutputMap[mmap];
	// printf("mmap is %d\n\n\n",mmap);
	evaluate(&garbledCircuit, extractedLabels, computedOutputMap);
	clock_t end = clock();
	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("[Final Hamm]: Circuit evaluation time is %lf\n",time_spent);
	
	begin = clock();
	int outputVals[mmap];
	mapOutputs(outputMap, computedOutputMap, outputVals, mmap);
	end = clock();
	time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("[Final Hamm]: Client takes to calculate %lf\n",time_spent);

	// for (i = 0; i <= m-1; i+=m/Tsize) {
	// 	int j;
	// 	long long ansval = 0;
	// 	for(j=m/Tsize-1;j>=0;j--) {
	// 		ansval = 2*ansval + outputVals[i+j];
	// 	}
	// 	// printf("%d",outputVals[i]);
	// 	printf("%lld::",ansval);
	// }
	// printf("\n");
	// printf("%d\n",m/Tsize);
	// for (i = 0; i <= m-1; i++) {
	// 	printf("%d",outputVals[i]);
	// }
	// printf("\n");
	// printf("%d\n",m/Tsize);
	long long ansval = 0;
	for (i = m-1; i >= 0; i--) {
		ansval = 2*ansval + outputVals[i];
		// printf("%d",outputVals[i]);
	}
	printf("[Ans]: %lld\n",ansval);


	return 0;

}
