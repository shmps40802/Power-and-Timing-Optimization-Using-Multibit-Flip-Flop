#ifndef CELL_H
#define CELL_H
#include <vector>
#include <string>
#include <set>
#include <map>
using namespace std;
class Cell;
struct Point {
	string name;     // name of point
	char type;       // D, Q, Clk, I, O
	int x;           // x coordinate of point
	int y;           // y coordinate of point
	vector<string> sourcename;
	vector<string> neighbor;
	Point(int x, int y, string name) {
		this->name = name;
		this->x = x;
		this->y = y;
	}
	Point(int x, int y) {
		this->x = x;
		this->y = y;
	}
	Point() {
		this->name = "";
		this->x = -1;
		this->y = -1;
	}
	bool operator<(Point& p) {
		if (x < p.x)return true;
		else if (x == p.x) {
			return y < p.y;
		}
		else return false;
	}
};
class Cell {
private:
	int InstNum;        // inst number of cell
	string CellName;        // flipflop number of cell
	int Width;
	int Height;
	int PinCounts;
	vector<Point> Pin;  // pins in cell
	int x;              // x coordinate of cell
	int y;              // y coordinate of cell
public:
	Cell();
	Cell(int, int, int);
	void setPos(int, int);
	int getX(void);
	int getRight(void);
	int getY(void);
	int getTop(void);
	void setPin(vector<Point>);
	vector<Point> getPin(void);
	int getWidth(void);
	int getHeight(void);
	int getArea(void);
	void setInstNum(int);
	int getInstNum(void) const;
	string getInstName(void) const;
	void setCellName(string);
	string getCellName(void) const;
	int getPinCount(void);
	Point getPoint(string);
	void setsource(string, string);
	vector<string> getsource(string);
};
#endif