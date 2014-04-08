#include "topOrdShortestP.h"

Graph::Graph(int V)
{
    this->V = V;
    adj = new list<AdjListNode>[V];
}

Graph::~Graph()
{
	delete []adj;
}

void Graph::addEdge(int u, int v, double weight)
{
    AdjListNode node(v, weight);
    adj[u].push_back(node); // Add v to u's list
}

// A recursive function used by shortestPath. See below link for details
// http://www.geeksforgeeks.org/topological-sorting/
void Graph::topologicalSortUtil(int v, bool visited[], stack<int> &Stack)
{
    // Mark the current node as visited
    visited[v] = true;

    // Recur for all the vertices adjacent to this vertex
    list<AdjListNode>::iterator i;
    for (i = adj[v].begin(); i != adj[v].end(); ++i)
    {
        AdjListNode node = *i;
        if (!visited[node.getV()])
            topologicalSortUtil(node.getV(), visited, Stack);
    }

    // Push current vertex to stack which stores topological sort
    Stack.push(v);
}

//L ← Empty list that will contain the sorted elements
//S(queue) ← Set of all nodes with no incoming edges
//while S is non-empty do
//    remove a node n from S
//    insert n into L
//    for each node m with an edge e from n to m do
//        remove edge e from the graph
//        if m has no other incoming edges then
//            insert m into S
//if graph has edges then
//    return error (graph has at least one cycle)
//else
//    return L (a topologically sorted order)

//void findNoIncomNode(vector<list<AdjListNode>> &adj, int noInNode)
//{
//}
void Graph::topologicalSortNoRecur(vector<int> &topOrd)
{
	queue<int> Q;
	vector< list<AdjListNode> > adjCopy;
	adjCopy.resize(V);
	vector<bool> nonIn(V, true);
	for(int i = 0; i < V; ++i)
	{
		for(list<AdjListNode>::iterator it = adj[i].begin(); it != adj[i].end(); ++it)
		{
			if(nonIn[(*it).getV()])
				nonIn[(*it).getV()] = false;

		    AdjListNode node((*it).getV(), (*it).getWeight());
		    adjCopy[i].push_back(node); // Add v to u's list
		}
	}

	for(int i = 0; i < V; ++i)
		if(nonIn[i])
			Q.push(i);

	while(!Q.empty())
	{
		int remNd = Q.front();
		topOrd.push_back(remNd);
		Q.pop();
		vector<bool> nextNonIn(V, false);
		for(list<AdjListNode>::iterator it = adjCopy[remNd].begin(); it != adjCopy[remNd].end(); ++it)
		{
			nextNonIn[(*it).getV()] = true;
			//adjCopy[remNd]
		}
		//adjCopy.erase(adjCopy.begin() + remNd);
		adjCopy[remNd].clear();
		for(int i = 0; i < V; ++i)
		{
			for(list<AdjListNode>::iterator it = adjCopy[i].begin(); it != adjCopy[i].end(); ++it)
			{
				if(nextNonIn[(*it).getV()])
					nextNonIn[(*it).getV()] = false;
			}
		}

		for(int i = 0; i < V; ++i)
			if(nextNonIn[i])
				Q.push(i);
	}

	return;
}

