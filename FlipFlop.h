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
	int cluster;               // the cluster it belongs to
	vector<FlipFlop> Bank;     //banking from
	vector<FlipFlop> DeBank;   //debanking from
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
    	map<string, float> getSlack(void);
	void setCluster(int);
	int getCluster(void);
	vector<FlipFlop> getbank();
	vector<FlipFlop> getdebank();
};
#endif
