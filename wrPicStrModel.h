#ifndef _WR_PICSTR_MODEL_H_
#define _WR_PICSTR_MODEL_H_

#include <vector>
#include <string>
#include <cv.h>
#include <highgui.h>

#include "commonStructs.h"

using namespace std;
using namespace cv;



//struct annotPlayStrInfo{
//	struct bndBox upRec, lowRec, scrimLn;
//	int playId;
//};

class wrPicStrModel
{
public:
	wrPicStrModel(direction d, string annotPath);
	void constructModel();
	//~wrPicStrModel();
public:
	direction dir;
	struct bndBox lowRecBndBox, upRecBndBox;
	Point2d vecScrimToUpRec, vecScrimToLowRec;
	double vecLenScrimToUpRec, vecLenScrimToLowRec;
	//double vecLenScrimToUpRec, vecLenScrimToLowRec;
	string annotFilePath;

};
struct playId parseImgPath(string imgPath);
bool readAnnotFile(string annotFilePath, vector<bndBox> &lowRecBoxes, vector<bndBox> &upRecBoxes, vector<playId> &pIds);


#endif
