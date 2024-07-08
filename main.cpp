#include <iostream>
#include <fstream>
#include <string>
#include "Board.h"
using namespace std;

int main() {
	Board B;
	B.ReadFile();
	B.Display();
    return 0;
}