#ifndef CELL_H
#define CELL_H
#include <vector>
#include <string>
using namespace std;
struct Point {
	string name;  // name of point
    char type;    // D,Q,Clk,I,O
    string prev;  // before map
    string cur;   // after map
	int x;        // x coordinate of point
	int y;        // y coordinate of point
	float slack;  // slack of input/output point
	Point(string name, int x, int y){
		this->name = name;
		this->x = x;
		this->y = y;
	}
	Point(){
		this->name = "";
		this->x = -1;
		this->y = -1;
	}
    bool operator<(Point &p) {
        if(x < p.x)return true;
        else if(x == p.x){
            return y < p.y;
        }
        else return false;
    }
};
class Cell{
private:
    string Name;
    int Width;
    int Height;
    int PinCount;
    vector<Point> Pin;
    int x;
    int y;
public:
    Cell();
    Cell(int, int, int);
    void setPos(int, int);
    void setPin(vector<Point>);
    vector<Point> getPin(void);
    int getWidth(void);
    int getHeight(void);
    int getArea(void);
    void setName(string);
    string getName(void);
    int getPinCount(void);
    Point getPoint(string);
};
#endif