// The function to find shortest paths from given vertex. It uses recursive
// topologicalSortUtil() to get topological sorting of given graph.
//void Graph::shortestDist(int s, double dist[])
//{
//    stack<int> Stack;
//    //int dist[V];
//
//    // Mark all the vertices as not visited
//    bool *visited = new bool[V];
//    for (int i = 0; i < V; i++)
//        visited[i] = false;
//
//    // Call the recursive helper function to store Topological Sort
//    // starting from all vertices one by one
//    for (int i = 0; i < V; i++)
//        if (visited[i] == false)
//            topologicalSortUtil(i, visited, Stack);
//
//    // Initialize distances to all vertices as infinite and distance
//    // to source as 0
//    for (int i = 0; i < V; i++)
//        dist[i] = INF;
//    dist[s] = .0;
//
//    // Process vertices in topological order
//    while (Stack.empty() == false)
//    {
//        // Get the next vertex from topological order
//        int u = Stack.top();
//        Stack.pop();
//
//        // Update distances of all adjacent vertices
//        list<AdjListNode>::iterator i;
//        if (dist[u] != INF)
//        {
//          for (i = adj[u].begin(); i != adj[u].end(); ++i)
//             if (dist[i->getV()] > dist[u] + i->getWeight())
//                dist[i->getV()] = dist[u] + i->getWeight();
//        }
//    }
//
//    // Print the calculated shortest distances
////    for (int i = 0; i < V; i++)
////        (dist[i] == INF)? cout << "INF ": cout << dist[i] << " ";
////    cout << endl;
//    return;
//}

void Graph::shortestDist(int s, double dist[])
{
    stack<int> Stack;
    //int dist[V];

    // Mark all the vertices as not visited
//    bool *visited = new bool[V];
//    for (int i = 0; i < V; i++)
//        visited[i] = false;

    vector<int> topOrd;
    topologicalSortNoRecur(topOrd);
    // Initialize distances to all vertices as infinite and distance
    // to source as 0
    for (int i = 0; i < V; i++)
        dist[i] = INF;
    dist[s] = .0;

    for(unsigned int i = 0; i < topOrd.size(); ++i)
    {
    	int u = topOrd[i];

        // Update distances of all adjacent vertices
        list<AdjListNode>::iterator i;
        if (dist[u] != INF)
        {
          for (i = adj[u].begin(); i != adj[u].end(); ++i)
             if (dist[i->getV()] > dist[u] + i->getWeight())
                dist[i->getV()] = dist[u] + i->getWeight();
        }
    }

    // Print the calculated shortest distances
//    for (int i = 0; i < V; i++)
//        (dist[i] == INF)? cout << "INF ": cout << dist[i] << " ";
//    cout << endl;
    return;
}


