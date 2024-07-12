#ifndef BOARD_H
#define BOARD_H
#include <vector>
#include <map>
#include <list>
#include <set>
#include <map>
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
	string name;
	row(int col, string name){
		this->col = col;
		this->name = name;
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
	int CellNumber;
	vector<Point> Input;                             // input pin(s)
	vector<Point> Output;                            // output pin(s)
	map<string, FlipFlop> FlipFlopLib;               // name to FlipFlop
	map<string, Gate> GateLib;                       // name to Gate
	map<string, FlipFlop> InstToFlipFlop;            // inst name to FlipFlop
	map<string, Gate> InstToGate;                    // inst name to Gate
	vector<net> Net;                                 // net connection
	map<int, list<row>> Location;                    // location of FlipFlop
	map<pair<int, int>, vector<int>> PlacementRows;  // grid point info
	set<string> NewFlipFlop;                         // initial FlipFlop
	map<string, string> PrevToCur;                   // 
	map<string, string> CurToPrev;                   //
public:
    Board();
	~Board();
    void ReadFile(); // read file
	void Display();
	Point NametoPoint(string);
	vector<FlipFlop> Banking(vector<FlipFlop>);  // only banking 1 bit
	vector<FlipFlop> Debanking(FlipFlop);        // only debanking into 1 bit
	FlipFlop getFlipFlop(int, int, int);         //
	bool Check(int, int);
	int ManhattanDist(Point, Point);
	int TNSCost();
	int PowerCost();
	int AreaCost();
	int BinCost();
    int Cost();
};
#endif
