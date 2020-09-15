#include "UnionFind.h"
//Reference Algorithm written in https://www.comp.nus.edu.sg/~stevenha/myteaching/competitive_programming/cp1.pdf
void UnionFind::initSet(int _size) { pset.resize(_size); REP (i, 0, _size - 1) pset[i] = i; }
int UnionFind::findSet(int i) { return (pset[i] == i) ? i : (pset[i] = findSet(pset[i])); }
void UnionFind::unionSet(int i, int j) { pset[findSet(i)] = findSet(j); }
bool UnionFind::isSameSet(int i, int j) { return findSet(i) == findSet(j); }