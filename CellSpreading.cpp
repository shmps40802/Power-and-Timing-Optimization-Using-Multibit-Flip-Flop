#include "CellSpreading.h"

CellSpreading::CellSpreading(int nodeNum, int source, int target) : MAXN(nodeNum) {
	s = source;
	t = target;
	maxFlow = 0;
	minCost = 0;
	G.assign(MAXN, vector<int>());
    h.assign(MAXN, 0);
	last.resize(MAXN);
	flow.resize(MAXN);
}
CellSpreading::~CellSpreading() {}
void CellSpreading::MCMF() {
	while (Dijkstra()); //while there is an augmenting path from source to target
    cout << "maxFlow: " << maxFlow << " minCost: " << minCost << endl;
}
bool CellSpreading::Dijkstra() {
    cost.assign(MAXN, INT_MAX);
    inQueue.assign(MAXN, false);

    cost[s] = 0;
    flow[s] = INT_MAX;

    using P = pair<int, int>; //(cost, node)
    priority_queue<P, vector<P>, greater<P>> Q;
    Q.push({ 0, s });
    while (!Q.empty()) {
        int d = Q.top().first;
        int nowAt = Q.top().second;
        Q.pop();

        if (d != cost[nowAt]) continue; //cost in queue is outdated, skip

        for (int i = 0; i < G[nowAt].size(); i++) {
            Arc& it = arcs[G[nowAt][i]];
            if (it.residual > 0 && cost[it.to] > cost[nowAt] + it.cost + h[nowAt] - h[it.to]) {
                cost[it.to] = cost[nowAt] + it.cost + h[nowAt] - h[it.to];
                flow[it.to] = min(flow[nowAt], it.residual);
                last[it.to] = G[nowAt][i];
                Q.push({ cost[it.to], it.to });
            }
        }
    }

    if (cost[t] == INT_MAX) return false; //No more augmenting path from source to target

    for (int i = 0; i < MAXN; i++) {
        if (cost[i] < INT_MAX) {
            h[i] += cost[i];
        }
    }
    maxFlow += flow[t];
    minCost += h[t] * flow[t]; //cost to send flow[t] flow from source to target
    int nowAt = t;
    while (nowAt != s) {
        arcs[last[nowAt]].residual -= flow[t];
        arcs[last[nowAt] ^ 1].residual += flow[t];
        nowAt = arcs[last[nowAt]].from;
    }
    return true;
}
void CellSpreading::addedge(int from, int to, int capacity, int cost) {
	G[from].push_back(arcs.size()); //edge index in edges
	arcs.push_back(Arc(from, to, capacity, cost)); //forward edge
	G[to].push_back(arcs.size()); //edge index in edges
	arcs.push_back(Arc(to, from, 0, -cost)); //backward edge
}
void CellSpreading::printFlows() {
    for (const auto& e : arcs) {
        if (e.capacity > 0) { //print only forward edges
            int flow = e.capacity - e.residual;
            std::cout << "Edge from " << e.from << " to " << e.to << " has flow: " << flow << endl;
        }
    }
}
int CellSpreading::getFlow(int from, int to) {
    for (const auto& e : arcs) {
        if (e.from == from && e.to == to) {
            return e.capacity - e.residual;
        }
    }
    return 0;
}
