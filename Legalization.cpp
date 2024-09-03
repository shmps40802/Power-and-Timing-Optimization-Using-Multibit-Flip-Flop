#include "Legalization.h"
#include <fstream>

Legalization::Legalization(Board& board) {
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
	siteWidth = placementRows.at(0).siteWidth; //510
	siteHeight = placementRows.at(0).siteHeight; //2100
	placementRowLLX = placementRows.at(0).startx; //15300
	placementRowLLY = placementRows.at(0).starty; //16800
	placementRowURX = placementRows.at(0).startx + placementRows.at(0).siteWidth * placementRows.at(0).numSites; //1284690
	placementRowURY = placementRows.at(placementRows.size() - 1).starty + placementRows.at(placementRows.size() - 1).siteHeight; //1285200
	maxHorizontalDisplacement = 100;
	maxVerticalDisplacement = 50;
	maxFailedHorizontalDisplacement = 150;
	maxFailedVerticalDisplacement = 50;
	maxReplaceNum = 1;
	siteHorizontal = placementRows.at(0).numSites;
	siteVertical = placementRows.size();
	grids.resize(siteHorizontal, vector<int>(siteVertical, 0));
	placeCosts.resize(siteHorizontal, vector<int>(siteVertical, 8));
	maxFFPushed = 100;
}
Legalization::~Legalization() {}
void Legalization::legalize(Board& board) {
	//if (checkLegal(board)) {
	//	cout << "The placement is legal." << endl;
	//}
	//else {
	//	cout << "The placement is illegal." << endl;
	//}

	initializeBins();
	cellToBins(board);
	findOverfilledBins();
	cellSpreading(board);
	parallelLegalization(board);

	//if (checkLegal(board)) {
	//	cout << "The placement is legal." << endl;
	//}
	//else {
	//	cout << "The placement is illegal." << endl;
	//}
}
void Legalization::initializeBins() {
	int numBinsHorizontal = (upperRightX - lowerLeftX) / BinWidth;
	int numBinsVertical = (upperRightY - lowerLeftY) / BinHeight;
	bins.resize(numBinsHorizontal, vector<bin>(numBinsVertical));
	for(int i = 0; i < numBinsHorizontal; i++) {
		for(int j = 0; j < numBinsVertical; j++) {
			bins.at(i).at(j) = bin(i * BinWidth, j * BinHeight);
			if (bins.at(i).at(j).x <= placementRowLLX) {
				bins.at(i).at(j).siteLLX = placementRowLLX;
			}
			else {
				int siteBefore = (bins.at(i).at(j).x - placementRowLLX + siteWidth - 1) / siteWidth;
				bins.at(i).at(j).siteLLX = placementRowLLX + siteBefore * siteWidth;
			}
			if (bins.at(i).at(j).x + BinWidth >= placementRowURX) {
				bins.at(i).at(j).siteHorizontal = (placementRowURX - bins.at(i).at(j).siteLLX) / siteWidth;
			}
			else {
				bins.at(i).at(j).siteHorizontal = (bins.at(i).at(j).x + BinWidth - bins.at(i).at(j).siteLLX) / siteWidth;
			}
			if (bins.at(i).at(j).y <= placementRowLLY) {
				bins.at(i).at(j).siteLLY = placementRowLLY;
			}
			else {
				int siteBefore = (bins.at(i).at(j).y - placementRowLLY + siteHeight - 1) / siteHeight;
				bins.at(i).at(j).siteLLY = placementRowLLY + siteBefore * siteHeight;
			}
			if (bins.at(i).at(j).y + BinHeight >= placementRowURY) {
				bins.at(i).at(j).siteVertical = (placementRowURY - bins.at(i).at(j).siteLLY) / siteHeight;
			}
			else {
				bins.at(i).at(j).siteVertical = (bins.at(i).at(j).y + BinHeight - bins.at(i).at(j).siteLLY) / siteHeight;
			}
			bins.at(i).at(j).sites.resize(bins.at(i).at(j).siteHorizontal, vector<int>(bins.at(i).at(j).siteVertical));
		}
	}
}
void Legalization::cellToBins(Board& board) {
	totalCellArea = 0;
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
		bins.at(binX).at(binY).occupiedArea += it.second.getArea();
		totalCellArea += it.second.getArea();
	}
	for (auto& it : board.InstToGate) {
		int x = it.second.getX();
		int y = it.second.getY();
		int width = it.second.getWidth();
		int height = it.second.getHeight();
		int LLBinX = (x - lowerLeftX) / BinWidth;
		int LLBinY = (y - lowerLeftY) / BinHeight;
		int URBinX = (x + width - lowerLeftX) / BinWidth;
		int URBinY = (y + height - lowerLeftY) / BinHeight;
		totalCellArea += it.second.getArea();
		if (LLBinX >= numBinsHorizontal) LLBinX = numBinsHorizontal - 1;
		if (LLBinY >= numBinsVertical) LLBinY = numBinsVertical - 1;
		if (URBinX >= numBinsHorizontal) URBinX = numBinsHorizontal - 1;
		if (URBinY >= numBinsVertical) URBinY = numBinsVertical - 1;
		if (LLBinX == URBinX && LLBinY == URBinY) {
			bins.at(LLBinX).at(LLBinY).gates.push_back(it.first);
			bins.at(LLBinX).at(LLBinY).occupiedArea += it.second.getArea();
		}
		else if (LLBinX == URBinX && LLBinY < URBinY) {
			bins.at(LLBinX).at(LLBinY).gates.push_back(it.first);
			bins.at(LLBinX).at(LLBinY).occupiedArea += it.second.getArea() / 2;
			bins.at(LLBinX).at(URBinY).gates.push_back(it.first);
			bins.at(LLBinX).at(URBinY).occupiedArea += it.second.getArea() / 2;
		}
		else if (LLBinX < URBinX && LLBinY == URBinY) {
			bins.at(LLBinX).at(LLBinY).gates.push_back(it.first);
			bins.at(LLBinX).at(LLBinY).occupiedArea += it.second.getArea() / 2;
			bins.at(URBinX).at(LLBinY).gates.push_back(it.first);
			bins.at(URBinX).at(LLBinY).occupiedArea += it.second.getArea() / 2;
		}
		else {
			bins.at(LLBinX).at(LLBinY).gates.push_back(it.first);
			bins.at(LLBinX).at(LLBinY).occupiedArea += it.second.getArea() / 4;
			bins.at(LLBinX).at(URBinY).gates.push_back(it.first);
			bins.at(LLBinX).at(URBinY).occupiedArea += it.second.getArea() / 4;
			bins.at(URBinX).at(LLBinY).gates.push_back(it.first);
			bins.at(URBinX).at(LLBinY).occupiedArea += it.second.getArea() / 4;
			bins.at(URBinX).at(URBinY).gates.push_back(it.first);
			bins.at(URBinX).at(URBinY).occupiedArea += it.second.getArea() / 4;
		}
	}
	calculateDensity(board);
}
void Legalization::calculateDensity(Board& board) {
	for (auto& it : bins) {
		for (auto& it2 : it) {
			//int area = 0;
			//for (auto& flipflop : it2.flipflops) {
			//	area += board.InstToFlipFlop.at(flipflop).getArea();
			//}
			//for (auto& gate : it2.gates) {
			//	area += board.InstToGate.at(gate).getArea();
			//}
			it2.density = (double)it2.occupiedArea / BinArea;
		}
	}
}
void Legalization::findOverfilledBins() {
	long double totalPlacementArea = (double)siteWidth * siteHeight * siteHorizontal * siteVertical;
	double averageDensity = totalCellArea / totalPlacementArea;
	//cout << "averageDensity: " << averageDensity << endl;
	if (averageDensity >= 0.8 && averageDensity <= 0.9) {
		if (BinMaxUtil > averageDensity + 0.05) {
			BinMaxUtil = averageDensity + 0.05;
		}
	}
	else if(averageDensity >= 0.7 && averageDensity < 0.8) {
		if (BinMaxUtil > averageDensity + 0.1) {
			BinMaxUtil = averageDensity + 0.1;
		}
	}
	else if(averageDensity >= 0.6 && averageDensity < 0.7) {
		if (BinMaxUtil > averageDensity + 0.15) {
			BinMaxUtil = averageDensity + 0.15;
		}
	}
	else if(averageDensity >= 0.5 && averageDensity < 0.6) {
		if (BinMaxUtil > averageDensity + 0.2) {
			BinMaxUtil = averageDensity + 0.2;
		}
	}
	else if (averageDensity < 0.5) {
		if (BinMaxUtil > averageDensity + 0.25) {
			BinMaxUtil = averageDensity + 0.25;
		}
	}
	int numBinsHorizontal = (upperRightX - lowerLeftX) / BinWidth;
	int numBinsVertical = (upperRightY - lowerLeftY) / BinHeight;
	for (int i = 0; i < numBinsHorizontal; i++) {
		for (int j = 0; j < numBinsVertical; j++) {
			if (bins.at(i).at(j).density >= BinMaxUtil) {
				overfilledBins.push_back(bins.at(i).at(j));
				int requiredArea = (bins.at(i).at(j).density - BinMaxUtil + 0.01) * BinArea;
				vector<reference_wrapper<bin>> temp;
				int radius = 1;
				while (requiredArea > 0) {
					//choose bins with manhattan distance equal to radius
					for (int x = -radius; x <= radius; x++) {
						int y = radius - abs(x);
						if (i + x >= 0 && i + x < numBinsHorizontal && j + y >= 0 && j + y < numBinsVertical && bins.at(i + x).at(j + y).density < BinMaxUtil) {
							temp.push_back(bins.at(i + x).at(j + y));
							requiredArea -= (BinMaxUtil - bins.at(i + x).at(j + y).density) * BinArea;
						}
						if (abs(x) != radius) {
							y = -radius + abs(x);
							if (i + x >= 0 && i + x < numBinsHorizontal && j + y >= 0 && j + y < numBinsVertical && bins.at(i + x).at(j + y).density < BinMaxUtil) {
								temp.push_back(bins.at(i + x).at(j + y));
								requiredArea -= (BinMaxUtil - bins.at(i + x).at(j + y).density) * BinArea;
							}
						}
					}
					radius++;
				}
				/*
				if (i != 0 && bins.at(i - 1).at(j).density < BinMaxUtil) {
					temp.push_back(bins.at(i - 1).at(j));
					requiredArea -= (BinMaxUtil - bins.at(i - 1).at(j).density) * BinArea;
				}
				if (i != numBinsHorizontal - 1 && bins.at(i + 1).at(j).density < BinMaxUtil) {
					temp.push_back(bins.at(i + 1).at(j));
					requiredArea -= (BinMaxUtil - bins.at(i + 1).at(j).density) * BinArea;
				}
				if (j != 0 && bins.at(i).at(j - 1).density < BinMaxUtil) {
					temp.push_back(bins.at(i).at(j - 1));
					requiredArea -= (BinMaxUtil - bins.at(i).at(j - 1).density) * BinArea;
				}
				if (j != numBinsVertical - 1 && bins.at(i).at(j + 1).density < BinMaxUtil) {
					temp.push_back(bins.at(i).at(j + 1));
					requiredArea -= (BinMaxUtil - bins.at(i).at(j + 1).density) * BinArea;
				}
				*/
				targetBins.push_back(temp);
			}
		}
	}
}
void Legalization::cellSpreading(Board& board) {
	set<bin> targetBinsInNetwork; //set of target bins that are in the network
	for (auto& it : targetBins) {
		for (auto& it2 : it) {
			bin temp = it2.get();
			targetBinsInNetwork.insert(temp);
		}
	}
	unordered_map<int, bin> indexToBin;
	unordered_map<bin, int> binToIndex;
	if (targetBinsInNetwork.size() > 0) {
		int index = 2;
		//index 0 is super source, index 1 is super target
		for (auto& it : overfilledBins) {
			bin temp = it.get();
			indexToBin[index] = temp;
			binToIndex[temp] = index;
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
			int capacity = (it.get().density - BinMaxUtil + 0.01) * BinArea; //movedOutArea
			int cost = 0;
			cs.addedge(0, binToIndex[it.get()], capacity, cost);
		}
		//middle two columns
		for (int i = 0; i < overfilledBins.size(); i++) {
			for (auto& it : targetBins[i]) {
				int capacity = INT_MAX;
				int cost = abs(overfilledBins[i].get().x - it.get().x) + abs(overfilledBins[i].get().y - it.get().y); //bin-wise displacement
				cs.addedge(binToIndex[overfilledBins[i].get()], binToIndex[it.get()], capacity, cost);
			}
		}
		//last two columns
		for (auto& it : targetBinsInNetwork) {
			int capacity = (BinMaxUtil-it.density) * BinArea; //free space
			int cost = 0;
			cs.addedge(binToIndex[it], 1, capacity, cost);
		}

		cs.MCMF();
		//cs.printFlows();
		//boundry cell moving
		for (int i = 0; i < overfilledBins.size(); i++) {
			for (auto& it : targetBins[i]) {
				long long int movedArea = cs.getFlow(binToIndex[overfilledBins[i].get()], binToIndex[it.get()]);
				if (movedArea > 0) {
					vector<int> names = findNearestCells(board, overfilledBins[i].get(), it.get(), movedArea);
					moveCells(board, overfilledBins[i].get(), it.get(), names);
				}
			}
		}
	}
}
vector<int> Legalization::findNearestCells(Board& board, bin& from, bin& to, long long int targetArea) {
	int targetBinX = to.x + BinWidth / 2;
	int targetBinY = to.y + BinHeight / 2;

	struct cell {
		int name;
		int area;
		int distance;
	};

	vector<cell> cells;
	for (auto& it : from.flipflops) {
		int x = board.InstToFlipFlop.at(it).getX();
		int y = board.InstToFlipFlop.at(it).getY();
		int width = board.InstToFlipFlop.at(it).getWidth();
		int height = board.InstToFlipFlop.at(it).getHeight();
		int name = board.InstToFlipFlop.at(it).getInstNum();
		int area = board.InstToFlipFlop.at(it).getArea();
		int distance = abs(targetBinX - (x + width / 2)) + abs(targetBinY - (y + height / 2));
		cells.push_back({ name,area,distance });
	}
	sort(cells.begin(), cells.end(), [](const cell& a, const cell& b) {
		return a.distance < b.distance;
		});
	long long int area = 0;
	vector<int> nearestCells;
	for (const auto& it : cells) {
		nearestCells.push_back(it.name);
		area += it.area;
		if (area + it.area >= targetArea) {
			break;
		}
	}

	return nearestCells;
}
void Legalization::moveCells(Board& board, bin& from, bin& to, vector<int> names) {
	vector<int> tempFlipflops = from.flipflops;
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
			to.flipflops.push_back(it);
			tempFlipflops.erase(remove(tempFlipflops.begin(), tempFlipflops.end(), it), tempFlipflops.end());
		}
	}
	from.flipflops = tempFlipflops;
}
void Legalization::parallelLegalization(Board& board) {
	const int numThreads = 4;
	vector<thread> threads;
	placeFailedFlipFlops.resize(numThreads);
	vector<vector<reference_wrapper<bin>>> placeBins(numThreads);
	//assign work to each thread
	int numBinsHorizontal = (upperRightX - lowerLeftX) / BinWidth;
	int numBinsVertical = (upperRightY - lowerLeftY) / BinHeight;
	bin middleBin = bins.at(numBinsHorizontal / 2).at(numBinsVertical / 2);
	int middleSiteX = (middleBin.siteLLX - placementRowLLX) / siteWidth;
	int middleSiteY = (middleBin.siteLLY - placementRowLLY) / siteHeight;
	placeGates(board);
	//time_t start, end;
	//start = time(NULL);
	for(int i = 0; i < numBinsHorizontal / 2; i++) {
		for (int j = 0; j < numBinsVertical / 2; j++) {
			placeBins.at(0).push_back(bins.at(i).at(j));
		}
	}
	threads.emplace_back(&Legalization::placeFFs, this, ref(board), ref(placeBins.at(0)), ref(placeFailedFlipFlops.at(0)), 0, 0, middleSiteX, middleSiteY);
	
	for (int i = 0; i < numBinsHorizontal / 2; i++) {
		for (int j = numBinsVertical / 2; j < numBinsVertical; j++) {
			placeBins.at(1).push_back(bins.at(i).at(j));
		}
	}
	threads.emplace_back(&Legalization::placeFFs, this, ref(board), ref(placeBins.at(1)), ref(placeFailedFlipFlops.at(1)), 0, middleSiteY, middleSiteX, siteVertical);
	
	for (int i = numBinsHorizontal / 2; i < numBinsHorizontal; i++) {
		for (int j = 0; j < numBinsVertical / 2; j++) {
			placeBins.at(2).push_back(bins.at(i).at(j));
		}
	}
	threads.emplace_back(&Legalization::placeFFs, this, ref(board), ref(placeBins.at(2)), ref(placeFailedFlipFlops.at(2)), middleSiteX, 0, siteHorizontal, middleSiteY);
	
	for (int i = numBinsHorizontal / 2; i < numBinsHorizontal; i++) {
		for (int j = numBinsVertical / 2; j < numBinsVertical; j++) {
			placeBins.at(3).push_back(bins.at(i).at(j));
		}
	}
	threads.emplace_back(&Legalization::placeFFs, this, ref(board), ref(placeBins.at(3)), ref(placeFailedFlipFlops.at(3)), middleSiteX, middleSiteY, siteHorizontal, siteVertical);
	for (auto& t : threads) {
		t.join();
	}
	//end = time(NULL);
	//cout << end - start << " s" << endl;

	//start = time(NULL);
	vector<thread> threads2;
	threads2.emplace_back(&Legalization::replaceFailedFFs, this, ref(board), ref(grids), ref(placeFailedFlipFlops.at(0)), maxReplaceNum, maxFailedHorizontalDisplacement, maxFailedVerticalDisplacement, 0, 0, middleSiteX, middleSiteY, false);
	threads2.emplace_back(&Legalization::replaceFailedFFs, this, ref(board), ref(grids), ref(placeFailedFlipFlops.at(1)), maxReplaceNum, maxFailedHorizontalDisplacement, maxFailedVerticalDisplacement, 0, middleSiteY, middleSiteX, siteVertical, false);
	threads2.emplace_back(&Legalization::replaceFailedFFs, this, ref(board), ref(grids), ref(placeFailedFlipFlops.at(2)), maxReplaceNum, maxFailedHorizontalDisplacement, maxFailedVerticalDisplacement, middleSiteX, 0, siteHorizontal, middleSiteY, false);
	threads2.emplace_back(&Legalization::replaceFailedFFs, this, ref(board), ref(grids), ref(placeFailedFlipFlops.at(3)), maxReplaceNum, maxFailedHorizontalDisplacement, maxFailedVerticalDisplacement, middleSiteX, middleSiteY, siteHorizontal, siteVertical, false);
	//wait for all threads to finish
	for (auto& t : threads2) {
		t.join();
	}
	//end = time(NULL);
	//cout << end - start << " s" << endl;


	//start = time(NULL);
	vector<int> totalFailedFlipFlops;
	for (auto& it : placeFailedFlipFlops) {
		for (auto& it2 : it) {
			totalFailedFlipFlops.push_back(it2);
		}
	}
	if (totalFailedFlipFlops.size() != 0) {
		maxFailedHorizontalDisplacement = siteHorizontal / 2;
		maxFailedVerticalDisplacement = siteVertical / 2;
		maxReplaceNum = 4;
		replaceFailedFFs(board, grids, totalFailedFlipFlops, maxReplaceNum, maxFailedHorizontalDisplacement, maxFailedVerticalDisplacement, 0, 0, siteHorizontal, siteVertical, false);
	}
	//end = time(NULL);
	//cout << end - start << " s" << endl;

	//if (totalFailedFlipFlops.size() != 0) {
	//	cout << "legalization failed" << endl;
	//	for(auto& it : totalFailedFlipFlops) {
	//		cout << it << " "<<board.InstToFlipFlop.at(it).getX() << " " << board.InstToFlipFlop.at(it).getY() << endl;
	//	}
	//}
}
void Legalization::placeGates(Board& board) {
	//mark the site occupied by the gate as 1
	int placementRowSiteLLX = 0;
	int placementRowSiteLLY = 0;
	int placementRowSiteURX = siteHorizontal;
	int placementRowSiteURY = siteVertical;
	for (auto& it : board.InstToGate) {
		int x = it.second.getX();
		int y = it.second.getY();
		int width = it.second.getWidth();
		int height = it.second.getHeight();
		int gateLLSiteX = (x - placementRowLLX) / siteWidth;
		int gateLLSiteY = (y - placementRowLLY) / siteHeight;
		int gateURSiteX = (x + width - placementRowLLX) / siteWidth;
		int gateURSiteY = (y + height - placementRowLLY) / siteHeight;
		for (int i = gateLLSiteX; i < gateURSiteX; i++) {
			for (int j = gateLLSiteY; j < gateURSiteY; j++) {
				grids.at(i).at(j) = -1;
				placeCosts.at(i).at(j) = INT_MAX;
			}
		}
		updateNearCosts(gateLLSiteX, gateLLSiteY, gateURSiteX, gateURSiteY, placementRowSiteLLX, placementRowSiteLLY, placementRowSiteURX, placementRowSiteURY);
	}
}
void Legalization::placeFFs(Board& board, vector<reference_wrapper<bin>>& bins, vector<int>& placeFailedFlipFlops, int placementRowSiteLLX, int placementRowSiteLLY, int placementRowSiteURX, int placementRowSiteURY) {
	vector<pair<int, int>> priority; //priority, flipflop index
	for (auto& it : bins) {
		for (auto& it2 : it.get().flipflops) {
			int p = board.InstToFlipFlop.at(it2).getArea() / (siteWidth * siteHeight);
			//int p = max(board.InstToFlipFlop.at(it2).getWidth(), board.InstToFlipFlop.at(it2).getHeight());
			priority.push_back(make_pair(p, it2));
			sort(priority.begin(), priority.end(), [](const pair<int, int>& a, const pair<int, int>& b) {
				return a.first > b.first;
				});
		}
	}

	for (auto& it : priority) {
		int x = board.InstToFlipFlop.at(it.second).getX();
		int y = board.InstToFlipFlop.at(it.second).getY();
		if (x < placementRowLLX) x = placementRowLLX;
		if (y < placementRowLLY) y = placementRowLLY;
		int width = board.InstToFlipFlop.at(it.second).getWidth();
		int height = board.InstToFlipFlop.at(it.second).getHeight();

		int flipflopLLSiteX = (x - placementRowLLX) / siteWidth;
		int flipflopLLSiteY = (y - placementRowLLY) / siteHeight;
		int flipflopURSiteX = (x + width - placementRowLLX) / siteWidth; //upper right site (won't occupy)
		int flipflopURSiteY = (y + height - placementRowLLY) / siteHeight;


		//find the nearest legal site
		int targetSiteX = -1;
		int targetSiteY = -1;
		int minCost = INT_MAX;
		// find legal site in specified range
		for (int i = flipflopLLSiteX - maxHorizontalDisplacement; i <= flipflopURSiteX + maxHorizontalDisplacement; i++) {
			for (int j = flipflopLLSiteY - maxVerticalDisplacement; j <= flipflopURSiteY + maxVerticalDisplacement; j++) {
				if (isLegal(i, j, i + flipflopURSiteX - flipflopLLSiteX, j + flipflopURSiteY - flipflopLLSiteY, placementRowSiteLLX,  placementRowSiteLLY, placementRowSiteURX, placementRowSiteURY)) {
					int distance = abs(i - flipflopLLSiteX) + abs(j - flipflopLLSiteY);
					int cost = calculatePlaceCost(i, j, i + flipflopURSiteX - flipflopLLSiteX, j + flipflopURSiteY - flipflopLLSiteY, distance);
					if (cost < minCost) {
						minCost = cost;
						targetSiteX = i;
						targetSiteY = j;
					}
				}
			}
		}
		if (targetSiteX != -1) { //legal site found
			//move the flipflop to the site
			int targetX = placementRowLLX + targetSiteX * siteWidth;
			int targetY = placementRowLLY + targetSiteY * siteHeight;
			board.InstToFlipFlop.at(it.second).setPos(targetX, targetY);
			//mark the site occupied by the flipflop as 1
			for (int i = targetSiteX; i < targetSiteX + flipflopURSiteX - flipflopLLSiteX; i++) {
				for (int j = targetSiteY; j < targetSiteY + flipflopURSiteY - flipflopLLSiteY; j++) {
					{
						lock_guard<mutex> lock(mtx); //lock the section
						grids.at(i).at(j) = it.second;
						placeCosts.at(i).at(j) = INT_MAX;
					}
				}
			}
			updateNearCosts(targetSiteX, targetSiteY, targetSiteX + flipflopURSiteX - flipflopLLSiteX, targetSiteY + flipflopURSiteY - flipflopLLSiteY, placementRowSiteLLX, placementRowSiteLLY, placementRowSiteURX, placementRowSiteURY);

		}
		else { //no legal site found
			placeFailedFlipFlops.push_back(it.second);
		}
	}
}
bool Legalization::isLegal(int flipflopLLSiteX, int flipflopLLSiteY, int flipflopURSiteX, int flipflopURSiteY, int placementRowSiteLLX, int placementRowSiteLLY, int placementRowSiteURX, int placementRowSiteURY) {
	//check if out of boundry
	if (flipflopLLSiteX < placementRowSiteLLX || flipflopLLSiteY < placementRowSiteLLY || flipflopURSiteX > placementRowSiteURX || flipflopURSiteY > placementRowSiteURY) {
		return false;
	}
	//check if the sites are occupied
	for (int i = flipflopLLSiteX; i < flipflopURSiteX; i++) {
		for (int j = flipflopLLSiteY; j < flipflopURSiteY; j++) {
			if (grids.at(i).at(j) != 0) {
				return false;
			}
		}
	}
	return true;
}
int Legalization::calculatePlaceCost(int flipflopLLSiteX, int flipflopLLSiteY, int flipflopURSiteX, int flipflopURSiteY, int distance) {
	int cost = 0;
	for (int i = flipflopLLSiteX; i < flipflopURSiteX; i++) {
		for (int j = flipflopLLSiteY; j < flipflopURSiteY; j++) {
			cost += placeCosts.at(i).at(j);
		}
	}
	return cost;
}
void Legalization::updateNearCosts(int cellLLSiteX, int cellLLSiteY, int cellURSiteX, int cellURSiteY, int placementRowSiteLLX, int placementRowSiteLLY, int placementRowSiteURX, int placementRowSiteURY) {
	//placeCosts.at(x).at(y) = INT_MAX;
	vector<pair<int, int>> directions = {
		{0, 1},
		{0, -1},
		{-1, 0},
		{1, 0},
		{-1, 1},
		{1, 1},
		{-1, -1},
		{1, -1}
	};
	for (int i = cellLLSiteX; i < cellURSiteX; i++) {
		lock_guard<mutex> lock(mtx); //lock the section
		for (int j = cellLLSiteY; j < cellURSiteY; j++) {
			for (const auto& dir : directions) {
				int x = i + dir.first;
				int y = j + dir.second;
				if (x >= placementRowSiteLLX && y >= placementRowSiteLLY && x < placementRowSiteURX && y < placementRowSiteURY && placeCosts.at(x).at(y) != INT_MAX) {
					placeCosts.at(x).at(y) -= 1;
				}
			}
		}
	}
}
void Legalization::updateBinDensity(int flipflopLLSiteX, int flipflopLLSiteY, int flipflopURSiteX, int flipflopURSiteY) {
	for(int i = flipflopLLSiteX; i < flipflopURSiteX; i++) {
		for (int j = flipflopLLSiteY; j < flipflopURSiteY; j++) {
			int x = (i * siteWidth + placementRowLLX) / BinWidth;
			int y = (j * siteHeight + placementRowLLY) / BinHeight;
			bins.at(x).at(y).placedArea += siteWidth * siteHeight;
			if (bins.at(x).at(y).placedArea / BinArea >= BinMaxUtil) {
				int p = (bins.at(x).at(y).siteLLX - placementRowLLX) / siteWidth;
				int q = (bins.at(x).at(y).siteLLY - placementRowLLY) / siteHeight;
				for (int s = 0; s < bins.at(x).at(y).siteHorizontal; s++) {
					for (int t = 0; t < bins.at(x).at(y).siteVertical; t++) {
						if (grids.at(p + s).at(q + t) == 0) {
							lock_guard<mutex> lock(mtx); //lock the section
							//grids.at(p + s).at(q + t) = -1;
							placeCosts.at(i).at(j) = 50000;
						}
					}
				}
			}
		}
	}
}
void  Legalization::legalizationInBin(Board& board, bin& bin) {
	/*
	//mark the site occupied by the gate as 1
	for (auto& it : bin.gates) {
		int x = board.InstToGate.at(it).getX();
		int y = board.InstToGate.at(it).getY();
		int width = board.InstToGate.at(it).getWidth();
		int height = board.InstToGate.at(it).getHeight();
		int gateLLSiteX = (x - bin.siteLLX) / siteWidth;
		int gateLLSiteY = (y - bin.siteLLY) / siteHeight;
		int gateURSiteX = (x + width - bin.siteLLX) / siteWidth;
		int gateURSiteY = (y + height - bin.siteLLY) / siteHeight;
		for (int i = gateLLSiteX; i < gateURSiteX; i++) {
			for (int j = gateLLSiteY; j < gateURSiteY; j++) {
				int x1 = (bin.siteLLX - placementRowLLX) / siteWidth + i;
				int y1 = (bin.siteLLY - placementRowLLY) / siteHeight + j;
				{
					lock_guard<mutex> lock(mtx); //lock the section
					grids.at(x1).at(y1) = -1;
				}
				if (i < 0 || j < 0 || i >= bin.siteHorizontal || j >= bin.siteVertical) { //out of boundry
					continue;
				}
				bin.sites.at(i).at(j) = 1;
			}
		}
		//if (!checkSingleGateLegal(board, it)) {
		//	cout << "bin.x " << bin.x << " bin.y " << bin.y << endl;
		//	cout << "bin.siteLLX " << bin.siteLLX << "bin.siteLLX " << bin.siteLLY << "bin.siteHorizontal " << bin.siteHorizontal << "bin.siteHorizontal " << bin.siteVertical << endl;
		//}
	}
	vector<pair<int, int>> priority; //priority, flipflop index
	for (auto& it : bin.flipflops) {
		int p = board.InstToFlipFlop.at(it).getArea() / (siteWidth * siteHeight);
		priority.push_back(make_pair(p, it));
		sort(priority.begin(), priority.end(), [](const pair<int, int>& a, const pair<int, int>& b) {
			return a.first > b.first;
			});
	}
	for (auto& it : priority) {
		int x = board.InstToFlipFlop.at(it.second).getX();
		int y = board.InstToFlipFlop.at(it.second).getY();
		if (x < bin.siteLLX) x = bin.siteLLX;
		if (y < bin.siteLLY) y = bin.siteLLY;
		int width = board.InstToFlipFlop.at(it.second).getWidth();
		int height = board.InstToFlipFlop.at(it.second).getHeight();

		int flipflopLLSiteX = (x - bin.siteLLX) / siteWidth;
		int flipflopLLSiteY = (y - bin.siteLLY) / siteHeight;
		int flipflopURSiteX = (x + width - bin.siteLLX) / siteWidth; //upper right site (won't occupy)
		int flipflopURSiteY = (y + height - bin.siteLLY) / siteHeight;

		//find the nearest legal site
		if (isLegalInBin(bin, flipflopLLSiteX, flipflopLLSiteY, flipflopURSiteX, flipflopURSiteY)) {
			//move the flipflop to the site
			board.InstToFlipFlop.at(it.second).setPos(bin.siteLLX + flipflopLLSiteX * siteWidth, bin.siteLLY + flipflopLLSiteY * siteHeight);
			//mark the site occupied by the flipflop as 1
			for (int i = flipflopLLSiteX; i < flipflopURSiteX; i++) {
				for (int j = flipflopLLSiteY; j < flipflopURSiteY; j++) {
					bin.sites.at(i).at(j) = 1;
					int x1 = (bin.siteLLX - placementRowLLX) / siteWidth + i;
					int y1 = (bin.siteLLY - placementRowLLY) / siteHeight + j;
					{
						lock_guard<mutex> lock(mtx); //lock the section
						grids.at(x1).at(y1) = it.second;
					}
				}
			}
			//if (!checkSingleFFLegal(board, it.second)) {
			//	cout << "bin.x " << bin.x << " bin.y " << bin.y << endl;
			//	cout << "bin.siteLLX " << bin.siteLLX << "bin.siteLLX " << bin.siteLLY << "bin.siteHorizontal " << bin.siteHorizontal << "bin.siteHorizontal " << bin.siteVertical << endl;
			//}
		}
		else {
			//find the nearest legal site
			int targetX = -1;
			int targetY = -1;
			int minDistance = INT_MAX;
			// find legal site in specified range
			for (int i = flipflopLLSiteX - maxHorizontalDisplacement; i <= flipflopURSiteX + maxHorizontalDisplacement; i++) {
				for (int j = flipflopLLSiteY - maxVerticalDisplacement; j <= flipflopURSiteY + maxVerticalDisplacement; j++) {
					if (isLegalInBin(bin, i, j, i + flipflopURSiteX - flipflopLLSiteX, j + flipflopURSiteY - flipflopLLSiteY)) {
						int distance = abs(i - flipflopLLSiteX) + abs(j - flipflopLLSiteY);
						if (distance < minDistance) {
							minDistance = distance;
							targetX = i;
							targetY = j;
						}
					}
				}
			}
			if (targetX != -1) { //legal site found
				//move the flipflop to the site
				int targetX1 = bin.siteLLX + targetX * siteWidth;
				int targetY1 = bin.siteLLY + targetY * siteHeight;
				board.InstToFlipFlop.at(it.second).setPos(targetX1, targetY1);
				//mark the site occupied by the flipflop as 1
				for (int i = targetX; i < targetX + flipflopURSiteX - flipflopLLSiteX; i++) {
					for (int j = targetY; j < targetY + flipflopURSiteY - flipflopLLSiteY; j++) {
						bin.sites.at(i).at(j) = 1;
						int x1 = (bin.siteLLX - placementRowLLX) / siteWidth + i;
						int y1 = (bin.siteLLY - placementRowLLY) / siteHeight + j;
						{
							lock_guard<mutex> lock(mtx); //lock the section
							grids.at(x1).at(y1) = it.second;
						}
					}
				}
				//if (!checkSingleFFLegal(board, it.second)) {
				//	cout << "bin.x " << bin.x << " bin.y " << bin.y << endl;
				//	cout << "bin.siteLLX " << bin.siteLLX << "bin.siteLLX " << bin.siteLLY << "bin.siteHorizontal " << bin.siteHorizontal << "bin.siteHorizontal " << bin.siteVertical << endl;
				//}
			}
			else { //no legal site found
				{
					lock_guard<mutex> lock(mtx); //lock the section
					//placeFailedFlipFlops.push_back({ bin, it.second });
					placeFailedFlipFlops.push_back(it.second);
				}
			}
		}
	}
	*/
}
bool Legalization::isLegalInBin(bin& bin, int flipflopLLSiteX, int flipflopLLSiteY, int flipflopURSiteX, int flipflopURSiteY) {
	/*
	//check if out of boundry
	if (flipflopLLSiteX < 0 || flipflopLLSiteY < 0 || flipflopURSiteX > bin.siteHorizontal || flipflopURSiteY > bin.siteVertical) {
		return false;
	}
	//check if the sites are occupied
	for (int i = flipflopLLSiteX; i < flipflopURSiteX; i++) {
		for (int j = flipflopLLSiteY; j < flipflopURSiteY; j++) {
			if (bin.sites.at(i).at(j) == 1) {
				return false;
			}
		}
	}
	return true;
	*/
	return true;
}
void Legalization::replaceFailedFFs(Board& board, vector<vector<int>>& grids, vector<int>& placeFailedFlipFlops, int maxReplaceNum, int maxFailedHorizontalDisplacement, int maxFailedVerticalDisplacement, int placementRowSiteLLX, int placementRowSiteLLY, int placementRowSiteURX, int placementRowSiteURY, bool debanking) {
	vector<pair<int, int>> priority;  //priority, flipflop index
	for (auto& it : placeFailedFlipFlops) {
		int p = board.InstToFlipFlop.at(it).getArea() / (siteWidth * siteHeight);
		priority.push_back(make_pair(p, it));
		sort(priority.begin(), priority.end(), [](const pair<int, int>& a, const pair<int, int>& b) {
			return a.first > b.first;
			});
	}

	for (auto& it : priority) {
		int x = board.InstToFlipFlop.at(it.second).getX();
		int y = board.InstToFlipFlop.at(it.second).getY();
		int width = board.InstToFlipFlop.at(it.second).getWidth();
		int height = board.InstToFlipFlop.at(it.second).getHeight();
		int flipflopLLSiteX = (x - placementRowLLX) / siteWidth;
		int flipflopLLSiteY = (y - placementRowLLY) / siteHeight;
		int flipflopURSiteX = (x + width - placementRowLLX) / siteWidth;
		int flipflopURSiteY = (y + height - placementRowLLY) / siteHeight;
		bool placed = false;
		
		// find legal site in specified range
		vector<int> xRange;
		vector<int> yRange;
		xRange.push_back(flipflopLLSiteX);
		for (int i = 1; i <= maxFailedHorizontalDisplacement; i++) {
			xRange.push_back(flipflopLLSiteX + i);
			xRange.push_back(flipflopLLSiteX - i);
		}
		yRange.push_back(flipflopLLSiteY);
		for (int i = 1; i <= maxFailedVerticalDisplacement; i++) {
			yRange.push_back(flipflopLLSiteY + i);
			yRange.push_back(flipflopLLSiteY - i);
		}
		for (auto& i : xRange) {
			if(placed) break;
			for (auto& j : yRange) {
				if (placed) break;
				int moveCounts = 0;
				vector<vector<int>> tempGrids = grids;
				vector<tuple<int, int, int>> moveFFs; //flipflop index, x, y
				if (pushAndPlace(board, tempGrids, moveFFs, moveCounts, it.second, i, j, i + flipflopURSiteX - flipflopLLSiteX, j + flipflopURSiteY - flipflopLLSiteY, placementRowSiteLLX, placementRowSiteLLY, placementRowSiteURX, placementRowSiteURY)) {
					for (int i = placementRowSiteLLX; i < placementRowSiteURX; i++) {
						for (int j = placementRowSiteLLY; j < placementRowSiteURY; j++) {
							lock_guard<mutex> lock(mtx);
							grids.at(i).at(j) = tempGrids.at(i).at(j);
						}
					}
					
					for (auto& it : moveFFs) {
						board.InstToFlipFlop.at(get<0>(it)).setPos(get<1>(it), get<2>(it));
						//if (!checkSingleFFLegal(board, get<0>(it))) {
						//	cout << "flipflop index " << get<0>(it) << "x " << get<1>(it) << "y " << get<2>(it) << endl;
						//}
					}
		
					placeFailedFlipFlops.erase(remove(placeFailedFlipFlops.begin(), placeFailedFlipFlops.end(), it.second), placeFailedFlipFlops.end()); //remove the flipflop from the failed list
					placed = true;
				}
			}
		}
	}
	if (!placeFailedFlipFlops.empty()) {
		//cout << "Failed to replace the following flipflops: ";
		//for (auto& it : placeFailedFlipFlops) {
		//	cout << it << ", ";
		//}
		//cout<< endl;
		maxFailedHorizontalDisplacement += 50;
		maxFailedVerticalDisplacement += 20;
		maxReplaceNum--;
		if (maxReplaceNum > 0) {
			if (debanking == true) {
				vector<int> temp = placeFailedFlipFlops;
				for (auto& it : placeFailedFlipFlops) {
					int x = board.InstToFlipFlop.at(it).getX();
					int y = board.InstToFlipFlop.at(it).getY();
					vector<int> newIndex = changeFF(board, it, x, y);
					if (newIndex.size() != 0) {
						temp.erase(remove(temp.begin(), temp.end(), it), temp.end());
						for (auto& it2 : newIndex) {
							temp.push_back(it2);
						}
					}
				}
				placeFailedFlipFlops = temp;
			}
			replaceFailedFFs(board, grids, placeFailedFlipFlops, maxReplaceNum, maxFailedHorizontalDisplacement, maxFailedVerticalDisplacement, placementRowLLX, placementRowLLY, placementRowURX, placementRowURY, false);
		}
	}
}
bool Legalization::isLegalInRegion(vector<vector<int>>& grids, int flipflopLLSiteX, int flipflopLLSiteY, int flipflopURSiteX, int flipflopURSiteY, int placementRowSiteLLX, int placementRowSiteLLY, int placementRowSiteURX, int placementRowSiteURY) {
	//check if out of boundry
	if (flipflopLLSiteX < placementRowSiteLLX || flipflopLLSiteY < placementRowSiteLLY || flipflopURSiteX > placementRowSiteURX || flipflopURSiteY > placementRowSiteURY) {
		return false;
	}
	//check if the sites are occupied
	for (int i = flipflopLLSiteX; i < flipflopURSiteX; i++) {
		for (int j = flipflopLLSiteY; j < flipflopURSiteY; j++) {
			if (grids.at(i).at(j) != 0) {
				return false;
			}
		}
	}
	return true;
}
bool Legalization::pushAndPlace(Board& board, vector<vector<int>>& grids, vector<tuple<int, int, int>>& moveFFs, int& moveCounts, int FFIndex, int flipflopLLSiteX, int flipflopLLSiteY, int flipflopURSiteX, int flipflopURSiteY, int placementRowSiteLLX, int placementRowSiteLLY, int placementRowSiteURX, int placementRowSiteURY) {
	moveCounts++;
	if (moveCounts > maxFFPushed) {
		return false;
	}
	if (isLegalInRegion(grids, flipflopLLSiteX, flipflopLLSiteY, flipflopURSiteX, flipflopURSiteY, placementRowSiteLLX, placementRowSiteLLY, placementRowSiteURX, placementRowSiteURY)) {
		int targetX = placementRowLLX + flipflopLLSiteX * siteWidth;
		int targetY = placementRowLLY + flipflopLLSiteY * siteHeight;
		moveFFs.push_back(make_tuple(FFIndex, targetX, targetY));
		for (int i = flipflopLLSiteX; i < flipflopURSiteX; i++) {
			for (int j = flipflopLLSiteY; j < flipflopURSiteY; j++) {
				grids.at(i).at(j) = FFIndex;
			}
		}
		return true;
	}
	else {
		//check if out of boundry
		if (flipflopLLSiteX < placementRowSiteLLX || flipflopLLSiteY < placementRowSiteLLY || flipflopURSiteX > placementRowSiteURX || flipflopURSiteY > placementRowSiteURY) {
			return false;
		}
		for (int i = flipflopLLSiteX; i < flipflopURSiteX; i++) {
			for (int j = flipflopLLSiteY; j < flipflopURSiteY; j++) {
				if (grids.at(i).at(j) == -1 ) { //occupied by gates
					return false;
				}
			}
		}
		for (int i = flipflopLLSiteX; i < flipflopURSiteX; i++) {
			for (int j = flipflopLLSiteY; j < flipflopURSiteY; j++) {
				if (grids.at(i).at(j) != 0) { //not empty
					int cellIndex = grids.at(i).at(j);
					int x = board.InstToFlipFlop.at(cellIndex).getX();
					int y = board.InstToFlipFlop.at(cellIndex).getY();
					int width = board.InstToFlipFlop.at(cellIndex).getWidth();
					int height = board.InstToFlipFlop.at(cellIndex).getHeight();
					int occupiedflipflopLLSiteX = (x - placementRowLLX) / siteWidth;
					int occupiedflipflopLLSiteY = (y - placementRowLLY) / siteHeight;
					int occupiedflipflopURSiteX = (x + width - placementRowLLX) / siteWidth;
					int occupiedflipflopURSiteY = (y + height - placementRowLLY) / siteHeight;
					float distance = (occupiedflipflopURSiteX + occupiedflipflopLLSiteX) / 2 - (flipflopURSiteX + flipflopLLSiteX) / 2;
					if (distance > 0) { //move the occupied cell to the right
						if (push(board, grids, moveFFs, moveCounts, cellIndex, 1, 0, placementRowSiteLLX, placementRowSiteLLY, placementRowSiteURX, placementRowSiteURY)) {
							return pushAndPlace(board, grids, moveFFs, moveCounts, FFIndex, flipflopLLSiteX, flipflopLLSiteY, flipflopURSiteX, flipflopURSiteY, placementRowSiteLLX, placementRowSiteLLY, placementRowSiteURX, placementRowSiteURY);
						}
						else {
								return false;
						}
					}
					else { //move the occupied cell to the left
						if (push(board, grids, moveFFs, moveCounts, cellIndex, -1, 0, placementRowSiteLLX, placementRowSiteLLY, placementRowSiteURX, placementRowSiteURY)) {
							return pushAndPlace(board, grids, moveFFs, moveCounts, FFIndex, flipflopLLSiteX, flipflopLLSiteY, flipflopURSiteX, flipflopURSiteY, placementRowSiteLLX, placementRowSiteLLY, placementRowSiteURX, placementRowSiteURY);
						}
						else {
								return false;
						}
					}
				}
			}
		}
		return false;
	}
}
bool Legalization::push(Board& board, vector<vector<int>>& grids, vector<tuple<int, int, int>>& moveFFs, int& moveCounts, int FFIndex, int dx, int dy, int placementRowSiteLLX, int placementRowSiteLLY, int placementRowSiteURX, int placementRowSiteURY) {
	//dx: move sites in x direction, dy: move sites in y direction, not used for now
	moveCounts++;
	if (moveCounts > maxFFPushed) {
		return false;
	}
	int x = board.InstToFlipFlop.at(FFIndex).getX();
	int y = board.InstToFlipFlop.at(FFIndex).getY();
	int width = board.InstToFlipFlop.at(FFIndex).getWidth();
	int height = board.InstToFlipFlop.at(FFIndex).getHeight();
	int flipflopLLSiteX = (x - placementRowLLX) / siteWidth;
	int flipflopLLSiteY = (y - placementRowLLY) / siteHeight;
	int flipflopURSiteX = (x + width - placementRowLLX) / siteWidth;
	int flipflopURSiteY = (y + height - placementRowLLY) / siteHeight;
	flipflopLLSiteX += dx;
	flipflopLLSiteY += dy;
	flipflopURSiteX += dx;
	flipflopURSiteY += dy;
	int siteHorizontal = (placementRowURX - placementRowLLX) / siteWidth;
	int siteVertical = (placementRowURY - placementRowLLY) / siteHeight;
	//check if out of boundry
	if (flipflopLLSiteX < placementRowSiteLLX || flipflopLLSiteY < placementRowSiteLLY || flipflopURSiteX > placementRowSiteURX || flipflopURSiteY > placementRowSiteURY) {
		return false;
	}
	//check if the sites are occupied
	if (dx > 0) { //move to the right
		for (int i = flipflopLLSiteX; i < flipflopURSiteX; i++) {
			for (int j = flipflopLLSiteY; j < flipflopURSiteY; j++) {
				if (grids.at(i).at(j) == -1) { //occupied by gates
					return false;
				}
			}
		}
		for (int i = flipflopLLSiteX; i < flipflopURSiteX; i++) {
			for (int j = flipflopLLSiteY; j < flipflopURSiteY; j++) {
				if (grids.at(i).at(j) != 0 && grids.at(i).at(j) != FFIndex) { //occupied by other flipflops
					if (!push(board, grids, moveFFs, moveCounts, grids.at(i).at(j), flipflopURSiteX - i, 0, placementRowSiteLLX, placementRowSiteLLY, placementRowSiteURX, placementRowSiteURY)) {
						return false;
					}
				}
			}
		}
	}
	else { //move to the left
		for (int i = flipflopLLSiteX; i < flipflopURSiteX; i++) {
			for (int j = flipflopLLSiteY; j < flipflopURSiteY; j++) {
				if (grids.at(i).at(j) == -1) { //occupied by gates
					return false;
				}
			}
		}
		for (int i = flipflopURSiteX - 1; i >= flipflopLLSiteX; i--) {
			for (int j = flipflopURSiteY - 1; j >= flipflopLLSiteY; j--) {
				if (grids.at(i).at(j) != 0 && grids.at(i).at(j) != FFIndex) { //occupied by other flipflops
					if (!push(board, grids, moveFFs, moveCounts, grids.at(i).at(j), flipflopLLSiteX - i - 1, 0, placementRowSiteLLX, placementRowSiteLLY, placementRowSiteURX, placementRowSiteURY)) {
						return false;
					}
				}
			}
		}
	}
	//move the flipflop to the site
	for (int i = flipflopLLSiteX; i < flipflopURSiteX; i++) {
		for (int j = flipflopLLSiteY; j < flipflopURSiteY; j++) {
			grids.at(i - dx).at(j - dy) = 0;
		}
	}
	for (int i = flipflopLLSiteX; i < flipflopURSiteX; i++) {
		for (int j = flipflopLLSiteY; j < flipflopURSiteY; j++) {
			grids.at(i).at(j) = FFIndex;
		}
	}
	int targetX = placementRowLLX + flipflopLLSiteX * siteWidth;
	int targetY = placementRowLLY + flipflopLLSiteY * siteHeight;
	moveFFs.push_back(make_tuple(FFIndex, targetX, targetY));
	return true;
}
bool Legalization::checkLegal(Board& board) {
	bool Legal = true;
	vector<vector<int>> grids(siteHorizontal, vector<int>(siteVertical, 0));
	for (auto& it : board.InstToGate) {
		int x = it.second.getX();
		int y = it.second.getY();
		if ((x - placementRowLLX) % siteWidth != 0) {
			cout << "gate " << it.first << "x not aligned with site" << endl;
			Legal = false;
		}
		if ((y - placementRowLLY) % siteHeight != 0) {
			cout << "gate " << it.first << "y not aligned with site" << endl;
			Legal = false;
		}
		int width = it.second.getWidth();
		int height = it.second.getHeight();
		int gateLLSiteX = (x - placementRowLLX) / siteWidth;
		int gateLLSiteY = (y - placementRowLLY) / siteHeight;
		int gateURSiteX = (x + width - placementRowLLX) / siteWidth;
		int gateURSiteY = (y + height - placementRowLLY) / siteHeight;
		if (gateLLSiteX < 0 || gateLLSiteY < 0 || gateURSiteX > siteHorizontal || gateURSiteY > siteVertical) {
			cout << "gate " << it.first << " out of boundry" << endl;
			Legal = false;
		}
		for (int i = gateLLSiteX; i < gateURSiteX; i++) {
			for (int j = gateLLSiteY; j < gateURSiteY; j++) {
				if (grids.at(i).at(j) != 0) {
					cout << "gate " << it.first << " overlapped with " << grids.at(i).at(j) << endl;
					Legal = false;
				}
			}
		}
		for (int i = gateLLSiteX; i < gateURSiteX; i++) {
			for (int j = gateLLSiteY; j < gateURSiteY; j++) {
				grids.at(i).at(j) = it.first;
			}
		}
	}
	for (auto& it : board.InstToFlipFlop) {
		int x = it.second.getX();
		int y = it.second.getY();
		if ((x - placementRowLLX) % siteWidth != 0) {
			cout << "flipflop " << it.first << " x not aligned with site" << endl;
			cout << "x: " << x << " y: " << y << endl;
			Legal = false;
		}
		if ((y - placementRowLLY) % siteHeight != 0) {
			cout << "flipflop " << it.first << " y not aligned with site" << endl;
			cout << "x: " << x << " y: " << y << endl;
			Legal = false;
		}
		int width = it.second.getWidth();
		int height = it.second.getHeight();
		int flipflopLLSiteX = (x - placementRowLLX) / siteWidth;
		int flipflopLLSiteY = (y - placementRowLLY) / siteHeight;
		int flipflopURSiteX = (x + width - placementRowLLX) / siteWidth;
		int flipflopURSiteY = (y + height - placementRowLLY) / siteHeight;
		if (flipflopLLSiteX < 0 || flipflopLLSiteY < 0 || flipflopURSiteX > siteHorizontal || flipflopURSiteY > siteVertical) {
			cout << "flipflop " << it.first << " out of boundry" << endl;
			Legal = false;
		}
		for (int i = flipflopLLSiteX; i < flipflopURSiteX; i++) {
			for (int j = flipflopLLSiteY; j < flipflopURSiteY; j++) {
				if (grids.at(i).at(j) != 0) {
					cout << "flipflop " << it.first << " overlapped with " << grids.at(i).at(j) << endl;
					Legal = false;
				}
			}
		}
		for (int i = flipflopLLSiteX; i < flipflopURSiteX; i++) {
			for (int j = flipflopLLSiteY; j < flipflopURSiteY; j++) {
				grids.at(i).at(j) = it.first;
			}
		}
	}
	return Legal;
}
bool Legalization::checkSingleFFLegal(Board& board, int FFIndex) {
	int x = board.InstToFlipFlop.at(FFIndex).getX();
	int y = board.InstToFlipFlop.at(FFIndex).getY();
	if ((x - placementRowLLX) % siteWidth != 0) {
		cout << "FF " << FFIndex << " x not aligned with site" << endl;
		return false;
	}
	if ((y - placementRowLLY) % siteHeight != 0) {
		cout << "FF " << FFIndex << " y not aligned with site" << endl;
		return false;
	}
	int width = board.InstToFlipFlop.at(FFIndex).getWidth();
	int height = board.InstToFlipFlop.at(FFIndex).getHeight();
	int flipflopLLSiteX = (x - placementRowLLX) / siteWidth;
	int flipflopLLSiteY = (y - placementRowLLY) / siteHeight;
	int flipflopURSiteX = (x + width - placementRowLLX) / siteWidth;
	int flipflopURSiteY = (y + height - placementRowLLY) / siteHeight;
	if (flipflopLLSiteX < 0 || flipflopLLSiteY < 0 || flipflopURSiteX > siteHorizontal || flipflopURSiteY > siteVertical) {
		cout << "FF " << FFIndex << " out of boundry" << endl;
		return false;
	}
	return true;
}
bool Legalization::checkSingleGateLegal(Board& board, int GateIndex) {
	int x = board.InstToGate.at(GateIndex).getX();
	int y = board.InstToGate.at(GateIndex).getY();
	if ((x - placementRowLLX) % siteWidth != 0) {
		cout << "Gate " << GateIndex << " x not aligned with site" << endl;
		return false;
	}
	if ((y - placementRowLLY) % siteHeight != 0) {
		cout << "Gate " << GateIndex << " y not aligned with site" << endl;
		return false;
	}
	int width = board.InstToGate.at(GateIndex).getWidth();
	int height = board.InstToGate.at(GateIndex).getHeight();
	int GateLLSiteX = (x - placementRowLLX) / siteWidth;
	int GateLLSiteY = (y - placementRowLLY) / siteHeight;
	int GateURSiteX = (x + width - placementRowLLX) / siteWidth;
	int GateURSiteY = (y + height - placementRowLLY) / siteHeight;
	if (GateLLSiteX < 0 || GateLLSiteY < 0 || GateURSiteX > siteHorizontal || GateURSiteY > siteVertical) {
		cout << "Gate " << GateIndex << " out of boundry" << endl;
		return false;
	}
	return true;
}
vector<int> Legalization::changeFF(Board& board, int FFIndex, int x, int y) {
	vector<tuple<string, int, int>> FFLib; //FFIndex, bits, area
	for (auto& it : board.FlipFlopLib) {
		FFLib.emplace_back(it.first, it.second.getN(), it.second.getWidth() / siteWidth * (it.second.getHeight() / siteHeight));
	}
	sort(FFLib.begin(), FFLib.end(), [](const tuple<string, int, int>& a, const tuple<string, int, int>& b) {
		return get<2>(a) > get<2>(b);
		});
	int minBit = INT_MAX;
	for (auto& it : FFLib) {
		minBit = min(minBit, get<1>(it));
	}
	FlipFlop before = board.InstToFlipFlop.at(FFIndex);
	int area = before.getWidth() / siteWidth * (before.getHeight() / siteHeight);
	int bits = before.getN();
	vector<FlipFlop> after;
	bool find = false;
	for(auto& it : FFLib) {
		if (get<2>(it) <= area * 0.8 && get<1>(it) == bits) {
			after.push_back(board.FlipFlopLib.at(get<0>(it)));
			find = true;
			break;
		}
	}
	if (!find && bits > minBit) {
		int totalBits = 0;
		for (auto& it : FFLib) {
			if (get<2>(it) < area*0.8 && get<1>(it) == minBit) {
				after.push_back(board.FlipFlopLib.at(get<0>(it)));
				totalBits += get<1>(it);
			}
			if (totalBits == bits) {
				find = true;
				break;
			}
			else if (totalBits > bits) {
				break;
			}
		}
	}
	if (!find) {
		vector<int> afterIndexs;
		return afterIndexs;
	}
	cout<< "change flipflop " << FFIndex << " from " << before.getCellName() << " to ";
	for (auto& it : after) {
		cout << it.getCellName() << endl;
		it.setPos(x, y);
	}
	if(after.size() == 1) { //banking
		vector<FlipFlop> beforeFFs;
		beforeFFs.push_back(before);
		int afterIndex = board.CellNumber;
		board.Banking(beforeFFs, after[0]);
		vector<int> afterIndexs;
		afterIndexs.push_back(afterIndex);
		return afterIndexs;
	}
	else { //debanking
		int afterIndex = board.CellNumber;
		board.Debanking(before, after);
		int afterIndex1 = board.CellNumber;
		vector<int> afterIndexs;
		for (int i = afterIndex; i < afterIndex1; i++) {
			afterIndexs.push_back(i);
		}
		return afterIndexs;
	}

}
