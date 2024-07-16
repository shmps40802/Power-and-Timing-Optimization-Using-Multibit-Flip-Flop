#ifndef BOARD_H
#define BOARD_H
#include <vector>
#include <set>
#include <map>
#include <unordered_map>
#include "Cell.h"
#include "FlipFlop.h"
#include "Gate.h"

class Board{
private:
	friend class Cluster;
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
	map<string, set<string>> Net;                    // net connection
	map<string, string> PointToNet;                  // point name to net name C1/D -> N1
	map<int, map<int,string>> Location;              // location of FlipFlop
	map<pair<int, int>, vector<int>> PlacementRows;  // grid point info
	set<string> NewFlipFlop;                         // initial FlipFlop
	map<string, string> PrevToCur;                   // previous pin to current pin
	map<int, map<int, float>> BinDensity;            // bin density of board
public:
    Board();
	~Board();
	void ReadFile();                                            // read file
	void Display();
	Point NametoPoint(string);
	void Banking(vector<vector<FlipFlop>>, vector<FlipFlop>);   // only banking 1 bit
	void Debanking(vector<FlipFlop>, vector<vector<FlipFlop>>);                       // only debanking into 1 bit
	void Ddfs(string, map<string, bool>&, float&, float&, int, int, bool&);
	void Qdfs(string, map<string, bool>&, float&, float&, int, int);
	float bankingCompare(vector<FlipFlop>, FlipFlop);
	float singleCompare(FlipFlop, FlipFlop);
	bool Check(int, int);
	int ManhattanDist(Point, Point);
	// cost function
	float TNSCost();
	float PowerCost();
	float AreaCost();
	float BinCost(); // on grid point
	float Cost();
	float getInstsize();
};
#endif
