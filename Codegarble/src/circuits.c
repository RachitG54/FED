
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


#include "../include/garble.h"
#include "../include/common.h"
#include "../include/circuits.h"
#include "../include/gates.h"
#include "../include/util.h"
#include "../include/justGarble.h"
#include "math.h"


int A2X1[8] = { 0x98, 0xF3, 0xF2, 0x48, 0x09, 0x81, 0xA9, 0xFF }, X2A1[8] = {
		0x64, 0x78, 0x6E, 0x8C, 0x68, 0x29, 0xDE, 0x60 }, X2S1[8] = { 0x58,
		0x2D, 0x9E, 0x0B, 0xDC, 0x04, 0x03, 0x24 }, S2X1[8] = { 0x8C, 0x79,
		0x05, 0xEB, 0x12, 0x04, 0x51, 0x53 };

int ANDCircuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext,
		int n, int* inputs, int* outputs) {
	int i;
	int oldInternalWire = getNextWire(garblingContext);
	int newInternalWire;

	ANDGate(garbledCircuit, garblingContext, inputs[0], inputs[1],
			oldInternalWire);

	for (i = 2; i < n - 1; i++) {
		newInternalWire = getNextWire(garblingContext);
		ANDGate(garbledCircuit, garblingContext, inputs[i], oldInternalWire,
				newInternalWire);
		oldInternalWire = newInternalWire;
	}
	outputs[0] = getNextWire(garblingContext);
	return ANDGate(garbledCircuit, garblingContext, inputs[n - 1],
			oldInternalWire, outputs[0]);
}

int MIXEDCircuit(GarbledCircuit *garbledCircuit,
		GarblingContext *garblingContext, int n, int* inputs, int* outputs) {
	int i;
	int oldInternalWire = inputs[0];
	int newInternalWire;

	for (i = 0; i < n - 1; i++) {
		newInternalWire = getNextWire(garblingContext);
		if (i % 3 == 2)
			ORGate(garbledCircuit, garblingContext, inputs[i + 1],
					oldInternalWire, newInternalWire);
		if (i % 3 == 1)
			ANDGate(garbledCircuit, garblingContext, inputs[i + 1],
					oldInternalWire, newInternalWire);
		if (i % 3 == 0)
			XORGate(garbledCircuit, garblingContext, inputs[i + 1],
					oldInternalWire, newInternalWire);
		oldInternalWire = newInternalWire;
	}
	outputs[0] = oldInternalWire;
	return 0;
}

int EncoderCircuit(GarbledCircuit *gc, GarblingContext *garblingContext,
		int* inputs, int* outputs, int enc[]) {
	int i, j, temp;
	int n = 8;
	int curWires[n];
	for (i = 0; i < n; i++) {
		curWires[i] = fixedZeroWire(gc, garblingContext);
	}
	for (i = 0; i < n; i++) {
		for (j = 0; j < n; j++) {
			if (fbits(enc[i],j)) {
				temp = getNextWire(garblingContext);
				XORGate(gc, garblingContext, curWires[j], inputs[i], temp);
				curWires[j] = temp;
			}
		}
	}
	for (i = 0; i < n; i++) {
		outputs[i] = curWires[i];
	}

	return 0;
}
int EncoderOneCircuit(GarbledCircuit *gc, GarblingContext *garblingContext,
		int* inputs, int* outputs, int enc[]) {
	int i, j, temp;
	int n = 8;
	int curWires[n];
	for (i = 0; i < n; i++) {
		curWires[i] = fixedOneWire(gc, garblingContext);
	}
	for (i = 0; i < n; i++) {
		for (j = 0; j < n; j++) {
			if (fbits(enc[i],j)) {
				temp = getNextWire(garblingContext);
				XORGate(gc, garblingContext, curWires[j], inputs[i], temp);
				curWires[j] = temp;
			}
		}
	}
	for (i = 0; i < n; i++) {
		outputs[i] = curWires[i];
	}

	return 0;
}

int GF16SQCLCircuit(GarbledCircuit *gc, GarblingContext *garblingContext,
		int* inputs, int* outputs) {
	int a[2], b[2];
	a[0] = inputs[2];
	a[1] = inputs[3];
	b[0] = inputs[0];
	b[1] = inputs[1];

	int ab[4];
	ab[0] = a[0];
	ab[1] = a[1];
	ab[2] = b[0];
	ab[3] = b[1];

	int tempx[2];
	XORCircuit(gc, garblingContext, 4, ab, tempx);
	int p[2], q[2];
	GF4SQCircuit(gc, garblingContext, tempx, p);
	int tempx2[4];
	GF4SQCircuit(gc, garblingContext, b, tempx2);
	GF4SCLN2Circuit(gc, garblingContext, tempx2, q);

	outputs[0] = q[0];
	outputs[1] = q[1];
	outputs[2] = p[0];
	outputs[3] = p[1];
	return 0;
}

