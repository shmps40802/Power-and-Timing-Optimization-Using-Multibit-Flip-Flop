#ifndef BOARD_H
#define BOARD_H
#include <vector>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <ctime>
#include <climits>
#include <cfloat>
#include "Cell.h"
#include "FlipFlop.h"
#include "Gate.h"

struct node {
	int sx;
	int ex;
	int sy;
	int ey;
	int index;
	node() : sx(INT_MIN), ex(INT_MAX), sy(INT_MIN), ey(INT_MAX), index(-1) {}
	node(int sx, int ex, int sy, int ey, int index)
	: sx(sx), ex(ex), sy(sy), ey(ey), index(index) {}
	node(int index) :sx(INT_MIN), ex(INT_MAX), sy(INT_MIN), ey(INT_MAX), index(index) {}
	bool operator<(node n) const {
		if (index < 0) return false;
		else if (n.index < 0) return true;
		if (sx != n.sx)return sx < n.sx;
		else return ex < n.ex;
	}
};
class Board {
private:
	friend class Cluster;
	friend class Legalization;
	friend class G;
	float Alpha;
	float Beta;
	float Gemma;
	float Delta;
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
	int minBit;
	int maxBit;
	vector<Point> Input;                               // input pin(s)
	vector<Point> Output;                              // output pin(s)
	map<int, FlipFlop> FlipFlopLib;                    // Cellnumber to FlipFlop
	map<int, Gate> GateLib;                            // Cellnumber to Gate
	unordered_map<int, FlipFlop> InstToFlipFlop;       // instnumber to FlipFlop
	unordered_map<int, Gate> InstToGate;               // instnumber to Gate
	vector<vector<int>> FlipFlopByClk;                 // flipflop(s) with the same clk
	unordered_map<int, unordered_set<string>> Net;        // netnumer to point name
	unordered_map<string, int> PointToNet;                // point name to net number C1/D -> (net)1
	map<int, map<int, vector<int>>> Location;          // location of (inst)number
	map<pair<int, int>, vector<int>> PlacementRows;    // grid point info
	set<int> NewFlipFlop;                              // new number (of FlipFlop)
	map<int, map<int, float>> BinDensity;              // bin density of board
	unordered_map<string, unordered_set<string>> Net2; // 
	map<string, float> DWL;                            // acceptable WL for each D pin
public:
	Board();
	~Board();
	void ReadFile(void);                                      // read file
	void Display(void);
	void outputFile(void);
	void Plot();
	Point NametoPoint(string);
	Cell getCell(int);                                     // get FlipFlop Gate
	void Ddfs(string, float&, int, int);
	void Qdfs(string, map<string, bool>&, int, float&, int, int, float);
	void updateDSlack(string, float&, int, int);
	void updateQSlack(string, map<string, bool>&, float, int, int, float);
	void Banking(vector<FlipFlop>, FlipFlop&);             // only banking 1 bit
	void Debanking(FlipFlop, vector<FlipFlop>&);           // only debanking into 1 bit
	float bankingCompare(vector<FlipFlop>, FlipFlop);
	float singleCompare(FlipFlop, FlipFlop);
	bool Check();
	int dist(string, string);
	// cost function
	float TNSCost();
	float PowerCost();
	float AreaCost();
	float BinCost(); // on grid point
	float Cost();
	int getInstsize();
	void addWL(string, map<string, bool>&, float, bool);
	void merge(vector<node>&, int, int, int);
	void mergeSort(vector<node>&, int, int);
	void addNeighbor(string, string);
};
#endif
