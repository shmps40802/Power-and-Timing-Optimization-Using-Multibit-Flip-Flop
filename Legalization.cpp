#include "Legalization.h"

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
	maxHorizontalDisplacement = 10;
	maxVerticalDisplacement = 4;
	maxFailedHorizontalDisplacement = 50;
	maxFailedVerticalDisplacement = 50;
	siteHorizontal = placementRows.at(0).numSites;
	siteVertical = placementRows.size();
	grids.resize(siteHorizontal, vector<bool>(siteVertical));
}
Legalization::~Legalization() {}
void Legalization::legalize(Board& board) {
	clock_t start;
	double elapsed;

	cout << "start" << endl;

	start = clock();
	initializeBins();
	elapsed = double(clock() - start) / CLOCKS_PER_SEC;
	cout << "initializeBins" << elapsed << " 秒" << endl;

	start = clock();
	cellToBins(board);
	elapsed = double(clock() - start) / CLOCKS_PER_SEC;
	cout << "cellToBins" << elapsed << " 秒" << endl;

	start = clock();
	findOverfilledBins();
	elapsed = double(clock() - start) / CLOCKS_PER_SEC;
	cout << "findOverfilledBins" << elapsed << " 秒" << endl;

	start = clock();
	cellSpreading(board);
	elapsed = double(clock() - start) / CLOCKS_PER_SEC;
	cout << "cellSpreading" << elapsed << " 秒" << endl;

	start = clock();
	parallelLegalization(board);
	elapsed = double(clock() - start) / CLOCKS_PER_SEC;
	cout << "parallelLegalization" << elapsed << " 秒" << endl;
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
				vector<reference_wrapper<bin>> temp;
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
		cs.printFlows();
		//boundry cell moving
		for (int i = 0; i < overfilledBins.size(); i++) {
			for (auto& it : targetBins[i]) {
				int movedArea = cs.getFlow(binToIndex[overfilledBins[i].get()], binToIndex[it.get()]);
				if (movedArea > 0) {
					vector<int> names = findNearestCells(board, overfilledBins[i].get(), it.get(), movedArea);
					moveCells(board, overfilledBins[i].get(), it.get(), names);
				}
			}
		}
	}
}
vector<int> Legalization::findNearestCells(Board& board, bin& from, bin& to, int targetArea) {
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
	int area = 0;
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

	const int numThreads = 4;
	int binsPerThread = bins.size() / numThreads;
	int remainder = bins.size() % numThreads;
	vector<thread> threads;

	//assign work to each thread
	auto worker = [&](int start, int end) {
		for (int i = start; i < end; i++) {
			for (auto& it2 : bins[i]) {
				legalizationInBin(board, it2);
			}
		}
	};
	int startIndex = 0;
	for (int i = 0; i < numThreads; i++) {
		int endIndex = startIndex + binsPerThread + (i < remainder ? 1 : 0);
		threads.emplace_back(worker, startIndex, endIndex);
		startIndex = endIndex;
	}
	//wait for all threads to finish
	for (auto& t : threads) {
		t.join();
	}
	
	//for (auto& it : bins) {
	//	for (auto& it2 : it) {
	//		legalizationInBin(board, it2);
	//	}
	//}
	if (placeFailedFlipFlops.size() != 0) {
		replaceFailedFFs(board);
	}
}
void  Legalization::legalizationInBin(Board& board, bin& bin) {
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
					grids.at(x1).at(y1) = 1;
				}
				if (i < 0 || j < 0 || i >= bin.siteHorizontal || j >= bin.siteVertical) { //out of boundry
					continue;
				}
				bin.sites.at(i).at(j) = 1;
			}
		}
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
						grids.at(x1).at(y1) = 1;
					}
				}
			}
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
							grids.at(x1).at(y1) = 1;
						}
					}
				}
			}
			else { //no legal site found
				{
					lock_guard<mutex> lock(mtx); //lock the section
					placeFailedFlipFlops.push_back({ bin, it.second });
				}
			}
		}
	}
}
bool Legalization::isLegalInBin(bin& bin, int flipflopLLSiteX, int flipflopLLSiteY, int flipflopURSiteX, int flipflopURSiteY) {
	//check if out of boundry
	if (flipflopLLSiteX < 0 || flipflopLLSiteY < 0 || flipflopURSiteX >= bin.siteHorizontal || flipflopURSiteY >= bin.siteVertical) {
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
}
void Legalization::replaceFailedFFs(Board& board) {
	vector<pair<int, int>> priority;  //priority, flipflop index
	for (auto& it : placeFailedFlipFlops) {
		int p = board.InstToFlipFlop.at(it.second).getArea() / (siteWidth * siteHeight);
		priority.push_back(make_pair(p, it.second));
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
		//find the nearest legal site
		int targetX = -1;
		int targetY = -1;
		int minDistance = INT_MAX;
		// find legal site in specified range
		for (int i = flipflopLLSiteX - maxFailedHorizontalDisplacement; i <= flipflopURSiteX + maxFailedHorizontalDisplacement; i++) {
			for (int j = flipflopLLSiteY - maxFailedVerticalDisplacement; j <= flipflopURSiteY + maxFailedVerticalDisplacement; j++) {
				if (isLegalInRegion(i, j, i + flipflopURSiteX - flipflopLLSiteX, j + flipflopURSiteY - flipflopLLSiteY)) {
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
			int targetX1 = placementRowLLX + targetX * siteWidth;
			int targetY1 = placementRowLLY + targetY * siteHeight;
			board.InstToFlipFlop.at(it.second).setPos(targetX1, targetY1);
			//mark the site occupied by the flipflop as 1
			for (int i = targetX; i < targetX + flipflopURSiteX - flipflopLLSiteX; i++) {
				for (int j = targetY; j < targetY + flipflopURSiteY - flipflopLLSiteY; j++) {
					grids.at(i).at(j) = 1;
				}
			}
			priority.erase(remove(priority.begin(), priority.end(), it), priority.end()); //remove the flipflop from the priority list
		}
	}
	if (!priority.empty()) {
		cout << "Failed to replace the following flipflops: ";
		for (auto& it : priority) {
			cout << it.second << " ";
		}
	}
}
bool Legalization::isLegalInRegion(int flipflopLLSiteX, int flipflopLLSiteY, int flipflopURSiteX, int flipflopURSiteY) {
	//check if out of boundry
	if (flipflopLLSiteX < 0 || flipflopLLSiteY < 0 || flipflopURSiteX >= siteHorizontal || flipflopURSiteY >= siteVertical) {
		return false;
	}
	//check if the sites are occupied
	for (int i = flipflopLLSiteX; i < flipflopURSiteX; i++) {
		for (int j = flipflopLLSiteY; j < flipflopURSiteY; j++) {
			if (grids.at(i).at(j) == 1) {
				return false;
			}
		}
	}
	return true;
}