int GF16MULCircuit(GarbledCircuit *gc, GarblingContext *garblingContext,
		int* inputs, int* outputs) {
	int temp[2];
	temp[0] = getNextWire(garblingContext);
	temp[1] = getNextWire(garblingContext);

	int ab[4], cd[4], e[4];
	ab[0] = inputs[2];
	ab[1] = inputs[3];
	ab[2] = inputs[0];
	ab[3] = inputs[1];

	cd[0] = inputs[2 + 4];
	cd[1] = inputs[3 + 4];
	cd[2] = inputs[0 + 4];
	cd[3] = inputs[1 + 4];

	int abcdx[4];
	XORCircuit(gc, garblingContext, 4, ab, abcdx);
	XORCircuit(gc, garblingContext, 4, cd, abcdx + 2);
	GF4MULCircuit(gc, garblingContext, abcdx, e);
	int em[2];
	GF4SCLNCircuit(gc, garblingContext, e, em);
	int p[2], q[2];

	int ac[4];
	ac[0] = ab[0];
	ac[1] = ab[1];
	ac[2] = cd[0];
	ac[3] = cd[1];

	int bd[4];
	bd[0] = ab[2 + 0];
	bd[1] = ab[2 + 1];
	bd[2] = cd[2 + 0];
	bd[3] = cd[2 + 1];

	int tmpx1[4], tmpx2[4];
	GF4MULCircuit(gc, garblingContext, ac, tmpx1);
	GF4MULCircuit(gc, garblingContext, bd, tmpx2);

	tmpx1[2] = em[0];
	tmpx1[3] = em[1];
	tmpx2[2] = em[0];
	tmpx2[3] = em[1];

	XORCircuit(gc, garblingContext, 4, tmpx1, p);
	XORCircuit(gc, garblingContext, 4, tmpx2, q);

	outputs[0] = q[0];
	outputs[1] = q[1];
	outputs[2] = p[0];
	outputs[3] = p[1];
	return 0;
}

int GF4MULCircuit(GarbledCircuit *gc, GarblingContext *garblingContext,
		int* inputs, int* outputs) {
	int a, b, c, d, e, p, q;

	a = inputs[1];
	b = inputs[0];
	c = inputs[3];
	d = inputs[2];
	int temp1 = getNextWire(garblingContext);
	XORGate(gc, garblingContext, a, b, temp1);

	int temp2 = getNextWire(garblingContext);
	XORGate(gc, garblingContext, c, d, temp2);

	e = getNextWire(garblingContext);
	ANDGate(gc, garblingContext, temp1, temp2, e);

	int temp3 = getNextWire(garblingContext);
	ANDGate(gc, garblingContext, a, c, temp3);
	p = getNextWire(garblingContext);
	XORGate(gc, garblingContext, temp3, e, p);

	int temp4 = getNextWire(garblingContext);
	ANDGate(gc, garblingContext, b, d, temp4);
	q = getNextWire(garblingContext);
	XORGate(gc, garblingContext, temp4, e, q);

	outputs[1] = p;
	outputs[0] = q;
	return 0;
}

int GF4SCLNCircuit(GarbledCircuit *gc, GarblingContext *garblingContext,
		int* inputs, int* outputs) {

	outputs[0] = getNextWire(garblingContext);
	XORGate(gc, garblingContext, inputs[0], inputs[1], outputs[0]);
	outputs[1] = inputs[0];

	return 0;
}

int GF4SQCircuit(GarbledCircuit *gc, GarblingContext *garblingContext,
		int* inputs, int* outputs) {

	outputs[0] = inputs[1];
	outputs[1] = inputs[0];

	return 0;
}

int NewSBOXCircuit(GarbledCircuit *gc, GarblingContext *garblingContext,
		int* inputs, int* outputs) {
	int temp1[8], temp2[8], temp3[8];
	EncoderCircuit(gc, garblingContext, inputs, temp1, A2X1);
	GF256InvCircuit(gc, garblingContext, temp1, temp2);
	EncoderOneCircuit(gc, garblingContext, temp2, outputs, S2X1);
	return 0;
}

int GF256InvCircuit(GarbledCircuit *gc, GarblingContext *garblingContext,
		int* inputs, int* outputs) {
	int A[4], B[4], C[4], D[4], E[4], P[4], Q[4], tempX[4];
	int i;
	for (i = 0; i < 4; i++) {
		A[i] = inputs[i];
		B[i] = inputs[i + 4];
	}

	XORCircuit(gc, garblingContext, 8, inputs, tempX);
	GF16SQCLCircuit(gc, garblingContext, tempX, C);
	GF16MULCircuit(gc, garblingContext, inputs, D);

	int CD[8];
	for (i = 0; i < 4; i++) {
		CD[i] = C[i];
		CD[i + 4] = D[i];
	}
	int tempX2[4];
	XORCircuit(gc, garblingContext, 8, CD, tempX2);
	GF16INVCircuit(gc, garblingContext, tempX2, E);
	int EB[8];
	for (i = 0; i < 4; i++) {
		EB[i] = E[i];
		EB[i + 4] = B[i];
	}

	int EA[8];
	for (i = 0; i < 4; i++) {
		EA[i] = E[i];
		EA[i + 4] = A[i];
	}

	GF16MULCircuit(gc, garblingContext, EB, P);
	GF16MULCircuit(gc, garblingContext, EA, Q);

	outputs[4] = P[0];
	outputs[5] = P[1];
	outputs[6] = P[2];
	outputs[7] = P[3];

	outputs[0] = Q[0];
	outputs[1] = Q[1];
	outputs[2] = Q[2];
	outputs[3] = Q[3];

	return 0;
}

