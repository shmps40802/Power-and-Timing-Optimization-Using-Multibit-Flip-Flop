#include "Cluster.h"
#include <ctime>
#include <random>
#include <cctype>
#include <cfloat>
#include <thread>
Cluster::Cluster() {
	n = 20;
	k = 2;
	l = 50;
	r = 10;
	epochs = 5;
	maxBits = 4;
	Max = DBL_MAX;
	p = 3;
	q = 5;
	// initializeVector(k, l);
}

Cluster::Cluster(int n, int k, int l, int r, int epochs, int maxBits, double MinVariance, double Max, int p, int q) {
	this->n = n;
	this->k = k;
	this->l = l;
	this->r = r;
	this->epochs = epochs;
	this->maxBits = maxBits;
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
		int s = (int)rand() % size;
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
		}
		for (auto& it : points) {
			int clusterId = it.getCluster();
			sumX[clusterId] += (double)it.getX() / counts[clusterId];
			sumY[clusterId] += (double)it.getY() / counts[clusterId];
		}
		for (int i = 0; i < kc; i++) {
			centroids[i].setPos(sumX[i], sumY[i]); //new cluster centroid position
		}
	}
}
double Cluster::distance(FlipFlop a, FlipFlop b) {
	return (double)abs(a.getX() - b.getX()) + abs(a.getY() - b.getY());
}
void Cluster::readData(Board& board) {
	for (auto& it : board.FlipFlopByClk) {
		vector<FlipFlop> tmp;
		for (auto& it2 : it) {
			tmp.push_back(board.InstToFlipFlop[it2]);
		}
		DataPoints.push_back(tmp);
	}
	for (auto& it : board.FlipFlopLib) {
		FlipFlopLib.push_back(it.second);
		int bits = it.second.getN();
		auto it2 = find(availableBits.begin(), availableBits.end(), bits);
		if (it2 == availableBits.end()) {
			availableBits.push_back(it.second.getN()); //add available n bits flip flops to availableBits
		}
	}
	sort(FlipFlopLib.begin(), FlipFlopLib.end(), compareFlipFlop); //sort FlipFlop library by number of bits and name
	int width = (*board.PlacementRows.begin()).second[0];
	for (auto& b : availableBits) {
		map<int, int> cnt;
		int t = 0;
		int m = 0;
		for (size_t i = 0; i < FlipFlopLib.size(); i++) {
			if (FlipFlopLib[i].getN() != b) continue;
			int n = FlipFlopLib[i].getWidth() / width + 1;
			cnt[n]++;
			if (m < cnt[n]) {
				t = n;
				m = cnt[n];
			}
		}
		for (size_t i = 0; i < FlipFlopLib.size(); i++) {
			if (FlipFlopLib[i].getN() != b) continue;
			int n = FlipFlopLib[i].getWidth() / width + 1;
			if (t != n) {
				FlipFlopLib.erase(FlipFlopLib.begin() + i);
				i--;
			}
		}
	}
}
float Cluster::Silhouette(vector<FlipFlop> points, int TK) {
	kMeansClustering(points, epochs, TK);
	float a = 0, b = 0, avea, aveb, coua = 0, coub = 0;
	for (size_t j = 0; j < points.size() - 1; j++) {//Silhouettescore
		for (size_t m = j + 1; m < points.size(); m++) {
			float inc = (float)abs(points[j].getX() - points[m].getX()) + abs(points[j].getY() - points[m].getY());
			if (points[j].getCluster() == points[m].getCluster()) {
				a += inc;
				coua++;
			}
			else {
				b += inc;
				coub++;
			}
		}
	}
	avea = coua == 0 ? 0 : a / coua;
	aveb = coub == 0 ? 0 : b / coub;
	return (aveb - avea) / max(avea, aveb);
}
void Cluster::kmeans(Board& board) {
	readData(board);
	//cout << "startkmean\n";
	for (size_t i = 0; i < DataPoints.size(); i++) {
		kmean2t(board, DataPoints[i]);
	}
	//cout << "start clustering\n";
	int t = 0;
	int val = 0;
	for (auto& it : KLClusters) {
		int c = 0;
		t++;
		//cout << t << "\n";
		for (auto& it2 : it) {
			val += it.size();
			if (it2.size() != 0) {
				c++;
				//cout << c << "\n";
				findOptimalGrouping(it2, board);
				//cout << "Cost : " << board.Cost() << "\n";
			}
		}

	}
}
void Cluster::kmean2t(Board& board, vector<FlipFlop>datatmp) {
	vector<vector<FlipFlop>>tmp2;
	size_t TK = 2;       //Silhouettescore
	int rec = 1;
	float Silhouettescore = -2;
	int t = 0;
	while (TK < datatmp.size()) {//Silhouettescore
		if (datatmp.size() < sqrt(board.getInstsize()) / 10) {
			rec = 1;
			for (auto& it : datatmp) {
				it.setCluster(0);
			}
			break;
		}
		float tmp = Silhouette(datatmp, TK);
		if (tmp > Silhouettescore) {
			Silhouettescore = tmp;
			rec = TK;
			t = 0;
		}
		else {
			t++;
			if (t > 1)break;
		}
		TK++;
	}
	kMeansClustering(datatmp, epochs, rec);
	tmp2.resize(rec);
	for (auto& p : datatmp) {
		tmp2[p.getCluster()].push_back(p);
	}
	vector<vector<FlipFlop>> tmp3;
	for (size_t i = 0; i < tmp2.size(); i++) {
		if (tmp2[i].size() > 100) {
			tmp3.push_back(tmp2[i]);
			tmp2.erase(tmp2.begin() + i);
			i--;
		}
		else if (tmp2[i].size() == 0) {
			tmp2.erase(tmp2.begin() + i);
			i--;
		}
	}
	KLClusters.push_back(tmp2);
	for (size_t i = 0; i < tmp3.size(); i++) {
		kmean2t(board, tmp3[i]);
	}
	tmp2.clear();
	tmp3.clear();
}

