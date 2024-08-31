#ifndef LEGALIZATION_H
#define LEGALIZATION_H
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <thread>
#include <mutex>
#include <functional>
#include <ctime>
#include <tuple>
#include <algorithm>
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
	int siteLLX; //coordinate of the lower left site
	int siteLLY;
	int siteVertical; //number of sites in the vertical direction, 14
	int siteHorizontal; //number of sites in the horizontal direction, 57
	int occupiedArea; //for cell spreading
	int placedArea; //for legalization
	vector<int> flipflops;
	vector<int> gates;
	vector<vector<int>> sites; //0: empty, 1: occupied
	bin() {}
	bin(int x, int y)
		: x(x), y(y), occupiedArea(0), placedArea(0) {}

	bool operator==(const bin& other) const {
		return x == other.x && y == other.y;
	}
	bool operator<(const bin& other) const {
		return tie(x, y) < tie(other.x, other.y);
	}
};
namespace std {
	template <>
	struct hash<bin> {
		size_t operator()(const bin& b) const {
			size_t h1 = hash<int>()(b.x);
			size_t h2 = hash<int>()(b.y);
			size_t h3 = hash<double>()(b.density);

			return h1 ^ (h2 << 1) ^ (h3 << 2);
		}
	};
}

class Legalization {
private:
	int lowerLeftX; //lower left coordinate of the placement region
	int lowerLeftY;
	int upperRightX; //upper right coordinate of the placement region
	int upperRightY;
	int BinWidth;
	int BinHeight;
	int BinArea;
	int siteWidth;
	int siteHeight;
	int siteHorizontal; //number of sites in the horizontal direction
	int siteVertical; //number of sites in the vertical direction
	float BinMaxUtil;
	int placementRowLLX; //coordinate of the lower left site of the placement row
	int placementRowLLY;
	int placementRowURX; //coordinate of the upper right site of the placement row
	int placementRowURY;
	int maxHorizontalDisplacement; //maximum horizontal displacement of the cells (counted in sites)
	int maxVerticalDisplacement; //maximum vertical displacement of the cells (counted in sites)
	int maxFailedHorizontalDisplacement; //maximum failed flipflop horizontal displacement of the cells (counted in sites)
	int maxFailedVerticalDisplacement; //maximum failed flipflop vertical displacement of the cells (counted in sites)
	int maxReplaceNum; //maximum number to replace failed flipflops
	int maxFFPushed; //maximum flipflop number to be pushed 
	long long int totalCellArea;
	vector<vector<bin>> bins;
	vector<placementRow> placementRows;
	vector<reference_wrapper<bin>> overfilledBins;
	vector<vector<reference_wrapper<bin>>> targetBins;
	//vector<pair<reference_wrapper<bin>, int>> placeFailedFlipFlops; //bin, flipflop index
	vector<vector<int>> placeFailedFlipFlops; //flipflop index
	vector<vector<int>> grids; //0: empty, -1: gates, others: flipflop index
	vector<vector<int>> placeCosts; //cost of placing flipflop in the site, integer(0 ~ 8): empty(8 - occupied site next to the site), INT_MAX: can't place(occupied by gates or flipflops, reach bin density constraint)
	mutex mtx;
public:
	Legalization(Board&);
	~Legalization();
	void legalize(Board&);
	void initializeBins();
	void cellToBins(Board&);
	void calculateDensity(Board&);
	void findOverfilledBins();
	void cellSpreading(Board&);
	vector<int> findNearestCells(Board&, bin&, bin&, long long int);
	void moveCells(Board&, bin&, bin&, vector<int>);
	void parallelLegalization(Board&);
	void placeGates(Board&);
	void placeFFs(Board&, vector<reference_wrapper<bin>>&, vector<int>&, int, int, int, int);
	bool isLegal(int, int, int, int, int, int, int, int);
	int calculatePlaceCost(int, int, int, int, int);
	void updateNearCosts(int, int, int, int, int, int, int, int);
	void updateBinDensity(int, int, int, int);


	void legalizationInBin(Board&, bin&);
	bool isLegalInBin(bin& bin, int, int, int, int); // no overlap or out of boundary

	void replaceFailedFFs(Board&, vector<vector<int>>&, vector<int>&, int, int, int, int, int, int, int, bool);
	bool isLegalInRegion(vector<vector<int>>&, int, int, int, int, int, int, int, int); // no overlap or out of boundary
	bool pushAndPlace(Board&, vector<vector<int>>&, vector<tuple<int, int, int>>&, int&, int, int, int, int, int, int, int, int, int);
	bool push(Board&, vector<vector<int>>&, vector<tuple<int, int, int>>&, int&, int, int, int, int, int, int, int);
	bool checkLegal(Board&);
	bool checkSingleFFLegal(Board&, int);
	bool checkSingleGateLegal(Board&, int);
	vector<int> changeFF(Board&, int, int, int);
};
#endif