int GF16INVCircuit(GarbledCircuit *gc, GarblingContext *garblingContext,
		int* inputs, int* outputs) {

	int a[2], b[2];
	a[0] = inputs[2];
	a[1] = inputs[3];
	b[0] = inputs[0];
	b[1] = inputs[1];

	int ab[4], cd[4];
	ab[0] = a[0];
	ab[1] = a[1];
	ab[2] = b[0];
	ab[3] = b[1];

	int tempx[2], tempxs[2];
	XORCircuit(gc, garblingContext, 4, ab, tempx);
	GF4SQCircuit(gc, garblingContext, tempx, tempxs);

	int c[2], d[2], e[2], p[2], q[2];
	GF4SCLNCircuit(gc, garblingContext, tempxs, c);

	GF4MULCircuit(gc, garblingContext, ab, d);

	cd[0] = c[0];
	cd[1] = c[1];
	cd[2] = d[0];
	cd[3] = d[1];

	int tempx2[2];
	XORCircuit(gc, garblingContext, 4, cd, tempx2);
	GF4SQCircuit(gc, garblingContext, tempx2, e);
	int eb[4], ea[4];
	ea[0] = e[0];
	ea[1] = e[1];
	ea[2] = a[0];
	ea[3] = a[1];

	eb[0] = e[0];
	eb[1] = e[1];
	eb[2] = b[0];
	eb[3] = b[1];

	GF4MULCircuit(gc, garblingContext, eb, p);
	GF4MULCircuit(gc, garblingContext, ea, q);

	outputs[0] = q[0];
	outputs[1] = q[1];
	outputs[2] = p[0];
	outputs[3] = p[1];
	return 0;

}

int GF4SCLN2Circuit(GarbledCircuit *gc, GarblingContext *garblingContext,
		int* inputs, int* outputs) {

	outputs[1] = getNextWire(garblingContext);
	XORGate(gc, garblingContext, inputs[0], inputs[1], outputs[1]);
	outputs[0] = inputs[1];

	return 0;
}

int RANDCircuit(GarbledCircuit *garbledCircuit,
		GarblingContext *garblingContext, int n, int* inputs, int* outputs,
		int q, int qf) {
	int i;
	int oldInternalWire = getNextWire(garblingContext);
	int newInternalWire;

	ANDGate(garbledCircuit, garblingContext, 0, 1, oldInternalWire);

	for (i = 2; i < q + qf - 1; i++) {
		newInternalWire = getNextWire(garblingContext);
		if (i < q)
			ANDGate(garbledCircuit, garblingContext, i % n, oldInternalWire,
					newInternalWire);
		else
			XORGate(garbledCircuit, garblingContext, i % n, oldInternalWire,
					newInternalWire);
		oldInternalWire = newInternalWire;
	}
	outputs[0] = oldInternalWire;
	return 0;
}

int INCCircuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext,
		int n, int* inputs, int* outputs) {
	int i;
	for (i = 0; i < n; i++)
		outputs[i] = getNextWire(garblingContext);

	NOTGate(garbledCircuit, garblingContext, inputs[0], outputs[0]);
	int carry = inputs[0];
	int newCarry;
	for (i = 1; i < n; i++) {
		XORGate(garbledCircuit, garblingContext, inputs[i], carry, outputs[i]);
		newCarry = getNextWire(garblingContext);
		ANDGate(garbledCircuit, garblingContext, inputs[i], carry, newCarry);
		carry = newCarry;
	}
	return 0;
}

int SUBCircuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext,
		int n, int* inputs, int* outputs) {
	int tempWires[n / 2];
	int tempWires2[n];
	int split = n / 2;
	NOTCircuit(garbledCircuit, garblingContext, n / 2, inputs + split,
			tempWires);
	INCCircuit(garbledCircuit, garblingContext, n / 2, tempWires,
			tempWires2 + split);
	memcpy(tempWires2, inputs, sizeof(int) * split);
	return ADDCircuit(garbledCircuit, garblingContext, n, tempWires2, outputs);
}

int SHLCircuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext,
		int n, int* inputs, int* outputs) {
	outputs[0] = fixedZeroWire(garbledCircuit, garblingContext);
	memcpy(outputs + 1, inputs, sizeof(int) * (n - 1));
	return 0;
}

int SHRCircuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext,
		int n, int* inputs, int* outputs) {
	outputs[n - 1] = fixedZeroWire(garbledCircuit, garblingContext);
	memcpy(outputs, inputs + 1, sizeof(int) * (n - 1));
	return 0;
}

int MULCircuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext,
		int nt, int* inputs, int* outputs) {
	int i, j;
	int n = nt / 2;
	int *A = inputs;
	int *B = inputs + n;

	int tempAnd[n][2 * n];
	int tempAddIn[4 * n];
	int tempAddOut[4 * n];

	for (i = 0; i < n; i++) {
		for (j = 0; j < i; j++) {
			tempAnd[i][j] = fixedZeroWire(garbledCircuit, garblingContext);
		}
		for (j = i; j < i + n; j++) {
			tempAnd[i][j] = getNextWire(garblingContext);
			ANDGate(garbledCircuit, garblingContext, A[j - i], B[i],
					tempAnd[i][j]);
		}
		for (j = i + n; j < 2 * n; j++)
			tempAnd[i][j] = fixedZeroWire(garbledCircuit, garblingContext);
	}

	for (j = 0; j < 2 * n; j++) {
		tempAddOut[j] = tempAnd[0][j];
	}
	for (i = 1; i < n; i++) {
		for (j = 0; j < 2 * n; j++) {
			tempAddIn[j] = tempAddOut[j];
		}
		for (j = 2 * n; j < 4 * n; j++) {
			tempAddIn[j] = tempAnd[i][j - 2 * n];
		}
		ADDCircuit(garbledCircuit, garblingContext, 4 * n, tempAddIn,
				tempAddOut);
	}
	for (j = 0; j < 2 * n; j++) {
		outputs[j] = tempAddOut[j];
	}
	return 0;

}

int GRECircuit(GarbledCircuit *gc, GarblingContext *garblingContext, int n,
		int* inputs, int* outputs) {
	int tempWires[n];
	int i;
	for (i = 0; i < n / 2; i++) {
		tempWires[i] = inputs[i + n / 2];
		tempWires[i + n / 2] = inputs[i];
	}
	return LESCircuit(gc, garblingContext, n, tempWires, outputs);

}

int MINCircuit(GarbledCircuit *gc, GarblingContext *garblingContext, int n,
		int* inputs, int* outputs) {
	int i;
	int leqOutput[1];
	int andOneOutput[n / 2];
	int andTwoOutput[n / 2];
	int notOutput = getNextWire(garblingContext);
	LEQCircuit(gc, garblingContext, n, inputs, leqOutput);
	NOTGate(gc, garblingContext, leqOutput[0], notOutput);
	for (i = 0; i < n / 2; i++) {
		andOneOutput[i] = getNextWire(garblingContext);
		andTwoOutput[i] = getNextWire(garblingContext);
		outputs[i] = getNextWire(garblingContext);
		ANDGate(gc, garblingContext, leqOutput[0], inputs[i], andOneOutput[i]);
		ANDGate(gc, garblingContext, notOutput, inputs[n / 2 + i],
				andTwoOutput[i]);
		XORGate(gc, garblingContext, andOneOutput[i], andTwoOutput[i],
				outputs[i]);
	}
	return 0;
}

int LEQCircuit2(GarbledCircuit *gc, GarblingContext *garblingContext, int n,
		int* inputs, int* outputs) {
	int split = n/2;
	int i;
	int leqOutput[split];
	int tempWire = getNextWire(garblingContext);
	ANDGate(gc, garblingContext, inputs[0], inputs[split], tempWire);
	int notWire = getNextWire(garblingContext);
	NOTGate(gc, garblingContext, inputs[0], notWire);
	leqOutput[0] = getNextWire(garblingContext);
	ORGate(gc, garblingContext, tempWire, notWire, leqOutput[0]);
	for(i = 1; i < split; i++) {
		tempWire = getNextWire(garblingContext);
		XORGate(gc, garblingContext, inputs[i], inputs[split+i], tempWire);

		leqOutput[i] = tempWire;

		notWire = getNextWire(garblingContext);
		NOTGate(gc, garblingContext, tempWire, notWire);
		leqOutput[i] = notWire;
		// printf("i is %d\n",i);
		int tempWire2;
		tempWire2 = getNextWire(garblingContext);
		ANDGate(gc, garblingContext, notWire, leqOutput[i-1], tempWire2);
		notWire = getNextWire(garblingContext);
		NOTGate(gc, garblingContext, inputs[i], notWire);
		tempWire = getNextWire(garblingContext);
		ANDGate(gc, garblingContext, notWire, inputs[split+i], tempWire);
		// leqOutput[i] = tempWire;
		leqOutput[i] = getNextWire(garblingContext);
		ORGate(gc, garblingContext, tempWire2, tempWire, leqOutput[i]);
	}
	// printf("ans is %d\n",split-1);
	outputs[0] = leqOutput[split-1];
	return 0;
}

