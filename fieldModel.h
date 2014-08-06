#ifndef _FIELD_MODEL_H_
#define _FIELD_MODEL_H_
#include <cv.h>

class fieldModel
{
public:
	//if fldModel is 1, high school model
	//if fldModel is 2, college model
	fieldModel(int fldModel);
	bool isPointInsideFld(cv::Point2d p);
	int fieldLength;
	int fieldWidth;
	int endZoneWidth;
	int yardLinesDist;
	int hashLinesDist;
	int hashToSideLineDist;
	int hashLinesLen;
	int hashNumToSideLineDist;
};

#endif
