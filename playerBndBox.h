#ifndef _PLAYER_BNDBOX_H_
#define _PLAYER_BNDBOX_H_

#include <vector>

#include "commonStructs.h"

using namespace std;

class playerBndBox{
public:
	vector<int> trksInsideIds;
	double longestTrksPath;
	struct rect bndRect;
	direction dir;
public:
	playerBndBox();
	playerBndBox(const struct rect &r);
	void addTrk(int trkId);
	void setLongestPath(double len);
	//double getLongestPath();
	void setDir(direction d);
	//check whether one bndBox has same trk with another bndBox
	bool trksIntersect(const playerBndBox &bndBox);

	playerBndBox operator = (const playerBndBox &p);

	bool intersectAnotherBndBox(const playerBndBox &pBnd, double ovlpThresh);
};

bool compBndBoxes(const playerBndBox &b1, const playerBndBox &b2);

#endif
