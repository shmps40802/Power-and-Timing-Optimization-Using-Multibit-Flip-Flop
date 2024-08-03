#ifndef LEGALIZATION_H
#define LEGALIZATION_H
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <thread>
#include "Board.h"
#include "CellSpreading.h"
using namespace std;

struct placementRow {
	int startx;
	int starty;
	int siteWidth;
	int siteHeight;
	int numSites;
	placementRow(int sx, int sy, int sw, int sh, int ns)
		: startx(sx), starty(sy), siteWidth(sw), siteHeight(sh), numSites(ns) {}
};
struct bin{
	int x;
	int y;
	double density;
	vector<string> flipflops;
	vector<string> gates;
	bin() {}
	bin(int x, int y)
		: x(x), y(y) {}
};
class Legalization {
private:
	int lowerLeftX;
	int upperRightX;
	int lowerLeftY;
	int upperRightY;
	int BinWidth;
	int BinHeight;
	int BinArea;
	float BinMaxUtil;
	int numBins;
	vector<vector<bin>> bins;
	vector<placementRow> placementRows;
	vector<bin> overfilledBins;
	vector<vector<bin>> targetBins;
public:
	Legalization(Board);
	~Legalization();
	void legalize(Board&);
	void initializeBins();
	void cellToBins(Board&);
	void calculateDensity(Board&);
	void findOverfilledBins();
	void cellSpreading(Board&);
	vector<string> findNearestCells(Board&, bin&, bin&, int);
	void moveCells(Board&, bin&, bin&, vector<string>);
	void parallelLegalization(Board&);
};
#endif
