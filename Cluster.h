#ifndef CLUSTER_H
#define CLUSTER_H   
#include <fstream>
#include <iostream>
#include <vector>
#include <set>
#include <cmath>
#include <algorithm>
#include <unordered_map>
#include "Board.h"
using namespace std;
class Cluster {
private:
    int n;
    int k; //分成k個cluster
    int l; //每個cluster再分成l個cluster
    int epochs; //將point分堆的迭代次數
    int maxBits;
    double MinVariance;
    double Max; // max dimension of board
    struct Point {
        double x, y;
        int cluster;
        Point() : x(0), y(0), cluster(-1) {}
        Point(double x, double y, int cluster = -1) : x(x), y(y), cluster(cluster) {}
        double distance(Point p) {
            return abs(x - p.x) + abs(y - p.y);
        }
    };
    vector<Point> DataPoints; //所有flip flop的代表點
    vector<vector<Point>> KClusters; //總共k個cluster，每個cluster裡包含的points
    vector<vector<vector<Point>>> KLClusters; //總共k個cluster，每個cluster再分成l個cluster，每個cluster裡包含的points
    vector<Point> centroids; //cluster質心座標
    vector<vector<Point>> KCentroids; //總共k個cluster，每個cluster再分成l個cluster，每個cluster的質心
    vector<int> counts; //cluster中的points數量
    vector<vector<int>> KCounts; //總共k個cluster，每個cluster再分成l個cluster，每個cluster的points數量
public:
    Cluster();
    Cluster(int, int, int, int, int, double, double);
    void initializeVector(int, int);
    double variance();
    void kMeansClustering(vector<Point>&, vector<Point>&, vector<int>&, int, int);
    void readData(Board&);
    void kmeans(Board&);
};

#endif#pragma once
