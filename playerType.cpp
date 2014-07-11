#include <iostream>
#include <string>
#include "commonStructs.h"
#include "fieldModel.h"
#include "playerType.h"

using namespace std;
using namespace cv;

playerType::playerType(playerTypeId pId)
{
	pTypeId = pId;
	//pTypeStr = NULL;
	setUpPlayerTypeStr();
}

void playerType::setUpPlayerTypeStr()
{
	switch(pTypeId){
	case upWR:
		pTypeStr = "upWR";
		break;
	case lowWR:
		pTypeStr = "lowWR";
		break;
	case runBack:
		pTypeStr = "rB";
		break;
	default:
		cout << "Non-existing player type!" << endl;
		break;
	}
}

void playerType::getSearchRng(const struct rect &imgBndRect, const cv::Point2d &rectLosCnt,
		direction offDir, struct rectHorzVertRng &rng)
{
	struct rect quadRng;//quadrangle range
	switch(pTypeId){
	case upWR:
		if(offDir == leftDir)
		{
			quadRng.b = rectLosCnt;
			quadRng.d = imgBndRect.a;
			quadRng.a.y = rectLosCnt.y;
			quadRng.a.x = imgBndRect.a.x - (imgBndRect.a.y - quadRng.a.y) *
					(imgBndRect.a.x - imgBndRect.b.x) / (imgBndRect.a.y - imgBndRect.b.y);
			quadRng.c.x = rectLosCnt.x;
			quadRng.c.y = imgBndRect.a.y - (imgBndRect.a.x - quadRng.c.x) *
					(imgBndRect.a.y - imgBndRect.d.y) / (imgBndRect.a.x - imgBndRect.d.x);

		}
		else if(offDir == rightDir)
		{
			quadRng.a = rectLosCnt;
			quadRng.c = imgBndRect.d;
			quadRng.b.y = rectLosCnt.y;
			quadRng.b.x = imgBndRect.d.x - (imgBndRect.d.y - quadRng.b.y) *
					(imgBndRect.d.x - imgBndRect.c.x) / (imgBndRect.d.y - imgBndRect.c.y);
			quadRng.d.x = rectLosCnt.x;
			quadRng.d.y = imgBndRect.a.y - (imgBndRect.a.x - quadRng.d.x) *
					(imgBndRect.a.y - imgBndRect.d.y) / (imgBndRect.a.x - imgBndRect.d.x);
		}
		else
		{
			cout << "Wrong offense direction!" << endl;
			return;
		}
		break;
	case lowWR:
		if(offDir == leftDir)
		{
			quadRng.c = rectLosCnt;
			quadRng.a = imgBndRect.b;
			quadRng.d.y = rectLosCnt.y;
			quadRng.d.x = imgBndRect.a.x - (imgBndRect.a.y - quadRng.d.y) *
					(imgBndRect.a.x - imgBndRect.b.x) / (imgBndRect.a.y - imgBndRect.b.y);
			quadRng.b.x = rectLosCnt.x;
			quadRng.b.y = imgBndRect.b.y - (imgBndRect.b.x - quadRng.b.x) *
					(imgBndRect.b.y - imgBndRect.c.y) / (imgBndRect.b.x - imgBndRect.c.x);
		}
		else if(offDir == rightDir)
		{
			quadRng.d = rectLosCnt;
			quadRng.b = imgBndRect.c;
			quadRng.c.y = rectLosCnt.y;
			quadRng.c.x = imgBndRect.d.x - (imgBndRect.d.y - quadRng.c.y) *
					(imgBndRect.d.x - imgBndRect.c.x) / (imgBndRect.d.y - imgBndRect.c.y);
			quadRng.a.x = rectLosCnt.x;
			quadRng.a.y = imgBndRect.b.y - (imgBndRect.b.x - quadRng.a.x) *
					(imgBndRect.b.y - imgBndRect.c.y) / (imgBndRect.b.x - imgBndRect.c.x);
		}
		else
		{
			cout << "Wrong offense direction!" << endl;
			return;
		}
		break;
	case runBack:
		if(offDir == leftDir)
		{
			quadRng.a = imgBndRect.a;
			quadRng.b = imgBndRect.b;
			quadRng.c.x = rectLosCnt.x;
			quadRng.c.y = imgBndRect.b.y - (imgBndRect.b.x - quadRng.c.x) *
					(imgBndRect.b.y - imgBndRect.c.y) / (imgBndRect.b.x - imgBndRect.c.x);
			quadRng.d.x = rectLosCnt.x;
			quadRng.d.y = imgBndRect.a.y - (imgBndRect.a.x - quadRng.d.x) *
					(imgBndRect.a.y - imgBndRect.d.y) / (imgBndRect.a.x - imgBndRect.d.x);
		}
		else if(offDir == rightDir)
		{
			quadRng.c = imgBndRect.c;
			quadRng.d = imgBndRect.d;
			quadRng.b.x = rectLosCnt.x;
			quadRng.b.y = imgBndRect.b.y - (imgBndRect.b.x - quadRng.b.x) *
					(imgBndRect.b.y - imgBndRect.c.y) / (imgBndRect.b.x - imgBndRect.c.x);
			quadRng.a.x = rectLosCnt.x;
			quadRng.a.y = imgBndRect.a.y - (imgBndRect.a.x - quadRng.a.x) *
					(imgBndRect.a.y - imgBndRect.d.y) / (imgBndRect.a.x - imgBndRect.d.x);
		}
		else
		{
			cout << "Wrong offense direction!" << endl;
			return;
		}
		break;
	default:
		cout << "Non-existing player type!" << endl;
		return;
		break;
	}

	rng.xMin = min(min(quadRng.a.x, quadRng.b.x), min(quadRng.c.x, quadRng.d.x));
	rng.xMax = max(max(quadRng.a.x, quadRng.b.x), max(quadRng.c.x, quadRng.d.x));
	rng.yMin = min(min(quadRng.a.y, quadRng.b.y), min(quadRng.c.y, quadRng.d.y));
	rng.yMax = max(max(quadRng.a.y, quadRng.b.y), max(quadRng.c.y, quadRng.d.y));
}


