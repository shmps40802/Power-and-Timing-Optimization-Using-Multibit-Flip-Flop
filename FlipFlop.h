#ifndef FLIPFLOP_H
#define FLIPFLOP_H
#include "Cell.h"
#include <map>
#include <iostream>
using namespace std;
class FlipFlop : public Cell {
private:
	int N;                       // n bit(s)
	float QpinDelay;             // delay time of Qpin
	float GatePower;             // power of flipflop
	map<string, float> slack;    // name to slack of flipflop
	int cluster;                 // the cluster it belongs to
	map<string, map<string, float>> Connection;  // acceptable WL between Q and IN
public:
	FlipFlop();
	FlipFlop(int, int, int, int, vector<Point>);  // N width height P Pin
	~FlipFlop();
	int getN(void);                               // get bit number
	void setPower(float);
	float getPower(void);
	void setQpinDelay(float);
	float getQpinDelay(void);
	void display(void);
	void setSlack(string, float);
	map<string, float> getSlack(void);
	void setCluster(int);
	int getCluster(void);
	bool operator==(const FlipFlop& other) const;
	void setConnection(string, string, float);
	map<string, map<string, float>> getConnection(void);
};
#endif
