#include "Board.h"
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <thread>
#include <cassert>
using namespace std;
bool ffComp(pair<string, FlipFlop> p1, pair<string, FlipFlop> p2) {
	return p1.second.getN() > p2.second.getN();
}
void Board::merge(vector<node>& arr, int front, int mid, int end) {
	vector<node> left(arr.begin() + front, arr.begin() + mid + 1);
	vector<node> right(arr.begin() + mid + 1, arr.begin() + end + 1);
	left.insert(left.end(), node());
	right.insert(right.end(), node());
	int l = 0, r = 0;
	for (int i = front; i <= end; i++) {
		if (left[l] < right[r]) {
			arr[i] = left[l];
			l++;
		}
		else {
			arr[i] = right[r];
			r++;
		}
	}
}
void Board::mergeSort(vector<node>& arr, int front, int end) {
	if (front < end) {
		int mid = (front + end) / 2;
		mergeSort(arr, front, mid);
		mergeSort(arr, mid + 1, end);
		merge(arr, front, mid, end);
	}
}
void sort(map<string, FlipFlop>& M) {
	vector<pair<string, FlipFlop>> A;
	for (auto& it : M) {
		A.push_back(it);
	}
	sort(A.begin(), A.end(), ffComp);
}
Board::Board() {
	CellNumber = 1;
	maxBit = 1;
	ReadFile();
	/*for (auto& f : InstToFlipFlop) {
		for (auto& p : f.second.getPin()) {
			if (p.type == 'D') {
				DWL["C" + to_string(f.first) + "/" + p.name] = FLT_MAX;
			}
		}
	}
	vector<thread> threads;
	vector<string> keys;
    vector<float> values;
	for (auto& f : InstToFlipFlop) {
		for (auto& p : f.second.getPin()) {
			if (p.type != 'D') continue;
			keys.push_back("C" + to_string(f.first) + "/" + p.name);
			values.push_back(f.second.getSlack()[p.name]);
		}
    }
	vector<map<string,bool>> myMap(keys.size());
	vector<bool> flag(keys.size(), 0);
	for (size_t i = 0; i < keys.size(); i++) {
		threads.emplace_back(&Board::addWL, this, keys[i], ref(myMap[i]), values[i], flag[i]);
	}
    for (auto& t : threads) {
		t.join();
    }*/
	/*int t = 0;
	for (auto& f : InstToFlipFlop) {
		for (auto& p : f.second.getPin()) {
			if (p.type != 'D') continue;
			t++;
			cout << t << " C" << f.first << "\n";
			map<string, bool> visited;
			addWL("C" + to_string(f.first) + "/" + p.name, visited, f.second.getSlack()[p.name], 0);
		}
	}*/
}
Board::~Board() {}
void Board::ReadFile(void) {
	ifstream fin;
	string Filename = "testcase1_balanced.txt";
	//cout << "filename :";
	//cin >> Filename;
	fin.open(Filename);
	while (!fin.is_open()) {
		cout << Filename << " can't be opened\nfilename : ";
		cin >> Filename;
		fin.open(Filename);
	}
	string Str;
	fin >> Str >> Alpha;
	fin >> Str >> Beta;
	fin >> Str >> Gemma;
	fin >> Str >> Delta;
	int NumInput, NumOutput;
	fin >> Str >> LowerLeftX >> LowerLeftY >> HigherRightX >> HigherRightY;
	fin >> Str >> NumInput;
	string name;
	int x, y;
	for (int i = 0; i < NumInput; i++) {
		fin >> Str >> name >> x >> y;
		Point tmp(x, y, name);
		Input.push_back(tmp);
	}
	fin >> Str >> NumOutput;
	for (int i = 0; i < NumOutput; i++) {
		fin >> Str >> name >> x >> y;
		Point tmp(x, y, name);
		Output.push_back(tmp);
	}
	int N, P;
	string FlipFlopName;
	string GateName;
	while (true) {
		int Width, Height;
		fin >> Str;
		if (Str == "FlipFlop") {
			fin >> N >> FlipFlopName >> Width >> Height >> P;
			maxBit = N > maxBit ? N : maxBit;
			vector<Point> pin;
			for (int i = 0; i < P; i++) {
				fin >> Str >> name >> x >> y;
				Point tmp(x, y, name);
				if (name.substr(0, 1) == "D") {
					tmp.type = 'D';
				}
				else if (name.substr(0, 1) == "Q") {
					tmp.type = 'Q';
				}
				else {
					tmp.type = 'C';
				}
				pin.push_back(tmp);
			}
			int fnum = stoi(FlipFlopName.substr(2));
			FlipFlopLib[fnum] = FlipFlop(N, Width, Height, P, pin);
			FlipFlopLib[fnum].setCellName(FlipFlopName);
		}
		else if (Str == "Gate") {
			fin >> GateName >> Width >> Height >> P;
			vector<Point> Pin;
			for (int i = 0; i < P; i++) {
				fin >> Str >> name >> x >> y;
				Point tmp(x, y, name);
				Pin.push_back(tmp);
			}
			int gnum = stoi(GateName.substr(1));
			GateLib[gnum] = Gate(Width, Height, P, Pin);
			GateLib[gnum].setCellName(GateName);
		}
		else break;
	}
	string InstName;
	int NumInst;
	fin >> NumInst;
	CellNumber += NumInst;
	for (int i = 0; i < NumInst; i++) {
		fin >> Str >> InstName >> name >> x >> y;
		int libnum;
		int cnum = stoi(InstName.substr(1));
		if (name.find("F") != string::npos) {
			libnum = stoi(name.substr(2));
			FlipFlop F(FlipFlopLib[libnum]);
			F.setPos(x, y);
			F.setInstNum(cnum);
			InstToFlipFlop[cnum] = F;
			for (auto& it : F.getPin()) {
				string instname = InstName + "/" + it.name;
				InstToFlipFlop[cnum].setsource(it.name, instname);
			}
			Location[x][y].push_back(cnum);
		}
		else {
			libnum = stoi(name.substr(1));
			Gate G(GateLib[libnum]);
			G.setPos(x, y);
			G.setInstNum(cnum);
			InstToGate[cnum] = G;
			Location[x][y].push_back(cnum);
		}
	}
	int NumNets;
	fin >> Str >> NumNets;
	for (int i = 0; i < NumNets; i++) {
		int n;
		string PinName;
		fin >> Str >> name >> n;
		int netnum = stoi(name.substr(3));
		unordered_set<string> tmp;
		vector<int> tmp2;
		for (int j = 0; j < n; j++) {
			fin >> Str >> PinName;
			auto end = string::npos;
			size_t pos = PinName.find("/");
			tmp.insert(PinName);
			PointToNet[PinName] = netnum;
			if (PinName.find("CLK") != end) {
				tmp2.push_back(stoi(PinName.substr(1, pos)));
			}
		}
		for (auto& it1 : tmp) {
			for (auto& it2 : tmp) {
				if (it1 != it2) {
					addNeighbor(it1, it2);
				}
			}
		}
		Net[netnum] = tmp;
		if (!tmp2.empty())FlipFlopByClk.push_back(tmp2);
	}
	fin >> Str >> BinWidth;
	fin >> Str >> BinHeight;
	fin >> Str >> BinMaxUtil;
	while (true) {
		fin >> Str;
		if (Str != "PlacementRows")break;
		fin >> StartX >> StartY >> SiteWidth >> SiteHeight >> TotalNumOfSites;
		PlacementRows[{StartX, StartY}] = { SiteWidth, SiteHeight, TotalNumOfSites };
		// PlacementRows
	}
	fin >> DisplacementDelay;
	float delay, slack;
	for (size_t i = 0; i < FlipFlopLib.size(); i++) {
		fin >> Str >> FlipFlopName >> delay;
		int fnum = stoi(FlipFlopName.substr(2));
		FlipFlopLib[fnum].setQpinDelay(delay);
		for (auto& it : InstToFlipFlop) {
			if(it.second.getCellName() == FlipFlopName) {
				it.second.setQpinDelay(delay);
			}
		}
	}
	int sum = 0;
	for (auto& it : InstToFlipFlop) {
		sum += it.second.getN();
	}
	for (int i = 0; i < sum; i++) {
		fin >> Str >> InstName >> name >> slack;
		int fnum = stoi(InstName.substr(1));
		InstToFlipFlop[fnum].setSlack(name, slack);
	}
	float power;
	for (auto& it : FlipFlopLib) {
		fin >> Str >> name >> power;
		int fnum = stoi(name.substr(2));
		FlipFlopLib[fnum].setPower(power);
		for (auto& it : InstToFlipFlop) {
			if (it.second.getCellName() == name) {
				it.second.setPower(power);
			}
		}
	}
	fin.close();
}
void Board::Display(void) {
	ofstream fout;
	fout.open("check.txt");
	for (auto& it : InstToFlipFlop) {
		it.second.display();
	}
	fout.close();
}
void Board::outputFile(void) {
	ofstream fout;
	fout.open("output.txt");
	fout << "CellInst " << NewFlipFlop.size() << "\n";
	for (auto& it : NewFlipFlop) {
		FlipFlop f = InstToFlipFlop[it];
		fout << "Inst C" << it << " FF" << f.getCellName() << " " << f.getX() << " " << f.getY() << "\n";
	}
	for (auto& it: InstToFlipFlop) {
		for (auto& it2 : it.second.getPin()) {
			for (size_t i = 0; i < it2.sourcename.size(); i++) {
				string ss= "C" + to_string(it.first) + "/" + it2.name;
				if (it2.sourcename[i]!= ss) {
					fout << it2.sourcename[i] << " map " << ss << endl; 
				}
			}
		}
	}
	fout.close();
}
void Board::Plot() {
	ofstream outFile;
	outFile.open("board_plot.m");
	outFile << "axis equal;\n" << "hold on\n" << "grid on\n";
	outFile << "rectangle('Position', [" << LowerLeftX << "," << LowerLeftY << "," << HigherRightX - LowerLeftX << "," << HigherRightY - LowerLeftY << "], 'EdgeColor', 'b'); \n";
	for (auto& it : InstToFlipFlop) {
		outFile << "rectangle('Position', [" << it.second.getX() << ", " << it.second.getY() << ", " << it.second.getWidth() << ", " << it.second.getHeight() << "], 'FaceColor', 'y');\n";
	}

	for (auto& it : InstToGate) {
		outFile << "rectangle('Position', [" << it.second.getX() << ", " << it.second.getY() << ", " << it.second.getWidth() << ", " << it.second.getHeight() << "], 'FaceColor', 'r');\n";
	}
	outFile.close();

	vector<vector<FlipFlop>> data;
	vector<string> Clk;
	for (auto& it : Net) {
		vector<FlipFlop> tmp;
		for (auto& n : it.second) {
			if (n.find("CLK") != string::npos) {
				tmp.push_back(InstToFlipFlop[stoi(n.substr(1, n.find("/")))]);
			}
		}
		if (!tmp.empty()) {
			data.push_back(tmp);
		}
	}
	outFile.open("flip_flop_plot.m");
	outFile << "axis equal;\n" << "hold on\n" << "grid on\n";
	outFile << "cmap = unique(rand(" << data.size() << ", 3), 'rows'); \n";
	outFile << "while size(cmap, 1) < " << data.size() << "\n";
	outFile << "\tcmap = unique([cmap; rand(" << data.size() << " - size(cmap, 1), 3)], 'rows');\n";
	outFile << "end\n";
	outFile << "rectangle('Position', [" << LowerLeftX << "," << LowerLeftY << "," << HigherRightX - LowerLeftX << "," << HigherRightY - LowerLeftY << "], 'EdgeColor', 'b'); \n";
	for (size_t t = 0; t < data.size(); t++) {
		for (size_t j = 0; j < data[t].size(); j++) {
			outFile << "rectangle('Position', [" << data[t][j].getX() << ", " << data[t][j].getY() << ", " << data[t][j].getWidth() << ", " << data[t][j].getHeight() << "], 'FaceColor', cmap(" << t + 1 << ", :)); \n";
		}
	}
	outFile.close();
}
Point Board::NametoPoint(string PinName) {
	// flipflop gate pin
	size_t pos = PinName.find("/");
	if (pos != string::npos) {
		int cnum = stoi(PinName.substr(1, pos));
		if (InstToFlipFlop.find(cnum) != InstToFlipFlop.end()) {
			string pname = PinName.substr(pos + 1, string::npos);
			return InstToFlipFlop[cnum].getPoint(pname);
		}
		if (InstToGate.find(cnum) != InstToGate.end()) {
			string pname = PinName.substr(pos + 1, string::npos);
			return InstToGate[cnum].getPoint(pname);
		}
	}
	// input output pin
	else {
		for (auto& it : Input) {
			if (it.name == PinName) {
				return it;
			}
		}
		for (auto& it : Output) {
			if (it.name == PinName) {
				return it;
			}
		}
	}
	cout << "Pin " << PinName << " can't be found\n";
	return Point();
}
Cell Board::getCell(int cnum) {
	if (InstToFlipFlop.find(cnum) != InstToFlipFlop.end()) {
		return InstToFlipFlop[cnum];
	}
	else if (InstToGate.find(cnum) != InstToGate.end()) {
		return InstToGate[cnum];
	}
	else {
		cout << "C" << cnum << " not find\n";
		return Cell();
	}
}
void Board::Ddfs(string PinName, float &NS, int x, int y) {
	int netnum = PointToNet[PinName];
	for (auto& it : Net[netnum]) {
		if (it.find("Q") != string::npos || it.find("OUT") != string::npos
			|| it.find("INPUT") != string::npos) {
			size_t pos = it.find("/");
			Point P = NametoPoint(PinName);
			Point tmp = NametoPoint(it);
			FlipFlop F1 = InstToFlipFlop[stoi(PinName.substr(1, PinName.find("/")))];
			int x2, y2;
			if (pos != string::npos) {
				int cnum = stoi(it.substr(1, pos));
				Cell F2;
				F2 = getCell(cnum);
				x2 = tmp.x + F2.getX();
				y2 = tmp.y + F2.getY();
			}
			else {
				x2 = tmp.x;
				y2 = tmp.y;
			}
			int x1 = P.x + F1.getX();
			int y1 = P.y + F1.getX();
			int WL1 = abs(x1 - x2) + abs(y1 - y2);
			int WL2 = abs(x - x2) - abs(y - y2);
			float WL = (float) WL1 - WL2;
			float nslack = NS < 0 ? NS : 0;
			float newslack = NS + DisplacementDelay * WL;
			float newnslack = newslack < 0 ? newslack : 0;
			NS = nslack - newnslack;
			return;
		}
	}
}
void Board::Qdfs(string PinName, map<string, bool>& visited, int WL, float &NS, int x, int y, float dq) {
	visited[PinName] = true;
	int netnum = PointToNet[PinName];
	for (auto& it : Net[netnum]) {
		size_t pos = it.find("/");
		if (visited[it] || pos == string::npos)continue;
		int cnum = stoi(it.substr(1, pos));
		string pname = it.substr(pos + 1, string::npos);
		if (PinName.find("Q") != string::npos) {
			Point P = NametoPoint(PinName);
			Point tmp = NametoPoint(it);
			FlipFlop F1 = InstToFlipFlop[stoi(PinName.substr(1, PinName.find("/")))];
			Cell F2 = getCell(cnum);
			int x1 = P.x + F1.getX();
			int y1 = P.y + F1.getY();
			int x2 = tmp.x + F2.getX();
			int y2 = tmp.y + F2.getY();
			int WL1 = abs(x1 - x2) + abs(y1 - y2);
			int WL2 = abs(x - x2) + abs(y - y2);
			WL = WL1 - WL2;
		}
		if (InstToFlipFlop.find(cnum) == InstToFlipFlop.end()) {
			for (auto &p : InstToGate[cnum].getPin()) {
				if (p.name.find("OUT") == string::npos) continue;
				Qdfs(it.substr(0, pos) + "/" + p.name, visited, WL, NS, x, y, dq);
			}
		}  // connect to gate
		else {
			float slack = InstToFlipFlop[cnum].getSlack()[pname];
			float nslack = slack < 0 ? slack : 0;
			float newslack = slack + DisplacementDelay * WL + dq;
			float newnslack = newslack < 0 ? newslack : 0;
			NS += nslack;
			NS -= newnslack;
		}  // connect to flipflop
	}
}
void Board::updateDSlack(string PinName, float& NS, int x, int y) {
	int netnum = PointToNet[PinName];
	for (auto& it : Net[netnum]) {
		if (it.find("Q") != string::npos || it.find("OUT") != string::npos
			|| it.find("INPUT") != string::npos) {
			size_t pos = PinName.find("/");
			int cnum = stoi(PinName.substr(1, pos));
			Point P = NametoPoint(PinName);
			Point tmp = NametoPoint(it);
			FlipFlop F1 = InstToFlipFlop[cnum];
			Cell F2;
			int x2 = tmp.x;
			int y2 = tmp.y;
			pos = it.find("/");
			if (pos != string::npos) {
				cnum = stoi(it.substr(1, pos));
				F2 = getCell(cnum);
				x2 += F2.getX();
				y2 += F2.getY();
			}
			int x1 = P.x + F1.getX();
			int y1 = P.y + F1.getY();
			float WL = (float) abs(x1 - x2) + abs(y1 - y2) - abs(x - x2) - abs(y - y2);
			NS = NS + DisplacementDelay * WL;
			return;
		}
	}
}
void Board::updateQSlack(string PinName, map<string, bool> &visited, float WL, int x, int y, float dq) {
	visited[PinName] = true;
	int netnum = PointToNet[PinName];
	for (auto& it : Net[netnum]) {
		size_t pos = it.find("/");
		if (visited[it] || pos == string::npos) continue;
		int cnum = stoi(it.substr(1, pos));
		string pname = it.substr(pos + 1, string::npos);
		if (PinName.find("Q") != string::npos) {
			Point P = NametoPoint(PinName);
			Point tmp = NametoPoint(it);
			FlipFlop F1 = InstToFlipFlop[stoi(PinName.substr(1, PinName.find("/")))];
			Cell F2 = getCell(cnum);
			// prev and connect pin
			int x1 = P.x + F1.getX();
			int y1 = P.y + F1.getY();
			int x2 = tmp.x + F2.getX();
			int y2 = tmp.y + F2.getY();
			WL = (float) abs(x1 - x2) + abs(y1 - y2) - abs(x - x2) - abs(y - y2);
		}
		if (InstToGate.find(cnum) != InstToGate.end()) {
			vector<Point> points = InstToGate[cnum].getPin();
			for (auto& p : points) {
				if (p.name.find("OUT") == string::npos) continue;
				updateQSlack(it.substr(0, pos) + "/" + p.name, visited, WL, x, y, dq);
			}
		}  // connect to gate
		else if(InstToFlipFlop.find(cnum) != InstToFlipFlop.end()) {
			float slack = InstToFlipFlop[cnum].getSlack()[pname];
			float newslack = slack + DisplacementDelay * WL + dq;
			InstToFlipFlop[cnum].setSlack(pname, newslack);
		}  // connect to flipflop
		else {
			cout << cnum << " not added yet\n";
			system("pause");
		}
	}
}
void Board::Banking(vector<FlipFlop> F1, FlipFlop& F2) {
	string FlipFlopName = "C" + to_string(CellNumber);
	NewFlipFlop.insert(CellNumber);
	F2.setInstNum(CellNumber);
	int x = F2.getX();
	int y = F2.getY();
	Location[x][y].push_back(CellNumber);
	InstToFlipFlop[CellNumber] = F2;
	vector<Point> curPin = F2.getPin();
	int d = 0, q = 0, c = 0, w = 0;
	for (auto& f : F1) {
		vector<Point> PrevPin = f.getPin();
		// add new flipflop
		if (NewFlipFlop.find(f.getInstNum()) != NewFlipFlop.end()) {
			NewFlipFlop.erase(f.getInstNum());
		}
		string prevcell = f.getInstName();
		vector<string> tmp1;  // prev
		vector<string> tmp2;  // cur
		for (auto& p : PrevPin) {
			string prev = prevcell + "/" + p.name;
			string cur;
			// Pin mapping to Pin for 1 bit
			int dd = 0;
			int qq = 0;
			if (p.type == 'D') {
				while (curPin[d].type != 'D') {
					d++;
				}
				string D = "D" + to_string(dd + w);
				if (InstToFlipFlop[CellNumber].getN() == 1) {
					D = "D";
				}
				InstToFlipFlop[CellNumber].setsource(D, p.sourcename[0]);
				dd++;
				cur = FlipFlopName + "/" + curPin[d].name;
				map<string, bool> visited;
				float NS = f.getSlack()[p.name];
				int fx = InstToFlipFlop[CellNumber].getX() + curPin[d].x;
				int fy = InstToFlipFlop[CellNumber].getY() + curPin[d].y;
				updateDSlack(prev, NS, fx, fy);
				InstToFlipFlop[CellNumber].setSlack(curPin[d].name, NS);
				d++;
			}
			else if (p.type == 'Q') {
				while (curPin[q].type != 'Q') {
					q++;
				}
				string Q = "Q" + to_string(qq + w);
				if (InstToFlipFlop[CellNumber].getN() == 1) {
					Q = "Q";
				}
				InstToFlipFlop[CellNumber].setsource(Q, p.sourcename[0]);
				qq++;
				cur = FlipFlopName + "/" + curPin[q].name;
				map<string, bool> visited;
				float WL = 0;
				float dq = f.getQpinDelay() - F2.getQpinDelay();
				int fx = InstToFlipFlop[CellNumber].getX() + curPin[q].x;
				int fy = InstToFlipFlop[CellNumber].getY() + curPin[q].y;
				updateQSlack(prev, visited, WL, fx, fy, dq);
				q++;
			}
			else if (p.type == 'C') {
				while (curPin[c].type != 'C') {
					c++;
				}
				for (int ww = 0; ww < p.sourcename.size(); ww++) {
					InstToFlipFlop[CellNumber].setsource("CLK", p.sourcename[ww]);
				}
				cur = FlipFlopName + "/" + curPin[c].name;
			}
			else {
				cout << prev << " not existing\n";
				break;
			}
			// modify net connection
			tmp1.push_back(prev);
			tmp2.push_back(cur);
		}
		// delete cell from location
		int fx = f.getX();
		int fy = f.getY();
		for (size_t k = 0; k < Location[fx][fy].size(); k++) {
			if (Location[fx][fy][k] == f.getInstNum()) {
				Location[fx][fy].erase(Location[fx][fy].begin() + k);
			}
		}
		for (size_t i = 0; i < tmp1.size(); i++) {
			int netnum = PointToNet[tmp1[i]];
			Net[netnum].erase(tmp1[i]);
			Net[netnum].insert(tmp2[i]);
			PointToNet.erase(tmp1[i]);
			PointToNet[tmp2[i]] = netnum;
		}
		InstToFlipFlop.erase(f.getInstNum());
		w += f.getN();
	}
	F2 = InstToFlipFlop[CellNumber];
	CellNumber++;
}
void Board::Debanking(FlipFlop F1, vector<FlipFlop>& F2) {
	vector<Point> prevPin = F1.getPin();
	int d = 0, q = 0, c = 0, w = 0;
	string prevcell = F1.getInstName();
	for (auto& f : F2) {
		string FlipFlopName = "C" + to_string(CellNumber);
		NewFlipFlop.insert(CellNumber);
		f.setInstNum(CellNumber);
		vector<Point> curPin = f.getPin();
		// add new flipflop
		if (NewFlipFlop.find(F1.getInstNum()) != NewFlipFlop.end()) {
			NewFlipFlop.erase(F1.getInstNum());
		}
		for (auto& p : curPin) {
			string cur = FlipFlopName + "/" + p.name;
			string prev;
			int netnum;
			// Pin mapping to Pin for 1 bit
			int dd = 0;
			int qq = 0;  //memory
			if (p.type == 'D') {
				while (prevPin[d].type != 'D') {
					d++;
				}
				string D = "D" + to_string(dd);
				if (f.getN() == 1) {
					D = "D";
				}
				f.setsource(D, prevPin[d].sourcename[0]);
				dd++;  //memory
				prev = prevcell + "/" + prevPin[d].name;
				map<string, bool> visited;
				float NS = F1.getSlack()[prevPin[d].name];
				int fx = f.getX() + p.x;
				int fy = f.getY() + p.y;
				updateDSlack(prev, NS, fx, fy);
				f.setSlack(p.name, NS);
				netnum = PointToNet[prev];
				Net[netnum].erase(prev);
				PointToNet.erase(prev);
				d++;
			}
			else if (p.type == 'Q') {
				while (prevPin[q].type != 'Q') {
					q++;
				}
				string Q = "Q" + to_string(qq);
				if (f.getN() == 1) {
					Q = "Q";
				}
				f.setsource(Q, prevPin[q].sourcename[0]);
				qq++;
				//memory
				prev = prevcell + "/" + prevPin[q].name;
				map<string, bool> visited;
				float WL = 0;
				float dq = F1.getQpinDelay() - f.getQpinDelay();
				int fx = f.getX() + p.x;
				int fy = f.getY() + p.y;
				updateQSlack(prev, visited, WL, fx, fy, dq);
				netnum = PointToNet[prev];
				Net[netnum].erase(prev);
				PointToNet.erase(prev);
				q++;
			}
			else if (p.type == 'C') {
				while (prevPin[c].type != 'C') {
					c++;
				}
				for (int ij = w; ij < f.getN() + w; ij++) {
					f.setsource("CLK", prevPin[c].sourcename[ij]);
				}  //memory
				prev = prevcell + "/" + prevPin[c].name;
				netnum = PointToNet[prev];
			}
			else {
				cout << prev << " not existing\n";
				break;
			}
			Net[netnum].insert(cur);
			PointToNet[cur] = netnum;
		}
		// add cell to location
		int x = f.getX();
		int y = f.getY();
		Location[x][y].push_back(CellNumber);
		InstToFlipFlop[CellNumber] = f;
		w += f.getN();  //memory
		CellNumber++;
	}
	string prev = prevcell + "/" + prevPin[c].name;
	int netnum = PointToNet[prev];
	Net[netnum].erase(prev);
	PointToNet.erase(prev);
	int fx = F1.getX();
	int fy = F1.getY();
	for (size_t k = 0; k < Location[fx][fy].size(); k++) {
		if (Location[fx][fy][k] == F1.getInstNum()) {
			Location[fx][fy].erase(Location[fx][fy].begin() + k);
		}
	}
	InstToFlipFlop.erase(F1.getInstNum());
}
float Board::bankingCompare(vector<FlipFlop> prev, FlipFlop cur) {
	int N = cur.getN();
	int x = cur.getX();
	int y = cur.getY();
	float sum = 0;
	float PowerComp = cur.getPower();
	float AreaComp = (float) cur.getArea();
	float NSComp = 0;
	for (auto& f : prev) {
		PowerComp -= f.getPower();
		AreaComp -= f.getArea();
	}
	vector<Point> curPin = cur.getPin();
	for (auto& f : prev) {
		vector<Point> prevPin = f.getPin();
		int d = 0, q = 0, c = 0;
		for (auto &p : prevPin) {
			if (p.type == 'D') {
				map<string, bool> visited;
				string PinName = f.getInstName() + "/" + p.name;
				float WL = 0, NS = f.getSlack()[p.name];
				while (curPin[d].type != 'D') {
					d++;
				}
				int fx = cur.getX() + curPin[d].x;
				int fy = cur.getY() + curPin[d].y;
				Ddfs(PinName, NS, fx, fy);
				d++;
				NSComp += NS;
			}
			else if (p.type == 'Q') {
				map<string, bool> visited;
				string PinName = f.getInstName() + "/" + p.name;
				int WL = 0;
				float NS = 0;
				float dq = f.getQpinDelay() - cur.getQpinDelay();
				while (curPin[q].type != 'Q') {
					q++;
				}
				int fx = cur.getX() + curPin[q].x;
				int fy = cur.getY() + curPin[q].y;
				Qdfs(PinName, visited, WL, NS, fx, fy, dq);
				q++;
				NSComp += NS;
			}
		}
	}
	sum += Alpha * NSComp + Beta * PowerComp + Gemma * AreaComp;
	return sum;
}
float Board::singleCompare(FlipFlop prev, FlipFlop cur) {
	int N = cur.getN();
	int x = cur.getX();
	int y = cur.getY();
	float sum = 0;
	float PowerComp = cur.getPower();
	float AreaComp = (float) cur.getArea();
	float NSComp = 0;
	PowerComp -= prev.getPower();
	AreaComp -= prev.getArea();
	vector<Point> prevPin = prev.getPin();
	vector<Point> curPin = cur.getPin();
	int d = 0, q = 0, c = 0;
	for (auto& p : prevPin) {
		if (p.type == 'D') {
			map<string, bool> visited;
			string PinName = "C" + to_string(prev.getInstNum()) + "/" + p.name;
			float WL = 0, NS = prev.getSlack()[p.name];
			while (curPin[d].type != 'D') {
				d++;
			}
			int fx = cur.getX() + curPin[d].x;
			int fy = cur.getY() + curPin[d].y;
			Ddfs(PinName, NS, fx, fy);
			NSComp += NS;
			d++;
		}
		else if (p.type == 'Q') {
			map<string, bool> visited;
			string PinName = "C" + to_string(prev.getInstNum()) + "/" + p.name;
			int WL = 0;
			float NS = 0;
			float dq = prev.getQpinDelay() - cur.getQpinDelay();
			while (curPin[q].type != 'Q') {
				q++;
			}
			int fx = cur.getX() + curPin[q].x;
			int fy = cur.getY() + curPin[q].y;
			Qdfs(PinName, visited, WL, NS, fx, fy, dq);
			NSComp += NS;
			q++;
		}
	}
	sum += Alpha * NSComp + Beta * PowerComp + Gemma * AreaComp;
	return sum;
}
bool Board::Check() {
	// not on grid point
	for (auto& it : InstToFlipFlop) {
		int x = it.second.getX();
		int y = it.second.getY();
		for (auto& p : PlacementRows) {
			if (y == p.first.second) {
				int sx = p.first.first;
				if (sx > x || ((x - sx) % p.second[0])
					|| (x - sx) / p.second[0] > p.second[2]) {
					cout << "x of " << it.first << " not on grid point\n";
					it.second.display();
					return false;
				}
				break;
			}
			else if (y < p.first.second) {
				cout << "y of " << it.first << " not on grid point\n";
				it.second.display();
				return false;
			}
		}
	}
	// overlap
	vector<node> arr;
	for (auto& it : Location) {
		for (auto& r : it.second) {
			int x = it.first;
			int y = r.first;
			for (auto& c : r.second) {
				Cell F = getCell(c);
				node n(F.getX(), F.getRight(), F.getY(), F.getTop(), c);
				arr.push_back(n);
			}
		}
	}
	int n = (int) arr.size();
	mergeSort(arr, 0, n - 1);
	for (auto& it : arr) {
		//cout << it.first.s << " " << it.first.e << " " << it.second.s << " " << it.second.e << "\n";
	}
	for (int i = 0; i < n - 1; i++) {
		for (int j = i + 1; j < n; j++) {
			if (arr[i].ex > arr[j].sx) {
				if ((arr[i].ey > arr[j].sy && arr[i].sy < arr[j].ey)
					|| (arr[j].ey > arr[i].sy && arr[j].sy < arr[i].ey)) {
					string s1 = "C" + to_string(arr[i].index);
					string s2 = "C" + to_string(arr[j].index);
					cout << s1 << " overlapped with " << s2 << "\n";
					cout << arr[i].sx << " " << arr[i].ex << " " << arr[i].sy << " " << arr[i].ey << "\n";
					cout << arr[j].sx << " " << arr[j].ex << " " << arr[j].sy << " " << arr[j].ey << "\n";
					return false;
				}
			}
			else break;
		}
	}
	return true;
}
int Board::dist(string p1, string p2) {
	size_t pos1 = p1.find("/");
	size_t pos2 = p2.find("/");
	auto end = string::npos;
	int x1, x2, y1, y2;
	if (pos1 != end) {
		int cnum1 = stoi(p1.substr(1, pos1));
		string pname1 = p1.substr(pos1 + 1, end);
		Cell c1 = getCell(cnum1);
		for (auto& p : c1.getPin()) {
			if(pname1 == p.name) {
				x1 = c1.getX() + p.x;
				y1 = c1.getY() + p.y;
			}
		}
	}
	else {
		for (auto& p : Input) {
			if (p.name == p1) {
				x1 = p.x;
				y1 = p.y;
			}
		}
		for (auto& p : Output) {
			if (p.name == p1) {
				x1 = p.x;
				y1 = p.y;
			}
		}
	}
	if (pos2 != end) {
		int cnum2 = stoi(p2.substr(1, pos2));
		string pname2 = p2.substr(pos2 + 1, end);
		Cell c2 = getCell(cnum2);
		for (auto& p : c2.getPin()) {
			if(pname2 == p.name) {
				x2 = c2.getX() + p.x;
				y2 = c2.getY() + p.y;
			}
		}
	}
	else {
		for (auto& p : Input) {
			if (p.name == p2) {
				x2 = p.x;
				y2 = p.y;
			}
		}
		for (auto& p : Output) {
			if (p.name == p2) {
				x2 = p.x;
				y2 = p.y;
			}
		}
	}
	return abs(x1 - x2) + abs(y1 - y2);
}
float Board::TNSCost() {
	float sum = 0;
	for (auto& it : InstToFlipFlop) {
		for (auto &p : it.second.getPin()) {
			if (p.type != 'D')continue;
			float negslack = it.second.getSlack()[p.name];
			if (negslack < 0)sum -= negslack;
		}
	}
	return sum;
}
float Board::PowerCost() {
	float sum = 0;
	for (auto& it : InstToFlipFlop) {
		sum += it.second.getPower();
	}
	return sum;
}
float Board::AreaCost() {
	float sum = 0;
	for (auto& it : InstToFlipFlop) {
		sum += it.second.getArea();
	}
	return sum;
}
float Board::BinCost() {
	float sum = 0;
	for (auto& it : InstToFlipFlop) {
		int fx = it.second.getX();
		int fy = it.second.getY();
		int fr = it.second.getRight();
		int ft = it.second.getTop();
		for (int i = fx / BinWidth * BinWidth; i < fr; i += BinWidth) {
			int w;
			if (i + BinWidth > fr)w = fr - i;
			else if (i >= fx)w = BinWidth;
			else w = i - fx;
			for (int j = fy / BinHeight * BinHeight; j < ft; j += BinHeight) {
				int h;
				if (j + BinHeight > ft)h = ft - j;
				else if (j >= fy)w = BinHeight;
				else h = j - fy;
				BinDensity[i][j] += (float) w * h;
			}
		}
	}
	for (auto &it : InstToGate) {
		int gx = it.second.getX();
		int gy = it.second.getY();
		int gr = it.second.getRight();
		int gt = it.second.getTop();
		for (int i = gx / BinWidth * BinWidth; i < gr; i += BinWidth) {
			int w;
			if (i + BinWidth > gr)w = gr - i;
			else if (i > gx)w = BinWidth;
			else w = i - gx;
			for (int j = gy / BinHeight * BinHeight; j < gt; j += BinHeight) {
				int h;
				if (j + BinHeight > gt)h = gt - j;
				else if (j > gy)w = BinHeight;
				else h = j - gy;
				BinDensity[i][j] += (float) w * h;
			}
		}
	}
	for (auto& it : BinDensity) {
		for (auto& d : it.second) {
			d.second /= (float) (BinWidth * BinHeight / 100);
			if (d.second >= BinMaxUtil) sum += 1;
		}
	}
	return sum;
}
float Board::Cost() {
	float sum = 0;
	sum += Alpha * TNSCost() + Beta * PowerCost() + Gemma * AreaCost() + Delta * BinCost();
	return sum;
}
int Board::getInstsize() {
	return (int) InstToFlipFlop.size();
}
void Board::addNeighbor(string p1, string p2) {
	auto end = string::npos;
	if (p1.find("INPUT") != end || p1.find("/OUT") != end || p1.find("Q") != end) {
		if (p2.find("OUTPUT") != end || p2.find("/IN") != end || p2.find("D") != end) {
			Net2[p1].insert(p2);
			Net2[p2].insert(p1);
		}
	}
	else if (p1.find("INPUT") != end || p1.find("/OUT") != end || p1.find("Q") != end) {
		if (p2.find("OUTPUT") != end || p2.find("/IN") != end || p2.find("D") != end) {
			Net2[p1].insert(p2);
			Net2[p2].insert(p1);
		}
	}
}
void Board::setWL(vector<pair<string,string>> Q, string D) {
	size_t pos1 = D.find("/");
	auto end = string::npos;
	int cnum1 = stoi(D.substr(1, pos1));
	string dname = D.substr(pos1 + 1, end);
	float slack = InstToFlipFlop[cnum1].getSlack()[dname];
	for (auto& q : Q) {
		size_t pos2 = q.first.find("/");
		int cnum2 = stoi(q.first.substr(1, pos2));
		string qname = q.first.substr(pos2 + 1, end);
		InstToFlipFlop[cnum2].setConnection(qname, q.second, slack);
	}
}
void Board::addWL(string PinName, map<string, bool>& visited, float slack, bool t) {
	visited[PinName] = true;
	for (auto& it : Net2[PinName]) {
		if (visited[it]) continue;
		size_t pos = it.find("/");
		auto end = string::npos;
		if (pos != end) {
			int cnum = stoi(it.substr(1, pos));
			Cell c1 = getCell(cnum);
			if (it.find("Q") != end) {
				string qname = it.substr(pos + 1, end);
				float WL2 = (float) dist(PinName, it) + slack / DisplacementDelay;
				InstToFlipFlop[cnum].setConnection(qname, PinName, WL2);
				if (t == 0) {
					DWL[it] = DWL[it] < WL2 ? DWL[it] : WL2;
				}
				visited[it] = true;
			}
			else if (it.find("OUT") != end) {
				if (t == 0) {
					float WL1 = (float) dist(PinName, it) + slack / DisplacementDelay;
					DWL[it] = DWL[it] < WL1 ? DWL[it] : WL1;
				}
				for (auto& p : c1.getPin()) {
					if (p.name.find("IN") != end) {
						addWL("C" + to_string(cnum) + "/" + p.name, visited, slack, 1);
					}
				}
			}
		}
		else {
			if (it.find("INPUT") != end) {
				if (t == 0) {
					float WL1 = (float) dist(PinName, it) + slack / DisplacementDelay;
					DWL[it] = DWL[it] < WL1 ? DWL[it] : WL1;
				}
				visited[it] = true;
			}
		}
	}
}
