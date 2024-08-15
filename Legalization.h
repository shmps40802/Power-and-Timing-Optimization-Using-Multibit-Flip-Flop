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
	int siteLLX; //coordinate of the lower left site
	int siteLLY;
	int siteVertical; //number of sites in the vertical direction, 14
	int siteHorizontal; //number of sites in the horizontal direction, 57
	vector<int> flipflops;
	vector<int> gates;
	vector<vector<int>> sites; //0: empty, 1: occupied
	bin() {}
	bin(int x, int y)
		: x(x), y(y) {}

	bool operator==(const bin& other) const {
		return x == other.x && y == other.y;
	}
	bool operator<(const bin& other) const {
		return tie(x, y) < tie(other.x, other.y);
	}
};
template <>
struct hash<bin> {
	size_t operator()(const bin& b) const {
		size_t h1 = hash<int>()(b.x);
		size_t h2 = hash<int>()(b.y);
		size_t h3 = hash<double>()(b.density);

		return h1 ^ (h2 << 1) ^ (h3 << 2);
	}
};

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
	vector<vector<bin>> bins;
	vector<placementRow> placementRows;
	vector<bin&> overfilledBins;
	vector<vector<bin&>> targetBins;
	vector<pair<bin&, int>> placeFailedFlipFlops; //bin, flipflop index
	vector<vector<bool>> grids; //0: empty, 1: occupied
public:
	Legalization(Board);
	~Legalization();
	void legalize(Board&);
	void initializeBins();
	void cellToBins(Board&);
	void calculateDensity(Board&);
	void findOverfilledBins();
	void cellSpreading(Board&);
	vector<int> findNearestCells(Board&, bin&, bin&, int);
	void moveCells(Board&, bin&, bin&, vector<int>);
	void parallelLegalization(Board&);
	void legalizationInBin(Board&, bin&);
	bool isLegalInBin(bin& bin, int, int, int, int); // no overlap or out of boundary
	void replaceFailedFFs(Board&);
	bool isLegalInRegion(int, int, int, int); // no overlap or out of boundary
};
#endif
