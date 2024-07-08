#ifndef BOARD_H
#define BOARD_H
#include <vector>
#include <map>
#include <list>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include "Cell.h"
#include "FlipFlop.h"
#include "Gate.h"

struct net{
	string name;
	set<string> points;
	net(string name, set<string> points){
		this->name = name;
		this->points = points;
	}
};
struct row{
	int col;
	Cell* c;
	row(int col, Cell* c){
		this->col = col;
		this->c = c;
	}
};
class Board{
private:
	int Alpha;
	int Beta;
	int Gemma;
	int Delta;
	int LowerLeftX;
	int HigherRightX;
	int LowerLeftY;
	int HigherRightY;
	int NumInput;
	int NumOutput;
	int NumInst;
	int BinWidth;
	int BinHeight;
	float BinMaxUtil;
	int StartX;
	int StartY;
	int SiteWidth;
	int SiteHeight;
	int TotalNumOfSites;
	float DisplacementDelay;
	vector<Point> Input;                            // input pin(s)
	vector<Point> Output;                           // output pin(s)
	map<string, FlipFlop> FlipFlopLib;              // name to FlipFlop
	map<string, Gate> GateLib;                      // name to Gate
	map<string, FlipFlop*> InstFlipFlop;            // inst name to FlipFlop
	map<string, Gate*> InstGate;                    // inst name to Gate
	vector<net> Net;                                // net connection
	map<int, list<row>> Location;                   // location of FlipFlop
	map<pair<int,int>, vector<int>> PlacementRows;  // grid point info
public:
    Board();
	~Board();
    void ReadFile(); // read file
	void Display();
	Point NametoPoint(string);
	vector<FlipFlop> Banking(vector<FlipFlop>);
	vector<FlipFlop> Debanking(FlipFlop);
	bool Check(int, int);
	int ManhattanDist(Point, Point);
	int TNSCost();
	int PowerCost();
	int AreaCost();
	int BinCost();
    int Cost();
};
#endif