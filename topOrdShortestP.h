#ifndef TOP_ORDER_SHORTEST_PATH_H_
#define TOP_ORDER_SHORTEST_PATH_H_

// A C++ program to find single source shortest paths for Directed Acyclic Graphs
#include <iostream>
#include <list>
#include <stack>
#include <vector>
#include <queue>
#include <limits.h>

#include "commonStructs.h"

//#define INF double(INT_MAX)
//#define INF numeric_limits<double>::max()
//#define EPS numeric_limits<double>::epsilon()

using namespace std;
using namespace cv;

// Graph is represented using adjacency list. Every node of adjacency list
// contains vertex number of the vertex to which edge connects. It also
// contains weight of the edge
class AdjListNode
{
    int v;
    double weight;
public:
    AdjListNode(int _v, double _w)  { v = _v;  weight = _w;}
    int getV()       {  return v;  }
    double getWeight()  {  return weight; }
};

// Class to represent a graph using adjacency list representation
class Graph
{
    int V;    // No. of vertices'

    // Pointer to an array containing adjacency lists
    list<AdjListNode> *adj;

    // A function used by shortestPath
    void topologicalSortUtil(int v, bool visited[], stack<int> &Stack);

    // topological sort without recursion
    void topologicalSortNoRecur(vector<int> &topOrd);

public:
    Graph(int V);   // Constructor

    ~Graph();

    // function to add an edge to graph
    void addEdge(int u, int v, double weight);

    // Finds shortest paths from given source vertex
    void shortestDist(int s, double dist[]);

    void shortestPath(const int &source, const int &destination, vector<int> &path);

    void constructGraphForTrks(const vector<track> &trks, int &source, int &destination);
};

#endif