void Cluster::findOptimalGrouping(vector<FlipFlop>& points, Board& board) {
	//already debanking every Flip Flop to smallest bit
	auto min_it = min_element(availableBits.begin(), availableBits.end());
	int minBits = *min_it;

	//optimize every Flip Flop in the cluster
	vector<FlipFlop> updatedPoints;
	for (const auto& it : points) {
		FlipFlop before = it;
		float Min = 0;
		FlipFlop tmp = it;
		for (auto& it2 : FlipFlopLib) {
			if (it2.getN() == minBits) {
				FlipFlop after = it2;
				after.setPos(before.getX(), before.getY());
				float comp = board.singleCompare(before, after);
				if (comp < Min) { //cost reduce
					tmp = after;
					Min = comp;
				}
			}
			else if (it2.getN() > minBits) {
				break;
			}
		}
		FlipFlop temp = updateFlipFlop(it, tmp, board);
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
					//for (auto& it2 : FlipFlopLib) {
					//	if (it2.getN() == bitNum) {
					//		FlipFlop after = it2;
					//		after.setPos(x, y);
					//		if (board.singleCompare(bankedFlipFlop, after) < 0) { //cost reduce
					//			bankedFlipFlop = after;
					//		}
					//	}
					//	else if (it2.getN() > bitNum) {
					//		break;
					//	}
					//}
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
	//cout << "updateFlipFlop   " << before.getInstName() << "  " << before.getCellName() << " -> " << after.getCellName() << "  ";
	//board.Banking(vectorBef, aft);
	board.Banking(bef, after);
	//cout << "  " << after.getInstName() << endl;
	return after;
}
void Cluster::updateBankedFlipFlop(vector<FlipFlop> before, FlipFlop after, Board& board) {
	//update banked flip flop
	//vector<vector<FlipFlop>> vectorBef;
	//vectorBef.push_back(before);
	//vector<FlipFlop> aft;
	//aft.push_back(after);
	//cout << "updateBankedFlipFlop   ";
	//for (auto it : before) {
	//	cout << it.getInstName() << "  " << it.getCellName() << ", ";
	//}
	//cout << " -> " << after.getCellName() << "  ";
	//board.Banking(vectorBef, aft);
	board.Banking(before, after);
	//cout << "  " << after.getInstName() << endl;
}
bool Cluster::compareFlipFlop(const FlipFlop& lhs, const FlipFlop& rhs) {
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