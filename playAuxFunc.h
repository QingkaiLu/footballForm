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

void readPlayerBndBoxes(const string &playersFilePath, 	vector<double> &scores,
		vector<struct rect> &players, vector<double> &areas);

void plotRectAvgClr(Mat& img, const struct rect& rct, Scalar clr, Point3d &avgClr);

void getOffensePlayers(vector<Point2d> &playersLocSet, vector<Point2d> &pLocSetFld,
		play* p, vector<struct rect> &players, direction offDir);

#endif
