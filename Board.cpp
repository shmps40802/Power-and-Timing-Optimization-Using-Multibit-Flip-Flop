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
bool fltComp(float f1, float f2) {
	return f1 > f2;
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
}
Board::~Board() {}
void Board::ReadFile(void) {
	ifstream fin;
	string Filename = "testcase1.txt";
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
			FlipFlopLib[FlipFlopName] = FlipFlop(N, Width, Height, P, pin);
			FlipFlopLib[FlipFlopName].setCellName(FlipFlopName);
		}
		else if (Str == "Gate") {
			fin >> GateName >> Width >> Height >> P;
			vector<Point> Pin;
			for (int i = 0; i < P; i++) {
				fin >> Str >> name >> x >> y;
				Point tmp(x, y, name);
				Pin.push_back(tmp);
			}
			GateLib[GateName] = Gate(Width, Height, P, Pin);
			GateLib[GateName].setCellName(GateName);
		}
		else break;
	}
	string InstName;
	int NumInst;
	fin >> NumInst;
	CellNumber += NumInst;
	for (int i = 0; i < NumInst; i++) {
		fin >> Str >> InstName >> name >> x >> y;
		int cnum = stoi(InstName.substr(1));
		if (name.find("F") != string::npos) {
			FlipFlop F(FlipFlopLib[name]);
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
			Gate G(GateLib[name]);
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
			addNet(netnum, PinName);
			tmp.insert(PinName);
			addNet(netnum, PinName);
			if (PinName.find("CLK") != end) {
				tmp2.push_back(stoi(PinName.substr(1, pos)));
			}
		}
		if (!tmp2.empty())FlipFlopByClk.push_back(tmp2);
	}
	fin >> Str >> BinWidth;
	fin >> Str >> BinHeight;
	fin >> Str >> BinMaxUtil;
	while (true) {
		fin >> Str;
		if (Str != "PlacementRows")break;
		fin >> StartX >> StartY >> SiteWidth >> SiteHeight >> TotalNumOfSites;
		PlacementRows[StartY] = make_pair(StartX, vector<int>{ SiteWidth, SiteHeight, TotalNumOfSites });
		// PlacementRows
	}
	fin >> DisplacementDelay;
	float delay, slack;
	for (size_t i = 0; i < FlipFlopLib.size(); i++) {
		fin >> Str >> FlipFlopName >> delay;
		FlipFlopLib[FlipFlopName].setQpinDelay(delay);
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
		FlipFlopLib[name].setPower(power);
		for (auto& it : InstToFlipFlop) {
			if (it.second.getCellName() == name) {
				it.second.setPower(power);
			}
		}
	}
	fin.close();
	/*ifstream fin2;
	fin2.open("connect.txt");
	string Q, in, D;
	int WL;
	while (fin2 >> Q >> in >> D >> WL) {
		size_t pos1 = D.find("/");
		auto end = string::npos;
		int cnum1 = stoi(D.substr(1, pos1));
		string dname = D.substr(pos1 + 1, end);
		float slack = InstToFlipFlop[cnum1].getSlack()[dname];
		size_t pos2 = Q.find("/");
		int cnum2 = stoi(Q.substr(1, pos2));
		string qname = Q.substr(pos2 + 1, end);
		float qdelay = InstToFlipFlop[cnum2].getQpinDelay();
		//InstToFlipFlop[cnum2].setQconnect(qname, D);
		//InstToFlipFlop[cnum1].setDWL(dname, Q, WL * DisplacementDelay + qdelay, in);
		Qconnect[Q].insert(D);
		Ddelay[D][Q] = make_pair(WL * DisplacementDelay + qdelay, in);
	}
	fin2.close();*/
}
void Board::Display(void) {
	ofstream fout;
	fout.open("check.txt");
	for (auto& it : Net) {
		fout << it.first << " ";
		for (auto& it2 : it.second) {
			fout << it2 << " ";
		}
		fout << "\n";
	}
	for (auto& it : Ddelay) {
		for (auto& it2 : it.second) {
			fout << it.first << " " << it2.first << " " << it2.second.first << " " << it2.second.second << "\n";
		}
	}
	for (auto& it : Qconnect) {
		fout << it.first << " ";
		for (auto& it2 : it.second) {
			fout << it2 << " ";
		}
		fout << "\n";
	}
	fout.close();
	system("pause");
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
		else if (InstToGate.find(cnum) != InstToGate.end()) {
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
	system("pause");
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
void Board::addNet(int netnum, string pin) {
	Net[netnum].insert(pin);
	PointToNet[pin] = netnum;
}
void Board::removeNet(int netnum, string pin) {
	Net[netnum].erase(pin);
	PointToNet.erase(pin);
}
void Board::Dslack(string dname, float &NS, int x, int y) {
	auto end = string::npos;
	size_t pos1 = dname.find("/");
	int cnum1 = stoi(dname.substr(1, pos1));
	FlipFlop f = InstToFlipFlop[cnum1];
	string pname = dname.substr(0, pos1);
	float slack = f.getSlack()[pname];
	Point p1 = NametoPoint(dname);
	int x1 = p1.x + f.getX();
	int y1 = p1.y + f.getY();
	int netnum = PointToNet[dname];
	for (auto& it : Net[netnum]) {
		if (it.find("D") != end || it.find("/IN") != end || it.find("OUTPUT") != end) continue;
		size_t pos2 = it.find("/");
		Point p2 = NametoPoint(it);
		int x2 = p1.x;
		int y2 = p1.y;
		if (pos2 != string::npos) {
			int cnum2 = stoi(it.substr(1, pos2));
			Cell c = getCell(cnum2);
			x2 += c.getX();
			y2 += c.getY();
		}
		int WL1 = abs(x1 - x2) + abs(y1 - y2);
		int WL2 = abs(x - x2) + abs(y - y2);
		float dWL = (float) WL2 - WL1;
		float newslack = slack - dWL * DisplacementDelay;
		slack = slack < 0 ? -slack : 0;
		newslack = newslack < 0 ? -newslack : 0;
		NS = newslack - slack;
		return;
	}
}
void Board::Qslack(string PinName, float &NS, int x, int y, float dq) {
	size_t pos1 = PinName.find("/");
	int cnum1 = stoi(PinName.substr(1, pos1));
	string qname = PinName.substr(pos1 + 1, string::npos);
	for (auto& it : Qconnect[PinName]) {
		size_t pos2 = it.find("/");
		int cnum2 = stoi(it.substr(1, pos2));
		string dname = it.substr(pos2 + 1, string::npos);
		float slack = InstToFlipFlop[cnum2].getSlack()[dname];
		float dWL;
		vector<float> tmp;
		float Max = 0;
		for (auto& f : Ddelay[it]) {
			float delay = f.second.first;
			string qpin = f.first;
			Max = delay > Max ? delay : Max;
			if (qpin == PinName) {
				FlipFlop F1 = InstToFlipFlop[cnum1];     // Q
				Point p1 = NametoPoint(PinName);
				int x1 = F1.getX() + p1.x;
				int y1 = F1.getY() + p1.y;
				Point p2 = NametoPoint(f.second.second); // in
				int x2 = p2.x;
				int y2 = p2.y;
				size_t pos3 = f.second.second.find("/");
				if (pos3 != string::npos) {
					int cnum3 = stoi(f.second.second.substr(1, pos3));
					Cell F3 = getCell(cnum3);
					x2 += F3.getX();
					y2 += F3.getY();
				}
				int WL1 = abs(x1 - x2) + abs(y1 - y2);
				int WL2 = abs(x - x2) + abs(y - y2);
				dWL = WL2 - WL1;
				delay += (float) dq + dWL * DisplacementDelay;
			}
			tmp.push_back(delay);
		}
		sort(tmp.begin(), tmp.end(), fltComp);
		if (tmp[0] != Max) {
			float ddelay = Max - tmp[0];
			float newslack = slack + ddelay;
			slack = slack < 0 ? -slack : 0;
			newslack = newslack < 0 ? -newslack : 0;
			NS += newslack - slack;
		}
	}
}
void Board::updateDSlack(string dname, float& dWL, int x, int y) {
	auto end = string::npos;
	size_t pos = dname.find("/");
	int cnum1 = stoi(dname.substr(1, pos));
	FlipFlop f = InstToFlipFlop[cnum1];
	Point p1 = NametoPoint(dname);
	int x1 = p1.x + f.getX();
	int y1 = p1.y + f.getY();
	int netnum = PointToNet[dname];
	for (auto& it : Net[netnum]) {
		if (it.find("D") != end || it.find("/IN") != end || it.find("OUTPUT") != end) continue;
		Point tmp = NametoPoint(it);
		int x2 = tmp.x;
		int y2 = tmp.y;
		size_t pos2 = it.find("/");
		if (pos2 != string::npos) {
			int cnum2 = stoi(it.substr(1, pos2));
			Cell F2 = getCell(cnum2);
			x2 += F2.getX();
			y2 += F2.getY();
		}
		int WL1 = abs(x1 - x2) + abs(y1 - y2);
		int WL2 = abs(x - x2) + abs(y - y2);
		dWL = (float) WL2 - WL1;
	}
}
void Board::updateQSlack(string qname, int x, int y, float dq) {
	size_t pos1 = qname.find("/");
	int cnum1 = stoi(qname.substr(1, pos1));
	for (auto& it : Qconnect[qname]) {
		size_t pos2 = it.find("/");
		int cnum2 = stoi(it.substr(1, pos2));
		string dname = it.substr(pos2 + 1, string::npos);
		vector<float> tmp;
		float Max = 0;
		for (auto& f : Ddelay[it]) {
			float delay = f.second.first;
			string qpin = f.first;
			Max = delay > Max ? delay : Max;
			if (qpin == qname) {
				FlipFlop F1 = InstToFlipFlop[cnum1];     // Q
				Point p1 = NametoPoint(qname);
				int x1 = F1.getX() + p1.x;
				int y1 = F1.getY() + p1.y;
				Point p2 = NametoPoint(f.second.second); // in
				int x2 = p2.x;
				int y2 = p2.y;
				size_t pos3 = f.second.second.find("/");
				if (pos3 != string::npos) {
					int cnum3 = stoi(f.second.second.substr(1, pos3));
					Cell F3 = getCell(cnum3);
					x2 += F3.getX();
					y2 += F3.getY();
				}
				int WL1 = abs(x1 - x2) + abs(y1 - y2);
				int WL2 = abs(x - x2) + abs(y - y2);
				float dWL = (float) WL2 - WL1;
				delay += dq + dWL * DisplacementDelay;
			}
			tmp.push_back(delay);
		}
		sort(tmp.begin(), tmp.end(), fltComp);
		if (tmp[0] != Max) {
			float pslack = InstToFlipFlop[cnum2].getSlack()[dname];
			InstToFlipFlop[cnum2].setSlack(dname, pslack - tmp[0] + Max);
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
				float NS = f.getSlack()[p.name];
				float dWL;
				int fx = InstToFlipFlop[CellNumber].getX() + curPin[d].x;
				int fy = InstToFlipFlop[CellNumber].getY() + curPin[d].y;
				updateDSlack(prev, dWL, fx, fy);
				InstToFlipFlop[CellNumber].setSlack(curPin[d].name, NS - DisplacementDelay * dWL);
				auto tmp = Ddelay[prev];
				for (auto& it : tmp) {
					if (it.second.second == prev) {
						Ddelay[cur][it.first] = make_pair(it.second.first + DisplacementDelay * dWL, cur);
					}
					else {
						Ddelay[cur][it.first] = make_pair(it.second.first + DisplacementDelay * dWL, it.second.second);
					}
					Qconnect[it.first].insert(cur);
					Ddelay[prev].erase(it.first);
					Qconnect[it.first].erase(prev);
				}
				Ddelay.erase(prev);
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
				int fx = InstToFlipFlop[CellNumber].getX() + curPin[q].x;
				int fy = InstToFlipFlop[CellNumber].getY() + curPin[q].y;
				float dq = F2.getQpinDelay() - f.getQpinDelay();
				updateQSlack(prev, fx, fy, dq);
				auto tmp = Qconnect[prev];
				for (auto& it : tmp) {
					Qconnect[cur].insert(it);
					pair<float, string> tmp2 = Ddelay[it][prev];
					Ddelay[it][cur] = make_pair(tmp2.first, tmp2.second);
					Qconnect[prev].erase(it);
					Ddelay[it].erase(prev);
				}
				Qconnect.erase(prev);
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
			int netnum = PointToNet[prev];
			addNet(netnum, cur);
			removeNet(netnum, prev);
		}
		// delete cell from location
		int fx = f.getX();
		int fy = f.getY();
		for (size_t k = 0; k < Location[fx][fy].size(); k++) {
			if (Location[fx][fy][k] == f.getInstNum()) {
				Location[fx][fy].erase(Location[fx][fy].begin() + k);
			}
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
		int x = f.getX();
		int y = f.getY();
		Location[x][y].push_back(CellNumber);
		InstToFlipFlop[CellNumber] = f;
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
			int qq = 0;
			if (p.type == 'D') {
				while (prevPin[d].type != 'D') {
					d++;
				}
				string D = "D" + to_string(dd);
				if (f.getN() == 1) {
					D = "D";
				}
				f.setsource(D, prevPin[d].sourcename[0]);
				dd++;
				prev = prevcell + "/" + prevPin[d].name;
				map<string, bool> visited;
				float NS = F1.getSlack()[prevPin[d].name];
				int fx = f.getX() + p.x;
				int fy = f.getY() + p.y;
				updateDSlack(prev, NS, fx, fy);
				f.setSlack(p.name, NS);
				netnum = PointToNet[prev];
				removeNet(netnum, prev);
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
				prev = prevcell + "/" + prevPin[q].name;
				float dq = F1.getQpinDelay() - f.getQpinDelay();
				int fx = f.getX() + p.x;
				int fy = f.getY() + p.y;
				updateQSlack(prev, fx, fy, dq);
				netnum = PointToNet[prev];
				removeNet(netnum, prev);
				q++;
			}
			else if (p.type == 'C') {
				while (prevPin[c].type != 'C') {
					c++;
				}
				for (int ij = w; ij < f.getN() + w; ij++) {
					f.setsource("CLK", prevPin[c].sourcename[ij]);
				}
				prev = prevcell + "/" + prevPin[c].name;
				netnum = PointToNet[prev];
			}
			else {
				cout << prev << " not existing\n";
				break;
			}
			addNet(netnum, cur);
		}
		// add cell to location
		int x = f.getX();
		int y = f.getY();
		Location[x][y].push_back(CellNumber);
		InstToFlipFlop[CellNumber] = f;
		w += f.getN();
		CellNumber++;
	}
	string prev = prevcell + "/" + prevPin[c].name;
	int netnum = PointToNet[prev];
	removeNet(netnum, prev);
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
				while (curPin[d].type != 'D') {
					d++;
				}
				string PinName = f.getInstName() + "/" + p.name;
				float NS = f.getSlack()[p.name];
				int fx = cur.getX() + curPin[d].x;
				int fy = cur.getY() + curPin[d].y;
				Dslack(PinName, NS, fx, fy);
				NSComp += NS;
				d++;
			}
			else if (p.type == 'Q') {
				while (curPin[q].type != 'Q') {
					q++;
				}
				string PinName = f.getInstName() + "/" + p.name;
				float NS = 0;
				int fx = cur.getX() + curPin[q].x;
				int fy = cur.getY() + curPin[q].y;
				float dq = cur.getQpinDelay() - f.getQpinDelay();
				Qslack(PinName, NS, fx, fy, dq);
				NSComp += NS;
				q++;
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
			while (curPin[d].type != 'D') {
				d++;
			}
			string PinName = prev.getInstName() + "/" + p.name;
			float NS = 0;
			int fx = cur.getX() + curPin[d].x;
			int fy = cur.getY() + curPin[d].y;
			Dslack(PinName, NS, fx, fy);
			NSComp += NS;
			d++;
		}
		else if (p.type == 'Q') {
			while (curPin[q].type != 'Q') {
				q++;
			}
			string PinName = prev.getInstName() + "/" + p.name;
			float NS = 0;
			int fx = cur.getX() + curPin[q].x;
			int fy = cur.getY() + curPin[q].y;
			float dq = cur.getQpinDelay() - prev.getQpinDelay();
			Qslack(PinName, NS, fx, fy, dq);
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
		if (PlacementRows.find(y) == PlacementRows.end())  {
			cout << "y of " << it.first << " not on grid point\n";
			it.second.display();
			return false;
		}
		auto p = PlacementRows[y];
		int sx = p.first;
		if (sx > x || ((x - sx) % p.second[0])
			|| (x - sx) / p.second[0] > p.second[2]) {
			cout << "x of " << it.first << " not on grid point\n";
			it.second.display();
			return false;
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
	for (int i = 0; i < n - 1; i++) {
		for (int j = i + 1; j < n; j++) {
			if (arr[i].ex > arr[j].sx) {
				if ((arr[i].ey > arr[j].sy && arr[i].sy < arr[j].ey)) {
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
void Board::setDelay(string Q, string in, string D, int WL) {
	auto end = string::npos;
	size_t pos2 = Q.find("/");
	int cnum2 = stoi(Q.substr(1, pos2));
	string qname = Q.substr(pos2 + 1, end);
	float delay = InstToFlipFlop[cnum2].getQpinDelay() + WL * DisplacementDelay;
	Qconnect[Q].insert(D);
	Ddelay[D][Q] = make_pair(delay, in);
}
void Board::setwl(unordered_map<string, pair<string, int>> DCON, unordered_map<string, unordered_map<string, int>> Qcon) {
	this->Dcon = DCON;
	this->Qcon = Qcon;
}
