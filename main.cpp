#include <iostream>
#include <time.h>
#include "Board.h"
#include "Cluster.h"
#include "Graph.h"
#include "Legalization.h"
using namespace std;

int main() {
	//auto START = clock();
	Board B;
	Graph g(B);
	Cluster C;
	C.kmeans(B);
	Legalization le(B);
	le.legalize(B);
	B.outputFile();
	//auto END = clock();
	//cout << (END - START) / CLOCKS_PER_SEC << "s\n";
	return 0;
}
