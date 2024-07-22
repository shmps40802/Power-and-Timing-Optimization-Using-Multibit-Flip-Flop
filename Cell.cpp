#include "Cell.h"
#include <iostream>
Cell::Cell() {
    Width = 0;
    Height = 0;
    PinCounts = 0;
}
Cell::Cell(int w, int h, int p) {
    Width = w;
    Height = h;
    PinCounts = p;
}
void Cell::setPos(int x, int y) {
    this->x = x;
    this->y = y;
}
int Cell::getX(void) {
    return x;
}
int Cell::getRight(void) {
    return x + Width;
}
int Cell::getY(void) {
    return y;
}
int Cell::getTop(void) {
    return y + Height;
}
void Cell::setPin(vector<Point> pin) {
    Pin = pin;
}
vector<Point> Cell::getPin(void) {
    if (Pin.empty())cout << "Pin in Cell " << InstName << " is empty\n";
    return Pin;
}
int Cell::getWidth(void) {
    return Width;
}
int Cell::getHeight(void) {
    return Height;
}
int Cell::getArea(void) {
    return Width * Height;
}
void Cell::setInstName(string name) {
    InstName = name;
}
string Cell::getInstName(void) const {
    return InstName;
}
void Cell::setCellName(string name) {
    CellName = name;
}
string Cell::getCellName(void) const {
    return CellName;
}
int Cell::getPinCount(void) {
    return PinCounts;
}
Point Cell::getPoint(string name) {
    for (auto& it : Pin) {
        if (it.name == name)return it;
    }
    std::cout << "Point name " << name << " in " << CellName << " not be found\n";
    return Point(-1, -1, "");
}
void Cell::setsource(string pinname, string source) {
    for (auto& it : Pin) {
        if (it.name == pinname) {
            it.sourcename.push_back(source);
        }
    }
}
vector<string> Cell::getsource(string pinname) {
    vector<string> result;
    for (auto& it : Pin) {
        if (it.name == pinname) {
            result = it.sourcename;
        }
    }
    if (result.empty())cout << "Pin " << pinname << " not found\n";
    return result;
}
