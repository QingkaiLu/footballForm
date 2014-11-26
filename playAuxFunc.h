#ifndef _PLAY_AUXILIARY_FUNCTIONS
#define _PLAY_AUXILIARY_FUNCTIONS

#include <vector>
#include <string>
#include <cv.h>
#include <highgui.h>

#include "commonStructs.h"
//#include "play.h"

using namespace std;
using namespace cv;

class play;

/*
 * distance from point to line
 */
double distFromPntToLine(Point2d pt0, struct yardLine yLine);

/*
 * the closest yard line distance among all distance of adjacent yard lines
 */
double closestLnDist(vector<struct yardLine> lines);

//void plotRect(Mat& img, struct rect& scrimLnRect, Scalar clr);
void plotRect(Mat& img, struct rect& rct, Scalar clr);

void drawLines(Mat& dst, struct yardLine yLine, CvScalar color);

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

void plotScanLines(Mat& img, vector<rect> &scanLines, const vector<int> &featureVec);
double avgLnDist(vector<struct yardLine> lines);

bool readGradientLos(const string &filePath, Point2d los[2]);

void plotYardLnsAndLos(Mat& img, struct rect& scrimLnRect, vector<yardLine> yardlns, Point2d los[2]);

void plotLos(Mat& img, struct rect& scrimLnRect);
void plotLos(Mat& img, struct rect& scrimLnRect, Point2d los[2]);

void plotYardLnsAndLos(Mat& img, struct rect& scrimLnRect, vector<vector<Point2d> > yardLns);
void plotYardLnsAndLos(Mat& img, struct rect& scrimLnRect, vector<vector<Point2d> > yardLns, Point2d los[2]);

void plotScanLines(Mat& img, vector<rect> &scanLines, const vector<int> &featureVec);

bool compLns(struct yardLine l1, struct yardLine l2);

void plotPlayerPosBox(Mat& img, struct rect& playerBox, string pTypeString);

Mat subtractEdgeImg(const Mat &img, const Mat &bg);

vector<Mat> readHomographs(const string &fileName);

void readFormsFile(const string &formsFile, direction offSide, vector<string> &formations);

void readFormsFile(const string &formsFile, vector<string> &formations);

void readPlayerBndBoxes(const string &playersFilePath, vector<double> &scores,
		vector<struct rect> &players, vector<double> &areas);

void readFormationGt(const string &formFilePath, vector<struct rect> &players,
	rect &losBndBox, Point2d &losCnt);

void readFormationGt(const string &formFilePath, vector<struct rect> &players,
	vector<string> &pTypes, rect &losBndBox, Point2d &losCnt);

void plotRectAvgClr(Mat& img, const struct rect& rct, Scalar clr, Point3d &avgClr);

void getRectAvgClr(Mat& img, const struct rect& rct, Point3d &avgClr);

void getOrgImgRectAvgClr(Mat& img, const struct rect& rct, const Mat &fldToOrgHMat, Point3d &avgClr);

void getOffensePlayers(vector<Point2d> &playersLocSet, vector<Point2d> &pLocSetFld,
		play* p, vector<struct rect> &players, direction offDir);

void getOffsPlayersByLos(vector<Point2d> &playersLocSet, vector<Point2d> &pLocSetFld, const vector<Point2d> &pFeetLocSetFld,
		play* p, vector<struct rect> &players, direction offDir);

void getPlayersInsdFld(vector<Point2d> &playersLocSet, vector<Point2d> &pLocSetFld,
		play* p, vector<struct rect> &players);

void getOffsPlayersByLos(vector<Point2d> &pLocSetFld, play* p, direction offDir);


//void getOffensePlayers(vector<Point2d> &playersLocSet, vector<Point2d> &pLocSetFld,
//		play* p, vector<struct rect> &players, direction offDir, vector<double> &scores);

void getRectLosPnts(const struct rect &rectLosBndBox, std::vector<cv::Point2d> &olLocSet);

int convertPTypeToPId(const string &pType);

string convertPIdToPType(int pId);

void computeLosCntPosCost(play* p, const vector<Point2d> &offsPLocSetFld, const Mat &trainFeaturesMat,
		const Mat &trainLabelsMat, vector<double> &allCosts, vector<string> &playersTypes);

double getUniformClrCost(const vector<struct rect> &offsPlayers, play* p);

double getColorDif(const Rect &r, Mat &img);

double getOrgImgColorDif(const Rect &r, Mat &img, const Mat &fldToOrgHMat);

double getFgAreaRatio(const Rect &r, const Mat &img);

void readKltTracks(string filePath, vector<track> &trks);

void transTracksFromOrgToFld(const vector<track> &trks, const Mat &orgToFldHMat, vector<track> &rectTrks);

void drawKltTracks(play *p, const vector<track> &trks, const Mat &orgToFldHMat);

bool isPlayerByKltTracks(const vector<track> &trks, const Point2d &pos, const Mat &orgToFldHMat);

#endif
