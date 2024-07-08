#include "Cell.h"
Cell::Cell(){
    Width = 0;
    Height = 0;
    PinCount = 0;
    this->x = 0;
    this->y = 0;
}
Cell::Cell(int w, int h, int p){
    Width = w;
    Height = h;
    PinCount = p;
    this->x = x;
    this->y = y;
}
void Cell::setPos(int x, int y){
    this->x = x;
    this->y = y;
}
void Cell::setPin(vector<Point> pin){
    Pin = pin;
}
vector<Point> Cell::getPin(void){
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
void Cell::setName(string name){
    Name = name;
}
string Cell::getName(void){
    return Name;
}
int Cell::getPinCount(void){
    return PinCount;
}
Point Cell::getPoint(string name){
    for(auto &it : Pin){
        if(it.name == name)return it;
    }
    return Point("",-1,-1);
}