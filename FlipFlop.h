#ifndef FLIPFLOP_H
#define FLIPFLOP_H
#include "Cell.h"
#include <unordered_map>
#include <unordered_set>
#include <iostream>
using namespace std;
class FlipFlop : public Cell {
private:
	int N;                                 // n bit(s)
	float QpinDelay;                       // delay time of Qpin
	float GatePower;                       // power of flipflop
	unordered_map<string, float> slack;    // name to slack of flipflop
	int cluster;                           // the cluster it belongs to
	unordered_map<string, unordered_map<string, pair<float, string>>> DWL;  // arrival time between D and Q
	unordered_map<string, unordered_set<string>> Qconnect;                  // arrival time between Q and D
public:
	FlipFlop();
	FlipFlop(int, int, int, int, vector<Point>);  // N width height P Pin
	~FlipFlop();
	int getN(void) const;                               // get bit number
	void setPower(float);
	float getPower(void);
	void setQpinDelay(float);
	float getQpinDelay(void);
	void display(void);
	void setSlack(string, float);
	unordered_map<string, float> getSlack(void);
	void setCluster(int);
	int getCluster(void);
	bool operator==(const FlipFlop& other) const;
	void setDWL(string, string, float, string);
	unordered_map<string, unordered_map<string, pair<float, string>>> getDWL();
	void setQconnect(string, string);
	unordered_map<string, unordered_set<string>> getQconnect(void);
};
#endif
