#include <iostream>
#include <time.h>
#include "Board.h"
#include "Algorithm.h"
#include "Cluster.h"
#include "Graph.h"
using namespace std;

int main() {
	//auto START = clock();
	Board B;
	Graph g(B);
	Cluster C;
	C.kmeans(B);
	//auto END = clock();
	//cout << (END - START) / CLOCKS_PER_SEC << "s\n";
    return 0;
}
