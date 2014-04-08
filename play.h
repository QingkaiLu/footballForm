#ifndef _PLAY_H_
#define _PLAY_H_

#include <vector>
#include <string>
#include <cv.h>
#include <highgui.h>

#include "commonStructs.h"
//#include "wrPicStrModel.h"
//#include "playerBndBox.h"

using namespace std;
using namespace cv;

// recMethod == 1 => compute the score for receiver by tracks' occupied time and length in X direction
// recMethod == 2 => compute the score for receiver by consistency of tracks as a shortest path problem
#define recMethod 2

// predMos: 0 -> true MOS; 1 -> predicated MOS
#define predMos 0

// playersRange == 1 => ranges for low and up players
// playersRange == 2 => ranges for low, up, left and right players
#define playersRange 1

//losMethod == 2 => new los based on sheng's gradient los; losMethod == 1 => old los based on klt
#define losMethod 1

//yardLinesMethod == 1 => old detected yard lines and vanishing points;
//yardLinesMethod == 2 => new detected yard lines and vanishing points
#define yardLinesMethod 1

//yardLnsDistMethod == 1 => closest distance between yard lines;
// yardLnsDistMethod == 2 => average distance between yard lines
#define yardLnsDistMethod 2


class wrPicStrModel;
class playerBndBox;
class formation;
class player;

class play{
public:
	play(struct playId p);
	~play();
	void getMos();
	bool getYardLines();
	void computeYardLnsDist();
	void getScrimLn();
	void getGradientScrimLn();
	//find the los bounding box where the players lined up
	void findLosBndBox();

	//compute n top bounding boxes which have the longest path
	void computeTopBoxes(unsigned int n, double boxXLen, double boxYLen, vector<playerBndBox> &topBoxes);
	void computeTopBoxesDir(const wrPicStrModel& leftModel, const wrPicStrModel& rightModel);
	void computeLosSideDir();
	void computeAllTracksDir();
	void getTracks();
	//void plot();
	void getTrueDir();
	void getVp();

	//bool insideRecRange(struct track &t);
	void computeRecSearchRng();

	void computeFormation(const vector<playerType> &pTypes);
	void computeFormation(const vector<playerType> &pTypes, vector<double> &weight, vector<double> &feature);
	void computeTopBoxesForm(const vector<playerType> &pTypes);
	//comupte offense direction based on the position of corner backs
	void computeCBDir(const vector<playerType> &pTypes);
	void computeFormDir(const vector<playerType> &pTypes);
	void computeFormDir(const vector<playerType> &pTypes, int iterateIdx, vector<double> &weight, vector<double> &feature, const vector<player> &gtPs);
	void computeFormDir(const vector<playerType> &pTypes, vector<double> &weight, int featureNum);
	//compute formation for different models
	void computeFormDir(const vector<playerType> &pTypes, vector<double> &weight, int featureNum, int model);

	void computeDirLosArea();

	void setUp();

	void saveMosFrm();

	void cvtFgImgFrmBmpToJpg();

	void extractOdGridsFeature(direction dir, vector<int> &featureVec);
	void extractOdStripsFeature(direction dir, vector<int> &featureVec);

	void detectEllipsesFromFg(vector<RotatedRect> &ellipses);
	void ellipseWrClassifier();

	void rectification();
	void extractOdStripsFeatRect(direction dir, vector<int> &featureVec);

public:
	struct playId pId;

	int mos;
	vector<struct yardLine> yardLines;
	double yardLnsDist;
	Point2d vp;
	bool vpExist;

	//line of scrimmage from Sheng's gradient results
	Point2d los[2];
	//los center bounding box
	struct rect scrimLn;
	Point2d scrimCnt;
	//los center after rectification
	Point2d rectScrimCnt;
	//los after rectification
	struct rect rectScrimLn;
	Point2d rectLos[2];


