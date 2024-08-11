#include "Graph.h"
#include <fstream>
Graph::Graph() {}
Graph::Graph(Board& B) {
    Num = 0;
    auto end = string::npos;
    // add node
    for (auto& in : B.Input) {
        addNode(in.name);
    }
    for (auto& f : B.InstToFlipFlop) {
        vector<int> U;
        vector<int> V;
        for (auto& p : f.second.getPin()) {
            string nodeName = f.second.getInstName() + "/" + p.name;
            addNode(nodeName);
        }
    }
    for (auto& g : B.InstToGate) {
        vector<int> U;
        vector<int> V;
        for (auto& p : g.second.getPin()) {
            string nodeName = g.second.getInstName() + "/" + p.name;
            addNode(nodeName);
            if (p.name.find("IN") != end) {
                U.push_back(Index[nodeName]);
            }
            else if (p.name.find("OUT") != end) {
                V.push_back(Index[nodeName]);
            }
        }
        for (auto& u : U) {
            for (auto& v : V) {
                addEdge(u, v, 0);
            }
        }
    }
    for (auto& out : B.Output) {
        addNode(out.name);
    }
    // add edge
    for (auto& it : B.Net) {
        auto tmp = it.second;
        vector<int> U;
        vector<string> uname;
        vector<int> V;
        vector<string> vname;
        for (auto& p : tmp) {
            if (p.find("INPUT") != end || p.find("/OUT") != end || p.find("Q") != end) {
                U.push_back(Index[p]);
                uname.push_back(p);
            } 
            if (p.find("OUTPUT") != end || p.find("/IN") != end || p.find("D") != end) {
                V.push_back(Index[p]);
                vname.push_back(p);
            }
        }
        for (size_t i = 0; i < U.size(); i++) {
            for (size_t j = 0; j < V.size(); j++) {
                addEdge(U[i], V[j], B.dist(uname[i], vname[j]));
            }
        }
    }
    setWL(B);
}
void Graph::addNode(const string& name) {
    nodes[Num] = {Num, name};
    Index[name] = Num;
    Num++;
}
void Graph::removeNode(int index) {
    nodes.erase(index);
}
void Graph::addEdge(int from, int to, int weight) {
    // adjList[from].push_back({from, to, weight, 1, 0});
    if (nodes[from].name.find("Q") != string::npos) {
        adjList[to].push_back({to, from, weight, 1});
    }
    adjList[to].push_back({to, from, weight, 0});
}
void Graph::removeEdge(int from, int to) {
    if (adjList.find(from) != adjList.end()) {
        auto& edges = adjList[from];
        for (auto it = edges.begin(); it != edges.end(); ++it) {
            if (it->to == to) {
                edges.erase(it);
                break;
            }
        }
    }
}
void Graph::setWL(Board& B) {
    //ofstream fout;
    //fout.open("connect.txt");
    for (auto& f : B.InstToFlipFlop) {
        for (auto& p : f.second.getPin()) {
            if(p.type != 'D') continue;
            int index = Index[f.second.getInstName() + "/" + p.name];
            unordered_map<int, pair<int, int>> S; // Q, WL, in
            dfs(index, 0, S);
            unordered_map<string, pair<int, string>> Q;
            for (auto& s : S) {
                //fout << nodes[s.first].name << " " << nodes[s.second.second].name << " "
                //<< nodes[index].name << " " << s.second.first << "\n";
                B.setWL(nodes[s.first].name, nodes[s.second.second].name, nodes[index].name, s.second.first);
            }
        }
    }
    //fout.close();
}
void Graph::dfs(int index, int WL, unordered_map<int, pair<int, int>>& S) {
    unordered_map<int, int> dist;
    for (auto& it : adjList[index]) {
        if (it.f == 1) {
            if (S.find(it.to) == S.end() || S[it.to].first < WL + it.weight) {
                S[it.to] = make_pair(WL + it.weight, it.from);
            }
        }
        else dfs(it.to, WL + it.weight, S);
    }
}