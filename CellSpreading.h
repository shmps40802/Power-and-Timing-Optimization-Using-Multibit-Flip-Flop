#ifndef CELLSPREADING_H
#define CELLSPREADING_H
#include <vector>
#include <queue>
#include <iostream>
#include <climits>
using namespace std;

struct Arc {
	int from, to, capacity, residual, cost; //residual is the remaining capacity
	Arc(int from, int to, int capacity, int cost)
		: from(from), to(to), capacity(capacity), residual(capacity), cost(cost) {}
};
class CellSpreading {
private:
	const int MAXN; //maximum number of nodes
	int s, t, maxFlow, minCost; //source, target, maximum flow, minimum cost
	vector<Arc> arcs;
	vector<vector<int>> G; //adjacency list, G[i] contains the index of edges that starts from node i
	vector<int> h; //use potential to make edge cost non-negative
	vector<int> last; //last edge on shortest path from source to node
	vector<int> flow; //minimum residual capacity on shortest path from source to node
	vector<int> cost; //minimum cost from source to node
	vector<bool> inQueue; //whether the node is in the queue
	

public:
	CellSpreading(int, int, int);
	~CellSpreading();
	void MCMF(); //Minimum Cost Maximum Flow
	bool Dijkstra();
	void addedge(int, int, int, int);
	void printFlows();
	int getFlow(int, int);
};
#endif

