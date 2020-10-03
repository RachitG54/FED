
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

#ifndef CIRCUITS_H_
#define CIRCUITS_H_

int SHLCircuit(GarbledCircuit *gc, GarblingContext *garblingContext, int n, int* inputs, int* outputs);
int SHRCircuit(GarbledCircuit *gc, GarblingContext *garblingContext, int n, int* inputs, int* outputs);
int MULCircuit(GarbledCircuit *gc, GarblingContext *garblingContext, int n, int* inputs, int* outputs);

int XORCircuit(GarbledCircuit *gc, GarblingContext *garblingContext, int n, int* inputs, int* outputs);

int NOTCircuit(GarbledCircuit *gc, GarblingContext *garblingContext, int n, int* inputs, int* outputs);
int ANDCircuit(GarbledCircuit *gc, GarblingContext *garblingContext, int n, int* inputs, int* outputs);
int MIXEDCircuit(GarbledCircuit *gc, GarblingContext *garblingContext, int n, int* inputs, int* outputs);
int INCCircuit(GarbledCircuit *gc, GarblingContext *garblingContext, int n, int* inputs, int* outputs);
int ORCircuit(GarbledCircuit *gc, GarblingContext *garblingContext, int n, int* inputs, int* outputs);
int EQUCircuit(GarbledCircuit *gc, GarblingContext *garblingContext, int n, int* inputs, int* outputs);
int LEQCircuit(GarbledCircuit *gc, GarblingContext *garblingContext, int n, int* inputs, int* outputs);
int GEQCircuit(GarbledCircuit *gc, GarblingContext *garblingContext, int n, int* inputs, int* outputs);
int LESCircuit(GarbledCircuit *gc, GarblingContext *garblingContext, int n, int* inputs, int* outputs);
int GRECircuit(GarbledCircuit *gc, GarblingContext *garblingContext, int n, int* inputs, int* outputs);
int XORCircuit(GarbledCircuit *gc, GarblingContext *garblingContext, int n, int* inputs, int* outputs);
int MultiXORCircuit(GarbledCircuit *gc, GarblingContext *garblingContext, int d, int n, int* inputs, int* outputs);

int MINCircuit(GarbledCircuit *gc, GarblingContext *garblingContext, int n, int* inputs, int* outputs);


int SBOXNOTABLE(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int* inputs, int* outputs);
int AddRoundKey(GarbledCircuit *gc, GarblingContext *garblingContext, int* inputs, int* outputs);
int SubBytes(GarbledCircuit *gc, GarblingContext *garblingContext, int* inputs, int* outputs);
int SubBytesTable(GarbledCircuit *gc, GarblingContext *garblingContext, int* inputs, int* outputs);
int ShiftRows(GarbledCircuit *gc, GarblingContext *garblingContext, int* inputs, int* outputs);
int MixColumns(GarbledCircuit *gc, GarblingContext *garblingContext, int* inputs, int* outputs);
int ADDCircuit(GarbledCircuit *gc, GarblingContext *garblingContext, int n, int* inputs, int* outputs);
int SUBCircuit(GarbledCircuit *gc, GarblingContext *garblingContext, int n, int* inputs, int* outputs);
int ADD32Circuit(GarbledCircuit *gc, GarblingContext *garblingContext, int* inputs, int* outputs);
int ADD22Circuit(GarbledCircuit *gc, GarblingContext *garblingContext, int* inputs, int* outputs);
int MULTE_GF16(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int* inputs, int* outputs);
int INV_GF16(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int* inputs, int* outputs);
int AFFINE(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int* inputs, int* outputs);
int SBOX(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int* inputs, int* outputs);
int INVMAP(GarbledCircuit *gc, GarblingContext *garblingContext, int* inputs, int* outputs);
int GF8MULCircuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int n, int* inputs, int* outputs);

int EncoderCircuit(GarbledCircuit *gc, GarblingContext *garblingContext,  int* inputs, int* outputs, int enc[]);
int GF4MULCircuit(GarbledCircuit *gc, GarblingContext *garblingContext,  int* inputs, int* outputs);
int GF4SQCircuit(GarbledCircuit *gc, GarblingContext *garblingContext,  int* inputs, int* outputs);
int GF4SCLNCircuit(GarbledCircuit *gc, GarblingContext *garblingContext,  int* inputs, int* outputs);
int GF4SCLN2Circuit(GarbledCircuit *gc, GarblingContext *garblingContext,  int* inputs, int* outputs);

int RANDCircuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int n, int* inputs, int* outputs, int q, int qf);

//Genomic Data defined circuits
int SHR_k_Circuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int n, int k, int* inputs, int* outputs);
int AND_B_k_Circuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int n, int k, int* inputs, int*  outputs);
int HammingCircuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int n, int* inputs, int*  outputs);
int SETZERO_bit_Circuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int n, int wire, int* outputs);
int ADDN1_Circuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int n, int* inputs, int wire, int* outputs);
int HammingCircuit2(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int n, int* inputs, int* outputs);
int GenomeCircuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int n, int colbits, int* inputs, int* outputs);
int GenomeCircuit2(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int n, int colbits, int Tsize, int noofcols, int* inputs, int* outputs);
int HammingCircuit2add(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int n, int m, int* inputs, int* outputs);
int LEQCircuit2(GarbledCircuit *gc, GarblingContext *garblingContext, int n, int* inputs, int* outputs);

#endif /* CIRCUITS_H_ */

