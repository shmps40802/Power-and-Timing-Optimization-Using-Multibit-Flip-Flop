#include "Legalization.h"

Legalization::Legalization(Board board) {
	lowerLeftX = board.LowerLeftX;
	upperRightX = board.HigherRightX;
	lowerLeftY = board.LowerLeftY;
	upperRightY = board.HigherRightY;
	BinWidth = board.BinWidth;
	BinHeight = board.BinHeight;
	BinArea = BinWidth * BinHeight;
	BinMaxUtil = board.BinMaxUtil / 100;
	for (auto& it : board.PlacementRows) {
		placementRows.push_back(placementRow(it.first.first, it.first.second, it.second.at(0), it.second.at(1), it.second.at(2)));
	}
}
Legalization::~Legalization() {}
void Legalization::legalize(Board& board) {
		initializeBins();
		cellToBins(board);
		findOverfilledBins();
		cellSpreading(board);
		parallelLegalization(board);
}
void Legalization::initializeBins() {
	int numBinsHorizontal = (upperRightX - lowerLeftX) / BinWidth;
	int numBinsVertical = (upperRightY - lowerLeftY) / BinHeight;
	bins.resize(numBinsHorizontal, vector<bin>(numBinsVertical));
	for(int i = 0; i < numBinsHorizontal; i++) {
		for(int j = 0; j < numBinsVertical; j++) {
			bins.at(i).at(j) = bin(i * BinWidth, j * BinHeight);
		}
	}
}
void Legalization::cellToBins(Board& board) {
	int numBinsHorizontal = (upperRightX - lowerLeftX) / BinWidth;
	int numBinsVertical = (upperRightY - lowerLeftY) / BinHeight;
	for (auto& it : board.InstToFlipFlop) {
		int x = it.second.getX();
		int y = it.second.getY();
		int binX = (x - lowerLeftX) / BinWidth;
		int binY = (y - lowerLeftY) / BinHeight;
		if(binX >= numBinsHorizontal) binX = numBinsHorizontal - 1;
		if(binY >= numBinsVertical) binY = numBinsVertical - 1;
		bins.at(binX).at(binY).flipflops.push_back(it.first);
	}
	for (auto& it : board.InstToGate) {
		int x = it.second.getX();
		int y = it.second.getY();
		int binX = (x - lowerLeftX) / BinWidth;
		int binY = (y - lowerLeftY) / BinHeight;
		if(binX >= numBinsHorizontal) binX = numBinsHorizontal - 1;
		if(binY >= numBinsVertical) binY = numBinsVertical - 1;
		bins.at(binX).at(binY).gates.push_back(it.first);
	}
	calculateDensity(board);
}
void Legalization::calculateDensity(Board& board) {
	for (auto& it : bins) {
		for (auto& it2 : it) {
			int area = 0;
			for (auto& flipflop : it2.flipflops) {
				area += board.InstToFlipFlop.at(flipflop).getArea();
			}
			for (auto& gate : it2.gates) {
				area += board.InstToGate.at(gate).getArea();
			}
			it2.density = (double)area / BinArea;
		}
	}
}
void Legalization::findOverfilledBins() {
	int numBinsHorizontal = (upperRightX - lowerLeftX) / BinWidth;
	int numBinsVertical = (upperRightY - lowerLeftY) / BinHeight;
	for (int i = 0; i < numBinsHorizontal; i++) {
		for (int j = 0; j < numBinsVertical; j++) {
			if (bins.at(i).at(j).density >= BinMaxUtil) {
				overfilledBins.push_back(bins.at(i).at(j));
				vector<bin> temp;
				if (i != 0 && bins.at(i - 1).at(j).density < BinMaxUtil) {
					temp.push_back(bins.at(i - 1).at(j));
				}
				if (i != numBinsHorizontal - 1 && bins.at(i + 1).at(j).density < BinMaxUtil) {
					temp.push_back(bins.at(i + 1).at(j));
				}
				if (j != 0 && bins.at(i).at(j - 1).density < BinMaxUtil) {
					temp.push_back(bins.at(i).at(j - 1));
				}
				if (j != numBinsVertical - 1 && bins.at(i).at(j + 1).density < BinMaxUtil) {
					temp.push_back(bins.at(i).at(j + 1));
				}
				targetBins.push_back(temp);
			}
		}
	}
}
void Legalization::cellSpreading(Board& board) {
	set<bin> targetBinsInNetwork; //set of target bins that are in the network
	for (auto& it : targetBins) {
		for (auto& it2 : it) {
			targetBinsInNetwork.insert(it2);
		}
	}
	unordered_map<int, bin> indexToBin;
	unordered_map<bin, int> binToIndex;
	if (targetBinsInNetwork.size() > 0) {
		int index = 2;
		//index 0 is super source, index 1 is super target
		for (auto& it : overfilledBins) {
			indexToBin[index] = it;
			binToIndex[it] = index;
			index++;
		}
		for (auto& it : targetBinsInNetwork) {
			indexToBin[index] = it;
			binToIndex[it] = index;
			index++;
		}

		CellSpreading cs(index, 0, 1);
		//budgeting
		//first two columns
		for (auto& it : overfilledBins) {
			int capacity = (it.density - BinMaxUtil + 0.01) * BinArea; //movedOutArea
			int cost = 0;
			cs.addedge(0, binToIndex[it], capacity, cost);
		}
		//middle two columns
		for (int i = 0; i < overfilledBins.size(); i++) {
			for (auto& it : targetBins[i]) {
				int capacity = INT_MAX;
				int cost = abs(overfilledBins[i].x - it.x) + abs(overfilledBins[i].y - it.y); //bin-wise displacement
				cs.addedge(binToIndex[overfilledBins[i]], binToIndex[it], capacity, cost);
			}
		}
		//last two columns
		for (auto& it : targetBinsInNetwork) {
			int capacity = (BinMaxUtil-it.density) * BinArea; //free space
			int cost = 0;
			cs.addedge(binToIndex[it], 1, capacity, cost);
		}

		cs.MCMF();

		//boundry cell moving
		for (int i = 0; i < overfilledBins.size(); i++) {
			for (auto& it : targetBins[i]) {
				int movedArea = cs.getFlow(binToIndex[overfilledBins[i]], binToIndex[it]);
				if (movedArea > 0) {
					vector<string> names = findNearestCells(board, overfilledBins[i], it, movedArea);
					moveCells(board, overfilledBins[i], it, names);
				}
			}
		}
	}
}
vector<string> Legalization::findNearestCells(Board& board, bin& from, bin& to, int targetArea) {
	int targetBinX = to.x + BinWidth / 2;
	int targetBinY = to.y + BinHeight / 2;

	struct cell {
		string name;
		int area;
		int distance;
	};

	vector<cell> cells;
	for (auto& it : from.flipflops) {
		int x = board.InstToFlipFlop.at(it).getX();
		int y = board.InstToFlipFlop.at(it).getY();
		int width = board.InstToFlipFlop.at(it).getWidth();
		int height = board.InstToFlipFlop.at(it).getHeight();
		string name = board.InstToFlipFlop.at(it).getInstName();
		int area = board.InstToFlipFlop.at(it).getArea();
		int distance = abs(targetBinX - (x + width / 2)) + abs(targetBinY - (y + height / 2));
		cells.push_back({ name,area,distance });
	}
	sort(cells.begin(), cells.end(), [](const cell& a, const cell& b) {
		return a.distance < b.distance;
		});
	int area = 0;
	vector<string> nearestCells;
	for (const auto& it : cells) {
		nearestCells.push_back(it.name);
		area += it.area;
		if (area + it.area >= targetArea) {
			break;
		}
	}

	return nearestCells;
}
void Legalization::moveCells(Board& board, bin& from, bin& to, vector<string> names) {
	for (auto& it : from.flipflops) {
		if (find(names.begin(), names.end(), it) != names.end()) {
			int x = board.InstToFlipFlop.at(it).getX();
			int y = board.InstToFlipFlop.at(it).getY();
			int targetBinX1 = to.x; //target bin's lower left corner
			int targetBinY1 = to.y;
			int targetBinX2 = to.x + BinWidth; //target bin's upper right corner
			int targetBinY2 = to.y + BinHeight;
			x = min(max(x, targetBinX1), targetBinX2);
			y = min(max(y, targetBinY1), targetBinY2);
			board.InstToFlipFlop.at(it).setPos(x, y); //move cell to the closest position inside the target bin
			from.flipflops.erase(remove(from.flipflops.begin(), from.flipflops.end(), it), from.flipflops.end());
			to.flipflops.push_back(it);
		}
	}
}
void Legalization::parallelLegalization(Board& board) {

}