void Graph::shortestPath(const int &source, const int &destination, vector<int> &path)
{
	double dist[V];
	shortestDist(source, dist);
	if(dist[destination] == INF)
		return;
	int curNode = destination;
	while(curNode != source)
	{
		path.push_back(curNode);
		//for(int i = 0; i < adj[curNode].size(); ++i)
//		for(list<AdjListNode>::iterator it = adj[curNode].begin(); it != adj[curNode].end(); ++it)
//		{
//			int preNode = (*it).getV();
//			double preNdW = (*it).getWeight();
//			if(abs(dist[preNode] + preNdW - dist[curNode]) < EPS)
//			{
//				curNode = preNode;
//				//break;
//			}
//		}

		for(int i = 0; i < V; ++i)
		{
			if(i == curNode)
				continue;
			//cout << i << ":" << endl;
			for(list<AdjListNode>::iterator it = adj[i].begin(); it != adj[i].end(); ++it)
			{
				//cout << " " << (*it).getV();
				if((*it).getV() == curNode)
				{
					//cout << "###" << endl;
					if(abs(dist[i] + (*it).getWeight() - dist[curNode]) < EPS)
					{
						//cout << "###" << endl;
						curNode = i;
						break;
					}
				}
			}
			//cout << endl;
		}
	}

	path.push_back(source);

	return;
}
//construct a graph for tracks according the consistency
void Graph::constructGraphForTrks(const vector<track> &trks, int &source, int &destination)
{
	if(trks.empty())
	{
		cout << "Empty tracks to construct graph!" << endl;
		return;
	}
	direction dir = nonDir;
	if(trks[0].start.x > trks[0].end.x )
		dir = leftDir;
	else
		dir = rightDir;
//	G.resize(trks.size() + 2);
	source = 0;
	destination = trks.size() + 1;
	vector<double> trksLen;
	for(unsigned int i = 0; i < trks.size(); ++i)
	{
		int trkGIdx = i + 1;
		trksLen.push_back(norm(Point2d(trks[i].start.x - trks[i].end.x, trks[i].start.y - trks[i].end.y)));
		addEdge(source, trkGIdx, -1.0 * trksLen[i]);
		addEdge(trkGIdx, destination, 0.0);
		//G[source].push_back(make_pair(trkGIdx, -1.0 * trksLen[i]));
		//G[source].push_back(make_pair(trkGIdx, 0.0));
		//G[trkGIdx].push_back(make_pair(destination, 0.0));
	}
	for(unsigned int i = 0; i < trks.size(); ++i)
	{
		for(unsigned int j = i + 1; j < trks.size(); ++j)
		{
			double tmMidI = double(trks[i].frameEnd + trks[i].frameStart) / 2.0;
			double tmMidJ = double(trks[j].frameEnd + trks[j].frameStart) / 2.0;
//			double tmMidI = double(trks[i].frameStart);
//			double tmMidJ = double(trks[j].frameStart);
			struct track earlyTrk, lateTrk;
			int earlyTrkIdx , lateTrkIdx;
			if(tmMidI > tmMidJ)
			{
				lateTrk = trks[i];
				lateTrkIdx = i;
				earlyTrk = trks[j];
				earlyTrkIdx = j;
			}
			else if(tmMidI < tmMidJ)
			{
				lateTrk = trks[j];
				lateTrkIdx = j;
				earlyTrk = trks[i];
				earlyTrkIdx = i;
			}
			else
				continue;

			double gapLateSToEarlyE = norm(Point2d(lateTrk.start.x - earlyTrk.end.x, lateTrk.start.y - earlyTrk.end.y));
			Point2d earlyTrkVec = Point2d(earlyTrk.end.x - earlyTrk.start.x, earlyTrk.end.y - earlyTrk.start.y);
			earlyTrkVec = earlyTrkVec * (1.0 / norm(earlyTrkVec));
			Point2d lateTrkVec = Point2d(lateTrk.end.x - lateTrk.start.x, lateTrk.end.y - lateTrk.start.y);
			lateTrkVec = lateTrkVec * (1.0 / norm(lateTrkVec));
			double dotPro = earlyTrkVec.dot(lateTrkVec);

			if(dir == leftDir)
			{
				if((lateTrk.end.x < earlyTrk.end.x) && (lateTrk.start.x < earlyTrk.start.x))
				{
//					double gapLateSToEarlyE = norm(Point2d(lateTrk.start.x - earlyTrk.end.x, lateTrk.start.y - earlyTrk.end.y));
//					Point2d earlyTrkVec = Point2d(earlyTrk.end.x - earlyTrk.start.x, earlyTrk.end.y - earlyTrk.start.y);
//					Point2d lateTrkVec = Point2d(lateTrk.end.x - lateTrk.start.x, lateTrk.end.y - lateTrk.start.y);
//					double dotPro = earlyTrkVec.dot(lateTrkVec);
					if( ((trksLen[earlyTrkIdx] + trksLen[lateTrkIdx]) >= gapLateSToEarlyE) && (dotPro >= cos(PI/9.0)) )
					{
						addEdge(earlyTrkIdx + 1, lateTrkIdx + 1, -1.0 * trksLen[lateTrkIdx]);
						//addEdge(earlyTrkIdx + 1, lateTrkIdx + 1, -1.0 * (trksLen[lateTrkIdx] + gapLateSToEarlyE));
						//G[earlyTrkIdx + 1].push_back(make_pair(lateTrkIdx + 1, -1.0 * trksLen[lateTrkIdx]));
						//G[earlyTrkIdx + 1].push_back(make_pair(lateTrkIdx + 1, -1.0));
					}
				}
				//G[lateTrkIdx + 1].push_back(make_pair(earlyTrkIdx + 1, -1.0 * trksLen[lateTrkIdx]));
			}
			if(dir == rightDir)
			{
				if((lateTrk.end.x > earlyTrk.end.x) && (lateTrk.start.x > earlyTrk.start.x))
				{
					//double gapLateSToEarlyE = norm(Point2d(lateTrk.start.x - earlyTrk.end.x, lateTrk.start.y - earlyTrk.end.y));
					if( ((trksLen[earlyTrkIdx] + trksLen[lateTrkIdx]) >= gapLateSToEarlyE) && (dotPro >= cos(PI/9.0)) )
					{
						addEdge(earlyTrkIdx + 1, lateTrkIdx + 1, -1.0 * trksLen[lateTrkIdx]);
						//addEdge(earlyTrkIdx + 1, lateTrkIdx + 1, -1.0 * (trksLen[lateTrkIdx] + gapLateSToEarlyE));
						//G[earlyTrkIdx + 1].push_back(make_pair(lateTrkIdx + 1, -1.0 * trksLen[lateTrkIdx]));
					}
				}
			}

		}
	}

	return;
}

