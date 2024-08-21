#ifndef CLUSTER_H
#define CLUSTER_H   
#include <fstream>
#include <iostream>
#include <vector>
#include <set>
#include <cmath>
#include <algorithm>
#include <map>
#include "Board.h"
using namespace std;
class Cluster {
private:
    int n;
    int k;      // divide into k clusters
    int l;      // every cluster divide into l clusters
    int r;      // (points / centroid)
    int epochs; // number of iterations to choose points' cluster
    int maxBits;
    double MinVariance;
    double Max; // max dimension of board
    int p;      // from p banking solutions, choose the best one
    int q;      // tries of banking in cluster
    /*struct Point {
        int x, y; //flip flop coordinate
        int N; // n bit(s)
        int width, height; //flip flop weight, height
        int cluster; // the cluster it belongs to
        string name; //Flip Flop name
        Point(int x = 0, int y = 0, int N = 0, int width = 0, int height = 0, string name = "", int cluster = -1) : x(x), y(y), N(N), width(width), height(height), cluster(cluster), name(name) {};
        double distance(Point p) {
            return abs(x - p.x) + abs(y - p.y);
        }
    };*/
    vector<vector<FlipFlop>> DataPoints; //represent point of all flip flops
    vector<vector<vector<FlipFlop>>> KLClusters; //k clusters in total, each cluster is divided into l clusters, and the points contained in each cluster
    vector<FlipFlop> FlipFlopLib; //create FlipFlop library
    vector<int> availableBits; //available bits in FlipFlopLib
public:
    Cluster();
    Cluster(int, int, int, int, int, int, double, double, int, int);
    void initializeVector(int, int);
    void kMeansClustering(vector<FlipFlop>&, int, int);
    double distance(FlipFlop, FlipFlop);
    void readData(Board&);
    float Silhouette(vector<FlipFlop>, int);
    void kmeans(Board&);
    static bool compareFlipFlop(FlipFlop&, FlipFlop&);
    void findOptimalGrouping(vector<FlipFlop>&, Board&);
    FlipFlop updateFlipFlop(FlipFlop, FlipFlop, Board&);
    void updateBankedFlipFlop(vector<FlipFlop>, FlipFlop, Board&);
};
#endif
