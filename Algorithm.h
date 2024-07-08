#ifndef ALGORITHM_H
#define ALGORITHM_H
#include <iostream>
#include <vector>
#include "Board.h"
using namespace std;
struct p{
    bool type;  // start or end
    int index;  // #flipflop
    p(bool type, int index){
        this->type = type;
        this->index = index;
    }
};
map<string, string> ;
class Cluster{
private:

public:
};
class G{
private:
    vector<p> array;
    map<vector<FlipFlop>, vector<FlipFlop>> Result;
public:
    void FeasibleRegion(vector<FlipFlop>, vector<net> Net);
    void FileOut();
};
#endif