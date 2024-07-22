#include <iostream>
#include "Board.h"
#include "Cluster.h"
using namespace std;

int main() {
	Board B;
	B.ReadFile();
	B.Display();
	Cluster C;
	C.kmeans(B);
    return 0;
}