	string scrimLnsKltFilePath, scrimLnsGradFilePath;
	string mosFilePath, yardLnsFilePath, trksFilePath;
	string plotPath, trueDirFilePath, vpFilePath;
	string vidIdxStr, gameIdStr;
	string fgImgPath, videoPath;
	string mosImgPath;

	//predicated and true offense direction
	direction preDir, trueDir;

	vector<track> tracks;

	Mat image, mosFrame;
	Mat rectImage;

	struct rect recSearchRng;

	formation *form;
};

direction trackDir(struct track trk);
/*
 * distance from point to line
 */
double distFromPntToLine(Point2d pt0, struct yardLine yLine);

/*
 * the closest yard line distance among all distance of adjacent yard lines
 */
double closestLnDist(vector<struct yardLine> lines);

bool readScrimLines(string filePath, vector<rect>& scrimLns);

int findScrimLine(vector<rect> srimLns, int vidIndex);

bool readTrks(string filePath, vector<track>& trks);

bool readVanishPnts(string filePath, int mos, Point2d& vp);

void plotTracks(Mat& img, vector<track> trks, struct rect& scrimLnRect, vector<yardLine> yardlns, struct rect& lowRecRect, struct rect& upRecRect);

void plotScrimToRecVecs(Mat& img, Point2d scrimCnt, Point2d vecScrimToUpRec, Point2d vecScrimToLowRec);

void plotTopBoxes(Mat& img, const vector<playerBndBox> &topBoxes, const vector<struct track> tracks);

void plotFormation(Mat& img, const formation *f, const vector<struct track> tracks, const struct rect& scrimLnRect);

void plotGtForm(Mat& img, const vector<player> &gtPlayers);

void plotRect(Mat& img, struct rect& scrimLnRect);

void drawLines(Mat& dst, struct yardLine yLine, CvScalar color);

void HSVtoRGB(float hsv[3], float rgb[3]);

//plot tracks with time visualization
void plotTmVisTracks(Mat& img, vector<track> trks, struct rect& scrimLnRect, vector<yardLine> yardlns, struct rect& lowRecRect, struct rect& upRecRect);

void plotTmVisTracks(Mat& img, vector<track> trks, struct rect& scrimLnRect, vector<yardLine> yardlns);

double computeTrksConsistScr(const vector<track> &trks);

bool computeTrksConsistPath(const vector<track> &trks, vector<int> &path);

bool isTrkInsideRect(const struct track &trk, const struct rect &scanRect);

bool isTrkInsideBndBox(const struct track &trk, const struct bndBox &bBox, double yardLnsDist);

bool isScanRectInsdRngRect(const struct rect &scanRect, const struct rect &rngRect);

// if the line of scrimmage is closer to the low end, return true
bool isLosCloserToLowEnd(const play *p);

//count the number of foreground pixels inside a rectangle box in X, Y direction
int fgPixelsInsideBoxXY(const play *pl, const struct rect &box);

//check whether a point is insider two lines or not.
//two lines are represented by a rect ABCD. AB is one line and CD is another line.
//A,B,C,D must be in the counter-clock wise direction.
bool isPntInsideTwoLines(Point2d pnt, const struct rect &rectLines);

//check if the point is inside the rectangle or not
bool isPntInsideRect(Point2d pnt, const struct rect &rect);

//count the number of foreground pixels inside a box
int fgPixelsInsideBox(const Mat &fgImg, const struct rect &box);

void plotYardLnsAndLos(Mat& img, struct rect& scrimLnRect, vector<yardLine> yardlns);

void plotScanLines(Mat& img, const vector<rect> &scanLines, const vector<int> &featureVec);
double avgLnDist(vector<struct yardLine> lines);

bool readGradientLos(const string &filePath, Point2d los[2]);

void plotYardLnsAndLos(Mat& img, struct rect& scrimLnRect, vector<yardLine> yardlns, Point2d los[2]);

void plotLos(Mat& img, struct rect& scrimLnRect);
void plotLos(Mat& img, struct rect& scrimLnRect, Point2d los[2]);

#endif