int LEQCircuit(GarbledCircuit *gc, GarblingContext *garblingContext, int n,
		int* inputs, int* outputs) {
	int tempWires;
	GRECircuit(gc, garblingContext, n, inputs, &tempWires);
	int outWire = getNextWire(garblingContext);
	NOTGate(gc, garblingContext, tempWires, outWire);
	outputs[0] = outWire;
	return 0;
}

int GEQCircuit(GarbledCircuit *gc, GarblingContext *garblingContext, int n,
		int* inputs, int* outputs) {
	int tempWires;
	LESCircuit(gc, garblingContext, n, inputs, &tempWires);
	int outWire = getNextWire(garblingContext);
	NOTGate(gc, garblingContext, tempWires, outWire);
	outputs[0] = outWire;
	return 0;
}

int LESCircuit(GarbledCircuit *gc, GarblingContext *garblingContext, int n,
		int* inputs, int* outputs) {
	int tempWires[n / 2];
	SUBCircuit(gc, garblingContext, n, inputs, tempWires);
	int test = tempWires[n / 2 - 1];
	int A = n / 2 - 1;
	int B = n - 1;

	int notA = getNextWire(garblingContext);
	NOTGate(gc, garblingContext, A, notA);

	int notB = getNextWire(garblingContext);
	NOTGate(gc, garblingContext, B, notB);

	int case1 = getNextWire(garblingContext);
	ANDGate(gc, garblingContext, A, notB, case1);

	int tmpCase2 = getNextWire(garblingContext);
	int case2 = getNextWire(garblingContext);
	ORGate(gc, garblingContext, notA, B, tmpCase2);
	NOTGate(gc, garblingContext, tmpCase2, case2);

	int tmpCase3 = getNextWire(garblingContext);
	int case3 = getNextWire(garblingContext);
	ANDGate(gc, garblingContext, notA, notB, tmpCase3);
	ANDGate(gc, garblingContext, tmpCase3, test, case3);

	int notTest = getNextWire(garblingContext);
	int tmpCase4 = getNextWire(garblingContext);
	int case4 = getNextWire(garblingContext);
	ANDGate(gc, garblingContext, A, B, tmpCase4);
	NOTGate(gc, garblingContext, test, notTest);
	ANDGate(gc, garblingContext, tmpCase4, notTest, case4);

	int tempFinal1 = getNextWire(garblingContext);
	int tempFinal2 = getNextWire(garblingContext);
	outputs[0] = getNextWire(garblingContext);
	ORGate(gc, garblingContext, case1, case2, tempFinal1);
	ORGate(gc, garblingContext, case3, case4, tempFinal2);
	ORGate(gc, garblingContext, tempFinal1, tempFinal2, outputs[0]);
	return 0;
}

int EQUCircuit(GarbledCircuit *gc, GarblingContext *garblingContext, int n,
		int* inputs, int* outputs) {
	int tempWires[n / 2];

	XORCircuit(gc, garblingContext, n, inputs, tempWires);
	int i;
	int tempWire1 = tempWires[0];
	int tempWire2;
	for (i = 1; i < n / 2; i++) {
		tempWire2 = getNextWire(garblingContext);
		ORGate(gc, garblingContext, tempWire1, tempWires[i], tempWire2);
		tempWire1 = tempWire2;
	}
	int outWire = getNextWire(garblingContext);

	NOTGate(gc, garblingContext, tempWire1, outWire);
	outputs[0] = outWire;
	return 0;
}

int NOTCircuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext,
		int n, int* inputs, int* outputs) {
	int i;
	for (i = 0; i < n; i++) {
		outputs[i] = getNextWire(garblingContext);
		NOTGate(garbledCircuit, garblingContext, inputs[i], outputs[i]);
	}
	return 0;
}

int ADDCircuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext,
		int n, int* inputs, int* outputs) {
	int i;
	int tempOut[3];
	int split = n / 2;
	int tempIn[3];

	tempIn[0] = inputs[0];
	tempIn[1] = inputs[split];
	ADD22Circuit(garbledCircuit, garblingContext, tempIn, tempOut);
	outputs[0] = tempOut[0];

	for (i = 1; i < split; i++) {
		tempIn[2] = tempOut[1];
		tempIn[1] = inputs[split + i];
		tempIn[0] = inputs[i];
		ADD32Circuit(garbledCircuit, garblingContext, tempIn, tempOut);
		outputs[i] = tempOut[0];
	}
	return 0;
}
int SHR_k_Circuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext,
		int n, int k, int* inputs, int* outputs) {
	int i;
	for(i = 0; i < k; i++) 
		outputs[n - 1 - i] = fixedZeroWire(garbledCircuit, garblingContext);
	memcpy(outputs, inputs + k, sizeof(int) * (n - k));
	return 0;
}

int HammingCircuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext,
		int n, int* inputs, int* outputs) {
	if(n==2) {
		return XORCircuit(garbledCircuit, garblingContext, n, inputs, outputs);
	}
	int i;
	int split = n/2;
	int lenk = ceil(log2(split));
	// printf("%f ",log(split));
	printf("%d\n",lenk);
	
	int* tempone = (int*)malloc(sizeof(int)*n);
	XORCircuit(garbledCircuit, garblingContext, n, inputs, tempone);
	
	int* temptwo = (int*)malloc(sizeof(int)*split);
	SHRCircuit(garbledCircuit, garblingContext, split, tempone, temptwo);
	
	int* tempthree = (int*)malloc(sizeof(int)*split);
	AND_B_k_Circuit(garbledCircuit, garblingContext, split, 0, temptwo, tempthree);

	memcpy(tempone+split,tempthree,sizeof(int)*split);
	
	// free(temptwo);
	// free(tempthree);

	SUBCircuit(garbledCircuit, garblingContext, n, tempone, temptwo);

	for(i = 1; i < lenk; i++) {
		int S_k = 1 << i;
		SHR_k_Circuit(garbledCircuit, garblingContext, split, S_k, temptwo, tempthree);
		AND_B_k_Circuit(garbledCircuit, garblingContext, split, i, tempthree, tempone);

		AND_B_k_Circuit(garbledCircuit, garblingContext, split, i, temptwo, tempthree);
		memcpy(tempone+split,tempthree,sizeof(int)*split);

		ADDCircuit(garbledCircuit, garblingContext, n, tempone, temptwo);

	}
	memcpy(outputs,temptwo,split*sizeof(int));
	// for(i = 0; i < split; i++) {
	// 	outputs[i] = temptwo[i];
	// }
	free(tempone);
	free(temptwo);
	free(tempthree);
	return 0;
}
int AND_B_k_Circuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext,
		int n, int k, int* inputs, int* outputs) {
	int i;
	int S_k = 1 << k;
	for(i =0;i<n;i++) {
		int j  = i %(2*S_k);
		// printf("i is %d, j is %d\t",i,j);
		if(j >= S_k) outputs[i] = fixedZeroWire(garbledCircuit, garblingContext);
		else {
			// int wire1 = getNextWire(garblingContext);
			// int wire2 = getNextWire(garblingContext);
			outputs[i] = inputs[i];
			// NOTGate(garbledCircuit, garblingContext,inputs[i],wire1);
			// NOTGate(garbledCircuit, garblingContext,wire1,wire2);
			// outputs[i] = wire2;
			// outputs[i] = fixedOneWire(garbledCircuit, garblingContext);
		}
	}
	// printf("\n");
	return 0;
}
int SETZERO_bit_Circuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext,
		int n, int wire, int* outputs) {
	outputs[0] = wire;
	int i;
	for(i = 1; i < n; i++) {
		outputs[i] = fixedZeroWire(garbledCircuit, garblingContext);
	}
}
int ADDN1_Circuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext,
		int n, int* inputs, int wire, int* outputs) {
	int i;
    // tempOut[0] = tempIn[0] ^ tempIn[1];
    // tempOut[1] = tempIn[0] & tempIn[1];
    int tempIn[2];
    int tempOut[2];

    tempIn[0] = inputs[0];
    tempIn[1] = wire;
	ADD22Circuit(garbledCircuit, garblingContext,tempIn,tempOut);
	outputs[0] = tempOut[0];

	for (i = 1; i < n; i++) {
		tempIn[0] = tempOut[1];
		tempIn[1] = inputs[i];
		ADD22Circuit(garbledCircuit, garblingContext,tempIn,tempOut);
		outputs[i] = tempOut[0];
	}

	return 0;
}
int HammingCircuit2(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext,
		int n, int* inputs, int* outputs) {
	if(n==2) {
		return XORCircuit(garbledCircuit, garblingContext, n, inputs, outputs);
	}
	int i;
	int split = n/2;
	int m = ceil(log2(split+1));
	// printf("%f ",log(split));
	// printf("%d\n",lenk);
	
	int* tempone = (int*)malloc(sizeof(int)*split);
	XORCircuit(garbledCircuit, garblingContext, n, inputs, tempone);
	
	int* temptwo = (int*)malloc(sizeof(int)*m);
	SETZERO_bit_Circuit(garbledCircuit, garblingContext, m, tempone[0], temptwo);
	
	int* tempthree = (int*)malloc(sizeof(int)*m);

	for(i = 1; i < split; i++) {
		ADDN1_Circuit(garbledCircuit, garblingContext, m, temptwo, tempone[i], tempthree);
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
int HammingCircuit2add(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext,
		int n, int m, int* inputs, int* outputs) {
	// if(n==2) {
	// 	return XORCircuit(garbledCircuit, garblingContext, n, inputs, outputs);
	// }
	int i;
	int split = n/2;
	// int m = ceil(log2(split+1));
	// printf("%f ",log(split));
	// printf("%d\n",lenk);
	
	int* tempone = (int*)malloc(sizeof(int)*split);
	XORCircuit(garbledCircuit, garblingContext, n, inputs, tempone);
	
	int* temptwo = (int*)malloc(sizeof(int)*m);
	SETZERO_bit_Circuit(garbledCircuit, garblingContext, m, tempone[0], temptwo);
	
	int* tempthree = (int*)malloc(sizeof(int)*m);

	for(i = 1; i < split; i++) {
		ADDN1_Circuit(garbledCircuit, garblingContext, m, temptwo, tempone[i], tempthree);
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
int GenomeCircuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext,
		int n, int colbits, int* inputs, int* outputs) {
	
	int Tsize = n/colbits-1;

	int *tempone = (int*)malloc(sizeof(int)*2*colbits);
	memcpy(tempone,inputs+(n-colbits),sizeof(int)*colbits);
	
	int midind = ceil(log2(colbits+1));
	int m = midind*Tsize;

	int *output1 = (int*)malloc(sizeof(int)*m);
	// int *output2 = (int*)malloc(sizeof(int)*2*m);

	memcpy(tempone+colbits,inputs,sizeof(int)*colbits);
	HammingCircuit2(garbledCircuit,garblingContext,2*colbits,tempone,output1);

	int i;
	for(i=colbits;i<n-colbits;i += colbits) {
		int no = i/colbits;
		memcpy(tempone+colbits,inputs+i,sizeof(int)*colbits);
		HammingCircuit2(garbledCircuit,garblingContext,2*colbits,tempone,output1+no*midind);
		// memcpy(output2+m,output1,sizeof(int)*m);
		// MINCircuit(garbledCircuit,garblingContext,2*m,output2,output1);
	}

	memcpy(outputs,output1,sizeof(int)*m);

	free(output1);
	// free(output2);
	free(tempone);
}
// int GenomeCircuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext,
// 		int n, int colbits, int* inputs, int* outputs) {
// 	int *tempone = (int*)malloc(sizeof(int)*2*colbits);
// 	memcpy(tempone,inputs+(n-colbits),sizeof(int)*colbits);
// 	int m = ceil(log2(colbits+1));

// 	int *output1 = (int*)malloc(sizeof(int)*m);
// 	int *output2 = (int*)malloc(sizeof(int)*2*m);

// 	int *reverseoutput = (int*)malloc(sizeof(int)*m);

// 	memcpy(tempone+colbits,inputs,sizeof(int)*colbits);
// 	HammingCircuit2(garbledCircuit,garblingContext,2*colbits,tempone,reverseoutput);
// 	int i;

// 	for(i=0;i<m;i++) {
// 		output1[i] = reverseoutput[m-1-i];
// 	}
	
// 	for(i=colbits;i<n-colbits;i += colbits) {
// 		memcpy(tempone+colbits,inputs+i,sizeof(int)*colbits);
// 		HammingCircuit2(garbledCircuit,garblingContext,2*colbits,tempone,reverseoutput);
// 		int j;
// 		for(j=0;j<m;j++) {
// 			output2[j] = reverseoutput[m-1-j];
// 		}
// 		memcpy(output2+m,output1,sizeof(int)*m);
// 		MINCircuit(garbledCircuit,garblingContext,2*m,output2,output1);
// 	}

// 	memcpy(outputs,output1,sizeof(int)*m);

// 	free(reverseoutput);
// 	free(output1);
// 	free(output2);
// 	free(tempone);
// }
int GenomeCircuit2(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext,
		int n, int colbits, int Tsize, int noofcols, int* inputs, int* outputs) {

	int i;
	int *tempone = (int*)malloc(sizeof(int)*2*colbits);
	memcpy(tempone,inputs+(n-colbits),sizeof(int)*colbits);
	
	// int m = Tsize*ceil(log2(colbits+1));
	int m = ceil(log2(2*noofcols*Tsize+1));


	int indim = ceil(log2(colbits+1));

	int *output1 = (int*)malloc(sizeof(int)*m);

	int *output2 = (int*)malloc(sizeof(int)*2*m);

	memcpy(tempone+colbits,inputs,sizeof(int)*colbits);
	HammingCircuit2add(garbledCircuit,garblingContext,2*colbits,m,tempone,output1);
	// for(i = indim; i < m; i ++) {
	// 	output1[i] = fixedZeroWire(garbledCircuit, garblingContext);
	// }
	for(i=colbits;i<n-colbits;i += colbits) {
		// int no = i/colbits;
		// printf("here\n");
		memcpy(tempone+colbits,inputs+i,sizeof(int)*colbits);
		HammingCircuit2add(garbledCircuit,garblingContext,2*colbits,m,tempone,output2);
		// int j;
		// for(j = indim; j < m; j ++) {
		// 	output2[j] = output1[j];//fixedZeroWire(garbledCircuit, garblingContext);
		// }

		memcpy(output2+m,output1,sizeof(int)*m);
		// MINCircuit(garbledCircuit,garblingContext,2*m,output2,output1);
		ADDCircuit(garbledCircuit,garblingContext,2*m,output2,output1);
		// memcpy(output2+m,output1,sizeof(int)*m);
		// MINCircuit(garbledCircuit,garblingContext,2*m,output2,output1);
	}

	memcpy(outputs,output1,sizeof(int)*m);
	free(output1);
	free(output2);
	free(tempone);
}
int ADD32Circuit(GarbledCircuit *garbledCircuit,
		GarblingContext *garblingContext, int* inputs, int* outputs) {
	int wire1 = getNextWire(garblingContext);
	int wire2 = getNextWire(garblingContext);
	int wire3 = getNextWire(garblingContext);
	int wire4 = getNextWire(garblingContext);
	int wire5 = getNextWire(garblingContext);

	XORGate(garbledCircuit, garblingContext, inputs[2], inputs[0], wire1);
	XORGate(garbledCircuit, garblingContext, inputs[1], inputs[0], wire2);
	XORGate(garbledCircuit, garblingContext, inputs[2], wire2, wire3);
	ANDGate(garbledCircuit, garblingContext, wire1, wire2, wire4);
	XORGate(garbledCircuit, garblingContext, inputs[0], wire4, wire5);
	outputs[0] = wire3;
	outputs[1] = wire5;
	return 0;
}

int ADD22Circuit(GarbledCircuit *garbledCircuit,
		GarblingContext *garblingContext, int* inputs, int* outputs) {
	int wire1 = getNextWire(garblingContext);
	int wire2 = getNextWire(garblingContext);

	XORGate(garbledCircuit, garblingContext, inputs[0], inputs[1], wire1);
	ANDGate(garbledCircuit, garblingContext, inputs[0], inputs[1], wire2);
	outputs[0] = wire1;
	outputs[1] = wire2;
	return 0;
}

int ORCircuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext,
		int n, int* inputs, int* outputs) {
	int i;
	int oldInternalWire = getNextWire(garblingContext);
	int newInternalWire;
	ORGate(garbledCircuit, garblingContext, inputs[0], inputs[1],
			oldInternalWire);
	for (i = 2; i < n - 1; i++) {
		newInternalWire = getNextWire(garblingContext);
		ORGate(garbledCircuit, garblingContext, inputs[i], oldInternalWire,
				newInternalWire);
		oldInternalWire = newInternalWire;
	}
	outputs[0] = getNextWire(garblingContext);
	int res = ORGate(garbledCircuit, garblingContext, inputs[n - 1],
			oldInternalWire, outputs[0]);
	return res;
}

int MultiXORCircuit(GarbledCircuit *gc, GarblingContext *garblingContext, int d,
		int n, int* inputs, int* outputs) {
	int i, j;
	int div = n / d;

	int tempInWires[n];
	int tempOutWires[n];
	int res = 0;
	for (i = 0; i < div; i++) {
		tempOutWires[i] = inputs[i];
	}

	for (i = 1; i < d; i++) {
		for (j = 0; j < div; j++) {
			tempInWires[j] = tempOutWires[j];
			tempInWires[div + j] = inputs[div * i + j];
		}
		res = XORCircuit(gc, garblingContext, 2 * div, tempInWires,
				tempOutWires);
	}
	for (i = 0; i < div; i++) {
		outputs[i] = tempOutWires[i];
	}

	return res;
}

int XORCircuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext,
		int n, int* inputs, int* outputs) {
	int i;
	int internalWire;
	int split = n / 2;
	int res = 0;
	for (i = 0; i < n / 2; i++) {
		internalWire = getNextWire(garblingContext);
		res = XORGate(garbledCircuit, garblingContext, inputs[i],
				inputs[split + i], internalWire);
		outputs[i] = internalWire;
	}
	return res;
}

//http://edipermadi.files.wordpress.com/2008/02/aes_galois_field.jpg

int GF8MULCircuit(GarbledCircuit *garbledCircuit,
		GarblingContext *garblingContext, int n, int* inputs, int* outputs) {

	outputs[0] = inputs[7];
	outputs[2] = inputs[1];
	outputs[3] = getNextWire(garblingContext);
	XORGate(garbledCircuit, garblingContext, inputs[7], inputs[2], outputs[3]);

	outputs[4] = getNextWire(garblingContext);
	XORGate(garbledCircuit, garblingContext, inputs[7], inputs[3], outputs[4]);

	outputs[5] = inputs[4];
	outputs[6] = inputs[5];
	outputs[7] = inputs[6];
	outputs[1] = getNextWire(garblingContext);
	XORGate(garbledCircuit, garblingContext, inputs[7], inputs[0], outputs[1]);

	return 0;
}
