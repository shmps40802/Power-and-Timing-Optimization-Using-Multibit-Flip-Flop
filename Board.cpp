#include "Board.h"
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>

float sum;
int Abs(int n) {
	return n > 0 ? n : -n;
}
bool ffComp(pair<string, FlipFlop> p1, pair<string, FlipFlop> p2) {
	return p1.second.getN() > p2.second.getN();
}
bool rowComp(row r1, row r2) {
	return r1.col < r2.col;
}
void sort(map<string, FlipFlop>& M) { 
    vector<pair<string, FlipFlop>> A; 
    for (auto& it : M) {
        A.push_back(it);
    }
    sort(A.begin(), A.end(), ffComp);
}
Board::Board() {
	Alpha = 0;
	Beta = 0;
	Gemma = 0;
	Delta = 0;
	LowerLeftX = 0;
	LowerLeftY = 0;
	HigherRightX = 0;
	HigherRightY = 0;
	CellNumber = 1;
}
void Board::ReadFile() {
	ifstream fin;
	string Filename;
	cout << "filename :";
	cin >> Filename;
	fin.open(Filename);
	while(!fin.is_open()){
		cout << Filename << " can't be opened\nfilename : ";
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
		if (Str == "FlipFlop"){
			fin >> N >> FlipFlopName >> Width >> Height >> P;
			vector<Point> pin;
			for(int i = 0; i < P; i++) {
				fin >> Str >> name >> x >> y;
				Point tmp(x, y, name);
				if(name.substr(0,1) == "D"){
					tmp.type = 'D';
				}
				else if(name.substr(0,1) == "Q"){
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
			Location[y].push_back(row(x, InstName));
		}
		else {
			Gate G = GateLib[name];
			G.setPos(x, y);
			G.setInstName(InstName);
			InstToGate[InstName] = G;
			Location[y].push_back(row(x, InstName));
		}
	}
	for(auto &l : Location){
		l.second.sort(rowComp);
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
	while(true){
		fin >> Str;
		if(Str != "PlacementRows")break;
		fin >> StartX >> StartY >> SiteWidth >> SiteHeight >> TotalNumOfSites;
		PlacementRows[{StartX,StartY}] = {SiteWidth, SiteHeight, TotalNumOfSites};
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
	int power;
	for (auto &it : FlipFlopLib) {
		fin >> Str >> name >> power;
		FlipFlopLib[name].setPower(power);
	}
	fin.close();
}
void Board::Display() {
	ofstream fout;
	fout.open("output.txt");
	for(auto &it : PointToNet){
		fout << it.first << " " << it.second << "\n";
	}
	fout.close();
}
Point Board::NametoPoint(string PinName) {
	// flipflop gate pin
	size_t pos = PinName.find('/');
	if(pos != string::npos) {
		string cname = PinName.substr(0, pos);
		for(auto &f : InstToFlipFlop) {
			if(f.first != cname)continue;
			string pname = PinName.substr(pos + 1, string::npos);
			return f.second.getPoint(pname);
		}
		for(auto &g : InstToGate) {
			if(g.first != cname)continue;
			string pname = PinName.substr(pos + 1, string::npos);
			return g.second.getPoint(pname);
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
	cout << "Pin " << PinName << " can't be found\n";
	return Point(-1, -1, "");
}
Board::~Board() {
	Input.clear();
	Output.clear();
}
void Board::Banking(vector<vector<FlipFlop>> F, vector<Point> pos, vector<int> n) {
	// slack need to be set
	for(int i = 0; i < n.size(); i++) {
		FlipFlop f = getFlipFlop(pos[i].x, pos[i].y, n[i]);
		string FlipFlopName = "C" + CellNumber;
		CellNumber++;
		NewFlipFlop.insert(FlipFlopName);
		f.setCellName(FlipFlopName);
		vector<Point> CurPin = f.getPin();
		for(int j = 0; j < n[i]; j++) {
			vector<Point> PrevPin = F[i][j].getPin();
			if(NewFlipFlop.find(F[i][j].getInstName()) != NewFlipFlop.end()){
				NewFlipFlop.erase(F[i][j].getInstName());
			}
			int d = 0, q = 0, c = 0;
			for(auto &p : PrevPin) {
				string prev = F[i][j].getInstName() + "/" + p.name;
				string cur;
				// Pin mapping to Pin for 1 bit
				if(p.type == 'D') {
					while(CurPin[d].type != 'D') {
						d++;
					}
					cur = FlipFlopName + "/" + CurPin[d].name;
				}
				else if(p.type == 'Q') {
					while(CurPin[q].type != 'Q') {
						q++;
					}
					cur = FlipFlopName + "/" + CurPin[q].name;
				}
				else if(p.type == 'C') {
					while(CurPin[c].type != 'C') {
						c++;
					}
					cur = FlipFlopName + "/" + CurPin[c].name;
				}
				else {
					cout << "not existing point\n";
					break;
				}
				if(CurToPrev[prev] == "") {
					PrevToCur[prev] = cur;
					CurToPrev[cur] = prev;
				}
				else {
					string tmp = CurToPrev[prev];
					PrevToCur[tmp] = cur;
					CurToPrev[cur] = tmp;
					CurToPrev.erase(prev);
				}
			}
		}
	}
}
vector<FlipFlop> Board::Debanking(FlipFlop F) {
	vector<FlipFlop> result;
	int sum = F.getN();
	vector<Point> PrevPin = F.getPin();
	for(int i = 0; i < sum; i++){
		FlipFlop f = getFlipFlop(0, 0, 1);
		string CellName = "C" + CellNumber;
		vector<Point> CurPin = f.getPin();
		CellNumber++;
		int d = 0, q = 0, c = 0;
		for(auto &p : PrevPin){
			string prev = F.getInstName() + "/" + p.name;
			string cur;
			if(p.type == 'D') {
				while(CurPin[d].type != 'D') {
					d++;
				}
				cur = CellName + "/" + CurPin[d].name;
			}
			else if(p.type == 'Q') {
				while(CurPin[q].type != 'Q') {
					q++;
				}
				cur = CellName + "/" + CurPin[q].name;
			}
			else if(p.type == 'C') {
				while(CurPin[c].type != 'C') {
					c++;
				}
				cur = CellName + "/" + CurPin[c].name;
			}
			else {
				cout << "not existing point\n";
				break;
			}
			if(CurToPrev[prev] == "") {
				PrevToCur[prev] = cur;
				CurToPrev[cur] = prev;
			}
			else {
				string tmp = CurToPrev[prev];
				PrevToCur[tmp] = cur;
				CurToPrev[cur] = tmp;
				CurToPrev.erase(prev);
			}
		}
	}
	return result;
}   //  no location
FlipFlop Board::getFlipFlop(int x, int y, int n) {
	for(auto &it : FlipFlopLib) {
		if(it.second.getN() == n)return it.second;
	}
	return FlipFlop();
}
bool Board::Check(int x, int y) {
	//  not on grid point
	for(auto &it : PlacementRows){
		int sx = it.first.first;
		int sy = it.first.second;
		if(sy == y){
			if(x < sx || (x - sx) % it.second[0])return false;
		}
	}
	return true;
}
int Board::ManhattanDist(Point P1, Point P2) {
	return Abs(P1.x - P2.x) + Abs(P1.y - P2.y);
}
float Board::TNSCost() {
	float sum = 0;
	return sum;
}
float Board::PowerCost() {
	int sum = 0;
	for (auto &it : InstToFlipFlop) {
		sum += it.second.getPower();
	}
	return sum;
}
float Board::AreaCost() {
	int sum = 0;
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
	for(auto &it : InstToGate){
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
