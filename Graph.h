#ifndef GRAPH_H
#define GRAPH_H
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <queue>
#include <stack>
#include <climits>
#include "Board.h"
using namespace std;
struct Node {
	int index;
	string name;
	Node() : index(-1), name("") {}
	Node(int index, string name) : index(index), name(name) {}
};
struct Edge {
	int from;
	int to;
	int weight;
	bool f; // from is Q 
};
class Graph {
private:
	int Num;
	unordered_map<int, Node> nodes;           // store id and node
	unordered_map<int, vector<Edge>> adjList; // store id and edge
	unordered_map<string, int> Index;         // 
public:
	Graph();
	Graph(Board&);
	void addNode(const string&);
	void removeNode(int);
	void addEdge(int, int, int);
	void removeEdge(int, int);
	void setWL(Board&);
	void dfs(int index, int WL, unordered_map<int, pair<int, pair<int, int>>>& S, int tmp, unordered_map<int, int>& QWL, int& maxwl, unordered_map<int, int>& pinwl);//Q dcon WL Qcon
};

#endif