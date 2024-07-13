# include "Cluster.h"
#include <ctime> 
Cluster::Cluster()  {
	n = 20;
	k = 100;
	l = 50;
	epochs = 50;
	maxBits = 4;
	MinVariance = DBL_MAX;
	Max = 1281000;
	initializeVector(k, l);
}

Cluster::Cluster(int n, int k, int l, int epochs, int maxBits, double MinVariance, double Max) {
	this->n = n;
	this->k = k;
	this->l = l;
	this->epochs = epochs;
	this->maxBits = maxBits;
	this->MinVariance = MinVariance;
	this->Max = Max;
	initializeVector(k, l);
}

void Cluster::initializeVector(int k, int l) {
	KClusters.resize(k);
	KLClusters.resize(k, vector<vector<Point>>(l));
	centroids.resize(k);
	KCentroids.resize(k, vector<Point>(l));
	counts.resize(k, 0);
	KCounts.resize(k, vector<int>(l, 0));
}
double Cluster::variance() {
	vector<double> arr(k, 0);
	for (auto& p : DataPoints) {
		int c = p.cluster;
		double d = p.distance(centroids[c]);
		arr[c] += d * d;
	}
	double sum = 0;
	for (int i = 0; i < k; i++) {
		arr[i] /= counts[i];
		sum += arr[i];
	}
	return sum;
}
void Cluster::kMeansClustering(vector<Point>& points, vector<Point>& centroids, vector<int>& counts, int epochs, int k) {
	srand(time(0));
	size_t n = points.size();
	set<int> S;
	for (int i = 0; i < k; ++i) {
		int s = rand() % n;
		while (S.find(s) != S.end()) {
			s = rand() % n;
		}
		S.insert(s);
		centroids[i] = points[s]; //找出k個質心，位於座標points[s]上
	}
	while (epochs--) { //迭代次數
		for (auto& p : points) {
			double minDist = Max;
			for (int i = 0; i < k; i++) {
				double dist = p.distance(centroids[i]); //p與質心的距離
				if (dist < minDist) {
					p.cluster = i;
					minDist = dist;
				}
			}
		}
		vector<double> sumX(k, 0);
		vector<double> sumY(k, 0);
		fill(counts.begin(), counts.end(), 0); //將counts設為0
		for (auto& it : points) {
			int clusterId = it.cluster;
			counts[clusterId]++;
			sumX[clusterId] += (it.x - Max);
			sumY[clusterId] += (it.y - Max);
		}
		for (int i = 0; i < k; i++) {
			centroids[i].x = Max + sumX[i] / (double)counts[i]; //新的cluster質心位置
			centroids[i].y = Max + sumY[i] / (double)counts[i];
		}
	}
}
void Cluster::readData(Board& board) {
	ifstream file;
	for (auto& it : board.InstToFlipFlop) {
		DataPoints.push_back(Point(it.second.getX(), it.second.getY()));
	}
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
	for (auto& p : DataPoints) {
		KClusters[p.cluster].push_back(p);
	}
	for (int i = 0; i < k; i++) {
		for (int j = 0; j < n; j++) {
			kMeansClustering(KClusters[i], KCentroids[i], KCounts[i], epochs, l);
		}
	}
	for (int i = 0; i < k; i++) {
		for (auto& p : KClusters[i]) {
			KLClusters[i][p.cluster].push_back(p);
		}
	}
	unordered_map<int, int> cnt; //cluster中flip flop數量分布
	int sum = 0;
	for (int i = 0; i < k; i++) {
		for (int j = 0; j < l; j++) {
			if (cnt.find(KCounts[i][j]) == cnt.end()) {
				cnt.insert({ KCounts[i][j],1 });
			}
			else {
				cnt.at(KCounts[i][j])++;
			}
		}
	}
	for (auto& it : cnt) {
		cout << "flip flop numbers: " << it.first << "   counts: " << it.second << endl;
	}
}