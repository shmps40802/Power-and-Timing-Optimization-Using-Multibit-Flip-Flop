#include "FlipFlop.h"
#include <iostream>
using namespace std;
FlipFlop::FlipFlop() : Cell(0, 0, 0) {
	this->N = 0;
}
FlipFlop::FlipFlop(int N, int width, int height, int P, vector<Point> pin)
	: Cell(width, height, P) {
	this->N = N;
	setPin(pin);
}
FlipFlop::~FlipFlop() {

}
int FlipFlop::getN(void) const {
	return N;
}
void FlipFlop::setPower(float power) {
	GatePower = power;
}
float FlipFlop::getPower(void) {
	return GatePower;
}
void FlipFlop::setQpinDelay(float delay) {
	QpinDelay = delay;
}
float FlipFlop::getQpinDelay(void) {
	return QpinDelay;
}
void FlipFlop::display(void) {
	cout << getInstNum() << " " << getX() << " " << getY() << "\n";
	cout << getCellName() << " " << N << " " << getWidth() << " " << getHeight() << " " << getPinCount() << "\n";
	for (auto& p : getPin()) {
		cout << "Pin " << p.name << " " << p.x << " " << p.y << "\n";
	}
	for (auto& s : slack) {
		cout << s.first << " " << s.second << "\n";
	}
}
void FlipFlop::setSlack(string PinName, float slack) {
	this->slack[PinName] = slack;
}
unordered_map<string, float> FlipFlop::getSlack(void) {
	return slack;
}
void FlipFlop::setCluster(int x) {
	cluster = x;
}
int FlipFlop::getCluster(void) {
	return cluster;
}
bool FlipFlop::operator==(const FlipFlop& other) const {
	if (this->getInstNum() == other.getInstNum() && this->getCellName() == other.getCellName()) {
		return true;
	}
	return false;
}
void FlipFlop::setDWL(string dname, string qname, float arrival, string center) {
	DWL[dname][qname] = make_pair(arrival, center);
}
unordered_map<string, unordered_map<string, pair<float, string>>> FlipFlop::getDWL(void) {
	return DWL;
}
void FlipFlop::setQconnect(string qname, string dname) {
	Qconnect[qname].insert(dname);
}
unordered_map<string, unordered_set<string>> FlipFlop::getQconnect(void) {
	return Qconnect;
}
