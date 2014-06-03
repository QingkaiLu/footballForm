#ifndef _COMMON_STRUCTS_H_
#define _COMMON_STRUCTS_H_

#include <cv.h>
#include <highgui.h>
//#include <algorithm>

//using namespace cv;
//using namespace std;

#define imgXLen 852
#define imgYLen 480
#define PI 3.14159265
//#define INF std::numeric_limits<double>::max()
#define INF std::numeric_limits<double>/*or float*/::infinity()
//#define NEGINF std::numeric_limits<double>::min() -1.0 * INF
#define NEGINF -1.0 * INF
#define EPS std::numeric_limits<double>::epsilon()
#define fNum 12;
//#define playerNum 4;
#define boxXLenToYardLns 1.95752 * 2.0 / 3.0;
#define boxYLenToYardLns 0.603402 / 2.0;

//number of features for each player
#define featureNumEachPlayer 4
#define featureNumEachPlay 16

//scan step for x and y direction of scanning windows
#define scanXStep 5
#define scanYStep 5

//#define featNumPerPlay 122*2
//#define fNumPerPlayOneSide 122
#define featNumPerPlay 170*2
#define fNumPerPlayOneSide 170

//class playerBndBox;
//#include "playerBndBox.h"

enum direction{
	leftDir,
	rightDir,
	nonDir
};

struct bndBox{
	cv::Point2d leftUpVert;
	double xLength, yLength;
};


struct yardLine
{
	double rho, theta;
	int index;
};

struct track{
	int frameStart, frameEnd;
	double dist, ydist;
	CvPoint start, end;
	//int col;				//Right = 1 , Left = 2;
	bool lowRec, upRec;

};

//struct track{
//	//int trackId;
//	int startFrm, endFrm;
//	CvPoint startPos, endPos;
//	bool lowRec, upRec;
//};
/*
 * 	a ##### d
 * 	#		#
 * 	#		#
 * 	b ##### c
 *
 * 	d ##### c
 * 	#		#
 * 	#		#
 * 	a ##### b
 *
 * 	a,b,c,d in a counter-clock wise direction, but a can not be guaranteed to be the left up vertex
 */
struct rect{
	cv::Point2d a, b, c, d;
	int trksNum;
	int vidId;
};

struct playId{
	int gameId, vidId;
};

//typedef std::vector<vector<pair<int,float> > > dijGraph;

enum playerType{
	nonPType,
	lowWR,
	lowCB,
	upWR,
	upCB
};

//struct player{
//	playerType pType;
//	playerBndBox *pBox;
//	//playerBndBox pBox;
//	double score;
//};

#endif
