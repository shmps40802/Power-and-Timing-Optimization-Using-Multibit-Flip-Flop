#include "FlipFlop.h"
#include <iostream>
using namespace std;
FlipFlop::FlipFlop() : Cell(0, 0, 0) {
	this->N = 0;
	this->BDB = 0;
}
FlipFlop::FlipFlop(int N, int width, int height, int P, vector<Point> pin) 
: Cell(width, height, P) {
	this->N = N;
	setPin(pin);
	this->BDB = 0;
}
int FlipFlop::getN(void){
	return N;
}
void FlipFlop::setPower(float power) {
	GatePower = power;
}
float FlipFlop::getPower(void) {
	return GatePower;
}
void FlipFlop::setQpinDelay(float delay){
	QpinDelay = delay;
}
int FlipFlop::getQpinDelay(void) {
	return QpinDelay;
}
void FlipFlop::display(void) {
	cout << getCellName() << " " << N << " " << getWidth() << " " << getHeight() << " " << getPinCount() << "\n";
	for (auto &p : getPin()) {
		cout << "Pin " <<p .name << " " << p.x << " " << p.y << "\n";
	}
}
void FlipFlop::setSlack(string PinName, float slack){
    this->slack[PinName] = slack;
}
map<string, float> FlipFlop::getSlack(void) {
    return slack;
}
void FlipFlop::setCluster(int x) {
	cluster = x;
}
int FlipFlop::getCluster(void) {
	return cluster;
}
vector<FlipFlop> FlipFlop::getbank() {
	return this->Bank;
}
vector<FlipFlop> FlipFlop::getdebank() {
	return this->DeBank;
}
void FlipFlop::setbdb(bool i) {
	this->BDB = i;
}
bool FlipFlop::getbdb() {
	return this->BDB;
}
