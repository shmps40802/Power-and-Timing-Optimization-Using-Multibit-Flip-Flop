#include "Cluster.h"
#include <ctime>
#include <random>
#include <cctype>
#include <float.h>
Cluster::Cluster() {
	n = 20;
	k = 100;
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
double Cluster::variance() {
	vector<double> arr(k, 0);
	for (auto& p : DataPoints) {
		int c = p.getCluster();
		//double d = p.distance(centroids[c]);
		double d = distance(p, centroids[c]);
		arr[c] += d * d;
	}
	double sum = 0;
	for (int i = 0; i < k; i++) {
		arr[i] /= counts[i];
		sum += arr[i];
	}
	return sum;
}
void Cluster::kMeansClustering(vector<FlipFlop>& points, vector<FlipFlop>& centroids, vector<int>& counts, int epochs, int k) {
	centroids.resize(k);
	counts.resize(k, 0);

	srand(time(0));
	size_t n = points.size();
	set<int> S;
	for (int i = 0; i < k; ++i) { //find k centroids
		int s = rand() % n;
		while (S.find(s) != S.end()) {
			s = rand() % n;
		}
		S.insert(s);
		centroids[i] = points[s]; //centroid located at points[s]
	}
	while (epochs--) { //iteration times
		for (auto& p : points) {
			double minDist = Max;
			for (int i = 0; i < k; i++) {
				double dist = distance(p, centroids[i]); //distance from p to centroids[i]
				if (dist < minDist) {
					//p.cluster = i;
					p.setCluster(i);
					minDist = dist;
				}
			}
		}
		vector<double> sumX(k, 0);
		vector<double> sumY(k, 0);
		fill(counts.begin(), counts.end(), 0); //set counts to 0
		for (auto& it : points) {
			int clusterId = it.getCluster();
			counts[clusterId]++;
			sumX[clusterId] += (it.getX() - Max);
			sumY[clusterId] += (it.getY() - Max);
		}
		for (int i = 0; i < k; i++) {
			centroids[i].setPos(Max + sumX[i] / (double)counts[i], Max + sumY[i] / (double)counts[i]); //new cluster centroid position
		}
	}
}
double Cluster::distance(FlipFlop a, FlipFlop b) {
	return abs(a.getX() - b.getX()) + abs(a.getY() - b.getY());
}
void Cluster::readData(Board& board) {
	for (auto& it : board.InstToFlipFlop) {
		DataPoints.push_back(it.second);
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
}
void Cluster::kmeans(Board& board) {
	readData(board);
	//int t = 0;
	for (int i = 0; i < n; i++) {
		//kMeansClustering(DataPoints, centroids, counts, epochs, k);
		/*double var = variance();
		cout << var << " " << MinVariance << "\n";
		if(var < MinVariance) {
			MinVariance = var;
			t++;
		}*/
	}
	kMeansClustering(DataPoints, centroids, counts, epochs, k);
	//cout << t << " variation\n";
	KClusters.resize(k);
	for (auto& p : DataPoints) {
		KClusters[p.getCluster()].push_back(p);
	}
	KCentroids.resize(k);
	KCounts.resize(k);
	for (int i = 0; i < k; i++) {
		for (int j = 0; j < n; j++) {
			//kMeansClustering(KClusters[i], KCentroids[i], KCounts[i], epochs, l);
		}
		if (KClusters[i].size() == 0) { //cluster has no points
			continue;
		}
		l = KClusters[i].size() / 10; //number of centroid
		if (l == 0) {
			l = 1;
		}
		kMeansClustering(KClusters[i], KCentroids[i], KCounts[i], epochs, l);
	}
	KLClusters.resize(k);
	for (int i = 0; i < k; i++) {
		KLClusters[i].resize(KCentroids[i].size());
		for (auto& p : KClusters[i]) {
			KLClusters[i][p.getCluster()].push_back(p);
		}
	}
	//----------------------------------------------------------------------------------------
	int sum = 0;
	for (auto& it : KClusters) {
		sum += it.size();
	}
	cout<<"number of Flip Flop in KClusters:  "<<sum<<endl;
	sum = 0;
	for (auto& it : KLClusters) {
		for (auto& it2 : it) {
			sum += it2.size();
		}
	}
	cout << "number of Flip Flop in KLClusters:  " << sum << endl;
	map<int, int> cnt;  //flip flop number distribution in cluster
	sum = 0;
	for (auto &it : KCounts) {
		for (auto& it2 : it) {
			if (cnt.find(it2) == cnt.end()) {
				cnt.insert({ it2,1 });
			}
			else {
				cnt.at(it2)++;
			}
		}
	}
	for (auto& it : cnt) {
		cout << "flip flop numbers: " << it.first << "   counts: " << it.second << endl;
	}
	cout << "KLClusters.size()  " << KLClusters.size() << endl;
	for (int i = 0; i < KLClusters.size(); i++) {
		cout << "KLClusters[" << i << "].size()  " << KLClusters[i].size() << endl;
	}
	//----------------------------------------------------------------------------------------
	for (auto& it : KLClusters) {
		for (auto& it2 : it) {
			if (it2.size() != 0) {
				findOptimalGrouping(it2, board);
			}
		}
	}
}
void Cluster::findOptimalGrouping(vector<FlipFlop>& points, Board& board) {
	//already debanking every Flip Flop to smallest bit
	auto min_it = min_element(availableBits.begin(), availableBits.end());
	int minBits = *min_it;

	//optimize every Flip Flop in the cluster
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
		updateFlipFlop(it, before, board);
	}

	vector<vector<pair<vector<FlipFlop>, FlipFlop>>> BankedFlipFlops(p);
	vector<float> reducedCosts(p, 0.0);
	for (int i = 0; i < p; i++) { //repeat p times, find the best one
		//bank some Flip Flop to multibits
		for (int j = 0; j < q; j++) {
			srand(time(0));
			int bitNum = availableBits.at(rand() % availableBits.size()); //bit number to be banked
			while (bitNum == minBits) {
				bitNum = availableBits.at(rand() % availableBits.size());
			}
			vector<FlipFlop> pointsToBank;
			int num = 0;
			default_random_engine rng(static_cast<unsigned int>(time(0)));
			shuffle(points.begin(), points.end(), rng); //Randomly arrange the points to be synthesized
			for (auto& it : points) { //choose some flip flops total bits = bitNum
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
			int x = 0; //(x,y) is the center of the points to be banked
			int y = 0;
			for (auto& it : pointsToBank) {
				x += it.getX();
				y += it.getY();
			}
			x /= pointsToBank.size();
			y /= pointsToBank.size();
			FlipFlop bankedFlipFlop;
			if (num == bitNum) {
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
void Cluster::updateFlipFlop(FlipFlop before, FlipFlop after, Board& board) {
	//update flip flop from before to after
	if (before.getCellName() == after.getCellName()) {
		return;
	}
	vector<FlipFlop> bef;
	bef.push_back(before);
	vector<vector<FlipFlop>> vectorBef;
	vectorBef.push_back(bef);
	vector<FlipFlop> aft;
	aft.push_back(after);
	cout << "updateFlipFlop   " << before.getInstName() << "  " << before.getCellName() << " -> " << after.getCellName() << "  ";
	board.Banking(vectorBef, aft);
}
void Cluster::updateBankedFlipFlop(vector<FlipFlop> before, FlipFlop after, Board& board) {
	//update banked flip flop
	vector<vector<FlipFlop>> vectorBef;
	vectorBef.push_back(before);
	vector<FlipFlop> aft;
	aft.push_back(after);
	cout << "updateBankedFlipFlop   ";
	for (auto it : before) {
		cout << it.getInstName() << "  " << it.getCellName() << ", ";
	}
	cout << " -> " << after.getCellName() << "  ";
	board.Banking(vectorBef, aft);
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
