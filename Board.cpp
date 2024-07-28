#include "Board.h"
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
using namespace std;
bool ffComp(pair<string, FlipFlop> p1, pair<string, FlipFlop> p2) {
	return p1.second.getN() > p2.second.getN();
}
void merge(vector<node>& arr, int front, int mid, int end) {
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
void mergeSort(vector<node>& arr, int front, int end) {
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
}
Board::~Board() {}
void Board::ReadFile() {
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
	fin >> NumInst;
	CellNumber += NumInst;
	for (int i = 0; i < NumInst; i++) {
		fin >> Str >> InstName >> name >> x >> y;
		if (FlipFlopLib.find(name) != FlipFlopLib.end()) {
			FlipFlop F = FlipFlopLib[name];
			F.setPos(x, y);
			F.setInstName(InstName);
			InstToFlipFlop[InstName] = F;
			for (auto& it : F.getPin()) {
				string instname = InstName + "/" + it.name;
				InstToFlipFlop[InstName].setsource(it.name, instname);
				//cout << "st\n";
				
			}//memory
			//cout << InstToFlipFlop[InstName].getsource("CLK")[0] << endl;
			Location[x][y].push_back(InstName);
		}
		else {
			Gate G = GateLib[name];
			G.setPos(x, y);
			G.setInstName(InstName);
			InstToGate[InstName] = G;
			Location[x][y].push_back(InstName);
		}
	}
	int NumNets;
	fin >> Str >> NumNets;
	for (int i = 0; i < NumNets; i++) {
		int n;
		string PinName;
		fin >> Str >> name >> n;
		set<string> tmp;
		vector<string> tmp2;
		for (int j = 0; j < n; j++) {
			fin >> Str >> PinName;
			tmp.insert(PinName);
			PointToNet[PinName] = name;
			if (PinName.find("CLK") != string::npos) {
				size_t pos = PinName.find("/");
				tmp2.push_back(PinName.substr(0, pos));
			}
		}
		Net[name] = tmp;
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
		FlipFlopLib[FlipFlopName].setQpinDelay(delay);
	}
	int sum = 0;
	for (auto& it : InstToFlipFlop) {
		sum += it.second.getN();
	}
	for (int i = 0; i < sum; i++) {
		fin >> Str >> InstName >> name >> slack;
		InstToFlipFlop[InstName].setSlack(name, slack);
	}
	float power;
	for (auto& it : FlipFlopLib) {
		fin >> Str >> name >> power;
		FlipFlopLib[name].setPower(power);
	}
	fin.close();
}

void Board::Display() {
	string c1 = "C101863", c2 = "C101864";
	vector<FlipFlop> F1;
	F1.push_back(InstToFlipFlop[c1]);
	F1.push_back(InstToFlipFlop[c2]);
	int x1 = F1[0].getX();
	int y1 = F1[0].getY();
	int x2 = F1[1].getX();
	int y2 = F1[1].getY();
	FlipFlop F2;
	for (auto& it : FlipFlopLib) {
		if (it.second.getN() == 2) {
			F2 = it.second;
			F2.setPos(F1[0].getX(), F1[0].getY());
			Banking(F1, F2);
			Debanking(F2, F1);
			break;
		}
	}
	ofstream fout;
	fout.open("output.txt");
	fout << "CellInst " << NewFlipFlop.size() << "\n";
	for (auto& it : NewFlipFlop) {
		FlipFlop f = InstToFlipFlop[it];
		fout << "Inst " << it << " " << f.getCellName() << " " << f.getX() << " " << f.getY() << "\n";
	}
	for (auto& it: InstToFlipFlop) {
		for (auto& it2 : it.second.getPin()) {
			for (size_t i = 0; i < it2.sourcename.size(); i++) {
				string ss= it.first + "/" + it2.name;
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
				tmp.push_back(InstToFlipFlop[n.substr(0, n.find("/"))]);
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
		string cname = PinName.substr(0, pos);
		if (InstToFlipFlop.find(cname) != InstToFlipFlop.end()) {
			string pname = PinName.substr(pos + 1, string::npos);
			return InstToFlipFlop[cname].getPoint(pname);
		}
		if (InstToGate.find(cname) != InstToGate.end()) {
			string pname = PinName.substr(pos + 1, string::npos);
			return InstToGate[cname].getPoint(pname);
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
	return Point(-1, -1, "");
}
Cell Board::getCell(string CellName) {
	if (InstToFlipFlop.find(CellName) != InstToFlipFlop.end()) {
		return InstToFlipFlop[CellName];
	}
	else if (InstToGate.find(CellName) != InstToGate.end()) {
		return InstToGate[CellName];
	}
	else {
		cout << CellName << " not find\n";
		return Cell();
	}
}
void Board::addNet(string NetName, string PinName) {
	Net[NetName].insert(PinName);
	PointToNet[PinName] = NetName;
}
void Board::removeNet(string NetName, string PinName) {
	Net[NetName].erase(PinName);
	PointToNet.erase(PinName);
}
void Board::Ddfs(string PinName, float &NS, int x, int y) {
	string netname = PointToNet[PinName];
	for (auto& it : Net[netname]) {
		if (it.find("Q") != string::npos || it.find("OUT") != string::npos
			|| it.find("INPUT") != string::npos) {
			size_t pos = it.find("/");
			string cname = it.substr(0, pos);
			Point P = NametoPoint(PinName);
			Point tmp = NametoPoint(it);
			FlipFlop F1 = InstToFlipFlop[PinName.substr(0, PinName.find("/"))];
			Cell F2;
			int x2, y2;
			if (pos != string::npos) {
				F2 = getCell(cname);
				x2 = tmp.x + F2.getX();
				y2 = tmp.y + F2.getY();
			}
			else {
				x2 = tmp.x;
				y2 = tmp.y;
			}
			int x1 = P.x + F1.getX();
			int y1 = P.y + F1.getX();
			float WL = (float)abs(x1 - x2) + abs(y1 - y2) - abs(x - x2) - abs(y - y2);
			float nslack = NS < 0 ? NS : 0;
			float newslack = NS + DisplacementDelay * WL;
			float newnslack = newslack < 0 ? newslack : 0;
			NS = newnslack - nslack;
			/*std::cout << it << " " << x2 << " " << y2 << "\n";
			std::cout << "Initial\n" << PinName << " " << x1 << " " << y1 <<"\n";
			std::cout << "After\n" << x << " " << y << "\n";
			std::cout << "WL = " << WL << "\n";*/
			return;
		}
	}
}
void Board::Qdfs(string PinName, map<string, bool>& visited, int WL, float &NS, int x, int y) {
	visited[PinName] = true;
	string netname = PointToNet[PinName];
	for (auto& it : Net[netname]) {
		size_t pos = it.find("/");
		if (visited[it] || pos == string::npos)continue;
		string cname = it.substr(0, pos);
		string pname = it.substr(pos + 1, string::npos);
		if (PinName.find("Q") != string::npos) {
			Point P = NametoPoint(PinName);
			Point tmp = NametoPoint(it);
			FlipFlop F1 = InstToFlipFlop[PinName.substr(0, PinName.find("/"))];
			Cell F2 = getCell(cname);
			int x1 = P.x + F1.getX();
			int y1 = P.y + F1.getX();
			int x2 = tmp.x + F2.getX();
			int y2 = tmp.y + F2.getY();
			WL = abs(x1 - x2) + abs(y1 - y2) - abs(x - x2) - abs(y - y2);
			/*std::cout << it << " " << x2 << " " << y2 << "\n";
			std::cout << "Initial\n" << PinName << " " << x1 << " " << y1 <<"\n";
			std::cout << "After\n" << x << " " << y << "\n";
			std::cout << "WL = " << WL << "\n";*/
		}
		//cout << it << " " << WL << "\n";
		if (InstToFlipFlop.find(cname) == InstToFlipFlop.end()) {
			for (auto &p : InstToGate[cname].getPin()) {
				if (p.name.find("OUT") == string::npos) continue;
				Qdfs(cname + "/" + p.name, visited, WL, NS, x, y);
			}
		}  // connect to gate
		else {
			float slack = InstToFlipFlop[cname].getSlack()[pname];
			float nslack = slack < 0 ? slack : 0;
			float newslack = slack + DisplacementDelay * WL;
			float newnslack = newslack < 0 ? newslack : 0;
			NS -= nslack;
			NS += newnslack;
		}  // connect to flipflop
	}
}
void Board::updateDSlack(string PinName, float &NS, int x, int y) {
	string netname = PointToNet[PinName];
	for (auto& it : Net[netname]) {
		if (it.find("Q") != string::npos || it.find("OUT") != string::npos
			|| it.find("INPUT") != string::npos) {
			size_t pos = PinName.find("/");
			string cname = PinName.substr(0, pos);
			Point P = NametoPoint(PinName);
			Point tmp = NametoPoint(it);
			FlipFlop F1 = InstToFlipFlop[cname];
			Cell F2;
			int x2, y2;
			pos = it.find("/");
			cname = it.substr(0, pos);
			if (pos != string::npos) {
				if (InstToFlipFlop.find(cname) != InstToFlipFlop.end()) {
					F2 = InstToFlipFlop[cname];
				}
				else if (InstToGate.find(cname) != InstToGate.end()) {
					F2 = InstToGate[cname];
				}
				else {
					cout << cname << " can't be found\n";
				}
				x2 = tmp.x + F2.getX();
				y2 = tmp.y + F2.getY();
			}
			else {
				x2 = tmp.x;
				y2 = tmp.y;
			}
			int x1 = P.x + F1.getX();
			int y1 = P.y + F1.getY();
			float WL = (float)abs(x1 - x2) + abs(y1 - y2) - abs(x - x2) - abs(y - y2);
			NS = NS + DisplacementDelay * WL;
			return;
		}
	}
}
void Board::updateQSlack(string PinName, map<string, bool> &visited, float WL, int x, int y) {
	visited[PinName] = true;
	string netname = PointToNet[PinName];
	for (auto& it : Net[netname]) {
		size_t pos = it.find("/");
		if (visited[it] || pos == string::npos) continue;
		string cname = it.substr(0, pos);
		string pname = it.substr(pos + 1, string::npos);
		if (PinName.find("Q") != string::npos) {
			Point P = NametoPoint(PinName);
			Point tmp = NametoPoint(it);
			FlipFlop F1 = InstToFlipFlop[PinName.substr(0, PinName.find("/"))];
			Cell F2;
			if (InstToFlipFlop.find(cname) != InstToFlipFlop.end()) {
				F2 = InstToFlipFlop[cname];
			}
			else if (InstToGate.find(cname) != InstToGate.end()) {
				F2 = InstToGate[cname];
			}
			// prev and connect pin
			int x1 = P.x + F1.getX();
			int y1 = P.y + F1.getY();
			int x2 = tmp.x + F2.getX();
			int y2 = tmp.y + F2.getY();
			WL = (float)abs(x1 - x2) + abs(y1 - y2) - abs(x - x2) - abs(y - y2);
		}
		if (InstToFlipFlop.find(cname) == InstToFlipFlop.end()) {
			for (auto& p : InstToGate[cname].getPin()) {
				if (p.name.find("OUT") == string::npos) continue;
				updateQSlack(cname + "/" + p.name, visited, WL, x, y);
			}
		}  // connect to gate
		else {
			float slack = InstToFlipFlop[cname].getSlack()[pname];
			float newslack = slack + DisplacementDelay * WL;
			InstToFlipFlop[cname].setSlack(pname, newslack);
			cout << it << " " << newslack << "\n";
		}  // connect to flipflop
	}
}
void Board::Banking(vector<FlipFlop> F1, FlipFlop& F2) {
	string FlipFlopName = "C" + to_string(CellNumber);
	CellNumber++;
	NewFlipFlop.insert(FlipFlopName);
	F2.setInstName(FlipFlopName);
	vector<Point> curPin = F2.getPin();
	int d = 0, q = 0, c = 0, w = 0;
	for (size_t j = 0; j < F1.size(); j++) {
		vector<Point> PrevPin = F1[j].getPin();
		// add new flipflop
		if (NewFlipFlop.find(F1[j].getInstName()) != NewFlipFlop.end()) {
			NewFlipFlop.erase(F1[j].getInstName());
		}
		string prevcell = F1[j].getInstName();
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
				if (F2.getN() == 1) {
					D = "D";
				}
				F2.setsource(D, p.sourcename[0]);
				dd++;  //memory
				cur = FlipFlopName + "/" + curPin[d].name;
				map<string, bool> visited;
				float NS = F1[j].getSlack()[p.name];
				int fx = F2.getX() + curPin[d].x;
				int fy = F2.getY() + curPin[d].y;
				updateDSlack(prev, NS, fx, fy);
				F2.setSlack(curPin[d].name, NS);
				d++;
			}
			else if (p.type == 'Q') {
				while (curPin[q].type != 'Q') {
					q++;
				}
				string Q = "Q" + to_string(qq + w);
				if (F2.getN() == 1) {
					Q = "Q";
				}
				F2.setsource(Q, p.sourcename[0]);
				qq++;  //memory
				cur = FlipFlopName + "/" + curPin[q].name;
				map<string, bool> visited;
				float WL = 0;
				int fx = F2.getX() + curPin[q].x;
				int fy = F2.getY() + curPin[q].y;
				updateQSlack(prev, visited, WL, fx, fy);
				q++;
			}
			else if (p.type == 'C') {
				while (curPin[c].type != 'C') {
					c++;
				}
				for (int ww = 0; ww < p.sourcename.size(); ww++) {
					F2.setsource("CLK", p.sourcename[ww]);
				}  //memory
				cur = FlipFlopName + "/" + curPin[c].name;
			}
			else {
				cout << prev << " not existing\n";
				break;
			}
			// modify net connection
			string netname = PointToNet[prev];
			Net[netname].erase(prev);
			Net[netname].insert(cur);
			PointToNet.erase(prev);
			PointToNet[cur] = netname;
		}
		// delete cell from location
		int fx = F1[j].getX();
		int fy = F1[j].getY();
		for (size_t k = 0; k < Location[fx][fy].size(); k++) {
			if (Location[fx][fy][k] == F1[j].getInstName()) {
				Location[fx][fy].erase(Location[fx][fy].begin() + k);
			}
		}
		InstToFlipFlop.erase(F1[j].getInstName());
		w += F1[j].getN();  //memory
	}
	//F2[i].display();
	int x = F2.getX();
	int y = F2.getY();
	Location[x][y].push_back(FlipFlopName);
	InstToFlipFlop[FlipFlopName] = F2;
}
void Board::Debanking(FlipFlop F1, vector<FlipFlop>& F2) {
	vector<Point> prevPin = F1.getPin();
	int d = 0, q = 0, c = 0, w = 0;
	string prevcell = F1.getInstName();
	for (size_t j = 0; j < F2.size(); j++) {
		string FlipFlopName = "C" + to_string(CellNumber);
		CellNumber++;
		NewFlipFlop.insert(FlipFlopName);
		F2[j].setInstName(FlipFlopName);
		vector<Point> curPin = F2[j].getPin();
		// add new flipflop
		if (NewFlipFlop.find(F1.getInstName()) != NewFlipFlop.end()) {
			NewFlipFlop.erase(F1.getInstName());
		}
		for (auto& p : curPin) {
			string cur = FlipFlopName + "/" + p.name;
			string prev, netname;
			// Pin mapping to Pin for 1 bit
			int dd = 0;
			int qq = 0;  //memory
			if (p.type == 'D') {
				while (prevPin[d].type != 'D') {
					d++;
				}
				string D = "D" + to_string(dd);
				if (F2[j].getN() == 1) {
					D = "D";
				}
				F2[j].setsource(D, prevPin[d].sourcename[0]);
				dd++;  //memory
				prev = prevcell + "/" + prevPin[d].name;
				map<string, bool> visited;
				float NS = F1.getSlack()[prevPin[d].name];
				int fx = F2[j].getX() + p.x;
				int fy = F2[j].getY() + p.y;
				updateDSlack(prev, NS, fx, fy);
				F2[j].setSlack(p.name, NS);
				netname = PointToNet[prev];
				Net[netname].erase(prev);
				PointToNet.erase(prev);
				d++;
			}
			else if (p.type == 'Q') {
				while (prevPin[q].type != 'Q') {
					q++;
				}
				string Q = "Q" + to_string(qq);
				if (F2[j].getN() == 1) {
					Q = "Q";
				}
				F2[j].setsource(Q, prevPin[q].sourcename[0]);
				qq++;
				//memory
				prev = prevcell + "/" + prevPin[q].name;
				map<string, bool> visited;
				float WL = 0;
				int fx = F2[j].getX() + p.x;
				int fy = F2[j].getY() + p.y;
				updateQSlack(prev, visited, WL, fx, fy);
				netname = PointToNet[prev];
				Net[netname].erase(prev);
				PointToNet.erase(prev);
				q++;
			}
			else if (p.type == 'C') {
				while (prevPin[c].type != 'C') {
					c++;
				}
				for (int ij = w; ij < F2[j].getN() + w; ij++) {
					F2[j].setsource("CLK", prevPin[c].sourcename[ij]);
				}  //memory
				prev = prevcell + "/" + prevPin[c].name;
				netname = PointToNet[prev];
			}
			else {
				cout << prev << " not existing\n";
				break;
			}
			Net[netname].insert(cur);
			PointToNet[cur] = netname;
		}
		// add cell to location
		int x = F2[j].getX();
		int y = F2[j].getY();
		Location[x][y].push_back(FlipFlopName);
		InstToFlipFlop[FlipFlopName] = F2[j];
		w += F2[j].getN();  //memory
	}
	string prev = prevcell + "/" + prevPin[c].name;
	string netname = PointToNet[prev];
	Net[netname].erase(prev);
	PointToNet.erase(prev);
	int fx = F1.getX();
	int fy = F1.getY();
	for (size_t k = 0; k < Location[fx][fy].size(); k++) {
		if (Location[fx][fy][k] == F1.getInstName()) {
			Location[fx][fy].erase(Location[fx][fy].begin() + k);
		}
	}
	InstToFlipFlop.erase(F1.getInstName());
}
float Board::bankingCompare(vector<FlipFlop> prev, FlipFlop cur) {
	int N = cur.getN();
	int x = cur.getX();
	int y = cur.getY();
	float sum = 0;
	float PowerComp = cur.getPower();
	float AreaComp = (float)cur.getArea();
	float NSComp = 0;
	for (auto& f : prev) {
		PowerComp -= f.getPower();
		AreaComp -= f.getArea();
	}
	for (auto& f : prev) {
		vector<Point> prevPin = f.getPin();
		vector<Point> curPin = cur.getPin();
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
				while (curPin[q].type != 'Q') {
					q++;
				}
				int fx = cur.getX() + curPin[q].x;
				int fy = cur.getY() + curPin[q].y;
				Qdfs(PinName, visited, WL, NS, fx, fy);
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
	float PowerComp = (float)cur.getPower();
	float AreaComp = (float)cur.getArea();
	float NSComp = 0;
	PowerComp -= prev.getPower();
	AreaComp -= prev.getArea();
	vector<Point> prevPin = prev.getPin();
	vector<Point> curPin = cur.getPin();
	int d = 0, q = 0, c = 0;
	for (auto& p : prevPin) {
		if (p.type == 'D') {
			map<string, bool> visited;
			string PinName = prev.getInstName() + "/" + p.name;
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
			string PinName = prev.getInstName() + "/" + p.name;
			int WL = 0;
			float NS = 0;
			while (curPin[q].type != 'Q') {
				q++;
			}
			int fx = cur.getX() + curPin[q].x;
			int fy = cur.getY() + curPin[q].y;
			Qdfs(PinName, visited, WL, NS, fx, fy);
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
				int num = stoi(c.substr(1));
				node n(F.getX(), F.getRight(), F.getY(), F.getTop(), num);
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
int Board::dist(Point P1, Point P2) {
	return abs(P1.x - P2.x) + abs(P1.y - P2.y);
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
				BinDensity[i][j] += (float)w * h;
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
				BinDensity[i][j] += (float)w * h;
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
	return (int)InstToFlipFlop.size();
}
