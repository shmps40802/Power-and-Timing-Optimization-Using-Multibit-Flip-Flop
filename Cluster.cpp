#include "Cluster.h"
#include <ctime>
#include <random>
#include <cctype>
#include <cfloat>
Cluster::Cluster() {
	n = 20;
	k = 2;
	l = 50;
	r = 10;
	epochs = 5;
	maxBits = 4;
	MinVariance = DBL_MAX;
	Max = 1281000;
	p = 10;
	q = 10;
	// initializeVector(k, l);
}

Cluster::Cluster(int n, int k, int l, int r, int epochs, int maxBits, double MinVariance, double Max, int p, int q) {
	this->n = n;
	this->k = k;
	this->l = l;
	this->r = r;
	this->epochs = epochs;
	this->maxBits = maxBits;
	this->MinVariance = MinVariance;
	this->Max = Max;
	this->p = p;
	this->q = q;
	// initializeVector(k, l);
}

void Cluster::initializeVector(int k, int l) {
	// KClusters.resize(k);
	// KLClusters.resize(k, vector<vector<FlipFlop>>(l));
	// centroids.resize(k);
	// KCentroids.resize(k, vector<FlipFlop>(l));
	// counts.resize(k, 0);
	// KCounts.resize(k, vector<int>(l, 0));
}
void Cluster::kMeansClustering(vector<FlipFlop>& points, int epochs, int kc) {
	vector<FlipFlop> centroids(kc);
	vector<int> counts(kc, 0);
	srand(time(0));
	size_t size = points.size();
	set<int> S;
	for (int i = 0; i < kc; i++) { //find k centroids
		int s = (int) rand() % size;
		while (S.find(s) != S.end()) {
			s = rand() % size;
		}
		S.insert(s);
		centroids[i] = points[s]; //centroid located at points[s]
	}
	while (epochs--) { //iteration times
		for (auto& p : points) {
			double minDist = Max;
			for (int i = 0; i < kc; i++) {
				double dist = distance(p, centroids[i]); //distance from p to centroids[i]
				if (dist < minDist) {
					p.setCluster(i);
					minDist = dist;
				}
			}
		}
		vector<double> sumX(kc, 0);
		vector<double> sumY(kc, 0);
		fill(counts.begin(), counts.end(), 0); //set counts to 0
		for (auto& it : points) {
			int clusterId = it.getCluster();
			counts[clusterId]++;
			sumX[clusterId] += (it.getX() - Max);
			sumY[clusterId] += (it.getY() - Max);
		}
		for (int i = 0; i < kc; i++) {
			centroids[i].setPos(Max + sumX[i] / (double)counts[i], Max + sumY[i] / (double)counts[i]); //new cluster centroid position
		}
	}
}
double Cluster::distance(FlipFlop a, FlipFlop b) {
	return abs(a.getX() - b.getX()) + abs(a.getY() - b.getY());
}
void Cluster::readData(Board& board) {
	for (auto& it : board.FlipFlopByClk) {
		vector<FlipFlop> tmp;
		for (auto& it2 : it) {
			tmp.push_back(board.InstToFlipFlop[it2]);
		}
		DataPoints.push_back(tmp);
	}
	KLClusters.resize(DataPoints.size());
	for (auto& it : board.FlipFlopLib) {
		FlipFlopLib.push_back(it.second);
		int bits = it.second.getN();
		auto it2 = find(availableBits.begin(), availableBits.end(), bits);
		if (it2 == availableBits.end()) {
			availableBits.push_back(it.second.getN()); //add available n bits flip flops to availableBits
		}
	}
	sort(FlipFlopLib.begin(), FlipFlopLib.end(), compareFlipFlop); //sort FlipFlop library by number of bits and name
}
void Cluster::kmeans(Board& board) {
	readData(board);
	for (size_t i = 0; i < DataPoints.size(); i++) {
		double tmp = 0;   //Silhouettescore
		int TK = 2;       //Silhouettescore
		int rec = 0;
		double Silhouettescore = -1;
		int t = 0;
		while (true) {//Silhouettescore
			if (DataPoints[i].size() < sqrt(board.getInstsize()) / 10) {
				rec = 1;
				for (auto& it : DataPoints[i]) {
					it.setCluster(0);
				}
				break;
			}
			float a = 0, b = 0, avea, aveb, coua = 0, coub = 0;
			kMeansClustering(DataPoints[i], epochs, TK);
			for (size_t j = 0; j < DataPoints[i].size() - 1; j++) {//Silhouettescore
				for (size_t m = j + 1; m < DataPoints[i].size(); m++) {
					if (DataPoints[i][j].getCluster() == DataPoints[i][m].getCluster()) {
						a += abs(DataPoints[i][j].getX() - DataPoints[i][m].getX()) + abs(DataPoints[i][j].getY() - DataPoints[i][m].getY());
						coua++;
					}
					else {
						b += abs(DataPoints[i][j].getX() - DataPoints[i][k].getX()) + abs(DataPoints[i][j].getY() - DataPoints[i][m].getY());
						coub++;
					}
				}
			}
			avea = coua == 0 ? 0 : a / coua;
			aveb = coub == 0 ? 0 : b / coub;
			//cout << "avea : " << avea << endl;
			//cout << "aveb : " << aveb << endl;
			tmp = (aveb - avea) / max(avea, aveb);
			//cout << "Silhouettescore : " << Silhouettescore << endl;
			//cout << "NEWSilhouettescore : " << tmp << endl;
			if (tmp > Silhouettescore) {
				Silhouettescore = tmp;
				l = TK;
				rec = TK;
				t = 0;
			}  // score improved
			else {
				t++;
				if (t > 0)break;
			}  // score no improved
			TK++;
		}
		kMeansClustering(DataPoints[i], epochs, rec);
		KLClusters[i].resize(rec);
		for (auto& p : DataPoints[i]) {
			KLClusters[i][p.getCluster()].push_back(p);
		}
	}
	//----------------------------------------------------------------------------------------
	//int sum = 0;
	//for (auto& it : KClusters) {
	//	sum += it.size();
	//}
	//cout<<"number of Flip Flop in KClusters:  "<<sum<<endl;
	//sum = 0;
	//for (auto& it : KLClusters) {
	//	for (auto& it2 : it) {
	//		sum += it2.size();
	//	}
	//}
	//cout << "number of Flip Flop in KLClusters:  " << sum << endl;
	//map<int, int> cnt;  //flip flop number distribution in cluster
	//sum = 0;
	//for (auto &it : KCounts) {
	//	for (auto& it2 : it) {
	//		if (cnt.find(it2) == cnt.end()) {
	//			cnt.insert({ it2,1 });
	//		}
	//		else {
	//			cnt.at(it2)++;
	//		}
	//	}
	//}
	//for (auto& it : cnt) {
	//	cout << "flip flop numbers: " << it.first << "   counts: " << it.second << endl;
	//}
	//cout << "KLClusters.size()  " << KLClusters.size() << endl;
	//for (int i = 0; i < KLClusters.size(); i++) {
	//	cout << "KLClusters[" << i << "].size()  " << KLClusters[i].size() << endl;
	//}
	//----------------------------------------------------------------------------------------
	int t = 0;
	for (auto& it : KLClusters) {
		int c = 0;
		t++;
		cout << t << "\n";
		for (auto& it2 : it) {
			if (it2.size() != 0) {
				c++;
				cout << c << "\n";
				findOptimalGrouping(it2, board);
				cout << "Cost : " << board.Cost() << "\n";
			}
		}
	}
}
void Cluster::findOptimalGrouping(vector<FlipFlop>& points, Board& board) {
	//already debanking every Flip Flop to smallest bit
	auto min_it = min_element(availableBits.begin(), availableBits.end());
	int minBits = *min_it;

	//optimize every Flip Flop in the cluster
	vector<FlipFlop> updatedPoints;
	for (const auto& it : points) {
		FlipFlop before = it;
		for (auto& it2 : FlipFlopLib) {
			if (it2.getN() == minBits) {
				FlipFlop after = it2;
				after.setPos(before.getX(), before.getY());
				if (board.singleCompare(before, after) < 0) { //cost reduce
					before = after;
				}
			}
			else if (it2.getN() > minBits) {
				break;
			}
		}
		FlipFlop temp = updateFlipFlop(it, before, board);
		updatedPoints.push_back(temp);
	}
	points = updatedPoints;

	vector<vector<pair<vector<FlipFlop>, FlipFlop>>> BankedFlipFlops(p);
	vector<float> reducedCosts(p, 0.0);
	for (int i = 0; i < p; i++) { //repeat p times, find the best one
		//bank some Flip Flop to multibits
		vector<FlipFlop> points2 = points;
		for (int j = 0; j < q; j++) {
			srand(time(0) + j);
			int bitNum = availableBits.at(rand() % availableBits.size()); //bit number to be banked
			while (bitNum == minBits) {
				bitNum = availableBits.at(rand() % availableBits.size());
			}
			vector<FlipFlop> pointsToBank;
			int num = 0;
			default_random_engine rng(static_cast<unsigned int>(time(0)));
			shuffle(points2.begin(), points2.end(), rng); //Randomly arrange the points to be synthesized
			for (auto& it : points2) { //choose some flip flops total bits = bitNum
				if (it.getN() == bitNum) //already banked
				{
					continue;
				}
				else if (num + it.getN() == bitNum) {
					pointsToBank.push_back(it);
					num += it.getN();
					break;
				}
				else if (num + it.getN() < bitNum) {
					pointsToBank.push_back(it);
					num += it.getN();
				}
				else {
					continue;
				}
			}
			//banking
			if (num == bitNum) {
				int x = 0; //(x,y) is the center of the points to be banked
				int y = 0;
				for (auto& it : pointsToBank) {
					x += it.getX();
					y += it.getY();
				}
				x /= pointsToBank.size();
				y /= pointsToBank.size();
				FlipFlop bankedFlipFlop;
				for (auto& it2 : FlipFlopLib) {
					if (it2.getN() == bitNum) {
						FlipFlop after = it2;
						after.setPos(x, y);
						if (board.bankingCompare(pointsToBank, after) < 0) { //cost reduce
							bankedFlipFlop = after;
							break;
						}
					}
					else if (it2.getN() > bitNum) {
						break;
					}
				}
				if (bankedFlipFlop.getN() == bitNum) { //found banked flip flop
					for (auto& it2 : FlipFlopLib) {
						if (it2.getN() == bitNum) {
							FlipFlop after = it2;
							after.setPos(x, y);
							if (board.singleCompare(bankedFlipFlop, after) < 0) { //cost reduce
								bankedFlipFlop = after;
							}
						}
						else if (it2.getN() > bitNum) {
							break;
						}
					}
					for (auto& it : pointsToBank) {
						points2.erase(remove(points2.begin(), points2.end(), it), points2.end());
					}
					BankedFlipFlops.at(i).push_back(make_pair(pointsToBank, bankedFlipFlop));
					//updateBankedFlipFlop(pointsToBank, bankedFlipFlop, board);
				}
			}
		}
		//calculate the total cost reduction
		for (auto& it : BankedFlipFlops.at(i)) {
			reducedCosts.at(i) += board.bankingCompare(it.first, it.second);
		}
	}
	float cost = 0;
	int best = 0;
	for (int i = 0; i < p; i++) {
		if (reducedCosts.at(i) < cost) {
			cost = reducedCosts.at(i);
			best = i;
		}
	}
	//update the flip flops from best banking solution
	for (auto& it : BankedFlipFlops.at(best)) {
		updateBankedFlipFlop(it.first, it.second, board);
	}
}
FlipFlop Cluster::updateFlipFlop(FlipFlop before, FlipFlop after, Board& board) {
	//update flip flop from before to after
	if (before.getCellName() == after.getCellName()) {
		return before;
	}
	vector<FlipFlop> bef;
	bef.push_back(before);
	//vector<vector<FlipFlop>> vectorBef;
	//vectorBef.push_back(bef);
	//vector<FlipFlop> aft;
	//aft.push_back(after);
	cout << "updateFlipFlop   " << before.getInstName() << "  " << before.getCellName() << " -> " << after.getCellName() << "  ";
	//board.Banking(vectorBef, aft);
	board.Banking(bef, after);
	cout << "  " << after.getInstName() << endl;
	return after;
}
void Cluster::updateBankedFlipFlop(vector<FlipFlop> before, FlipFlop after, Board& board) {
	//update banked flip flop
	//vector<vector<FlipFlop>> vectorBef;
	//vectorBef.push_back(before);
	//vector<FlipFlop> aft;
	//aft.push_back(after);
	cout << "updateBankedFlipFlop   ";
	for (auto it : before) {
		cout << it.getInstName() << "  " << it.getCellName() << ", ";
	}
	cout << " -> " << after.getCellName() << "  ";
	//board.Banking(vectorBef, aft);
	board.Banking(before, after);
	cout << "  " << after.getInstName() << endl;
}
bool Cluster::compareFlipFlop(FlipFlop& lhs, FlipFlop& rhs) {
	if (lhs.getN() == rhs.getN()) {
		string lnum = "";
		string rnum = "";
		for (char c : lhs.getCellName()) {
			if (std::isdigit(c)) {
				lnum += c;
			}
		}
		for (char c : rhs.getCellName()) {
			if (std::isdigit(c)) {
				rnum += c;
			}
		}
		return stoi(lnum) < stoi(rnum);
	}
	return lhs.getN() < rhs.getN();
}
