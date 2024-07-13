#ifndef FLIPFLOP_H
#define FLIPFLOP_H
#include "Cell.h"
#include <map>
using namespace std;
class FlipFlop : public Cell {
private:
	int N;                     // n bit(s)
	float QpinDelay;           // delay time of Qpin
	float GatePower;           // power of flipflop
	map<string, float> slack;  // slack of flipflop
public:
	FlipFlop();
	FlipFlop(int, int, int, int, vector<Point>);  // N width height P Pin
	int getN(void);                               // get bit number
	void setPower(float);
	float getPower(void);
	void setQpinDelay(float);
	int getQpinDelay(void);
	void display(void);
	void setSlack(string, float);
	float getSlack(string);
};
#endif
