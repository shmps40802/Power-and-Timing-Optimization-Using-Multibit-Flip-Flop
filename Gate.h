#ifndef GATE_H
#define GATE_H
#include <iostream>
#include <string>
#include "Cell.h"
using namespace std;
class Gate : public Cell {
private:
    string Name;
public:
    Gate();
    Gate(int, int, int, vector<Point>);
};
#endif
