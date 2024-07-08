#include "Board.h"
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>

int Abs(int n) {
	return n > 0 ? n : -n;
}
bool ffComp(pair<string, FlipFlop> p1, pair<string, FlipFlop> p2) {
	return p1.second.getN() > p2.second.getN();
}
bool rowComp(row r1, row r2){
	return r1.col < r2.col;
}
void sort(map<string, FlipFlop>& M) { 
    vector<pair<string, FlipFlop>> A; 
    for (auto& it : M) { 
        A.push_back(it);
    }
    sort(A.begin(), A.end(), ffComp);
} 
Board::Board(){
	Alpha = 0;
	Beta = 0;
	Gemma = 0;
	Delta = 0;
	LowerLeftX = 0;
	LowerLeftY = 0;
	HigherRightX = 0;
	HigherRightY = 0;
	NumInput = 0;
	NumOutput = 0;
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
	fin >> Str >> LowerLeftX >> LowerLeftY >> HigherRightX >> HigherRightY;
	fin >> Str >> NumInput;
	string name;
	int x, y;
	for (int i = 0; i < NumInput; i++) {
		fin >> Str >> name >> x >> y;
		Point tmp(name, x, y);
		Input.push_back(tmp);
	}
	fin >> Str >> NumOutput;
	for (int i = 0; i < NumOutput; i++) {
		fin >> Str >> name >> x >> y;
		Point tmp(name, x, y);
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
				Point tmp(name, x, y);
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
		}
		else if(Str == "Gate"){
			fin >> GateName >> Width >> Height >> P;
			vector<Point> Pin;
			for(int i = 0; i < P; i++){
				fin >> Str >> name >> x >> y;
				Point tmp(name, x, y);
				Pin.push_back(tmp);
			}
			GateLib[GateName] = Gate(Width, Height, P, Pin);
		}
		else break;
	}
	sort(FlipFlopLib);
	/*
	NumInstances 2
	Inst C1 FF1 15 20
	Inst C2 FF1 15 0
	*/
	string InstName, Name;
	fin >> NumInst;
	for (int i = 0; i < NumInst; i++) {
		fin >> Str >> InstName >> Name >> x >> y;
		if(FlipFlopLib.find(Name) != FlipFlopLib.end()) {
			FlipFlop* F = new FlipFlop(FlipFlopLib[Name]);
			F->setPos(x, y);
			F->setName(InstName);
			InstFlipFlop[InstName] = F;
			Location[y].push_back(row(x, F));
			for(auto &it : F->getPin()){
				it.prev = InstName + "/" + it.name;
			}
		}
		else {
			Gate* G = new Gate(GateLib[Name]);
			G->setPos(x, y);
			G->setName(InstName);
			Location[y].push_back(row(x, G));
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
		}
		Net.push_back(net(name, tmp));
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
	for (auto &it : InstFlipFlop) {
		sum += it.second->getN();
	}
	for(int i = 0; i < sum; i++) {
		fin >> Str >> InstName >> name >> slack;
		InstFlipFlop[InstName]->setSlack(name, slack);
	}
	int power;
	for (auto &it : FlipFlopLib) {
		fin >> Str >> name >> power;
		FlipFlopLib[name].setPower(power);
	}
	fin.close();
}
void Board::Display() {
	/*cout << "Alpha " << Alpha << "\nBeta " << Beta << " \nGemma " << Gemma << "\nDelta " << Delta << "\n";
	cout << "Diesize " << LowerLeftX << " " << LowerLeftY << " " << HigherRightX << " " << HigherRightY << "\n";
	cout << "NumInput " << NumInput << "\n";
	for (int i = 0; i < NumInput; i++) {
		cout << "Input " << Input[i]->name << " " << Input[i]->x << " " << Input[i]->y << "\n";
	}
	cout << "NumOutput" << NumOutput << "\n";
	for (int i = 0; i < NumOutput; i++) {
		cout << "Output " << Output[i]->name << " " << Output[i]->x << " " << Output[i]->y << "\n";
	}
	for (auto &it : FlipFlopLib) {
		cout << it.first << "\n";
		it.second->display();
	}*/
	ofstream fout;
	fout.open("output.txt");
	fout << "X = [ ";
	for(int i = 0; i < HigherRightY; i += SiteHeight){
		if(Location[i].empty())continue;
		for(auto &it : Location[i]) {
			fout << it.col << ", ";
		}
		fout << "];\n";
	}
	fout << "Y = [ ";
	for(int i = 0; i < HigherRightY; i += SiteHeight){
		if(Location[i].empty())continue;
		fout << i << ", ";
	}
	fout << "];\n";
	fout.close();
	/*cout << "NumNets " << Net.size() << "\n";
	for (auto &it : Net) {
		cout << it.name << " " << it.points.size() << "\n";
		for (auto &net : it.points) {
			cout << "Pin " << net << "\n";
		}
	}
	cout << BinWidth << "\n";
	cout << BinHeight << "\n";
	cout << BinMaxUtil << "\n";
	cout << "PlacementRow " << StartX << " " << StartY << " " << RowWidth << " " << RowHeight << "\n";
	cout << "Displacement delay " << DisplacementDelay << "\n";
	for (auto &it : FlipFlopLib) {
		cout << "Qpindelay " << it.second->getQpinDelay() << "\n";
	}
	for (auto &it : InstFlipFlop) {
		for (auto &d : it.second->getD()) {
			cout << "TimingSlack " << it.first << " " << d->name << " " << it.second->getSlack(d->name) << "\n";
		}
		for (auto &q : it.second->getQ()) {
			cout << "TimingSlack " << it.first << " " << q->name << " " << it.second->getSlack(q->name) << "\n";
		}
	}
	for (auto &it : FlipFlopLib) {
		cout << "GatePower " << it.first << " " << it.second->getPower() << "\n";
	}*/
}
Point Board::NametoPoint(string PinName){
	// flipflop pin
	size_t pos = PinName.find('/');
	if(pos != string::npos){
		string cname = PinName.substr(0, pos);
		for(auto &f : InstFlipFlop){
			if(f.first != cname)continue;
			string pname = PinName.substr(pos + 1, string::npos);
			return f.second->getPoint(pname);
		}
		for(auto &g : InstGate){
			if(g.first != cname)continue;
			string pname = PinName.substr(pos + 1, string::npos);
			return g.second->getPoint(pname);
		}
	}
	// input output pin
	else {
		for(auto &it : Input){
			if(it.name == PinName){
				return it;
			}
		}
		for(auto &it : Output){
			if(it.name == PinName){
				return it;
			}
		}
	}
	return Point("",-1,-1);
}
Board::~Board() {
	Input.clear();
	Output.clear();
}
vector<FlipFlop> Board::Banking(vector<FlipFlop> F){
	vector<FlipFlop> result;
	int n = 1; // total flipflop
	int d = 0; // total pin
	for(auto &it : F){
		d += it.getN();
	}
	return result;
}  // no location
vector<FlipFlop> Board::Debanking(FlipFlop F){
	vector<FlipFlop> result;
	return result;
}  //  no location
bool Board::Check(int x, int y){
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
int Board::TNSCost() {
	int sum = 0;
	return sum;
}
int Board::PowerCost() {
	int sum = 0;
	for (auto &it : InstFlipFlop) {
		sum += it.second->getPower();
	}
	return sum;
}
int Board::AreaCost() {
	int sum = 0;
	return sum;
}
int Board::BinCost() {
	int sum = 0;
	for(int i = LowerLeftY; i < HigherRightY; i += SiteHeight){
		for(int j = LowerLeftX; j < HigherRightX; j += SiteWidth){
			
		}
	}
	return sum;
}
int Board::Cost() {
	int sum = 0;
	sum += Alpha * TNSCost() + Beta * PowerCost() + Gemma * AreaCost() + Delta * BinCost();
    return sum;
}