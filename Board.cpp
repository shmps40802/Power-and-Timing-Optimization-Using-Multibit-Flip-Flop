#include "Board.h"
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
bool ffComp(pair<string, FlipFlop> p1, pair<string, FlipFlop> p2) {
	return p1.second.getN() > p2.second.getN();
}
void sort(map<string, FlipFlop>& M) { 
    vector<pair<string, FlipFlop>> A; 
    for (auto& it : M) {
        A.push_back(it);
    }
    sort(A.begin(), A.end(), ffComp);
}
Board::Board() {
	/*Alpha = 0;
	Beta = 0;
	Gemma = 0;
	Delta = 0;
	LowerLeftX = 0;
	LowerLeftY = 0;
	HigherRightX = 0;
	HigherRightY = 0;*/
	CellNumber = 1;
}
void Board::ReadFile() {
	ifstream fin;
	string Filename = "testcase1.txt";
	//cout << "filename :";
	//cin >> Filename;
	fin.open(Filename);
	while(!fin.is_open()){
		std::cout << Filename << " can't be opened\nfilename : ";
		cin >> Filename;
		fin.open(Filename);
	}
	/*
	Alpha 1
	Beta 5
	Gamma 5
	Delta 1
	*/
	string Str;
	fin >> Str >> Alpha;
	fin >> Str >> Beta;
	fin >> Str >> Gemma;
	fin >> Str >> Delta;
	/*
	DieSize 0 0 50 30
	NumInput 2
	Input INPUT0 0 25
	Input INPUT1 0 5
	NumOutput 2
	Output OUTPUT0 50 25
	Output OUTPUT1 50 5
	*/
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
	/*
	FlipFlop 1 FF1 5 10 2
	Pin D 0 8
	Pin Q 5 8
	FlipFlop 2 FF2 8 10 4
	Pin D0 0 9
	Pin D1 0 6
	Pin Q0 8 9
	Pin Q1 8 6
	*/
	int N, P;
	string FlipFlopName;
	string GateName;
	while (true) {
		int Width, Height;
		fin >> Str;
		if (Str == "FlipFlop") {
			fin >> N >> FlipFlopName >> Width >> Height >> P;
			vector<Point> pin;
			for(int i = 0; i < P; i++) {
				fin >> Str >> name >> x >> y;
				Point tmp(x, y, name);
				if(name.substr(0, 1) == "D") {
					tmp.type = 'D';
				}
				else if(name.substr(0, 1) == "Q") {
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
		else if(Str == "Gate") {
			fin >> GateName >> Width >> Height >> P;
			vector<Point> Pin;
			for(int i = 0; i < P; i++){
				fin >> Str >> name >> x >> y;
				Point tmp(x, y, name);
				Pin.push_back(tmp);
			}
			GateLib[GateName] = Gate(Width, Height, P, Pin);
			GateLib[GateName].setCellName(GateName);
		}
		else break;
	}
	//sort(FlipFlopLib);
	/*
	NumInstances 2
	Inst C1 FF1 15 20
	Inst C2 FF1 15 0
	*/
	string InstName;
	fin >> NumInst;
	CellNumber += NumInst;
	for (int i = 0; i < NumInst; i++) {
		fin >> Str >> InstName >> name >> x >> y;
		if(FlipFlopLib.find(name) != FlipFlopLib.end()) {
			FlipFlop F = FlipFlopLib[name];
			F.setPos(x, y);
			F.setInstName(InstName);
			InstToFlipFlop[InstName] = F;
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
	/*
	NumNets 4
	Net N1 2
	Pin INPUT0
	Pin C1/D
	Net N2 2
	Pin INPUT1
	Pin C2/D
	Net N3 2
	Pin C1/Q
	Pin OUTPUT0
	Net N4 2
	Pin C2/Q
	Pin OUTPUT1
	*/
	int NumNets;
	fin >> Str >> NumNets;
	for(int i = 0; i < NumNets; i++) {
		int n;
		string PinName;
		fin >> Str >> name >> n;
		set<string> tmp;
		for(int j = 0; j < n; j++) {
			fin >> Str >> PinName;
			tmp.insert(PinName);
			PointToNet[PinName] = name;
		}
		Net[name] = tmp;
	}
	/*
	BinWidth 10
	BinHeight 10
	BinMaxUtil 79
	PlacementRows 0 0 2 10
	*/
	fin >> Str >> BinWidth;
	fin >> Str >> BinHeight;
	fin >> Str >> BinMaxUtil;
	while(true) {
		fin >> Str;
		if(Str != "PlacementRows")break;
		fin >> StartX >> StartY >> SiteWidth >> SiteHeight >> TotalNumOfSites;
		PlacementRows[{StartX, StartY}] = {SiteWidth, SiteHeight, TotalNumOfSites};
		// PlacementRows
	}
	/*
	DisplacementDelay 0.01
	QpinDelay FF1 1
	QpinDelay FF2 2
	TimingSlack C1 D 1
	TimingSlack C1 Q 0
	TimingSlack C2 D 1
	TimingSlack C2 Q 0
	GatePower FF1 10
	GatePower FF2 17
	*/
	fin >> DisplacementDelay;
	float delay, slack;
	for(size_t i = 0; i < FlipFlopLib.size(); i++) {
		fin >> Str >> FlipFlopName >> delay;
		FlipFlopLib[FlipFlopName].setQpinDelay(delay);
	}
	int sum = 0;
	for (auto &it : InstToFlipFlop) {
		sum += it.second.getN();
	}
	for(int i = 0; i < sum; i++) {
		fin >> Str >> InstName >> name >> slack;
		InstToFlipFlop[InstName].setSlack(name, slack);
	}
	float power;
	for (auto &it : FlipFlopLib) {
		fin >> Str >> name >> power;
		FlipFlopLib[name].setPower(power);
	}
	fin.close();
}
void Board::Display() {
	/*vector<vector<FlipFlop>> data;
	map<int, int> count;
	vector<string> Clk;
	for(auto &it : Net) {
		vector<FlipFlop> tmp;
		for(auto &n : it.second) {
			if(n.find("CLK") != string::npos) {
				tmp.push_back(InstToFlipFlop[n.substr(0, n.find("/"))]);
			}
		}
		if(!tmp.empty()) {
			data.push_back(tmp);\
			count[tmp.size()]++;
		}
	}
	int sum = 0;
	for(auto &it : count){
		cout << it.first << " " << it.second << "\n";
		sum += it.first * it.second;
	}
	int t = 0;
	fout << "axis equal;\n";
	fout << "hold on;\n";
	fout << "grid on;\n";
	for(size_t j = 0; j < data[t].size(); j++) {
		int sx = data[t][j].getX();
		int sy = data[t][j].getY();
		int ex = sx + data[t][j].getWidth();
		int ey = sy + data[t][j].getHeight();
		fout << "cap_x = [" << sx << " " << sx << " " << ex << " " << ex << " " << sx << "];\n";
		fout << "cap_y = [" << sy << " " << ey << " " << ey << " " << sy << " " << sy << "];\n";
		fout <<	"fill(cap_x, cap_y, 'c');\n";
	}*/
	string c1 = "C101863", c2 = "C101864";
	vector<FlipFlop> F1;
	F1.push_back(InstToFlipFlop[c1]);
	F1.push_back(InstToFlipFlop[c2]);
	int x1 = F1[0].getX();
	int y1 = F1[0].getY();
	int x2 = F1[1].getX();
	int y2 = F1[1].getY();
	vector<FlipFlop> F2;
	for(auto &it : FlipFlopLib) {
		if(it.second.getN() == 2) {
			F2.push_back(it.second);
			F2[0].setPos(F1[0].getX(), F1[0].getY());
			Banking(vector<vector<FlipFlop>>{F1}, F2);
			Debanking(vector<FlipFlop>{F2}, vector<vector<FlipFlop>>{F1});
			break;
		}
	}
	ofstream fout;
	fout.open("output.txt");
	fout.close();
}
Point Board::NametoPoint(string PinName) {
	// flipflop gate pin
	size_t pos = PinName.find("/");
	if(pos != string::npos) {
		string cname = PinName.substr(0, pos);
		if(InstToFlipFlop.find(cname) != InstToFlipFlop.end()) {
			string pname = PinName.substr(pos + 1, string::npos);
			return InstToFlipFlop[cname].getPoint(pname);
		}
		if(InstToGate.find(cname) != InstToGate.end()) {
			string pname = PinName.substr(pos + 1, string::npos);
			return InstToGate[cname].getPoint(pname);
		}
	}
	// input output pin
	else {
		for(auto &it : Input) {
			if(it.name == PinName) {
				return it;
			}
		}
		for(auto &it : Output){
			if(it.name == PinName) {
				return it;
			}
		}
	}
	std::cout << "Pin " << PinName << " can't be found\n";
	return Point(-1, -1, "");
}
Board::~Board() {}
void Board::Ddfs(string PinName, float &NS, int x, int y) {
	string netname = PointToNet[PinName];
	for(auto &it : Net[netname]) {
		if(it.find("Q") != string::npos || it.find("OUT") != string::npos
		|| it.find("INPUT") != string::npos) {
			size_t pos = it.find("/");
			string cname = it.substr(0, pos);
			Point P = NametoPoint(PinName);
			Point tmp = NametoPoint(it);
			FlipFlop F1 = InstToFlipFlop[PinName.substr(0, PinName.find("/"))];
			Cell F2;
			int x2, y2;
			if(pos != string::npos) {
				if(InstToFlipFlop.find(cname) != InstToFlipFlop.end()) {
					F2 = InstToFlipFlop[cname];
				}
				else if(InstToGate.find(cname) != InstToGate.end()) {
					F2 = InstToGate[cname];
				}
				else {
					std::cout << cname << " can't be found\n";
				}
				x2 = tmp.x + F2.getX();
				y2 = tmp.y + F2.getY();
			}
			else{
				x2 = tmp.x;
				y2 = tmp.y;
			}
			int x1 = P.x + F1.getX();
			int y1 = P.y + F1.getX();
			float WL = abs(x1 - x2) + abs(y1 - y2) - abs(x - x2) - abs(y - y2);
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
void Board::Qdfs(string PinName, map<string, bool> &visited, int WL, float &NS, int x, int y) {
	visited[PinName] = true;
	string netname = PointToNet[PinName];
	for(auto &it : Net[netname]) {
		size_t pos = it.find("/");
		if(visited[it] || pos == string::npos)continue;
		string cname = it.substr(0, pos);
		string pname = it.substr(pos + 1, string::npos);
		if(PinName.find("Q") != string::npos) {
			Point P = NametoPoint(PinName);
			Point tmp = NametoPoint(it);
			FlipFlop F1 = InstToFlipFlop[PinName.substr(0, PinName.find("/"))];
			Cell F2;
			if(InstToFlipFlop.find(cname) != InstToFlipFlop.end()) {
				F2 = InstToFlipFlop[cname];
			}
			else if(InstToGate.find(cname) != InstToGate.end()) {
				F2 = InstToGate[cname];
			}
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
		if(InstToFlipFlop.find(cname) == InstToFlipFlop.end()) {
			for(auto &p : InstToGate[cname].getPin()) {
				if(p.name.find("OUT") == string::npos) continue;
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
	for(auto &it : Net[netname]) {
		if(it.find("Q") != string::npos || it.find("OUT") != string::npos
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
			if(pos != string::npos) {
				if(InstToFlipFlop.find(cname) != InstToFlipFlop.end()) {
					F2 = InstToFlipFlop[cname];
				}
				else if(InstToGate.find(cname) != InstToGate.end()) {
					F2 = InstToGate[cname];
				}
				else {
					std::cout << cname << " can't be found\n";
				}
				x2 = tmp.x + F2.getX();
				y2 = tmp.y + F2.getY();
			}
			else{
				x2 = tmp.x;
				y2 = tmp.y;
			}
			int x1 = P.x + F1.getX();
			int y1 = P.y + F1.getY();
			float WL = abs(x1 - x2) + abs(y1 - y2) - abs(x - x2) - abs(y - y2);
			NS = NS + DisplacementDelay * WL;
			return;
		}
	}
}
void Board::updateQSlack(string PinName, map<string, bool> &visited, float WL, int x, int y) {
	visited[PinName] = true;
	string netname = PointToNet[PinName];
	for(auto &it : Net[netname]) {
		size_t pos = it.find("/");
		if(visited[it] || pos == string::npos) continue;
		string cname = it.substr(0, pos);
		string pname = it.substr(pos + 1, string::npos);
		if(PinName.find("Q") != string::npos) {
			Point P = NametoPoint(PinName);
			Point tmp = NametoPoint(it);
			FlipFlop F1 = InstToFlipFlop[PinName.substr(0, PinName.find("/"))];
			Cell F2;
			if(InstToFlipFlop.find(cname) != InstToFlipFlop.end()) {
				F2 = InstToFlipFlop[cname];
			}
			else if(InstToGate.find(cname) != InstToGate.end()) {
				F2 = InstToGate[cname];
			}
			// prev and connect pin
			int x1 = P.x + F1.getX();
			int y1 = P.y + F1.getY();
			int x2 = tmp.x + F2.getX();
			int y2 = tmp.y + F2.getY();
			WL = abs(x1 - x2) + abs(y1 - y2) - abs(x - x2) - abs(y - y2);
		}
		if(InstToFlipFlop.find(cname) == InstToFlipFlop.end()) {
			for(auto &p : InstToGate[cname].getPin()) {
				if(p.name.find("OUT") == string::npos) continue;
				updateQSlack(cname + "/" + p.name, visited, WL, x, y);
			}
		}  // connect to gate
		else {
			float slack = InstToFlipFlop[cname].getSlack()[pname];
			float newslack = slack + DisplacementDelay * WL;
			InstToFlipFlop[cname].setSlack(pname, newslack);
		}  // connect to flipflop
	}
}
void Board::Banking(vector<vector<FlipFlop>> F1, vector<FlipFlop>& F2) {
	int n = F1.size();
	for(int i = 0; i < n; i++) {
		string FlipFlopName = "C" + to_string(CellNumber);
		CellNumber++;
		NewFlipFlop.insert(FlipFlopName);
		F2[i].setInstName(FlipFlopName);
		vector<Point> curPin = F2[i].getPin();
		int d = 0, q = 0, c = 0;
		for(int j = 0; j < F1[i].size(); j++) {
			//F1[i][j].display();
			vector<Point> PrevPin = F1[i][j].getPin();
			// add new flipflop
			if(NewFlipFlop.find(F1[i][j].getInstName()) != NewFlipFlop.end()) {
				NewFlipFlop.erase(F1[i][j].getInstName());
			}
			string prevcell = F1[i][j].getInstName();
			for(auto &p : PrevPin) {
				string prev = prevcell + "/" + p.name;
				string cur;
				// Pin mapping to Pin for 1 bit
				if(p.type == 'D') {
					while(curPin[d].type != 'D') {
						d++;
					}
					cur = FlipFlopName + "/" + curPin[d].name;
					map<string, bool> visited;
					float NS = F1[i][j].getSlack()[p.name];
					int fx = F2[i].getX() + curPin[d].x;
					int fy = F2[i].getY() + curPin[d].y;
					updateDSlack(prev, NS, fx, fy);
					F2[i].setSlack(curPin[d].name, NS);
					d++;
				}
				else if(p.type == 'Q') {
					while(curPin[q].type != 'Q') {
						q++;
					}
					cur = FlipFlopName + "/" + curPin[q].name;
					map<string, bool> visited;
					float WL = 0;
					int fx = F2[i].getX() + curPin[q].x;
					int fy = F2[i].getY() + curPin[q].y;
					updateQSlack(prev, visited, WL, fx, fy);
					q++;
				}
				else if(p.type == 'C') {
					while(curPin[c].type != 'C') {
						c++;
					}
					cur = FlipFlopName + "/" + curPin[c].name;
				}
				else {
					std::cout << prev << " not existing\n";
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
			int fx = F1[i][j].getX();
			int fy = F1[i][j].getY();
			for(size_t k = 0; k < Location[fx][fy].size(); k++) {
				if(Location[fx][fy][k] == F1[i][j].getInstName()) {
					Location[fx][fy].erase(Location[fx][fy].begin() + k);
				}
			}
			InstToFlipFlop.erase(F1[i][j].getInstName());
		}
		//F2[i].display();
		int x = F2[i].getX();
		int y = F2[i].getY();
		Location[x][y].push_back(FlipFlopName);
		InstToFlipFlop[FlipFlopName] = F2[i];
	}
}
void Board::Debanking(vector<FlipFlop> F1, vector<vector<FlipFlop>> F2) {
	int n = F1.size();
	for(int i = 0; i < n; i++) {
		vector<Point> prevPin = F1[i].getPin();
		int d = 0, q = 0, c = 0;
		string prevcell = F1[i].getInstName();
		for(int j = 0; j < F2[i].size(); j++) {
			string FlipFlopName = "C" + to_string(CellNumber);
			CellNumber++;
			NewFlipFlop.insert(FlipFlopName);
			F2[i][j].setInstName(FlipFlopName);
			vector<Point> curPin = F2[i][j].getPin();
			// add new flipflop
			if(NewFlipFlop.find(F1[i].getInstName()) != NewFlipFlop.end()) {
				NewFlipFlop.erase(F1[i].getInstName());
			}
			for(auto &p : curPin) {
				string cur = FlipFlopName + "/" + p.name;
				string prev, netname;
				// Pin mapping to Pin for 1 bit
				if(p.type == 'D') {
					while(prevPin[d].type != 'D') {
						d++;
					}
					prev = prevcell + "/" + prevPin[d].name;
					map<string, bool> visited;
					float NS = F1[i].getSlack()[prevPin[d].name];
					int fx = F2[i][j].getX() + p.x;
					int fy = F2[i][j].getY() + p.y;
					updateDSlack(prev, NS, fx, fy);
					F2[i][j].setSlack(p.name, NS);
					netname = PointToNet[prev];
					Net[netname].erase(prev);
					PointToNet.erase(prev);
					d++;
				}
				else if(p.type == 'Q') {
					while(prevPin[q].type != 'Q') {
						q++;
					}
					prev = prevcell + "/" + prevPin[q].name;
					map<string, bool> visited;
					float WL = 0;
					int fx = F2[i][j].getX() + p.x;
					int fy = F2[i][j].getY() + p.y;
					updateQSlack(prev, visited, WL, fx, fy);
					netname = PointToNet[prev];
					Net[netname].erase(prev);
					PointToNet.erase(prev);
					q++;
				}
				else if(p.type == 'C') {
					while(prevPin[c].type != 'C') {
						c++;
					}
					prev = prevcell + "/" + prevPin[c].name;
					netname = PointToNet[prev];
				}
				else {
					std::cout << prev << " not existing\n";
					break;
				}
				Net[netname].insert(cur);
				PointToNet[cur] = netname;
			}
			// add cell to location
			int x = F2[i][j].getX();
			int y = F2[i][j].getY();
			Location[x][y].push_back(FlipFlopName);
			InstToFlipFlop[FlipFlopName] = F2[i][j];
		}
		string prev = prevcell + "/" + prevPin[c].name;
		string netname = PointToNet[prev];
		Net[netname].erase(prev);
		PointToNet.erase(prev);
		int fx = F1[i].getX();
		int fy = F1[i].getY(); 
		for(size_t k = 0; k < Location[fx][fy].size(); k++) {
			if(Location[fx][fy][k] == F1[i].getInstName()) {
				Location[fx][fy].erase(Location[fx][fy].begin() + k);
			}
		}
		InstToFlipFlop.erase(F1[i].getInstName());
	}
}
float Board::bankingCompare(vector<FlipFlop> prev, FlipFlop cur) {
    int N = cur.getN();
	int x = cur.getX();
	int y = cur.getY();
	float sum = 0;
	float PowerComp = cur.getPower();
	float AreaComp = cur.getArea();
    float NSComp = 0;
	for(auto &f : prev) {
		PowerComp -= f.getPower();
		AreaComp -= f.getArea();
	}
	for(auto &f : prev) {
		vector<Point> prevPin = f.getPin();
		vector<Point> curPin = cur.getPin();
		int d = 0, q = 0, c = 0;
		for(auto &p : prevPin) {
			if(p.type == 'D') {
				map<string, bool> visited;
				string PinName = f.getInstName() + "/" + p.name;
				float WL = 0, NS = f.getSlack()[p.name];
				while(curPin[d].type != 'D') {
					d++;
				}
				int fx = cur.getX() + curPin[d].x;
				int fy = cur.getY() + curPin[d].y;
				Ddfs(PinName, NS, fx, fy);
				d++;
				NSComp += NS;
			}
			else if(p.type == 'Q') {
				map<string, bool> visited;
				string PinName = f.getInstName() + "/" + p.name;
				int WL = 0;
				float NS = 0;
				while(curPin[q].type != 'Q') {
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
	float PowerComp = cur.getPower();
	float AreaComp = cur.getArea();
    float NSComp = 0;
	PowerComp -= prev.getPower();
	AreaComp -= prev.getArea();
	vector<Point> prevPin = prev.getPin();
	vector<Point> curPin = cur.getPin();
	int d = 0, q = 0, c = 0;
	for(auto &p : prevPin) {
		if(p.type == 'D') {
			map<string, bool> visited;
			string PinName = prev.getInstName() + "/" + p.name;
			float WL = 0, NS = prev.getSlack()[p.name];
			while(curPin[d].type != 'D') {
				d++;
			}
			int fx = cur.getX() + curPin[d].x;
			int fy = cur.getY() + curPin[d].y;
			Ddfs(PinName, NS, fx, fy);
			d++;
			NSComp += NS;
		}
		else if(p.type == 'Q') {
			map<string, bool> visited;
			string PinName = prev.getInstName() + "/" + p.name;
			float WL = 0, NS = 0;
			while(curPin[q].type != 'Q') {
				q++;
			}
			int fx = cur.getX() + curPin[q].x;
			int fy = cur.getY() + curPin[q].y;
			Qdfs(PinName, visited, WL, NS, fx, fy);
			q++;
			NSComp += NS;
		}
	}
    sum += Alpha * NSComp + Beta * PowerComp + Gemma * AreaComp;
    return sum;
}
bool Board::Check(int x, int y) {
	//  not on grid point
	for(auto &it : PlacementRows) {
		int sx = it.first.first;
		int sy = it.first.second;
		if(sy == y) {
			if(x < sx || (x - sx) % it.second[0])return false;
		}
	}
	return true;
}
int Board::dist(Point P1, Point P2) {
	return abs(P1.x - P2.x) + abs(P1.y - P2.y);
}
float Board::TNSCost() {
	float sum = 0;
	for(auto &it : InstToFlipFlop) {
		for(auto &p : it.second.getPin()){
			if(p.type != 'D')continue;
			float negslack = it.second.getSlack()[p.name];
			if(negslack < 0)sum -= negslack;
		}
	}
	return sum;
}
float Board::PowerCost() {
	float sum = 0;
	for (auto &it : InstToFlipFlop) {
		sum += it.second.getPower();
	}
	return sum;
}
float Board::AreaCost() {
	float sum = 0;
	for(auto &it : InstToFlipFlop){
		sum += it.second.getArea();
	}
	return sum;
}
float Board::BinCost() {
	float sum = 0;
	for(auto &it : InstToFlipFlop){
		int fx = it.second.getX();
		int fy = it.second.getY();
		int fw = it.second.getWidth();
		int fh = it.second.getHeight();
		for(int i = fx / BinWidth * BinWidth; i < fx + fw; i += BinWidth) {
			int w;
			if(i + BinWidth > fx + fw)w = fx + fw - i;
			else if(i >= fx)w = BinWidth;
			else w = i - fx;
			for(int j = fy / BinHeight * BinHeight; j < fy + fh; j += BinHeight) {
				int h;
				if(j + BinHeight > fy + fh)h = fy + fh - j;
				else if(j >= fy)w = BinHeight;
				else h = j - fy;
				BinDensity[i][j] += (float) w * h;
			}
		}
	}
	for(auto &it : InstToGate) {
		int gx = it.second.getX();
		int gy = it.second.getY();
		int gw = it.second.getWidth();
		int gh = it.second.getHeight();
		for(int i = gx / BinWidth * BinWidth; i < gx + gw; i += BinWidth) {
			int w;
			if(i + BinWidth > gx + gw)w = gx + gw - i;
			else if(i > gx)w = BinWidth;
			else w = i - gx;
			for(int j = gy / BinHeight * BinHeight; j < gy + gh; j += BinHeight) {
				int h;
				if(j + BinHeight > gy + gh)h = gy + gh - j;
				else if(j > gy)w = BinHeight;
				else h = j - gy;
				BinDensity[i][j] += (float) w * h;
			}
		}
	}
	for(auto &it : BinDensity) {
		for(auto &d : it.second) {
			d.second /= (float) (BinWidth * BinHeight / 100);
			if(d.second >= BinMaxUtil) sum += 1;
		}
	}
	return sum;
}
float Board::Cost() {
	int sum = 0;
	sum += Alpha * TNSCost() + Beta * PowerCost() + Gemma * AreaCost() + Delta * BinCost();
    return sum;
}
float Board::getInstsize() {
	return InstToFlipFlop.size();
}
