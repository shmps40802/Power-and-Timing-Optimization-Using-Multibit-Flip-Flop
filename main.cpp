#include <iostream>
#include <time.h>
#include "Board.h"
#include "Cluster.h"
#include "Graph.h"
#include "Legalization.h"
using namespace std;

int main(int argc, char* argv[]) {
	auto START = clock();

	auto START2 = clock();
	//string inputFile = *++argv;
	//string outputFile = *++argv;
	string inputFile = "testcase1.txt";
	string outputFile = "output1.txt";
	Board B(inputFile, outputFile);

	auto END = clock();
	cout << "read file: " << (END - START2) / CLOCKS_PER_SEC << "s" << endl;

	START2 = clock();
	Graph g(B);
	END = clock();
	cout << "build graph: " << (END - START2) / CLOCKS_PER_SEC << "s" << endl;


	cout << "Total Cost " << B.Cost() << "\n";
	cout << "Power : " << B.PowerCost() << "\n";
	cout << "Area : " << B.AreaCost() << "\n";
	cout << "TNS " << B.TNSCost() << "\n";
	cout << "Bin " << B.BinCost() << "\n";


	START2 = clock();
	Cluster C;
	C.kmeans(B);
	END = clock();
	cout << "cluster: " << (END - START2) / CLOCKS_PER_SEC << "s" << endl;

	START2 = clock();
	Legalization le(B);
	le.legalize(B);
	END = clock();
	cout << "legalize: " << (END - START2) / CLOCKS_PER_SEC << "s" << endl;

	B.outputFile();
	END = clock();
	cout << "total time: " << (END - START) / CLOCKS_PER_SEC << "s" << endl;

	cout << "Total Cost " << B.Cost() << "\n";
	cout << "Power : " << B.PowerCost() << "\n";
	cout << "Area : " << B.AreaCost() << "\n";
	cout << "TNS " << B.TNSCost() << "\n";
	cout << "Bin " << B.BinCost() << "\n";
	B.Plot();
	return 0;
}