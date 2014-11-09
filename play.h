#ifndef _PLAY_H_
#define _PLAY_H_

#include <vector>
#include <string>
#include <cv.h>
#include <highgui.h>

#include "commonStructs.h"

using namespace std;
using namespace cv;


// predMos: 0 -> true MOS; 1 -> predicated MOS
#define predMos 0

//losMethod == 2 => new los based on sheng's gradient los;
//losMethod == 1 => old los based on klt or new los based on the
//rectfied foreground
#define losMethod 1

//rectLosMethod == 1 => rectified los box and los center are transformed the unrectified ones;
//rectLosMethod == 2 => rectified los box and los center are found using the rectified foreground;
#define rectLosMethod 2

//yardLinesMethod == 1 => old detected yard lines and vanishing points;
//yardLinesMethod == 2 => new detected yard lines and vanishing points
#define yardLinesMethod 2

//yardLnsDistMethod == 1 => closest distance between yard lines;
// yardLnsDistMethod == 2 => average distance between yard lines
#define yardLnsDistMethod 2

//number of bins of the los cnt between two hash lines
#define losCntBins 3

class fieldModel;

class play{
public:
	play(struct playId p);
	play(struct playId p, int fldModel);
	~play();
	void getMos();
	bool getYardLines();
	void computeYardLnsDist();
//	void getScrimLn();
	//Sheng's line of scrimmage based on gradient
	void getGradientScrimLn();
	//find the los bounding box where the players lined up
	void findLosBndBox();
	//find the los on the rectified foreground
	void findLosOnRectFg(const Mat &homoMat);
	int getLosCntIdx();

	void getTrueDir();

	void setUp();

	void saveMosFrm();
	void saveMosFrmToClst(int clst);

	void cvtFgImgFrmBmpToJpg();

	void detectEllipsesFromFg(vector<RotatedRect> &ellipses);

	void rectification();
	void rectification(Mat& orgToFldHMat);
	void rctfWithoutDetectLos(Mat& orgToFldHMat);
	void writeMatchPnts();
	void extractOdStripsFeatRect(direction dir, vector<int> &featureVec);
	void extractOdGridsFeatRect(direction dir, vector<int> &featureVec);
	//expMode == 0: no expectation
	//expMode == 1: with expectation
	void extractOdGridsFeatRect(direction dir, vector<int> &featureVec,
			const vector<CvSize> &gridSizes, const vector<Point2i> &gridsNum, int expMode);
	//extract features with indicator response for missing features.
	void extractOdGridsFeatRectIndRsp(direction dir, vector<int> &featureVec,
			const vector<CvSize> &gridSizes, const vector<Point2i> &gridsNum);

	//get the coordinate from the overhead field model
	void getOverheadFieldHomo(Mat &homoMat);
	//
	void extOdStripsFeatFldCrdOrigImg(direction dir, vector<int> &featureVec);
	void extOdGridsFeatFldCrdOrigImg(direction dir, vector<int> &featureVec);
	void extOdGridsFeatFldCrdOrigImg(direction dir, vector<int> &featureVec,
			const vector<CvSize> &gridSizes, const vector<Point2i> &gridsNum, int expMode);

	void detectOnePlayerTypePosRect(playerTypeId pTypeId, direction offSide);
	void detectPlayerTypesPosRect(const vector<playerTypeId> &pTypeIds, direction offSide);

	void detectOnePlayerTypePosOrig(playerTypeId pTypeId, direction offSide, const Mat &fldToOrgHMat);
	void detectPlayerTypesPosOrig(const vector<playerTypeId> &pTypeIds, direction offSide);

	//generate the foreground of the rectfied Mos frame by background subtraction with panorama.
	void genRectMosFrmFgBgSub();
	//generate the foreground of the original Mos frame by background subtraction with panorama.
	void genOrigMosFrmFgBgSub();

	void getBgImg();
	void cutAreaOutsideFld();

	void drawPlayerBndBoxes();
	void drawPlayerBndBoxesRectFld();

	void detectForms(direction offSide);
	//detect formation with ground truth of player positions.
	void detectFormsGt(direction offSide);

	//label player types based on angel to LOS center with formation annotations
	void labelPlayersAngleGt();
	void labelPlayersAngle(direction offSide);

	void getPlayersToLosVecs(vector<Point2d> &pToLosVec, vector<int> &pTypesId);
	void lablePTypesKnnFixedLosCnt(direction offSide, const Mat &trainFeaturesMat, const Mat &trainLabelsMat);
	void lablePTypesKnnVarLosCnts(direction offSide, const Mat &trainFeaturesMat, const Mat &trainLabelsMat);
//	void lablePTypesKnnVarLosCnts2(direction offSide, const Mat &trainFeaturesMat, const Mat &trainLabelsMat);
	void getLosBndBoxByUfmClr();
	void getLosBndBoxByClrAndFg();

public:
	struct playId pId;

	int mos;
	vector<struct yardLine> yardLines;
	vector<vector<Point2d> > yardLnsFldModel;
	double yardLnsDist;

	//line of scrimmage from Sheng's gradient results
	Point2d losLine[2];
	//los center bounding box
	struct rect losBndBox;
	Point2d losCnt;

	Point2d rectLosLine[2];
	//los after rectification
	struct rect rectLosBndBox;
	//los center after rectification
	Point2d rectLosCnt;

	//rectified image boundary
	struct rect imgBndRect;

	string scrimLnsGradFilePath;
	string yardLnsFilePath, mosFilePath;
	string plotPath, trueDirFilePath;
	string vidIdxStr, gameIdStr;
	string fgImgPath, videoPath;
	string mosImgPath;

	//predicated and true offense direction
	direction preDir, trueDir;
	Mat fgImage, mosFrame;
	Mat rectImage, rectMosFrame;

	fieldModel* fld;
	int fldModType;

};

#endif