// Driver program to test above functions
//int main()
////int testSp()
//{
//    // Create a graph given in the above diagram.  Here vertex numbers are
//    // 0, 1, 2, 3, 4, 5 with following mappings:
//    // 0=r, 1=s, 2=t, 3=x, 4=y, 5=z
//    Graph g(6);
//    g.addEdge(0, 1, 5.0);
//    g.addEdge(0, 2, 3.0);
//    g.addEdge(1, 3, 6.0);
//    g.addEdge(1, 2, 2.0);
//    g.addEdge(2, 4, 4.0);
//    g.addEdge(2, 5, 2.0);
//    g.addEdge(2, 3, 7.0);
//    g.addEdge(3, 4, -1.0);
//    g.addEdge(4, 5, -2.0);
//
//    //int s = 1;
////    cout << "Following are shortest distances from source " << s <<" \n";
////    double dist[6];
////    g.shortestDist(s, dist);
//    vector<int> path;
//    g.shortestPath(0, 4, path);
//
//    for(unsigned int i = 0; i < path.size(); ++i)
//    	cout << path[i] << "<-";
//    cout << endl;
//
//    cout << abs(-1.2) << endl;
//    return 0;
//}


//int main()
////int testDij()
//{
//	vector<struct track> trks;
//	track t1, t2, t3, t4, t5;
//	t1.start.x = 100;
//	t1.start.y = 200;
//	t1.frameStart = 1;
//	t1.end.x = 150;
//	t1.end.y = 200;
//	t1.frameEnd = 2;
//	trks.push_back(t1);
//
//	t2.start.x = 200;
//	t2.start.y = 200;
//	t2.frameStart = 3;
//	t2.end.x = 250;
//	t2.end.y = 200;
//	t2.frameEnd = 4;
//	trks.push_back(t2);
//
//	t3.start.x = 200;
//	t3.start.y = 210;
//	t3.frameStart = 3;
//	t3.end.x = 260;
//	t3.end.y = 210;
//	t3.frameEnd = 4;
//	trks.push_back(t3);
//
//	t4.start.x = 300;
//	t4.start.y = 200;
//	t4.frameStart = 5;
//	t4.end.x = 350;
//	t4.end.y = 200;
//	t4.frameEnd = 6;
//	trks.push_back(t4);
//
////	t5.start.x = 300;
////	t5.start.y = 200;
////	t5.frameStart = 5;
////	t5.end.x = 360;
////	t5.end.y = 200;
////	t5.frameEnd = 6;
////	trks.push_back(t5);
//
//	Graph G(trks.size() + 2);
//	int source, destination;
//	G.constructGraphForTrks(trks, source, destination);
//	vector<int> path;
//	G.shortestPath(source, destination, path);
//
//	for(unsigned int i = 0; i < path.size(); ++i)
//		cout << path[i] << " ";
//	cout << endl;
//
//	return 1;
//}