void playerType::getSearchRng(int fldModel, const cv::Point2d &rectLosCnt,
		direction offDir, struct rectHorzVertRng &rng)
{
	fieldModel fld(fldModel);
	switch(pTypeId){
	case upWR:
		if(offDir == leftDir)
		{
			rng.xMin = 0;
			rng.xMax = rectLosCnt.x;
			rng.yMin = 0;
			rng.yMax = rectLosCnt.y;
		}
		else if(offDir == rightDir)
		{
			rng.xMin = rectLosCnt.x;
			rng.xMax = fld.fieldWidth;
			rng.yMin = 0;
			rng.yMax = rectLosCnt.y;
		}
		else
		{
			cout << "Wrong offense direction!" << endl;
			return;
		}
		break;
	case lowWR:
		if(offDir == leftDir)
		{
			rng.xMin = 0;
			rng.xMax = rectLosCnt.x;
			rng.yMin = rectLosCnt.y;
			rng.yMax = fld.fieldLength;
		}
		else if(offDir == rightDir)
		{
			rng.xMin = rectLosCnt.x;
			rng.xMax = fld.fieldWidth;
			rng.yMin = rectLosCnt.y;
			rng.yMax = fld.fieldLength;
		}
		else
		{
			cout << "Wrong offense direction!" << endl;
			return;
		}
		break;
	case runBack:
		if(offDir == leftDir)
		{
			rng.xMin = 0;
			rng.xMax = rectLosCnt.x;
			rng.yMin = 0;
			rng.yMax = fld.fieldLength;
		}
		else if(offDir == rightDir)
		{
			rng.xMin = rectLosCnt.x;
			rng.xMax = fld.fieldWidth;
			rng.yMin = 0;
			rng.yMax = fld.fieldLength;
		}
		else
		{
			cout << "Wrong offense direction!" << endl;
			return;
		}
		break;
	default:
		cout << "Non-existing player type!" << endl;
		return;
		break;
	}
}


void playerType::getModelVec(const cv::Point2d &rectLosCnt, int fldModel,
		direction offDir, cv::Point2d &modelVec)
{
	fieldModel fld(fldModel);
//	Point2d playerModelPos;
	switch(pTypeId){
	case upWR:
//		playerModelPos.x = rectLosCnt.x;
//		playerModelPos.y = fld.hashNumToSideLineDist;
		modelVec = Point2d(.0, fld.hashNumToSideLineDist - rectLosCnt.y);
		break;
	case lowWR:
//		playerModelPos.x = rectLosCnt.x;
//		playerModelPos.y = fld.fieldLength - fld.hashNumToSideLineDist;
		modelVec = Point2d(.0, fld.fieldLength - fld.hashNumToSideLineDist - rectLosCnt.y);
		break;
	case runBack:
		//playerModelPos.y = rectLosCnt.y;
		if(offDir == leftDir)
		{
			//playerModelPos.x = rectLosCnt.x - 2 * fld.yardLinesDist;
			modelVec = Point2d(-2 * fld.yardLinesDist, .0);
		}
		else if(offDir == rightDir)
		{
			//playerModelPos.x = rectLosCnt.x + 2 * fld.yardLinesDist;
			modelVec = Point2d(2 * fld.yardLinesDist, .0);
		}
		else
		{
			cout << "Wrong offense direction!" << endl;
			return;
		}
		break;
	default:
		cout << "Non-existing player type!" << endl;
		break;
	}

	//modelVec = playerModelPos - rectLosCnt;
}
