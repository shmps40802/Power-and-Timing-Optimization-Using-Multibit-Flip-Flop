#include "Cell.h"
#include <iostream>
Cell::Cell(){
    Width = 0;
    Height = 0;
    PinCounts = 0;
    this->x = 0;
    this->y = 0;
}
Cell::Cell(int w, int h, int p) {
    Width = w;
    Height = h;
    PinCounts = p;
    this->x = x;
    this->y = y;
}
void Cell::setPos(int x, int y) {
    this->x = x;
    this->y = y;
}
int Cell::getX(void){
    return x;
}
int Cell::getY(void){
    return y;
}
void Cell::setPin(vector<Point> pin) {
    Pin = pin;
}
vector<Point> Cell::getPin(void) {
    return Pin;
}
int Cell::getWidth(void){
    return Width;
}
int Cell::getHeight(void){
    return Height;
}
int Cell::getArea(void){
    return Width * Height;
}
void Cell::setInstName(string name){
    InstName = name;
}
string Cell::getInstName(void){
    return InstName;
}
void Cell::setCellName(string name){
    CellName = name;
}
string Cell::getCellName(void){
    return CellName;
}
int Cell::getPinCount(void){
    return PinCounts;
}
Point Cell::getPoint(string name){
    for(auto &it : Pin){
        if(it.name == name)return it;
    }
    std::cout << "Point name " << name << " not be found\n";
    return Point(-1, -1, "");
}
void Cell::setNetName(string PinName, string NetName){
	for(auto &it : Pin) {
		if(it.name == PinName){
			it.netname = NetName;
		}
	}
}
