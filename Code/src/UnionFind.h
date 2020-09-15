#ifndef UF_H
#define UF_H

#include "SSEincludes.h"

class UnionFind {
private:
public:
	vi pset;
	void initSet(int _size);
	int findSet(int i);
	void unionSet(int i, int j);
	bool isSameSet(int i, int j);
};

#endif // UF_H