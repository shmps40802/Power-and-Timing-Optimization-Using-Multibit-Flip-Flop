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
	nodes[Num] = { Num, name };
	Index[name] = Num;
	Num++;
}
void Graph::removeNode(int index) {
	nodes.erase(index);
}
void Graph::addEdge(int from, int to, int weight) {
	// adjList[from].push_back({from, to, weight, 1, 0});
	if (nodes[from].name.find("INPUT") != string::npos || nodes[from].name.find("Q") != string::npos) {
		adjList[to].push_back({ to, from, weight, 1 });
	}
	else { adjList[to].push_back({ to, from, weight, 0 }); }

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
	unordered_map<int, unordered_map<int, vector<int>>> Dcon;  //D Q DConect WL Qconect QWL
	unordered_map<string, pair<string, int>> DCON;             //D DConect DWL
	unordered_map<string, unordered_map<string, int>> Qcon;    //Q Qconect QWL
	for (auto& f : B.InstToFlipFlop) {
		for (auto& p : f.second.getPin()) {
			if (p.type != 'D') continue;
			int index = Index[f.second.getInstName() + "/" + p.name];
			unordered_map<string, float>slack = f.second.getSlack();
			unordered_map<int, pair<int, pair<int, int>>> S; // Q,DConect WL, Qconect
			unordered_map<int, int> QWL;                     //Q QWL
			int maxwl = 0;
			unordered_map<int, int> pinwl;
			dfs(index, 0, S, -1, QWL, maxwl, pinwl);
			for (auto& s : S) {
				s.second.second.first = maxwl;
			}
			unordered_map<int, vector<int>> SS;
			unordered_map<string, pair<int, string>> Q;
			for (auto& s : S) {
				int tt;
				tt = s.second.second.first - QWL[s.first] + slack[p.name] / B.DisplacementDelay;
				int wl = 0;
				for (auto& it : adjList[index]) {
					if (it.to == s.second.first) {
						wl = it.weight + slack[p.name] / B.DisplacementDelay;
					}
				}
				vector<int> tmp0({ s.second.first, wl, s.second.second.second, tt });
				SS[s.first] = tmp0;
				//fout << nodes[s.first].name<<" "<< nodes[s.second.first].name << " " << nodes[s.second.second.second].name << " "
				//<< nodes[index].name << " " << s.second.second.first << " "<< tt  <<"\n";
				B.setDelay(nodes[s.first].name, nodes[s.second.second.second].name, nodes[index].name, s.second.second.first);
			}
			Dcon[index] = SS;
		}
	}
	for (auto& s : Dcon) {
		for (auto& s1 : s.second) {
			DCON[nodes[s.first].name] = make_pair(nodes[s1.second[0]].name, s1.second[1]);
			if (nodes[s1.first].name.find("INPUT") != string::npos) {
				continue;
			}
			if (Qcon[nodes[s1.first].name].find(nodes[s1.second[2]].name) == Qcon[nodes[s1.first].name].end() || Qcon[nodes[s1.first].name][nodes[s1.second[2]].name] > s1.second[3])
			{
				Qcon[nodes[s1.first].name][nodes[s1.second[2]].name] = s1.second[3];
			}
		}
	}
	//fout << "D: " << endl;
	for (auto& s : DCON) {
		//fout << s.first << " " << s.second.first << " " << s.second.second << " " << endl;
	}
	//fout << "Q: " << endl;
	for (auto& s : Qcon) {
		for (auto& s1 : s.second) {
			//fout << s.first << " " << s1.first << " " << s1.second << " " << endl;
		}
	}
	B.setwl(DCON, Qcon);
	//fout.close();
}
void Graph::dfs(int index, int WL, unordered_map<int, pair<int, pair<int, int>>>& S, int tmp, unordered_map<int, int>& QWL, int& maxwl, unordered_map<int, int>& pinwl) {
	unordered_map<int, int> dist;
	for (auto& it : adjList[index]) {
		if (tmp == -1) {
			tmp = it.to;
		}
		if (it.f == 1) {
			if (S.find(it.to) == S.end() || S[it.to].second.first < WL + it.weight) {
				if (QWL[it.to] < WL) {
					QWL[it.to] = WL;
				}
				S[it.to] = make_pair(tmp, make_pair(WL + it.weight, it.from));
				if (WL + it.weight > maxwl) {
					maxwl = WL + it.weight;
				}
			}
		}
		else {
			if (pinwl.find(it.to) == pinwl.end() || pinwl[it.to] < it.weight + pinwl[it.from]) {
				pinwl[it.to] = it.weight + pinwl[it.from];
				dfs(it.to, WL + it.weight, S, tmp, QWL, maxwl, pinwl);
			}
		}
	}
}