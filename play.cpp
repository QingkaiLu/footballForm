#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <limits>
#include <cv.h>
#include <ml.h>

#include "play.h"
#include "blob_HUDL.h"
#include "ellipse.h"
#include "imgRectification.h"
#include "playAuxFunc.h"
#include "matchImgs.h"
#include "fieldModel.h"
#include "playerType.h"
#include "formTree.h"
#include "knn.h"

play::play(struct playId p)
{
	pId.gameId = p.gameId;
	pId.vidId = p.vidId;
	fld = NULL;
	fldModType = 0;

	mos = -1;
	yardLnsDist = -1.0;

	preDir = nonDir;
	trueDir = nonDir;

	ostringstream convertGameId;
	convertGameId << pId.gameId ;
	gameIdStr = convertGameId.str();

	if(pId.gameId < 10)
		gameIdStr = "0" + gameIdStr;

	ostringstream convertVidId;
	convertVidId << pId.vidId;
	vidIdxStr = convertVidId.str();
	if(pId.vidId < 10)
		vidIdxStr = "00" + vidIdxStr;
	else if(pId.vidId < 100 )
		vidIdxStr = "0" + vidIdxStr;

#if predMos == 1
	mosFilePath = "Mos/Game" + gameIdStr  + "/annotation.pred";
	scrimLnsGradFilePath = "scrimLine/Game" + gameIdStr + "/losPredMos/video0" + vidIdxStr + ".glos4";
	mosImgPath = "predMosImages/Game" + gameIdStr + "/vid" + vidIdxStr + ".jpg";
	fgImgPath = "predFgMosImages/Game" + gameIdStr + "/video0" + vidIdxStr +".bmp";
#elif predMos == 0
	mosFilePath = "Mos/Game" + gameIdStr  + "/mos_id_new.txt";
	scrimLnsGradFilePath = "scrimLine/Game" + gameIdStr + "/losTrueMos/video0" + vidIdxStr + ".glos4";
	mosImgPath = "mosImages/Game" + gameIdStr + "/vid" + vidIdxStr + ".jpg";
	fgImgPath = "fgMosImages/Game" + gameIdStr + "/video0" + vidIdxStr +".bmp";
#endif


#if yardLinesMethod == 1
	yardLnsFilePath = "yardLines/Game" +  gameIdStr + "/LinesAllFrames/video0" +  vidIdxStr +".line_new";
#elif yardLinesMethod == 2
	yardLnsFilePath = "yardLines/Game" +  gameIdStr + "/newLinesAllFrames/video0" +  vidIdxStr +".line_new";
#endif
	trueDirFilePath = "ODKGt/Game" + gameIdStr + "/game" + gameIdStr + "_wr_new";

//	fgImgPath = "fgMosImages/Game" + gameIdStr + "/video0" + vidIdxStr +".bmp";
	videoPath = "../videos/Game" + gameIdStr + "/Game" + gameIdStr + "_video0" + vidIdxStr +".avi";
//	mosImgPath = "mosImages/Game" + gameIdStr + "/vid" + vidIdxStr + ".jpg";
	fgImage = imread(fgImgPath.c_str(), CV_LOAD_IMAGE_COLOR);
	mosFrame = imread(mosImgPath.c_str(), CV_LOAD_IMAGE_COLOR);
//	fgImage.create(imgYLen, imgXLen, CV_32FC3);
//	fgImage = cv::Scalar(0,0,0);

	return;
}

play::play(struct playId p, int fldModel)
{
	pId.gameId = p.gameId;
	pId.vidId = p.vidId;
	fld = new fieldModel(fldModel);
	fldModType = fldModel;

	mos = -1;
	yardLnsDist = -1.0;

	preDir = nonDir;
	trueDir = nonDir;

	ostringstream convertGameId;
	convertGameId << pId.gameId ;
	gameIdStr = convertGameId.str();

	if(pId.gameId < 10)
		gameIdStr = "0" + gameIdStr;

	ostringstream convertVidId;
	convertVidId << pId.vidId;
	vidIdxStr = convertVidId.str();
	if(pId.vidId < 10)
		vidIdxStr = "00" + vidIdxStr;
	else if(pId.vidId < 100 )
		vidIdxStr = "0" + vidIdxStr;

#if predMos == 1
	mosFilePath = "Mos/Game" + gameIdStr  + "/annotation.pred";
	scrimLnsGradFilePath = "scrimLine/Game" + gameIdStr + "/losPredMos/video0" + vidIdxStr + ".glos4";
	mosImgPath = "predMosImages/Game" + gameIdStr + "/vid" + vidIdxStr + ".jpg";
	fgImgPath = "predFgMosImages/Game" + gameIdStr + "/video0" + vidIdxStr +".bmp";
#elif predMos == 0
	mosFilePath = "Mos/Game" + gameIdStr  + "/mos_id_new.txt";
	scrimLnsGradFilePath = "scrimLine/Game" + gameIdStr + "/losTrueMos/video0" + vidIdxStr + ".glos4";
	mosImgPath = "mosImages/Game" + gameIdStr + "/vid" + vidIdxStr + ".jpg";
	fgImgPath = "fgMosImages/Game" + gameIdStr + "/video0" + vidIdxStr +".bmp";
#endif


#if yardLinesMethod == 1
	yardLnsFilePath = "yardLines/Game" +  gameIdStr + "/LinesAllFrames/video0" +  vidIdxStr +".line_new";
#elif yardLinesMethod == 2
	yardLnsFilePath = "yardLines/Game" +  gameIdStr + "/newLinesAllFrames/video0" +  vidIdxStr +".line_new";
#endif
	trueDirFilePath = "ODKGt/Game" + gameIdStr + "/game" + gameIdStr + "_wr_new";

//	fgImgPath = "fgMosImages/Game" + gameIdStr + "/video0" + vidIdxStr +".bmp";
	videoPath = "../videos/Game" + gameIdStr + "/Game" + gameIdStr + "_video0" + vidIdxStr +".avi";
//	mosImgPath = "mosImages/Game" + gameIdStr + "/vid" + vidIdxStr + ".jpg";
	fgImage = imread(fgImgPath.c_str(), CV_LOAD_IMAGE_COLOR);
	mosFrame = imread(mosImgPath.c_str(), CV_LOAD_IMAGE_COLOR);
//	fgImage.create(imgYLen, imgXLen, CV_32FC3 );
//	fgImage = cv::Scalar(0,0,0);

	return;
}

play::~play()
{
	//imwrite(plotPath, fgImage);
	fgImage.release();

	if(fld != NULL)
		delete fld;
}
void play::getMos()
{
	vector<int> mosAllVids;
	ifstream finMos(mosFilePath.c_str());
	if(!finMos.is_open())
	{
		cout<<"Can't open MOS file "<<mosFilePath<<endl;
		return;
	}

	while(!finMos.eof())
	{
		int m = -100;
		finMos >> m;
//			if((mos == -2) || (mos == 0))//kick or early MOS
//				mos = 1;
//			if((mos == -2) || (mos == 0) || (mos == -1))
//				mos = 1;
		if( (m <= 0) && (m != -100))
			m = 1;
		if(m != -100)
			mosAllVids.push_back(m);
	}

	finMos.close();

	mos = mosAllVids[pId.vidId - 1];

	return;
}

bool play::getYardLines()
{
	getMos();

	string filePath = yardLnsFilePath;

	ifstream fin(filePath.c_str());
	if(!fin.is_open())
	{
		cout<<"Can't open file "<<filePath<<endl;
		return false;
	}

	//cout<<"Can open file "<<filePath<<endl;

	fin.seekg(0, ios::end);
	if (fin.tellg() == 0) {
		cout<<"Empty line file"<<filePath<<endl;
		return false;
	}

	fin.seekg(0, ios::beg);
	int frame = 1;
	while(!fin.eof())
	{
		int lineNum = -1;
		fin >> lineNum;
		//cout << lineNum << endl;

		if( (lineNum >= 2) && (frame >= mos) )
		{
			for(int i = 0; i < lineNum; ++i)
			{
				struct yardLine l;
				fin >> l.rho >> l.theta >> l.index;
//				cout << l.rho << " " << l.theta << " " << l.index << endl;
				//cout<< "frame: " << frame << endl;;
				yardLines.push_back(l);
			}
			return true;
		}
		else
		{
			struct yardLine ln;
			for(int i = 0; i < lineNum; ++i)
			fin >> ln.rho >> ln.theta >> ln.index;
		}
		if(frame > mos + 45)
			break;
		++frame;
	}

	fin.close();

	return false;

}

bool compLns(struct yardLine l1, struct yardLine l2)
{
	float rho1 = l1.rho;
	float theta1 = l1.theta;
	double cosTheta1 = cos(theta1), sinTheta1 = sin(theta1);
	double x1 = (rho1 - 240 * sinTheta1) / cosTheta1;

	float rho2 = l2.rho;
	float theta2 = l2.theta;
	double cosTheta2 = cos(theta2), sinTheta2 = sin(theta2);
	double x2 = (rho2 - 240 * sinTheta2) / cosTheta2;

	return (x1 < x2);
}

void play::computeYardLnsDist()
{
	bool yLnsExist = getYardLines();
	if(yLnsExist)
	{
#if yardLnsDistMethod == 1
		yardLnsDist = closestLnDist(yardLines);
#elif yardLnsDistMethod == 2
		yardLnsDist = avgLnDist(yardLines);
#endif
	}
	else
		yardLnsDist = 100.0;

}

void play::getGradientScrimLn()
{
	bool losExist = readGradientLos(scrimLnsGradFilePath, losLine);

	if(!losExist)
	{
		losLine[0].x = imgXLen * 0.5;
		losLine[0].y = 0.0;
		losLine[1].x = imgXLen * 0.5;;
		losLine[1].y = imgYLen;
	}

	findLosBndBox();
}

void play::findLosBndBox()
{
	Point2d vecLos = losLine[1] - losLine[0];
	double vecLosLen = norm(vecLos);
//	cout << losLine[0].x << " " << losLine[0].y << endl;
//	cout << losLine[1].x << " " << losLine[1].y << endl;
//	cout << "vecLosLen: " << vecLosLen << endl;
	vecLos *= 1.0 / vecLosLen;
	//vecLos.y should be < 0.0
	if(vecLos.y > 0)
		vecLos *= -1.0;
	//vecPerpLos.x should be < 0.0
	Point2d vecPerpLos = Point2d(vecLos.y, -1.0 * vecLos.x);
	double scanStep = 4.0;
	int maxFgPixNum = NEGINF;
	double vecLosBoxLen = yardLnsDist, vecPerpLosBoxLen = 0.5 * yardLnsDist;

	for(int i = 1; i < vecLosLen; i += scanStep)
	{
		Point2d boxCnt = losLine[0] - i * scanStep * vecLos;
		struct rect bndBox;
		bndBox.a = boxCnt + 0.5 * vecLosBoxLen * vecLos + 0.5 * vecPerpLosBoxLen * vecPerpLos;
		bndBox.b = boxCnt - 0.5 * vecLosBoxLen * vecLos + 0.5 * vecPerpLosBoxLen * vecPerpLos;
		bndBox.c = boxCnt - 0.5 * vecLosBoxLen * vecLos - 0.5 * vecPerpLosBoxLen * vecPerpLos;
		bndBox.d = boxCnt + 0.5 * vecLosBoxLen * vecLos - 0.5 * vecPerpLosBoxLen * vecPerpLos;

		int fgPixNum = fgPixelsInsideBox(fgImage, bndBox);

		if(fgPixNum >= maxFgPixNum)
		{
			losBndBox = bndBox;
			maxFgPixNum = fgPixNum;
		}

//		losBndBox = bndBox;

//		plotYardLnsAndLos(fgImage, losBndBox, yardLines, losLine);
//
//		plotPath = "Results/Game" + gameIdStr + "/plots/vid" + vidIdxStr + ".jpg";
//
//		imwrite(plotPath, fgImage);

		//cout << i << " " << fgPixNum << endl;

	}

	losCnt = (losBndBox.a + losBndBox.b + losBndBox.c + losBndBox.d) * 0.25;

	if(losCnt.y < imgYLen * 0.3)
		losCnt.y = imgYLen * 0.3;
	else if(losCnt.y > imgYLen * 0.7)
		losCnt.y = imgYLen * 0.7;

}

void play::findLosOnRectFg(const Mat &homoMat)
{
//	struct rect imgBnd, imgBndRect;
	struct rect imgBnd;
	imgBnd.a = Point2d(.0, .0);
	imgBnd.b = Point2d(.0, imgYLen - 1);
	imgBnd.c = Point2d(imgXLen - 1, imgYLen - 1);
	imgBnd.d = Point2d(imgXLen - 1, .0);
	vector<Point2d> srcImgBndVec, dstImgBndVec;
	srcImgBndVec.push_back(imgBnd.a);
	srcImgBndVec.push_back(imgBnd.b);
	srcImgBndVec.push_back(imgBnd.c);
	srcImgBndVec.push_back(imgBnd.d);
	perspectiveTransform(srcImgBndVec, dstImgBndVec, homoMat);
	imgBndRect.a = dstImgBndVec[0];
	imgBndRect.b = dstImgBndVec[1];
	imgBndRect.c = dstImgBndVec[2];
	imgBndRect.d = dstImgBndVec[3];

	int maxFgPixelsNum = NEGINF;
//	for(int xCnt = fld->endZoneWidth - 1; xCnt < (fld->fieldWidth - fld->endZoneWidth); xCnt += 5)
//		for(int yCnt = fld->hashToSideLineDist - 1; yCnt < (fld->fieldLength - fld->hashToSideLineDist); yCnt += 5)
	for(int xCnt = fld->endZoneWidth; xCnt < (fld->fieldWidth - fld->endZoneWidth); xCnt += 5)
		for(int yCnt = fld->hashToSideLineDist; yCnt < (fld->fieldLength - fld->hashToSideLineDist); yCnt += 5)
		{
			Point2d scanRectCnt = Point2d(xCnt, yCnt);
			if(!isPntInsideRect(scanRectCnt, imgBndRect))
				continue;
//			double lowY = yCnt - 2 * fld->yardLinesDist;
//			double upY = yCnt + 2 * fld->yardLinesDist;
			double lowY = yCnt - fld->yardLinesDist;
			double upY = yCnt + fld->yardLinesDist;
			double leftX = xCnt - 0.5 * fld->yardLinesDist;
			double rightX = xCnt + 0.5 * fld->yardLinesDist;
			int fgPixelsNum = 0;
			for(int y = lowY; y < upY; ++y)
				for(int x = leftX; x < rightX; ++x)
				{
					if( y < 0 || y >= fld->fieldLength)
						continue;
					if( x < 0 || x >= fld->fieldWidth)
						continue;
					const Point3_<uchar>* p = rectImage.ptr<Point3_<uchar> >(y, x);
					if(int(p->z) == 255)
					{
	//					Point2d pnt = Point2d(x, y);
	//					if((pnt.x >= minYardLnXCoord) && (pnt.x < maxYardLnXCoord))
							++fgPixelsNum;
					}
				}
			if(fgPixelsNum >= maxFgPixelsNum)
			{
				maxFgPixelsNum = fgPixelsNum;
				rectLosCnt = Point2d(xCnt, yCnt);
				rectLosBndBox.a = Point2d(leftX, lowY);
				rectLosBndBox.b = Point2d(leftX, upY);
				rectLosBndBox.c = Point2d(rightX, upY);
				rectLosBndBox.d = Point2d(rightX, lowY);
			}
		}
}


int play::getLosCntIdx()
{
	double distUpHashToLosY = rectLosCnt.y - fld->hashToSideLineDist;
	int losCntIdx = distUpHashToLosY / ((fld->fieldLength - 2 * fld->hashToSideLineDist) / (double)losCntBins);
	return losCntIdx;
}



void play::getTrueDir()
{
	ifstream finDirGt(trueDirFilePath.c_str());
	if(!finDirGt.is_open())
	{
		cout << "Can't open direction GT file!" << trueDirFilePath << endl;
		return;
	}
	//int vId = 0;
	vector<string> dirGt;
	while(!finDirGt.eof())
	{
		string d = "NULL";
		finDirGt >> d;
		//cout << "d: " << d << endl;
		if(d.compare("NULL") != 0)
			dirGt.push_back(d);
	}

	finDirGt.close();

	if(dirGt[pId.vidId - 1].compare("l") == 0)
		trueDir = leftDir;
	else if(dirGt[pId.vidId - 1].compare("r") == 0)
		trueDir = rightDir;
	else
		trueDir = nonDir;
	return;
}


void play::setUp()
{
	getMos();
//	computeYardLnsDist();
	getTrueDir();

}

void play::saveMosFrm()
{
	VideoCapture capture(videoPath);
	if (!capture.isOpened()) {
		cout << "Error in opening the video";
		return;
	}

	if(mos < 1)
		mos = 1;
	if(mos > capture.get(CV_CAP_PROP_FRAME_COUNT))
		mos = capture.get(CV_CAP_PROP_FRAME_COUNT);
//	cout << capture.get(CV_CAP_PROP_FRAME_COUNT) << endl;
	Mat frame;
	for(int i = 1; i <= mos; ++i)
		capture >> frame;
//
//	plotPath = "mosImages/Game" + gameIdStr + "/vid" + vidIdxStr + ".jpg";


	ostringstream convertVidId;
	int mosImgsVidIdx = pId.vidId;
	convertVidId << mosImgsVidIdx;
	string mosImgsVidIdxStr = convertVidId.str();
	if(mosImgsVidIdx < 10)
		mosImgsVidIdxStr = "00" + mosImgsVidIdxStr;
	else if(mosImgsVidIdx < 100 )
		mosImgsVidIdxStr = "0" + mosImgsVidIdxStr;
//
#if predMos == 0
	string odplotPath = "mosImages/Game" + gameIdStr + "/od/vid" + mosImgsVidIdxStr + ".jpg";
	string nonOdPlotPath = "mosImages/Game" + gameIdStr + "/nonOd/vid" + mosImgsVidIdxStr + ".jpg";
#elif predMos == 1
	string odplotPath = "predMosImages/Game" + gameIdStr + "/od/vid" + mosImgsVidIdxStr + ".jpg";
	string nonOdPlotPath = "predMosImages/Game" + gameIdStr + "/nonOd/vid" + mosImgsVidIdxStr + ".jpg";
#endif
	if(trueDir != nonDir)
		imwrite(odplotPath, frame);
	else
		imwrite(nonOdPlotPath, frame);

//	imwrite(mosImgPath, frame);

}

void play::saveMosFrmToClst(int clst)
{
	ostringstream convertClstIdx;
	convertClstIdx << clst;
	string clstIdxStr = convertClstIdx.str();
//file:///scratch/workspace/picStrucWR/playsClusters/1
	string clstPath = "playsClusters/" + clstIdxStr + "/" + vidIdxStr + ".jpg";
	imwrite(clstPath, mosFrame);
	string clstFgPath = "playsClusters/" + clstIdxStr + "/" + vidIdxStr + "Fg.jpg";
	imwrite(clstFgPath, fgImage);

}

void play::cvtFgImgFrmBmpToJpg()
{
	string jpgImgPath = "fgMosJpgImages/Game" + gameIdStr + "/000" + vidIdxStr +".jpg";
	imwrite(jpgImgPath, fgImage);
}

void play::extractOdStripsFeatRect(direction dir, vector<int> &featureVec)
{
	rectification();

	int d = 0;
	string dirStr;
	if(dir == leftDir)
	{
		d = -1;
		dirStr = "Left";
	}
	else if (dir == rightDir)
	{
		d = 1;
		dirStr = "Right";
	}
	else
		return;

	vector<rect> scanLines;
	//vector<rect> scanLinesPerp;

//		cout << "pLos.x: " << pLos.x << endl;
	for(int i = 0; i < 5; ++i)
	{
		int fgPixelsNum = 0;
		struct rect rectLines;

		double maxYardLnXCoord, minYardLnXCoord;

		if(dir == rightDir)
		{
			maxYardLnXCoord = rectLosCnt.x + d * (i + 1) * fld->yardLinesDist;
			minYardLnXCoord = rectLosCnt.x + d * i * fld->yardLinesDist;
		}
		else
		{
			maxYardLnXCoord = rectLosCnt.x + d * i * fld->yardLinesDist;
			minYardLnXCoord = rectLosCnt.x + d * (i + 1) * fld->yardLinesDist;
		}

		rectLines.a = Point2d(minYardLnXCoord, .0);
		rectLines.b = Point2d(minYardLnXCoord, fld->fieldLength);
		rectLines.c = Point2d(maxYardLnXCoord, fld->fieldLength);
		rectLines.d = Point2d(maxYardLnXCoord, .0);


		for(int y = 0; y < fld->fieldLength; ++y)
			for(int x = minYardLnXCoord - 1; x < maxYardLnXCoord - 1; ++x)
			{
				if( x < 0 || x >= fld->fieldWidth)
					continue;

				const Point3_<uchar>* p = rectImage.ptr<Point3_<uchar> >(y, x);
				if(int(p->z) == 255)
				{
//					Point2d pnt = Point2d(x, y);
//					if((pnt.x >= minYardLnXCoord) && (pnt.x < maxYardLnXCoord))
						++fgPixelsNum;
				}
			}
		scanLines.push_back(rectLines);
		featureVec.push_back(fgPixelsNum);

	}
//	plotYardLnsAndLos(fgImage, losBndBox, yardLines);

#if losMethod == 1
	plotLos(rectImage, rectLosBndBox);
#elif losMethod == 2
	plotLos(rectImage, rectLosBndBox, rectLosLine);
#endif

	circle(rectImage, rectLosCnt, 3, CV_RGB(0, 0, 250), 3);

	plotScanLines(rectImage, scanLines, featureVec);

	plotPath = "Results/Game" + gameIdStr + "/plots/vid" + vidIdxStr + dirStr + ".jpg";

	imwrite(plotPath, rectImage);
}

void play::extractOdGridsFeatRect(direction dir, vector<int> &featureVec)
{
	rectification();

	int d = 0;
	string dirStr;
	if(dir == leftDir)
	{
		d = -1;
		dirStr = "Left";
	}
	else if (dir == rightDir)
	{
		d = 1;
		dirStr = "Right";
	}
	else
		return;

	vector<rect> scanRects;
	//vector<rect> scanLinesPerp;

//		cout << "pLos.x: " << pLos.x << endl;
	for(int i = 0; i < 5; ++i)
	{
		double maxYardLnXCoord, minYardLnXCoord;

		if(dir == rightDir)
		{
			maxYardLnXCoord = rectLosCnt.x + d * (i + 1) * fld->yardLinesDist;
			minYardLnXCoord = rectLosCnt.x + d * i * fld->yardLinesDist;
		}
		else
		{
			maxYardLnXCoord = rectLosCnt.x + d * i * fld->yardLinesDist;
			minYardLnXCoord = rectLosCnt.x + d * (i + 1) * fld->yardLinesDist;
		}

		for(int j = -5; j < 5; ++j)
		{
			int fgPixelsNum = 0;
			struct rect scanR;

//			double lowY = rectLosCnt.y + j * (fld->fieldLength / 10.0);
//			double upY = rectLosCnt.y + (j + 1) * (fld->fieldLength / 10.0);
			double lowY = rectLosCnt.y + j * (fld->yardLinesDist * 2.0);
			double upY = rectLosCnt.y + (j + 1) * (fld->yardLinesDist * 2.0);
//			cout << "lowY: " << lowY << endl;
//			cout << "upY: " << upY << endl;
			scanR.a = Point2d(minYardLnXCoord, lowY);
			scanR.b = Point2d(minYardLnXCoord, upY);
			scanR.c = Point2d(maxYardLnXCoord, upY);
			scanR.d = Point2d(maxYardLnXCoord, lowY);


			for(int y = lowY; y < upY; ++y)
				for(int x = minYardLnXCoord; x < maxYardLnXCoord; ++x)
				{
					if( y < 0 || y >= fld->fieldLength)
						continue;
					if( x < 0 || x >= fld->fieldWidth)
						continue;
					const Point3_<uchar>* p = rectImage.ptr<Point3_<uchar> >(y, x);
					if(int(p->z) == 255)
					{
	//					Point2d pnt = Point2d(x, y);
	//					if((pnt.x >= minYardLnXCoord) && (pnt.x < maxYardLnXCoord))
							++fgPixelsNum;
					}
				}
			scanRects.push_back(scanR);
			featureVec.push_back(fgPixelsNum);

		}
	}
//	plotYardLnsAndLos(fgImage, losBndBox, yardLines);

#if losMethod == 1
	plotLos(rectImage, rectLosBndBox);
	plotLos(rectMosFrame, rectLosBndBox);
#elif losMethod == 2
	plotLos(rectImage, rectLosBndBox, rectLosLine);
#endif

	circle(rectImage, rectLosCnt, 3, CV_RGB(0, 0, 250), 3);
	circle(rectMosFrame, rectLosCnt, 3, CV_RGB(0, 0, 250), 3);

	plotScanLines(rectImage, scanRects, featureVec);
	plotScanLines(rectMosFrame, scanRects, featureVec);

	plotPath = "Results/Game" + gameIdStr + "/plots/vid" + vidIdxStr + dirStr + ".jpg";
	string mosPlotPath = "Results/Game" + gameIdStr + "/plots/vidMos" + vidIdxStr + dirStr + ".jpg";

	imwrite(plotPath, rectImage);
	imwrite(mosPlotPath, rectMosFrame);
}

void play::extractOdGridsFeatRect(direction dir, vector<int> &featureVec,
		const vector<CvSize> &gridSizes, const vector<Point2i> &gridsNum, int expMode)
{

	rectification();

	int d = 0;
	string dirStr;
	if(dir == leftDir)
	{
		d = -1;
		dirStr = "Left";
	}
	else if (dir == rightDir)
	{
		d = 1;
		dirStr = "Right";
	}
	else
		return;

	vector<rect> scanRects;

	for(unsigned int k = 0; k < gridSizes.size(); ++k)
	{

		vector<rect> scanRectsOneLevel;
		vector<int> featureVecLevel;

//		for(int i = 0; i < gridsNum[k].x; ++i)
		for(double i = 0; i < gridsNum[k].x; ++i)
		{
			double maxYardLnXCoord, minYardLnXCoord;

			if(dir == rightDir)
			{
				maxYardLnXCoord = rectLosCnt.x + d * (i + 1) * gridSizes[k].width;
				minYardLnXCoord = rectLosCnt.x + d * i * gridSizes[k].width;
			}
			else
			{
				maxYardLnXCoord = rectLosCnt.x + d * i * gridSizes[k].width;
				minYardLnXCoord = rectLosCnt.x + d * (i + 1) * gridSizes[k].width;
			}

//			for(int j = -0.5 * gridsNum[k].y; j < gridsNum[k].y * 0.5; ++j)
			for(double j = -0.5 * gridsNum[k].y; j < gridsNum[k].y * 0.5; ++j)
			{
				int fgPixelsNum = 0;
				struct rect scanR;

	//			double lowY = rectLosCnt.y + j * (fld->fieldLength / 10.0);
	//			double upY = rectLosCnt.y + (j + 1) * (fld->fieldLength / 10.0);
				double lowY = rectLosCnt.y + j * gridSizes[k].height;
				double upY = rectLosCnt.y + (j + 1) * gridSizes[k].height;
	//			cout << "lowY: " << lowY << endl;
	//			cout << "upY: " << upY << endl;
				scanR.a = Point2d(minYardLnXCoord, lowY);
				scanR.b = Point2d(minYardLnXCoord, upY);
				scanR.c = Point2d(maxYardLnXCoord, upY);
				scanR.d = Point2d(maxYardLnXCoord, lowY);

	//			expMode = 1;
				if(expMode)
				{
	//				Point2d scanRCnt = 0.25 * (scanR.a + scanR.b + scanR.c + scanR.d);
					bool insideFld = false, visible = false;

	//				if(scanRCnt.x >= 0 && scanRCnt.x < fld->fieldWidth &&
	//						scanRCnt.y >= 0 && scanRCnt.y < fld->fieldLength)
	//					outsideFld = false;
					if(scanR.a.x >= 0 && scanR.a.x < fld->fieldWidth &&
							scanR.a.y >= 0 && scanR.a.y < fld->fieldLength)
						insideFld = true;
					if(scanR.b.x >= 0 && scanR.b.x < fld->fieldWidth &&
							scanR.b.y >= 0 && scanR.b.y < fld->fieldLength)
						insideFld = true;
					if(scanR.c.x >= 0 && scanR.c.x < fld->fieldWidth &&
							scanR.c.y >= 0 && scanR.c.y < fld->fieldLength)
						insideFld = true;
					if(scanR.d.x >= 0 && scanR.d.x < fld->fieldWidth &&
							scanR.d.y >= 0 && scanR.d.y < fld->fieldLength)
						insideFld = true;

					if(isPntInsideRect(scanR.a, imgBndRect) || isPntInsideRect(scanR.b, imgBndRect)
							|| isPntInsideRect(scanR.c, imgBndRect) || isPntInsideRect(scanR.d, imgBndRect))
	//				if(isPntInsideRect(scanRCnt, imgBndRect))
						visible = true;

					if(!insideFld)
//						fgPixelsNum = -2;
						fgPixelsNum = 0;
					else if(!visible)
						fgPixelsNum = -1;
					else
					{
						for(int y = lowY; y < upY; ++y)
							for(int x = minYardLnXCoord; x < maxYardLnXCoord; ++x)
							{
								if( y < 0 || y >= fld->fieldLength)
									continue;
								if( x < 0 || x >= fld->fieldWidth)
									continue;
								const Point3_<uchar>* p = rectImage.ptr<Point3_<uchar> >(y, x);
								if(int(p->z) == 255)
										++fgPixelsNum;
							}
					}
				}
				else
				{
					for(int y = lowY; y < upY; ++y)
							for(int x = minYardLnXCoord; x < maxYardLnXCoord; ++x)
							{
								if( y < 0 || y >= fld->fieldLength)
									continue;
								if( x < 0 || x >= fld->fieldWidth)
									continue;
								const Point3_<uchar>* p = rectImage.ptr<Point3_<uchar> >(y, x);
								if(int(p->z) == 255)
										++fgPixelsNum;
							}
				}


				scanRects.push_back(scanR);
				featureVec.push_back(fgPixelsNum);

				scanRectsOneLevel.push_back(scanR);
				featureVecLevel.push_back(fgPixelsNum);

			}
		}

	Mat fgImageLevel, mosImageLevel;
	rectImage.copyTo(fgImageLevel);
	rectMosFrame.copyTo(mosImageLevel);
#if losMethod == 1
	plotLos(fgImageLevel, rectLosBndBox);
	plotLos(mosImageLevel, rectLosBndBox);
#elif losMethod == 2
	plotLos(fgImageLevel, rectLosBndBox, rectLosLine);
#endif

	circle(fgImageLevel, rectLosCnt, 3, CV_RGB(0, 0, 250), 3);
	circle(mosImageLevel, rectLosCnt, 3, CV_RGB(0, 0, 250), 3);

	plotScanLines(fgImageLevel, scanRectsOneLevel, featureVecLevel);
	plotScanLines(mosImageLevel, scanRectsOneLevel, featureVecLevel);

	ostringstream convertLevel;
	convertLevel << k;
	string levelStr = convertLevel.str();

	string fgPlotPath = "Results/Game" + gameIdStr + "/plots/vid" + vidIdxStr + dirStr + levelStr + ".jpg";
	string mosPlotPath = "Results/Game" + gameIdStr + "/plots/vidMos" + vidIdxStr + dirStr + levelStr +".jpg";

	imwrite(fgPlotPath, fgImageLevel);
	imwrite(mosPlotPath, mosImageLevel);


	}

//	plotYardLnsAndLos(fgImage, losBndBox, yardLines);

#if losMethod == 1
	plotLos(rectImage, rectLosBndBox);
	plotLos(rectMosFrame, rectLosBndBox);
#elif losMethod == 2
	plotLos(rectImage, rectLosBndBox, rectLosLine);
#endif

	circle(rectImage, rectLosCnt, 3, CV_RGB(0, 0, 250), 3);
	circle(rectMosFrame, rectLosCnt, 3, CV_RGB(0, 0, 250), 3);

	plotScanLines(rectImage, scanRects, featureVec);
	plotScanLines(rectMosFrame, scanRects, featureVec);

	plotPath = "Results/Game" + gameIdStr + "/plots/vid" + vidIdxStr + dirStr + ".jpg";
	string mosPlotPath = "Results/Game" + gameIdStr + "/plots/vidMos" + vidIdxStr + dirStr + ".jpg";

	imwrite(plotPath, rectImage);
	imwrite(mosPlotPath, rectMosFrame);
}



void play::extractOdGridsFeatRectIndRsp(direction dir, vector<int> &featureVec,
		const vector<CvSize> &gridSizes, const vector<Point2i> &gridsNum)
{

	rectification();

	int d = 0;
	string dirStr;
	if(dir == leftDir)
	{
		d = -1;
		dirStr = "Left";
	}
	else if (dir == rightDir)
	{
		d = 1;
		dirStr = "Right";
	}
	else
		return;

	vector<rect> scanRects;

	for(unsigned int k = 0; k < gridSizes.size(); ++k)
	{

		vector<rect> scanRectsOneLevel;
		vector<int> featureVecLevel;

		for(int i = 0; i < gridsNum[k].x; ++i)
		{
			double maxYardLnXCoord, minYardLnXCoord;

			if(dir == rightDir)
			{
				maxYardLnXCoord = rectLosCnt.x + d * (i + 1) * gridSizes[k].width;
				minYardLnXCoord = rectLosCnt.x + d * i * gridSizes[k].width;
			}
			else
			{
				maxYardLnXCoord = rectLosCnt.x + d * i * gridSizes[k].width;
				minYardLnXCoord = rectLosCnt.x + d * (i + 1) * gridSizes[k].width;
			}

//			for(int j = -0.5 * gridsNum[k].y; j < gridsNum[k].y * 0.5; ++j)
			for(double j = -0.5 * gridsNum[k].y; j < gridsNum[k].y * 0.5; ++j)
			{
				int fgPixelsNum = 0;
				struct rect scanR;

	//			double lowY = rectLosCnt.y + j * (fld->fieldLength / 10.0);
	//			double upY = rectLosCnt.y + (j + 1) * (fld->fieldLength / 10.0);
				double lowY = rectLosCnt.y + j * gridSizes[k].height;
				double upY = rectLosCnt.y + (j + 1) * gridSizes[k].height;
	//			cout << "lowY: " << lowY << endl;
	//			cout << "upY: " << upY << endl;
				scanR.a = Point2d(minYardLnXCoord, lowY);
				scanR.b = Point2d(minYardLnXCoord, upY);
				scanR.c = Point2d(maxYardLnXCoord, upY);
				scanR.d = Point2d(maxYardLnXCoord, lowY);

				for(int y = lowY; y < upY; ++y)
					for(int x = minYardLnXCoord; x < maxYardLnXCoord; ++x)
					{
						if( y < 0 || y >= fld->fieldLength)
							continue;
						if( x < 0 || x >= fld->fieldWidth)
							continue;
						const Point3_<uchar>* p = rectImage.ptr<Point3_<uchar> >(y, x);
						if(int(p->z) == 255)
								++fgPixelsNum;
					}

				bool insideFld = false, visible = false;
				int indicator = 0;

				if(scanR.a.x >= 0 && scanR.a.x < fld->fieldWidth &&
						scanR.a.y >= 0 && scanR.a.y < fld->fieldLength)
					insideFld = true;
				if(scanR.b.x >= 0 && scanR.b.x < fld->fieldWidth &&
						scanR.b.y >= 0 && scanR.b.y < fld->fieldLength)
					insideFld = true;
				if(scanR.c.x >= 0 && scanR.c.x < fld->fieldWidth &&
						scanR.c.y >= 0 && scanR.c.y < fld->fieldLength)
					insideFld = true;
				if(scanR.d.x >= 0 && scanR.d.x < fld->fieldWidth &&
						scanR.d.y >= 0 && scanR.d.y < fld->fieldLength)
					insideFld = true;

				if(isPntInsideRect(scanR.a, imgBndRect) || isPntInsideRect(scanR.b, imgBndRect)
						|| isPntInsideRect(scanR.c, imgBndRect) || isPntInsideRect(scanR.d, imgBndRect))
//				if(isPntInsideRect(scanRCnt, imgBndRect))
					visible = true;

				if(!insideFld)
					indicator = 0;
				else if(!visible)
					indicator = 0;
				else
				{
					indicator = 1;
				}

				scanRects.push_back(scanR);
				featureVec.push_back(fgPixelsNum);

				scanRectsOneLevel.push_back(scanR);
				featureVecLevel.push_back(fgPixelsNum);

				featureVec.push_back(indicator);

//				scanRects.push_back(scanR);
//				featureVec.push_back(fgPixelsNum);
//
//				scanRectsOneLevel.push_back(scanR);
//				featureVecLevel.push_back(fgPixelsNum);

			}
		}
	}
}

void play::extOdStripsFeatFldCrdOrigImg(direction dir, vector<int> &featureVec)
{
	rectification();
	Mat homoMat;
	getOverheadFieldHomo(homoMat);

	int d = 0;
	string dirStr;
	if(dir == leftDir)
	{
		d = -1;
		dirStr = "Left";
	}
	else if (dir == rightDir)
	{
		d = 1;
		dirStr = "Right";
	}
	else
		return;

	vector<rect> scanLines;
	//vector<rect> scanLinesPerp;

//		cout << "pLos.x: " << pLos.x << endl;
	for(int i = 0; i < 5; ++i)
	{
		int fgPixelsNum = 0;
		struct rect rectLines;

		if(dir == rightDir)
		{
			rectLines.a = Point2d(rectLosCnt.x + d * i * fld->yardLinesDist, .0);
			rectLines.b = Point2d(rectLosCnt.x + d * i * fld->yardLinesDist, fld->fieldLength);
			rectLines.c = Point2d(rectLosCnt.x + d * (i + 1) * fld->yardLinesDist, fld->fieldLength);
			rectLines.d = Point2d(rectLosCnt.x + d * (i + 1) * fld->yardLinesDist, .0);
		}
		else
		{
			rectLines.a = Point2d(rectLosCnt.x + d * (i + 1) * fld->yardLinesDist, .0);
			rectLines.b = Point2d(rectLosCnt.x + d * (i + 1) * fld->yardLinesDist, fld->fieldLength);
			rectLines.c = Point2d(rectLosCnt.x + d * i * fld->yardLinesDist, fld->fieldLength);
			rectLines.d = Point2d(rectLosCnt.x + d * i * fld->yardLinesDist, .0);
		}

		vector<Point2d> srcLosVec, dstLosVec;
		srcLosVec.push_back(rectLines.a);
		srcLosVec.push_back(rectLines.b);
		srcLosVec.push_back(rectLines.c);
		srcLosVec.push_back(rectLines.d);
		perspectiveTransform(srcLosVec, dstLosVec, homoMat);
		rectLines.a = dstLosVec[0];
		rectLines.b = dstLosVec[1];
		rectLines.c = dstLosVec[2];
		rectLines.d = dstLosVec[3];

		for(int y = 0; y < imgYLen; ++y)
			for(int x = 0; x < imgXLen; ++x)
			{
				const Point3_<uchar>* p = fgImage.ptr<Point3_<uchar> >(y, x);
				if(int(p->z) == 255)
				{
					Point2d pnt = Point2d(x, y);//Point2d pnt = Point2d(x + 1, y + 1);
					if(isPntInsideTwoLines(pnt, rectLines))
						++fgPixelsNum;
				}
			}
		scanLines.push_back(rectLines);
		featureVec.push_back(fgPixelsNum);

	}

	imgRectfication imgRect(fldModType);
	imgRect.getFieldYardLines(yardLnsFldModel);
	for(unsigned int ln = 0; ln < yardLnsFldModel.size(); ++ln)
	{
		vector<Point2d> yardLnFld;
		perspectiveTransform(yardLnsFldModel[ln], yardLnFld, homoMat);
		yardLnsFldModel[ln] = yardLnFld;
//		yardLnsFldModel[ln][0] = yardLnFld[0];
//		yardLnsFldModel[ln][1] = yardLnFld[1];
	}
//	plotYardLnsAndLos(fgImage, losBndBox, yardLines);

#if losMethod == 1
	plotYardLnsAndLos(mosFrame, losBndBox, yardLnsFldModel);
	plotYardLnsAndLos(fgImage, losBndBox, yardLnsFldModel);
	//plotLos(fgImage, losBndBox);
#elif losMethod == 2
	plotYardLnsAndLos(fgImage, losBndBox, yardLnsFldModel, losLine);
	//plotLos(fgImage, losBndBox, losLine);
#endif

	circle(mosFrame, losCnt, 3, CV_RGB(0, 0, 250), 3);
	circle(fgImage, losCnt, 3, CV_RGB(0, 0, 250), 3);

	plotScanLines(mosFrame, scanLines, featureVec);
	plotScanLines(fgImage, scanLines, featureVec);

	plotPath = "Results/Game" + gameIdStr + "/plots/vid" + vidIdxStr + dirStr + ".jpg";
	string mosPlotPath = "Results/Game" + gameIdStr + "/plots/vidMos" + vidIdxStr + dirStr + ".jpg";

	imwrite(plotPath, fgImage);
	imwrite(mosPlotPath, mosFrame);

}

void play::extOdGridsFeatFldCrdOrigImg(direction dir, vector<int> &featureVec)
{
	rectification();
	Mat homoMat;
	getOverheadFieldHomo(homoMat);

	int d = 0;
	string dirStr;
	if(dir == leftDir)
	{
		d = -1;
		dirStr = "Left";
	}
	else if (dir == rightDir)
	{
		d = 1;
		dirStr = "Right";
	}
	else
		return;

	struct rect fieldRange;
	fieldRange.a = Point2d(fld->endZoneWidth, .0);
	fieldRange.b = Point2d(fld->endZoneWidth, fld->fieldLength);
	fieldRange.c = Point2d(fld->fieldWidth - fld->endZoneWidth, fld->fieldLength);
	fieldRange.d = Point2d(fld->fieldWidth - fld->endZoneWidth, .0);

	vector<Point2d> srcFRPnts, dstFRPnts;
	srcFRPnts.push_back(fieldRange.a);
	srcFRPnts.push_back(fieldRange.b);
	srcFRPnts.push_back(fieldRange.c);
	srcFRPnts.push_back(fieldRange.d);
	perspectiveTransform(srcFRPnts, dstFRPnts, homoMat);
	fieldRange.a = dstFRPnts[0];
	fieldRange.b = dstFRPnts[1];
	fieldRange.c = dstFRPnts[2];
	fieldRange.d = dstFRPnts[3];

	vector<rect> scanRects;

	for(int i = 0; i < 5; ++i)
	{
		double maxYardLnXCoord, minYardLnXCoord;

		if(dir == rightDir)
		{
			maxYardLnXCoord = rectLosCnt.x + d * (i + 1) * fld->yardLinesDist;
			minYardLnXCoord = rectLosCnt.x + d * i * fld->yardLinesDist;
		}
		else
		{
			maxYardLnXCoord = rectLosCnt.x + d * i * fld->yardLinesDist;
			minYardLnXCoord = rectLosCnt.x + d * (i + 1) * fld->yardLinesDist;
		}

		for(int j = -10; j < 10; ++j)
		{
			int fgPixelsNum = 0;
			struct rect scanR;

			double lowY = rectLosCnt.y + j * (fld->fieldLength / 10.0);
			double upY = rectLosCnt.y + (j + 1) * (fld->fieldLength / 10.0);
			scanR.a = Point2d(minYardLnXCoord, lowY);
			scanR.b = Point2d(minYardLnXCoord, upY);
			scanR.c = Point2d(maxYardLnXCoord, upY);
			scanR.d = Point2d(maxYardLnXCoord, lowY);

			vector<Point2d> srcLosVec, dstLosVec;
			srcLosVec.push_back(scanR.a);
			srcLosVec.push_back(scanR.b);
			srcLosVec.push_back(scanR.c);
			srcLosVec.push_back(scanR.d);
			perspectiveTransform(srcLosVec, dstLosVec, homoMat);
			scanR.a = dstLosVec[0];
			scanR.b = dstLosVec[1];
			scanR.c = dstLosVec[2];
			scanR.d = dstLosVec[3];

			for(int y = 0; y < imgYLen; ++y)
				for(int x = 0; x < imgXLen; ++x)
				{
//					if(outsideField)
//						break;
					if( y < 0 || y >= imgYLen)
						continue;
					if( x < 0 || x >= imgXLen)
						continue;
					const Point3_<uchar>* p = fgImage.ptr<Point3_<uchar> >(y, x);
					if(int(p->z) == 255)
					{
						Point2d pnt = Point2d(x, y);//Point2d pnt = Point2d(x + 1, y + 1);
						if(isPntInsideRect(pnt, scanR) && isPntInsideRect(pnt, fieldRange))
							++fgPixelsNum;
					}
				}

			scanRects.push_back(scanR);
			featureVec.push_back(fgPixelsNum);

		}
	}

	imgRectfication imgRect(fldModType);
	imgRect.getFieldYardLines(yardLnsFldModel);
	for(unsigned int ln = 0; ln < yardLnsFldModel.size(); ++ln)
	{
		vector<Point2d> yardLnFld;
		perspectiveTransform(yardLnsFldModel[ln], yardLnFld, homoMat);
		yardLnsFldModel[ln] = yardLnFld;
//		yardLnsFldModel[ln][0] = yardLnFld[0];
//		yardLnsFldModel[ln][1] = yardLnFld[1];
	}
//	plotYardLnsAndLos(fgImage, losBndBox, yardLines);

#if losMethod == 1
	plotYardLnsAndLos(fgImage, losBndBox, yardLnsFldModel);
	plotYardLnsAndLos(mosFrame, losBndBox, yardLnsFldModel);
	//plotLos(fgImage, losBndBox);
#elif losMethod == 2
	plotYardLnsAndLos(fgImage, losBndBox, yardLnsFldModel, losLine);
	//plotLos(fgImage, losBndBox, losLine);
#endif

	circle(fgImage, losCnt, 3, CV_RGB(0, 0, 250), 3);
	circle(mosFrame, losCnt, 3, CV_RGB(0, 0, 250), 3);

	plotScanLines(fgImage, scanRects, featureVec);
	plotScanLines(mosFrame, scanRects, featureVec);

	plotPath = "Results/Game" + gameIdStr + "/plots/vid" + vidIdxStr + dirStr + ".jpg";
	string mosPlotPath = "Results/Game" + gameIdStr + "/plots/vidMos" + vidIdxStr + dirStr + ".jpg";

	imwrite(plotPath, fgImage);
	imwrite(mosPlotPath, mosFrame);
}


void play::extOdGridsFeatFldCrdOrigImg(direction dir, vector<int> &featureVec,
		const vector<CvSize> &gridSizes, const vector<Point2i> &gridsNum, int expMode)
{

	Mat orgToFldHMat;
	rectification(orgToFldHMat);
	Mat fldToOrgHMat;
	getOverheadFieldHomo(fldToOrgHMat);

	int d = 0;
	string dirStr;
	if(dir == leftDir)
	{
		d = -1;
		dirStr = "Left";
	}
	else if (dir == rightDir)
	{
		d = 1;
		dirStr = "Right";
	}
	else
		return;

//	struct rect fieldRange;
//	fieldRange.a = Point2d(fld->endZoneWidth, .0);
//	fieldRange.b = Point2d(fld->endZoneWidth, fld->fieldLength);
//	fieldRange.c = Point2d(fld->fieldWidth - fld->endZoneWidth, fld->fieldLength);
//	fieldRange.d = Point2d(fld->fieldWidth - fld->endZoneWidth, .0);
////	fieldRange.a = Point2d(.0, .0);
////	fieldRange.b = Point2d(.0, fld->fieldLength);
////	fieldRange.c = Point2d(fld->fieldWidth, fld->fieldLength);
////	fieldRange.d = Point2d(fld->fieldWidth, .0);
//
//	vector<Point2d> srcFRPnts, dstFRPnts;
//	srcFRPnts.push_back(fieldRange.a);
//	srcFRPnts.push_back(fieldRange.b);
//	srcFRPnts.push_back(fieldRange.c);
//	srcFRPnts.push_back(fieldRange.d);
//	perspectiveTransform(srcFRPnts, dstFRPnts, fldToOrgHMat);
//	fieldRange.a = dstFRPnts[0];
//	fieldRange.b = dstFRPnts[1];
//	fieldRange.c = dstFRPnts[2];
//	fieldRange.d = dstFRPnts[3];

	vector<Point2d> srcLosVec, dstLosVec;
	srcLosVec.push_back(rectLosBndBox.a);
	srcLosVec.push_back(rectLosBndBox.b);
	srcLosVec.push_back(rectLosBndBox.c);
	srcLosVec.push_back(rectLosBndBox.d);
	srcLosVec.push_back(rectLosCnt);
	perspectiveTransform(srcLosVec, dstLosVec, fldToOrgHMat);
	losBndBox.a = dstLosVec[0];
	losBndBox.b = dstLosVec[1];
	losBndBox.c = dstLosVec[2];
	losBndBox.d = dstLosVec[3];
	losCnt = dstLosVec[4];

	imgRectfication imgRect(fldModType);
	imgRect.getFieldYardLines(yardLnsFldModel);
	for(unsigned int ln = 0; ln < yardLnsFldModel.size(); ++ln)
	{
		vector<Point2d> yardLnFld;
		perspectiveTransform(yardLnsFldModel[ln], yardLnFld, fldToOrgHMat);
		yardLnsFldModel[ln] = yardLnFld;
	}

	vector<Point2d> srcImgVec, dstImgVec;

	for(int y = 0; y < imgYLen; ++y)
		for(int x = 0; x < imgXLen; ++x)
			srcImgVec.push_back(Point2d(x, y));

	perspectiveTransform(srcImgVec, dstImgVec, orgToFldHMat);


	vector<rect> scanRects;

	for(unsigned int k = 0; k < gridSizes.size(); ++k)
	{

		vector<rect> scanRectsOneLevel;
		vector<int> featureVecLevel;

		//for(int i = 0; i < gridsNum[k].x; ++i)
		for(double i = 0; i < gridsNum[k].x; ++i)
		{
			double maxYardLnXCoord, minYardLnXCoord;

			if(dir == rightDir)
			{
				maxYardLnXCoord = rectLosCnt.x + d * (i + 1) * gridSizes[k].width;
				minYardLnXCoord = rectLosCnt.x + d * i * gridSizes[k].width;
			}
			else
			{
				maxYardLnXCoord = rectLosCnt.x + d * i * gridSizes[k].width;
				minYardLnXCoord = rectLosCnt.x + d * (i + 1) * gridSizes[k].width;
			}
			for(double j = -0.5 * gridsNum[k].y; j < gridsNum[k].y * 0.5; ++j)
			{
				int fgPixelsNum = 0;
				struct rect scanR, scanRField;
				double lowY = rectLosCnt.y + j * gridSizes[k].height;
				double upY = rectLosCnt.y + (j + 1) * gridSizes[k].height;
				scanRField.a = Point2d(minYardLnXCoord, lowY);
				scanRField.b = Point2d(minYardLnXCoord, upY);
				scanRField.c = Point2d(maxYardLnXCoord, upY);
				scanRField.d = Point2d(maxYardLnXCoord, lowY);

				vector<Point2d> srcScanRVec, dstScanRVec;
				srcScanRVec.push_back(scanRField.a);
				srcScanRVec.push_back(scanRField.b);
				srcScanRVec.push_back(scanRField.c);
				srcScanRVec.push_back(scanRField.d);
				perspectiveTransform(srcScanRVec, dstScanRVec, fldToOrgHMat);
				scanR.a = dstScanRVec[0];
				scanR.b = dstScanRVec[1];
				scanR.c = dstScanRVec[2];
				scanR.d = dstScanRVec[3];

	//			expMode = 1;
				if(expMode)
				{
					bool insideFld = false, visible = false;

					if(scanRField.a.x >= 0 && scanRField.a.x < fld->fieldWidth &&
							scanRField.a.y >= 0 && scanRField.a.y < fld->fieldLength)
						insideFld = true;
					if(scanRField.b.x >= 0 && scanRField.b.x < fld->fieldWidth &&
							scanRField.b.y >= 0 && scanRField.b.y < fld->fieldLength)
						insideFld = true;
					if(scanRField.c.x >= 0 && scanRField.c.x < fld->fieldWidth &&
							scanRField.c.y >= 0 && scanRField.c.y < fld->fieldLength)
						insideFld = true;
					if(scanRField.d.x >= 0 && scanRField.d.x < fld->fieldWidth &&
							scanRField.d.y >= 0 && scanRField.d.y < fld->fieldLength)
						insideFld = true;

					if(isPntInsideRect(scanRField.a, imgBndRect) || isPntInsideRect(scanRField.b, imgBndRect)
							|| isPntInsideRect(scanRField.c, imgBndRect) || isPntInsideRect(scanRField.d, imgBndRect))
						visible = true;

					if(!insideFld)
						fgPixelsNum = 0;
					else if(!visible)
						fgPixelsNum = -1;
					else
					{
						int imgPntsIdx = 0;
						for(int y = 0; y < imgYLen; ++y)
							for(int x = 0; x < imgXLen; ++x)
							{
								double fldX = dstImgVec[imgPntsIdx].x;
								double fldY = dstImgVec[imgPntsIdx].y;
//								dstVec.erase(dstVec.begin());
								++imgPntsIdx;
								if( fldY < 0 || fldY >= fld->fieldLength)
									continue;
								if( fldX < 0 || fldX >= fld->fieldWidth)
									continue;

								const Point3_<uchar>* p = fgImage.ptr<Point3_<uchar> >(y, x);
								if(int(p->z) == 255)
								{
									Point2d pnt = Point2d(x, y);//Point2d pnt = Point2d(x + 1, y + 1);
									if(isPntInsideRect(pnt, scanR))// && isPntInsideRect(pnt, fieldRange))
										++fgPixelsNum;
								}
							}
					}
				}
				else
				{
					int imgPntsIdx = 0;
					for(int y = 0; y < imgYLen; ++y)
						for(int x = 0; x < imgXLen; ++x)
						{
							double fldX = dstImgVec[imgPntsIdx].x;
							double fldY = dstImgVec[imgPntsIdx].y;
//								dstVec.erase(dstVec.begin());
							++imgPntsIdx;
							if( fldY < 0 || fldY >= fld->fieldLength)
								continue;
							if( fldX < 0 || fldX >= fld->fieldWidth)
								continue;

							const Point3_<uchar>* p = fgImage.ptr<Point3_<uchar> >(y, x);
							if(int(p->z) == 255)
							{
								Point2d pnt = Point2d(x, y);//Point2d pnt = Point2d(x + 1, y + 1);
								if(isPntInsideRect(pnt, scanR))// && isPntInsideRect(pnt, fieldRange))
									++fgPixelsNum;
							}
						}

				}


				scanRects.push_back(scanR);
				featureVec.push_back(fgPixelsNum);

				scanRectsOneLevel.push_back(scanR);
				featureVecLevel.push_back(fgPixelsNum);

			}
		}

	Mat fgImageLevel, mosImageLevel;
	fgImage.copyTo(fgImageLevel);
	mosFrame.copyTo(mosImageLevel);

#if losMethod == 1
	plotYardLnsAndLos(fgImageLevel, losBndBox, yardLnsFldModel);
	plotYardLnsAndLos(mosImageLevel, losBndBox, yardLnsFldModel);
	//plotLos(fgImage, losBndBox);
#elif losMethod == 2
	plotYardLnsAndLos(fgImage, losBndBox, yardLnsFldModel, losLine);
	//plotLos(fgImage, losBndBox, losLine);
#endif

	circle(fgImageLevel, losCnt, 3, CV_RGB(0, 0, 250), 3);
	circle(mosImageLevel, losCnt, 3, CV_RGB(0, 0, 250), 3);

	plotScanLines(fgImageLevel, scanRectsOneLevel, featureVecLevel);
	plotScanLines(mosImageLevel, scanRectsOneLevel, featureVecLevel);

	ostringstream convertLevel;
	convertLevel << k;
	string levelStr = convertLevel.str();

	string fgPlotPath = "Results/Game" + gameIdStr + "/plots/vid" + vidIdxStr + dirStr + levelStr + ".jpg";
	string mosPlotPath = "Results/Game" + gameIdStr + "/plots/vidMos" + vidIdxStr + dirStr + levelStr +".jpg";

	imwrite(fgPlotPath, fgImageLevel);
	imwrite(mosPlotPath, mosImageLevel);
	}

#if losMethod == 1
	plotYardLnsAndLos(fgImage, losBndBox, yardLnsFldModel);
	plotYardLnsAndLos(mosFrame, losBndBox, yardLnsFldModel);
	//plotLos(fgImage, losBndBox);
#elif losMethod == 2
	plotYardLnsAndLos(fgImage, losBndBox, yardLnsFldModel, losLine);
	//plotLos(fgImage, losBndBox, losLine);
#endif

	circle(fgImage, losCnt, 3, CV_RGB(0, 0, 250), 3);
	circle(mosFrame, losCnt, 3, CV_RGB(0, 0, 250), 3);

	plotScanLines(fgImage, scanRects, featureVec);
	plotScanLines(mosFrame, scanRects, featureVec);

	plotPath = "Results/Game" + gameIdStr + "/plots/vid" + vidIdxStr + dirStr + ".jpg";
	string mosPlotPath = "Results/Game" + gameIdStr + "/plots/vidMos" + vidIdxStr + dirStr + ".jpg";

	imwrite(plotPath, fgImage);
	imwrite(mosPlotPath, mosFrame);
}



void play::detectEllipsesFromFg(vector<RotatedRect> &ellipses)
{
	string blobOutputFile = "blobs/Game" + gameIdStr + "/video0" + vidIdxStr +".blob";
	string blobOutputImg = "blobs/Game" + gameIdStr + "/video0" + vidIdxStr +".bmp";
	blob(fgImgPath, blobOutputFile, blobOutputImg);

	string eliOutputFile = "ellipses/Game" + gameIdStr + "/video0" + vidIdxStr +".ellipse";
	ellipse(blobOutputImg, eliOutputFile, ellipses);
}

void play::rectification()
{
	imgRectfication imgRect(fldModType);

//	string matchesFile = "imgRectMatches/Game" + gameIdStr + "/vid" + vidIdxStr + "RectMatches";
	string matchesFile = "imgRectMatches/Game" + gameIdStr + "/vid" + vidIdxStr + "RectMatchesNew";
	Mat dstImg, homoMat;
	//rectify mos frame
#if predMos == 1

	imgRect.rectifyImageToField(matchesFile, mosFrame, dstImg, homoMat);
	//rectfy foreground fgImage
	imgRect.rectifyImageToField(matchesFile, fgImage, rectImage, homoMat);

#elif predMos == 0
	imgRect.rectifyImageToField(matchesFile, mosFrame, dstImg, homoMat);
	//rectfy foreground fgImage
	imgRect.rectifyImageToField(matchesFile, fgImage, rectImage, homoMat);
#endif

	rectMosFrame.create(fld->fieldLength, fld->fieldWidth, CV_32FC3);
	dstImg.copyTo(rectMosFrame);


#if rectLosMethod == 1
	vector<Point2d> srcLosVec, dstLosVec;
	srcLosVec.push_back(losBndBox.a);
	srcLosVec.push_back(losBndBox.b);
	srcLosVec.push_back(losBndBox.c);
	srcLosVec.push_back(losBndBox.d);
	srcLosVec.push_back(losCnt);
	perspectiveTransform(srcLosVec, dstLosVec, homoMat);
	rectLosBndBox.a = dstLosVec[0];
	rectLosBndBox.b = dstLosVec[1];
	rectLosBndBox.c = dstLosVec[2];
	rectLosBndBox.d = dstLosVec[3];
	rectLosCnt = dstLosVec[4];
#elif rectLosMethod == 2
	findLosOnRectFg(homoMat);

#endif

	//plotLos(dstImg, rectLosBndBox);

	//homoTransPoint(losCnt, homoMat, dstscrimCnt);
	//circle(dstImg, rectLosCnt, 3, CV_RGB(0, 0, 250), 3);

#if losMethod == 2
	vector<Point2d> srcRectLosVec, dstRectLosVec;
	srcRectLosVec.push_back(losLine[0]);
	srcRectLosVec.push_back(losLine[1]);
	perspectiveTransform(srcRectLosVec, dstRectLosVec, homoMat);
	rectLosLine[0] = dstRectLosVec[0];
	rectLosLine[1] = dstRectLosVec[1];
#endif
	Mat rectFgImgTmp;
	rectImage.copyTo(rectFgImgTmp);

#if losMethod == 1
	plotLos(dstImg, rectLosBndBox);
	plotLos(rectFgImgTmp, rectLosBndBox);
#elif losMethod == 2
	plotLos(dstImg, rectLosBndBox, rectLosLine);
#endif

	circle(dstImg, rectLosCnt, 3, CV_RGB(0, 0, 250), 3);
	circle(rectFgImgTmp, rectLosCnt, 3, CV_RGB(0, 0, 250), 3);

	string dstImgPath = "rectImages/Game" + gameIdStr + "/vid" + vidIdxStr + "Rect.jpg";
	imwrite(dstImgPath, dstImg);
	string fgImgPath = "rectImages/Game" + gameIdStr + "/vid" + vidIdxStr + "RectFg.jpg";
	imwrite(fgImgPath, rectFgImgTmp);
}

void play::rectification(Mat& orgToFldHMat)
{
	imgRectfication imgRect(fldModType);
//	string matchesFile = "imgRectMatches/Game" + gameIdStr + "/vid" + vidIdxStr + "RectMatches";
	string matchesFile = "imgRectMatches/Game" + gameIdStr + "/vid" + vidIdxStr + "RectMatchesNew";
	Mat dstImg;
	//rectify mos frame
#if predMos == 1

	imgRect.rectifyImageToField(matchesFile, mosFrame, dstImg, orgToFldHMat);
	//rectfy foreground fgImage
	imgRect.rectifyImageToField(matchesFile, fgImage, rectImage, orgToFldHMat);

#elif predMos == 0
	imgRect.rectifyImageToField(matchesFile, mosFrame, dstImg, orgToFldHMat);
	//rectfy foreground fgImage
	imgRect.rectifyImageToField(matchesFile, fgImage, rectImage, orgToFldHMat);
#endif

	rectMosFrame.create(fld->fieldLength, fld->fieldWidth, CV_32FC3);
	dstImg.copyTo(rectMosFrame);

#if rectLosMethod == 1
	vector<Point2d> srcLosVec, dstLosVec;
	srcLosVec.push_back(losBndBox.a);
	srcLosVec.push_back(losBndBox.b);
	srcLosVec.push_back(losBndBox.c);
	srcLosVec.push_back(losBndBox.d);
	srcLosVec.push_back(losCnt);
	perspectiveTransform(srcLosVec, dstLosVec, orgToFldHMat);
	rectLosBndBox.a = dstLosVec[0];
	rectLosBndBox.b = dstLosVec[1];
	rectLosBndBox.c = dstLosVec[2];
	rectLosBndBox.d = dstLosVec[3];
	rectLosCnt = dstLosVec[4];
#elif rectLosMethod == 2
	findLosOnRectFg(orgToFldHMat);
#endif

#if losMethod == 2
	vector<Point2d> srcRectLosVec, dstRectLosVec;
	srcRectLosVec.push_back(losLine[0]);
	srcRectLosVec.push_back(losLine[1]);
	perspectiveTransform(srcRectLosVec, dstRectLosVec, orgToFldHMat);
	rectLosLine[0] = dstRectLosVec[0];
	rectLosLine[1] = dstRectLosVec[1];
#endif
	Mat rectFgImgTmp;
	rectImage.copyTo(rectFgImgTmp);

#if losMethod == 1
	plotLos(dstImg, rectLosBndBox);
	plotLos(rectFgImgTmp, rectLosBndBox);
#elif losMethod == 2
	plotLos(dstImg, rectLosBndBox, rectLosLine);
#endif

	circle(dstImg, rectLosCnt, 3, CV_RGB(0, 0, 250), 3);
	circle(rectFgImgTmp, rectLosCnt, 3, CV_RGB(0, 0, 250), 3);

	string dstImgPath = "rectImages/Game" + gameIdStr + "/vid" + vidIdxStr + "Rect.jpg";
	imwrite(dstImgPath, dstImg);
//	string fgImgPath = "rectImages/Game" + gameIdStr + "/vid" + vidIdxStr + "RectFg.jpg";
//	imwrite(fgImgPath, rectFgImgTmp);
}
void play::rctfWithoutDetectLos(Mat& orgToFldHMat)
{
	imgRectfication imgRect(fldModType);
//	string matchesFile = "imgRectMatches/Game" + gameIdStr + "/vid" + vidIdxStr + "RectMatches";
	string matchesFile = "imgRectMatches/Game" + gameIdStr + "/vid" + vidIdxStr + "RectMatchesNew";
	Mat dstImg;
	//rectify mos frame
#if predMos == 1

	imgRect.rectifyImageToField(matchesFile, mosFrame, dstImg, orgToFldHMat);
	imgRect.rectifyImageToField(matchesFile, fgImage, rectImage, orgToFldHMat);

#elif predMos == 0
	imgRect.rectifyImageToField(matchesFile, mosFrame, dstImg, orgToFldHMat);
	imgRect.rectifyImageToField(matchesFile, fgImage, rectImage, orgToFldHMat);
#endif

	rectMosFrame.create(fld->fieldLength, fld->fieldWidth, CV_32FC3);
	dstImg.copyTo(rectMosFrame);

//	string dstImgPath = "rectImages/Game" + gameIdStr + "/vid" + vidIdxStr + "Rect.jpg";
//	imwrite(dstImgPath, dstImg);
//	string fgImgPath = "rectImages/Game" + gameIdStr + "/vid" + vidIdxStr + "RectFg.jpg";
//	imwrite(fgImgPath, rectImage);

}
void play::writeMatchPnts()
{
	string matchesFile = "imgRectMatches/Game" + gameIdStr + "/vid" + vidIdxStr + "RectMatches";
	vector<Point2f> srcPoints, dstPoints;
	imgRectfication imgRect(fldModType);
	imgRect.readMatches(matchesFile, srcPoints, dstPoints);
	string matchePntsPath = "imgRectMatches/Game" + gameIdStr + "/vid" + vidIdxStr + "RectMatchesNew";
	ofstream fout(matchePntsPath.c_str());
	for(unsigned int i = 0; i < srcPoints.size(); ++i)
		fout << dstPoints[i].x << " " << dstPoints[i].y
		<< " "<< srcPoints[i].x << " " << srcPoints[i].y << endl;
	fout.close();
}

void play::getOverheadFieldHomo(Mat &homoMat)
{
//	string matchesFile = "imgRectMatches/Game" + gameIdStr + "/vid" + vidIdxStr + "RectMatches";
	string matchesFile = "imgRectMatches/Game" + gameIdStr + "/vid" + vidIdxStr + "RectMatchesNew";
	Mat dstImg;
	imgRectfication imgRect(fldModType);
	imgRect.transFieldToImage(matchesFile, dstImg, homoMat);

//	Mat blendImg, mosFCvtImg;
//	mosFrame.convertTo(mosFCvtImg, CV_32FC3);
//	addWeighted(mosFCvtImg, 0.5, dstImg, 0.5, 0.0, blendImg);
//	string dstImgPath = "rectImages/Game" + gameIdStr + "/vid" + vidIdxStr + "FldCoord.jpg";
//
//	imwrite(dstImgPath, blendImg);

}

void play::detectOnePlayerTypePosRect(playerTypeId pTypeId, direction offSide)
{
	playerType pType(pTypeId);
	struct rectHorzVertRng rng;
//	pType.getSearchRng(imgBndRect, rectLosCnt, offSide, rng);
	pType.getSearchRng(fldModType, rectLosCnt, offSide, rng);
	Point2d modelVec;
	pType.getModelVec(rectLosCnt, fldModType, offSide, modelVec);
	double finalScore = NEGINF;
	struct rect playerPos;
	for(double xCnt = rng.xMin; xCnt <= rng.xMax; xCnt += 5.0)
		for(double yCnt = rng.yMin; yCnt <= rng.yMax; yCnt += 5.0)
		{
			if(!isPntInsideRect(Point2d(xCnt, yCnt), imgBndRect))
				continue;
			int xMin = int(xCnt - fld->yardLinesDist * 0.25);
			int xMax = int(xCnt + fld->yardLinesDist * 0.25);
			int yMin = int(yCnt - fld->yardLinesDist * 0.5);
			int yMax = int(yCnt + fld->yardLinesDist * 0.5);
			int fgPixelsNum = 0;
			for(int y = yMin; y <= yMax; ++y)
					for(int x = xMin; x <= xMax; ++x)
					{
						const Point3_<uchar>* p = rectImage.ptr<Point3_<uchar> >(y, x);
						if(int(p->z) == 255)
								++fgPixelsNum;
					}
//			double appScore = (double)fgPixelsNum / (double)(fld->yardLinesDist * fld->yardLinesDist);
			double appScore = (double)fgPixelsNum / (double)((xMax - xMin) * (yMax - yMin));
//			if(fgPixelsNum)
//			{
//				cout << fgPixelsNum << endl;
//				cout << appScore << endl;
//				cout << fld->yardLinesDist * fld->yardLinesDist << endl;
//			}
//			appScore = .0;
			if(appScore < 0.3)
				appScore = NEGINF;
			else
				appScore = 1.0;
			Point2d cnt(xCnt, yCnt);
			Point2d matchVec = cnt - rectLosCnt;
			double spaScore = -1.0 * norm(matchVec - modelVec) / norm(modelVec);
			double totalScore = appScore + spaScore;
			//cout << totalScore << endl;
			if(totalScore >= finalScore)
			{
				finalScore = totalScore;
				playerPos.a = Point2d(xMin, yMin);
				playerPos.b = Point2d(xMin, yMax);
				playerPos.c = Point2d(xMax, yMax);
				playerPos.d = Point2d(xMax, yMin);
			}
		}
	plotPlayerPosBox(rectMosFrame, playerPos, pType.pTypeStr);
	plotPlayerPosBox(rectImage, playerPos, pType.pTypeStr);
//	RNG rand(12345);
//	Scalar color = Scalar(rand.uniform(0, 255), rand.uniform(0,255), rand.uniform(0,255));
	Scalar color(0, 0, 250);
//	struct rect rngRect;
//	rngRect.a = Point2d(rng.xMin, rng.yMin);
//	rngRect.b = Point2d(rng.xMin, rng.yMax);
//	rngRect.c = Point2d(rng.xMax, rng.yMax);
//	rngRect.d = Point2d(rng.xMax, rng.yMin);
//	plotRect(rectMosFrame, rngRect, color);
//	plotRect(rectImage, rngRect, color);
	line(rectMosFrame, rectLosCnt, rectLosCnt + modelVec, color, 2, 8, 0);
	line(rectImage, rectLosCnt, rectLosCnt + modelVec, color, 2, 8, 0);
}
void play::detectPlayerTypesPosRect(const vector<playerTypeId> &pTypeIds, direction offSide)
{
	rectification();

	for(unsigned int i = 0; i < pTypeIds.size(); ++i)
		detectOnePlayerTypePosRect(pTypeIds[i], offSide);

#if losMethod == 1
	plotLos(rectImage, rectLosBndBox);
	plotLos(rectMosFrame, rectLosBndBox);
#elif losMethod == 2
	plotLos(rectImage, rectLosBndBox, rectLosLine);
#endif

	circle(rectImage, rectLosCnt, 3, CV_RGB(0, 0, 250), 3);
	circle(rectMosFrame, rectLosCnt, 3, CV_RGB(0, 0, 250), 3);


	string fgPlotPath = "formResults/Game" + gameIdStr + "/vid" + vidIdxStr + "FormFg.jpg";
	string mosPlotPath = "formResults/Game" + gameIdStr + "/vid" + vidIdxStr + "Form.jpg";

	imwrite(fgPlotPath, rectImage);
	imwrite(mosPlotPath, rectMosFrame);
}


void play::detectOnePlayerTypePosOrig(playerTypeId pTypeId, direction offSide, const Mat &fldToOrgHMat)
{
	playerType pType(pTypeId);
	struct rectHorzVertRng rng;
//	pType.getSearchRng(imgBndRect, rectLosCnt, offSide, rng);
	pType.getSearchRng(fldModType, rectLosCnt, offSide, rng);
	Point2d modelVec;
	pType.getModelVec(rectLosCnt, fldModType, offSide, modelVec);
	double finalScore = NEGINF;
	struct rect playerPos;
	for(double xCnt = rng.xMin; xCnt <= rng.xMax; xCnt += 5.0)
		for(double yCnt = rng.yMin; yCnt <= rng.yMax; yCnt += 5.0)
		{
			//cout << "#" << endl;
			if(!isPntInsideRect(Point2d(xCnt, yCnt), imgBndRect))
				continue;
			int xMin = int(xCnt - fld->yardLinesDist * 0.25);
			int xMax = int(xCnt + fld->yardLinesDist * 0.25);
			int yMin = int(yCnt - fld->yardLinesDist * 0.5);
			int yMax = int(yCnt + fld->yardLinesDist * 0.5);
			struct rect scanRField, scanROrig;
			scanRField.a = Point2d(xMin, yMin);
			scanRField.b = Point2d(xMin, yMax);
			scanRField.c = Point2d(xMax, yMax);
			scanRField.d = Point2d(xMax, yMin);

			vector<Point2d> srcScanRVec, dstScanRVec;
			srcScanRVec.push_back(scanRField.a);
			srcScanRVec.push_back(scanRField.b);
			srcScanRVec.push_back(scanRField.c);
			srcScanRVec.push_back(scanRField.d);
			perspectiveTransform(srcScanRVec, dstScanRVec, fldToOrgHMat);
			scanROrig.a = dstScanRVec[0];
			scanROrig.b = dstScanRVec[1];
			scanROrig.c = dstScanRVec[2];
			scanROrig.d = dstScanRVec[3];

			int fgPixelsNum = 0;

			for(int y = 0; y < imgYLen; ++y)
				for(int x = 0; x < imgXLen; ++x)
				{
					const Point3_<uchar>* p = fgImage.ptr<Point3_<uchar> >(y, x);
					if(int(p->z) == 255)
					{
						Point2d pnt = Point2d(x, y);
						if(isPntInsideRect(pnt, scanROrig))
							++fgPixelsNum;
					}
				}

			double appScore = (double)fgPixelsNum / (double)((xMax - xMin) * (yMax - yMin));
//			if(fgPixelsNum)
//			{
//				cout << fgPixelsNum << endl;
//				cout << appScore << endl;
//				cout << fld->yardLinesDist * fld->yardLinesDist << endl;
//			}
//			appScore = .0;
			if(appScore < 0.3)
				appScore = NEGINF;
			else
				appScore = 1.0;
			Point2d cnt(xCnt, yCnt);
			Point2d matchVec = cnt - rectLosCnt;
			double spaScore = -1.0 * norm(matchVec - modelVec) / norm(modelVec);
			double totalScore = appScore + spaScore;
			//cout << totalScore << endl;
			if(totalScore >= finalScore)
			{
				finalScore = totalScore;
				playerPos = scanROrig;
			}
		}
	plotPlayerPosBox(mosFrame, playerPos, pType.pTypeStr);
	plotPlayerPosBox(fgImage, playerPos, pType.pTypeStr);
//	RNG rand(12345);
//	Scalar color = Scalar(rand.uniform(0, 255), rand.uniform(0,255), rand.uniform(0,255));
	Scalar color(0, 0, 250);
//	struct rect rngRect;
//	rngRect.a = Point2d(rng.xMin, rng.yMin);
//	rngRect.b = Point2d(rng.xMin, rng.yMax);
//	rngRect.c = Point2d(rng.xMax, rng.yMax);
//	rngRect.d = Point2d(rng.xMax, rng.yMin);
//	plotRect(rectMosFrame, rngRect, color);
//	plotRect(rectImage, rngRect, color);

	vector<Point2d> srcModelVec, dstModelVec;
	srcModelVec.push_back(rectLosCnt + modelVec);
	perspectiveTransform(srcModelVec, dstModelVec, fldToOrgHMat);
	Point2d transModelVec = dstModelVec[0];

	line(mosFrame, losCnt, transModelVec, color, 2, 8, 0);
	line(fgImage, losCnt, transModelVec, color, 2, 8, 0);
}
void play::detectPlayerTypesPosOrig(const vector<playerTypeId> &pTypeIds, direction offSide)
{
	rectification();
	Mat fldToOrgHMat;
	getOverheadFieldHomo(fldToOrgHMat);

	vector<Point2d> srcLosVec, dstLosVec;
	srcLosVec.push_back(rectLosBndBox.a);
	srcLosVec.push_back(rectLosBndBox.b);
	srcLosVec.push_back(rectLosBndBox.c);
	srcLosVec.push_back(rectLosBndBox.d);
	srcLosVec.push_back(rectLosCnt);
	perspectiveTransform(srcLosVec, dstLosVec, fldToOrgHMat);
	losBndBox.a = dstLosVec[0];
	losBndBox.b = dstLosVec[1];
	losBndBox.c = dstLosVec[2];
	losBndBox.d = dstLosVec[3];
	losCnt = dstLosVec[4] ;

	for(unsigned int i = 0; i < pTypeIds.size(); ++i)
		detectOnePlayerTypePosOrig(pTypeIds[i], offSide, fldToOrgHMat);

#if losMethod == 1
	plotLos(fgImage, losBndBox);
	plotLos(mosFrame, losBndBox);
#elif losMethod == 2
	plotLos(fgImage, rectLosBndBox, rectLosLine);
#endif

	circle(fgImage, losCnt, 3, CV_RGB(0, 0, 250), 3);
	circle(mosFrame, losCnt, 3, CV_RGB(0, 0, 250), 3);


	string fgPlotPath = "formResults/Game" + gameIdStr + "/vid" + vidIdxStr + "FormFg.jpg";
	string mosPlotPath = "formResults/Game" + gameIdStr + "/vid" + vidIdxStr + "Form.jpg";

	imwrite(fgPlotPath, fgImage);
	imwrite(mosPlotPath, mosFrame);
}

void play::genRectMosFrmFgBgSub()
{
	imgRectfication imgRect(fldModType);
	string matchesFile = "imgRectMatches/Game" + gameIdStr + "/vid" + vidIdxStr + "RectMatchesNew";
	Mat dstImg, homoMat;
	imgRect.rectifyImageToField(matchesFile, mosFrame, dstImg, homoMat);
	rectMosFrame.create(fld->fieldLength, fld->fieldWidth, CV_32FC3);
	dstImg.copyTo(rectMosFrame);

	string rectPanoPath = "panorama/rectPanoGame" + gameIdStr + ".jpg";
	Mat rectPano = imread(rectPanoPath.c_str(), CV_LOAD_IMAGE_COLOR);
//	Mat fgImg = dstImg - rectPano;

	Mat dstGrayImg;
	cvtColor(dstImg, dstGrayImg, CV_RGB2GRAY);
	Mat rectPanoGrayImg;
	cvtColor(rectPano, rectPanoGrayImg, CV_RGB2GRAY);

	Mat fgImg = dstGrayImg - rectPanoGrayImg;

	Scalar m = mean(fgImg);
//	cout << m << endl;
//	cout << fgImg.at<float>(100, 100) << endl;
//	fgImg = fgImg - m;
//	cout << fgImg.at<float>(100, 100) << endl;
	Mat dstFgImg;
	double threshold_value = m.val[0] * 10;
//	cout << threshold_value << endl;
	double max_BINARY_value = 255;
	threshold(fgImg, dstFgImg, threshold_value, max_BINARY_value, THRESH_BINARY);
	fgImg = dstFgImg;

	string fgImgPath = "fgRectMosImagesPano/Game" + gameIdStr + "/video0" + vidIdxStr + ".jpg";
	imwrite(fgImgPath, fgImg);

	//cout << m << endl;

//	Mat fgGrayImg;
//	cvtColor(fgImg, fgGrayImg, CV_RGB2GRAY);
//	string fgGrayImgPath = "fgRectMosImagesPano/Game" + gameIdStr + "/video0" + vidIdxStr + "Gray.jpg";
//	imwrite(fgGrayImgPath, fgGrayImg);
}

void play::genOrigMosFrmFgBgSub()
{
	string rectPanoPath = "panorama/rectPanoGame" + gameIdStr + ".jpg";
	Mat rectPano = imread(rectPanoPath.c_str(), CV_LOAD_IMAGE_COLOR);

	string matchesFile = "imgRectMatches/Game" + gameIdStr + "/vid" + vidIdxStr + "RectMatchesNew";
	imgRectfication imgRect(fldModType);
	Mat bgOrig, homoMat;
	imgRect.transFieldToImage(matchesFile, rectPano, bgOrig, homoMat);

//	string panoPath = "panorama/panoGame" + gameIdStr + ".jpg";
//	Mat pano = imread(panoPath.c_str(), CV_LOAD_IMAGE_COLOR);
//	string homMatPath = "homog/Game" + gameIdStr + "/" + vidIdxStr + ".txt";
//	vector<Mat> homoList = readHomographs(homMatPath);
//	Mat bgOrig;
//	warpPerspective(pano, bgOrig, homoList[mos - 1].inv(), mosFrame.size());

	//Mat fgImg = mosFrame - bgOrig;

	Mat mosGrayFrm;
	cvtColor(mosFrame, mosGrayFrm, CV_RGB2GRAY);
	Mat bgOrigGray;
	cvtColor(bgOrig, bgOrigGray, CV_RGB2GRAY);

	Mat mosFrmEdge, bgOrigEdge;
	Canny(mosGrayFrm, mosFrmEdge, 50, 200, 3);
	Canny(bgOrigGray, bgOrigEdge, 50, 200, 3);

//	Mat fgImg = subtractEdgeImg(mosFrmEdge, bgOrigEdge);
//	Mat fgImg = mosFrmEdge - bgOrigEdge;

//	Mat fgImg = mosGrayFrm - bgOrigGray;

//	Scalar m = mean(fgImg);
//	Mat dstFgImg;
//	double threshold_value = m.val[0] * 3.0;
//	double max_BINARY_value = 255;
//	threshold(fgImg, dstFgImg, threshold_value, max_BINARY_value, THRESH_BINARY);
//	fgImg = dstFgImg;

	Mat fgImg = mosFrmEdge;
//	fgImg = mosFrame - bgOrig;

//	string fgImagePath = "fgMosImagesPano/Game" + gameIdStr + "/video0" + vidIdxStr + "Bg.jpg";
	string fgImagePath = "fgMosImagesPano/Game" + gameIdStr + "/video0" + vidIdxStr + ".jpg";
	imwrite(fgImagePath, fgImg);
}

void play::getBgImg()
{
	string rectPanoPath = "panorama/rectPanoGame" + gameIdStr + ".jpg";
	Mat rectPano = imread(rectPanoPath.c_str(), CV_LOAD_IMAGE_COLOR);

	string matchesFile = "imgRectMatches/Game" + gameIdStr + "/vid" + vidIdxStr + "RectMatchesNew";
	imgRectfication imgRect(fldModType);
	Mat bgOrig, homoMat;
	imgRect.transFieldToImage(matchesFile, rectPano, bgOrig, homoMat);

	Mat bgOrigGray;
	cvtColor(bgOrig, bgOrigGray, CV_RGB2GRAY);

	Mat bgOrigEdge;
	Canny(bgOrigGray, bgOrigEdge, 50, 200, 3);

	string bgImagePath = "bgMosImagesPano/Game" + gameIdStr + "/" + gameIdStr + "1" +  vidIdxStr + ".jpg";
//	imwrite(bgImagePath, bgOrig);
	Mat bgOrigEdgeClr;
	cvtColor(bgOrigEdge,bgOrigEdgeClr, COLOR_GRAY2RGB);
	imwrite(bgImagePath, bgOrigEdgeClr);

}

void play::cutAreaOutsideFld()
{
//	Mat orgToFldHMat;
//	rectification(orgToFldHMat);
//
//	vector<Point2d> srcImgVec, dstImgVec;
//
//	for(int y = 0; y < imgYLen; ++y)
//		for(int x = 0; x < imgXLen; ++x)
//			srcImgVec.push_back(Point2d(x, y));
//
//	perspectiveTransform(srcImgVec, dstImgVec, orgToFldHMat);
//
//	int imgPntsIdx = 0;
//	for(int y = 0; y < imgYLen; ++y)
//		for(int x = 0; x < imgXLen; ++x)
//		{
//			double fldX = dstImgVec[imgPntsIdx].x;
//			double fldY = dstImgVec[imgPntsIdx].y;
//			++imgPntsIdx;
//			bool outside = false;
//			if( fldY < 0 || fldY >= fld->fieldLength)
//				outside = true;
//			if( fldX < 0 || fldX >= fld->fieldWidth)
//				outside = true;
//
//			if(outside)
//			{
//				mosFrame.at<cv::Vec3b>(y,x)[0] = 0;
//				mosFrame.at<cv::Vec3b>(y,x)[1] = 0;
//				mosFrame.at<cv::Vec3b>(y,x)[2] = 0;
//			}
////			Point3_<uchar>* p = fgImage.ptr<Point3_<uchar> >(y, x);
////			p->x = 0;
////			p->y = 0;
////			p->z = 0;
//		}
//
//	Mat mosGrayFrm;
//	cvtColor(mosFrame, mosGrayFrm, CV_RGB2GRAY);
//
//	Mat mosFrmEdge;
//	Canny(mosGrayFrm, mosFrmEdge, 50, 200, 3);

	string mosImagePath = "mosImagesCut/Game" + gameIdStr + "/" + gameIdStr +"0" + vidIdxStr + ".jpg";
	imwrite(mosImagePath, mosFrame);

//	Mat mosFrmEdgeClr;
//	cvtColor(mosFrmEdge, mosFrmEdgeClr, COLOR_GRAY2RGB);
//	imwrite(mosImagePath, mosFrmEdgeClr);

}


void play::drawPlayerBndBoxes()
{
	Mat orgToFldHMat;
	rectification(orgToFldHMat);
	Mat fldToOrgHMat;
	getOverheadFieldHomo(fldToOrgHMat);
	vector<Point2d> srcLosVec, dstLosVec;
	srcLosVec.push_back(rectLosBndBox.a);
	srcLosVec.push_back(rectLosBndBox.b);
	srcLosVec.push_back(rectLosBndBox.c);
	srcLosVec.push_back(rectLosBndBox.d);
	srcLosVec.push_back(rectLosCnt);
	perspectiveTransform(srcLosVec, dstLosVec, fldToOrgHMat);
	losBndBox.a = dstLosVec[0];
	losBndBox.b = dstLosVec[1];
	losBndBox.c = dstLosVec[2];
	losBndBox.d = dstLosVec[3];
	losCnt = dstLosVec[4];
	plotRect(mosFrame, losBndBox, Scalar(255, 0, 0));


	string playersFilePath = "playerBndBoxes/Game" + gameIdStr + "/" + gameIdStr +"0" + vidIdxStr + ".players";
	ifstream fin(playersFilePath.c_str());

	if(!fin.is_open())
	{
		cout << "Can't open file " << playersFilePath << endl;
		return;
	}

	fin.seekg(0, ios::end);
	if (fin.tellg() == 0) {
		cout << "Empty file " << playersFilePath << endl;
		return;
	}
	fin.seekg(0, ios::beg);
	vector<double> scores;
	vector<struct rect> players;
	vector<double> areas;
	while(!fin.eof())
	{
		int playId = NEGINF;
		double score = NEGINF;
		double xMin, yMin, xMax, yMax;
		fin >> playId >> score >> xMin >> yMin >> xMax >> yMax;
		if(score == NEGINF)
			break;
		struct rect player;
		player.a = Point2d(xMin, yMin);
		player.b = Point2d(xMin, yMax);
		player.c = Point2d(xMax, yMax);
		player.d = Point2d(xMax, yMin);
		scores.push_back(score);
		players.push_back(player);
		double area = (xMax - xMin) * (yMax - yMin);
		areas.push_back(area);
	}

	int fontFace = 0;
	double fontScale = 1;
	int thickness = 2;
	Mat samples(players.size(), 3, CV_32F);
	for(unsigned int i = 0; i < players.size(); ++i)
	{
//		plotRect(mosFrame, players[i], Scalar(0, 0, 255));
		Point3d avgClr;
		plotRectAvgClr(mosFrame, players[i], Scalar(0, 0, 255), avgClr);
		samples.at<float>(i, 0) = avgClr.x;
		samples.at<float>(i, 1) = avgClr.y;
		samples.at<float>(i, 2) = avgClr.z;

		//string scoreStr = to_string(scores[i]);
//		double score = double(int(scores[i] * 100)) / 100.0;
//		ostringstream convertScore;
//		convertScore << score;
//		string scoreStr = convertScore.str();
//		putText(mosFrame, scoreStr, players[i].b, fontFace, fontScale, CV_RGB(0, 0, 255), thickness,8);
//
//		double area = double(int(areas[i] * 100)) / 100.0;
//		ostringstream convertArea;
//		convertArea << area;
//		string areaStr = convertArea.str();
//		putText(mosFrame, areaStr, players[i].a, fontFace, fontScale, CV_RGB(255, 255, 0), thickness,8);
	}
	fin.close();

	int K = 2;
	Mat labels;
	int attempts = 5;
	Mat centers;
	kmeans(samples, K, labels, TermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 10000, 0.0001), attempts, KMEANS_PP_CENTERS, centers );

	for(unsigned int i = 0; i < players.size(); ++i)
	{
		ostringstream convertScore;
		convertScore << labels.at<int>(i, 0);
		string scoreStr = convertScore.str();
		putText(mosFrame, scoreStr, players[i].a, fontFace, fontScale, CV_RGB(0, 0, 255), thickness,8);

	}

	string playersImagePath = "Results/Game" + gameIdStr + "/playersPlots/" + gameIdStr +"0" + vidIdxStr + ".jpg";
	imwrite(playersImagePath, mosFrame);
}

void play::drawPlayerBndBoxesRectFld()
{
	Mat orgToFldHMat;
	rectification(orgToFldHMat);
	Mat fldToOrgHMat;
	getOverheadFieldHomo(fldToOrgHMat);

	string playersFilePath = "playerBndBoxes/Game" + gameIdStr + "/" + gameIdStr +"0" + vidIdxStr + ".players";
	vector<double> scores;
	vector<struct rect> players;
	vector<double> areas;
	readPlayerBndBoxes(playersFilePath, scores, players, areas);
//	ifstream fin(playersFilePath.c_str());
//
//	if(!fin.is_open())
//	{
//		cout << "Can't open file " << playersFilePath << endl;
//		return;
//	}
//
//	fin.seekg(0, ios::end);
//	if (fin.tellg() == 0) {
//		cout << "Empty file " << playersFilePath << endl;
//		return;
//	}
//	fin.seekg(0, ios::beg);
//	vector<double> scores;
//	vector<struct rect> players;
//	vector<double> areas;
//	while(!fin.eof())
//	{
//		int playId = NEGINF;
//		double score = NEGINF;
//		double xMin, yMin, xMax, yMax;
//		fin >> playId >> score >> xMin >> yMin >> xMax >> yMax;
//		if(score == NEGINF)
//			break;
//		struct rect player;
//		player.a = Point2d(xMin, yMin);
//		player.b = Point2d(xMin, yMax);
//		player.c = Point2d(xMax, yMax);
//		player.d = Point2d(xMax, yMin);
//		scores.push_back(score);
//		players.push_back(player);
//		double area = (xMax - xMin) * (yMax - yMin);
//		areas.push_back(area);
//	}

	int fontFace = 0;
	double fontScale = 1;
	int thickness = 2;

	for(unsigned int i = 0; i < players.size(); ++i)
	{
		struct rect player = players[i];
		vector<Point2d> srcPlayerVec, dstPlayerVec;
		srcPlayerVec.push_back(player.a);
		srcPlayerVec.push_back(player.b);
		srcPlayerVec.push_back(player.c);
		srcPlayerVec.push_back(player.d);
		perspectiveTransform(srcPlayerVec, dstPlayerVec, orgToFldHMat);
		player.a = dstPlayerVec[0];
		player.b = dstPlayerVec[1];
		player.c = dstPlayerVec[2];
		player.d = dstPlayerVec[3];

		plotRect(rectMosFrame, player, Scalar(0, 0, 255));
		//string scoreStr = to_string(scores[i]);
		double score = double(int(scores[i] * 100)) / 100.0;
		ostringstream convertScore;
		convertScore << score;
		string scoreStr = convertScore.str();
		putText(rectMosFrame, scoreStr, player.b, fontFace, fontScale, CV_RGB(0, 0, 255), thickness,8);

		double area = double(int(areas[i] * 100)) / 100.0;
		ostringstream convertArea;
		convertArea << area;
		string areaStr = convertArea.str();
		putText(rectMosFrame, areaStr, player.a, fontFace, fontScale, CV_RGB(255, 255, 0), thickness,8);
	}

//	fin.close();

	string playersImagePath = "Results/Game" + gameIdStr + "/playersPlots/" + gameIdStr +"0" + vidIdxStr + ".jpg";
	imwrite(playersImagePath, rectMosFrame);


}

void play::detectForms(direction offSide)
{
	Mat orgToFldHMat;
	rectification(orgToFldHMat);
	Mat fldToOrgHMat;
	getOverheadFieldHomo(fldToOrgHMat);

	vector<Point2d> srcLosVec, dstLosVec;
	srcLosVec.push_back(rectLosBndBox.a);
	srcLosVec.push_back(rectLosBndBox.b);
	srcLosVec.push_back(rectLosBndBox.c);
	srcLosVec.push_back(rectLosBndBox.d);
	srcLosVec.push_back(rectLosCnt);
	perspectiveTransform(srcLosVec, dstLosVec, fldToOrgHMat);
	losBndBox.a = dstLosVec[0];
	losBndBox.b = dstLosVec[1];
	losBndBox.c = dstLosVec[2];
	losBndBox.d = dstLosVec[3];
	losCnt = dstLosVec[4];
	plotRect(mosFrame, losBndBox, Scalar(255, 0, 0));

	string formsFile = "formModel/formations";
	vector<string> formations;
//	readFormsFile(formsFile, rightDir, formations);
	readFormsFile(formsFile, formations);

	string playersFilePath = "playerBndBoxes/Game" + gameIdStr + "/" + gameIdStr +"0" + vidIdxStr + ".players";
	vector<double> scores;
	vector<struct rect> players;
	vector<double> areas;
	readPlayerBndBoxes(playersFilePath, scores, players, areas);

	vector<Point2d> playersLocSet, pLocSetFld, offensePLocSetFld;
	for(unsigned int i = 0; i < players.size(); ++i)
	{
//		Point2d p = Point2d(0.5 * (players[i].a.x + players[i].c.x),
//				0.5 * (players[i].a.y + players[i].c.y) );
		Point2d p = Point2d(0.5 * (players[i].b.x + players[i].c.x),
				0.5 * (players[i].b.y + players[i].c.y) );
		playersLocSet.push_back(p);
		plotRect(mosFrame, players[i], Scalar(0, 0, 255));
	}
	perspectiveTransform(playersLocSet, pLocSetFld, orgToFldHMat);

//	getOffensePlayers(playersLocSet, pLocSetFld, this, players, offSide);
//	getOffensePlayers(playersLocSet, pLocSetFld, this, players, offSide, scores);

	double bestFormScore = NEGINF;
	formTree* bestForm;
	vector<formTree*> fTrees;
	for(unsigned int i = 0; i < formations.size(); ++i)
	{
//		formTree* f = new formTree(formations[i]);
		formTree* f = new formTree(formations[i], offSide);
//		f->setupPartsLocSetStarModel(rectLosCnt, pLocSetFld, scores);
//		f->findBestFormStarModel();

//		f->setupPartsLocSetHungarian(rectLosCnt, pLocSetFld);
//		string vidFormId = "Hungarian/Game" + gameIdStr + "/vid" + vidIdxStr + "F";
//		ostringstream convertVidForm;
//		convertVidForm << i;
//		vidFormId = vidFormId + convertVidForm.str();
//		cout << vidFormId << endl;
////		f->getScoreMat(vidFormId);
//		f->findBestFormHungarian(vidFormId);

		vector<Point2d> olLocSet;
//		getRectLosPnts(rectLosBndBox, olLocSet);
		olLocSet.push_back(rectLosCnt);
		f->setupPartsLocSetHungarian(olLocSet, pLocSetFld);
//		f->setupPartsLocSetHungarian(olLocSet, pLocSetFld, scores);
		f->findBestFormHungarian();
		if(f->formBestScore >= bestFormScore)
		{
			bestFormScore = f->formBestScore;
			bestForm = f;
			fTrees.push_back(f);
		}
	}


	cout << bestForm->formName << bestForm->formBestScore << endl;

	bestForm->plotFormOrigImg(mosFrame, fldToOrgHMat);
	string playersImagePath = "Results/Game" + gameIdStr + "/playersPlots/" + gameIdStr +"0" + vidIdxStr + ".jpg";
	imwrite(playersImagePath, mosFrame);

}

void play::detectFormsGt(direction offSide)
{
	Mat orgToFldHMat;
	rectification(orgToFldHMat);
	Mat fldToOrgHMat;
	getOverheadFieldHomo(fldToOrgHMat);

	string formFilePath = "formAnnotations/Game" + gameIdStr + "/" + "vid" + vidIdxStr + ".form";
	vector<struct rect> players;
	readFormationGt(formFilePath, players, losBndBox, losCnt);
//	cout << "losCnt " << losCnt.x << " " << losCnt.y << endl;
	vector<Point2d> srcLosVec, dstLosVec;
	srcLosVec.push_back(losBndBox.a);
	srcLosVec.push_back(losBndBox.b);
	srcLosVec.push_back(losBndBox.c);
	srcLosVec.push_back(losBndBox.d);
	srcLosVec.push_back(losCnt);
	perspectiveTransform(srcLosVec, dstLosVec, orgToFldHMat);
	rectLosBndBox.a = dstLosVec[0];
	rectLosBndBox.b = dstLosVec[1];
	rectLosBndBox.c = dstLosVec[2];
	rectLosBndBox.d = dstLosVec[3];
	rectLosCnt = dstLosVec[4];
	plotRect(mosFrame, losBndBox, Scalar(255, 0, 0));

	string formsFile = "formModel/formations";
	vector<string> formations;
//	readFormsFile(formsFile, rightDir, formations);
	readFormsFile(formsFile, formations);

	vector<Point2d> playersLocSet, pLocSetFld, offensePLocSetFld;
	for(unsigned int i = 0; i < players.size(); ++i)
	{
		Point2d p = Point2d(0.5 * (players[i].a.x + players[i].c.x),
				0.5 * (players[i].a.y + players[i].c.y) );
//		Point2d p = Point2d(0.5 * (players[i].b.x + players[i].c.x),
//				0.5 * (players[i].b.y + players[i].c.y) );
		playersLocSet.push_back(p);
		plotRect(mosFrame, players[i], Scalar(0, 0, 255));
	}
	perspectiveTransform(playersLocSet, pLocSetFld, orgToFldHMat);

//	getOffensePlayers(playersLocSet, pLocSetFld, this, players, offSide);
//	getOffensePlayers(playersLocSet, pLocSetFld, this, players, offSide, scores);

	double bestFormScore = NEGINF;
	formTree* bestForm;
	vector<formTree*> fTrees;
	for(unsigned int i = 0; i < formations.size(); ++i)
	{
//		formTree* f = new formTree(formations[i]);
		formTree* f = new formTree(formations[i], offSide);
//		f->setupPartsLocSetStarModel(rectLosCnt, pLocSetFld, scores);
//		f->findBestFormStarModel();

//		f->setupPartsLocSetHungarian(rectLosCnt, pLocSetFld);
//		string vidFormId = "Hungarian/Game" + gameIdStr + "/vid" + vidIdxStr + "F";
//		ostringstream convertVidForm;
//		convertVidForm << i;
//		vidFormId = vidFormId + convertVidForm.str();
//		cout << vidFormId << endl;
////		f->getScoreMat(vidFormId);
//		f->findBestFormHungarian(vidFormId);

		vector<Point2d> olLocSet;
//		getRectLosPnts(rectLosBndBox, olLocSet);
		olLocSet.push_back(rectLosCnt);
		f->setupPartsLocSetHungarian(olLocSet, pLocSetFld);
//		f->setupPartsLocSetHungarian(olLocSet, pLocSetFld, scores);
		f->findBestFormHungarian();
		if(f->formBestScore >= bestFormScore)
		{
			bestFormScore = f->formBestScore;
			bestForm = f;
			fTrees.push_back(f);
		}
	}


	cout << bestForm->formName << bestForm->formBestScore << endl;

	bestForm->plotFormOrigImg(mosFrame, fldToOrgHMat);
	bestForm->plotFormRectImg(rectMosFrame);
	string playersImagePath = "Results/Game" + gameIdStr + "/playersPlots/" + gameIdStr +"0" + vidIdxStr + ".jpg";
	imwrite(playersImagePath, mosFrame);
	string formImageRectPath = "Results/Game" + gameIdStr + "/playersPlots/" + gameIdStr +"0" + vidIdxStr + "Rect.jpg";
	imwrite(formImageRectPath, rectMosFrame);

}

void play::labelPlayersAngleGt()
{
	Mat orgToFldHMat;
	rectification(orgToFldHMat);
	Mat fldToOrgHMat;
	getOverheadFieldHomo(fldToOrgHMat);

	string formFilePath = "formAnnotations/Game" + gameIdStr + "/" + "vid" + vidIdxStr + ".form";
	vector<struct rect> players;
	readFormationGt(formFilePath, players, losBndBox, losCnt);
//	cout << "losCnt " << losCnt.x << " " << losCnt.y << endl;
	vector<Point2d> srcLosVec, dstLosVec;
	srcLosVec.push_back(losBndBox.a);
	srcLosVec.push_back(losBndBox.b);
	srcLosVec.push_back(losBndBox.c);
	srcLosVec.push_back(losBndBox.d);
	srcLosVec.push_back(losCnt);
	perspectiveTransform(srcLosVec, dstLosVec, orgToFldHMat);
	rectLosBndBox.a = dstLosVec[0];
	rectLosBndBox.b = dstLosVec[1];
	rectLosBndBox.c = dstLosVec[2];
	rectLosBndBox.d = dstLosVec[3];
	rectLosCnt = dstLosVec[4];
	plotRect(mosFrame, losBndBox, Scalar(255, 0, 0));

	string formsFile = "formModel/formations";
	vector<string> formations;
//	readFormsFile(formsFile, rightDir, formations);
	readFormsFile(formsFile, formations);

	vector<Point2d> playersLocSet, pLocSetFld, offensePLocSetFld;
	for(unsigned int i = 0; i < players.size(); ++i)
	{
//		Point2d p = Point2d(0.5 * (players[i].b.x + players[i].c.x),
//				0.5 * (players[i].b.y + players[i].c.y) );
		Point2d p = Point2d(0.25 * (players[i].a.x + players[i].b.x + players[i].c.x + players[i].d.x),
				0.25 * (players[i].a.y + players[i].b.y + players[i].c.y + players[i].d.y) );
		playersLocSet.push_back(p);
		plotRect(mosFrame, players[i], Scalar(0, 0, 255));
	}
	perspectiveTransform(playersLocSet, pLocSetFld, orgToFldHMat);

	vector<string> playersTypes;
	Point2d vectVec(.0, 1);
//	double angThresh = 1 / sqrt(2);
	double angThresh = 0.5 * sqrt(3);
//	cout << "angThresh " << angThresh << endl;
	for(unsigned int i = 0; i < pLocSetFld.size(); ++i)
	{
		Point2d vecLosToP = pLocSetFld[i] - rectLosCnt;
		vecLosToP *= 1.0 / norm(vecLosToP);
		double dotPr = vecLosToP.ddot(vectVec);
		cout << "dotPr " << dotPr << endl;
		if(abs(dotPr) >= angThresh)
			playersTypes.push_back("WR");
		else
			playersTypes.push_back("HB");
	}

	double minDistToLos = INF;
	int qbIdx = -1;
	for(unsigned int i = 0; i < playersTypes.size(); ++i)
	{
//		cout << playersTypes[i] << endl;
		if(playersTypes[i].compare("HB") == 0)
		{
//			double yDistToLos = abs(pLocSetFld[i].y - rectLosCnt.y);
			double distToLos = norm(pLocSetFld[i] - rectLosCnt);
			if(distToLos < minDistToLos)
			{
				minDistToLos = distToLos;
				qbIdx = i;
			}
		}
	}
	if(qbIdx >= 0)
		playersTypes[qbIdx] = "QB";

	string formName = "unknown";
	double qbToLosDist = INF, hbToLosDist = INF;
	for(unsigned int i = 0; i < playersTypes.size(); ++i)
	{
		if(playersTypes[i].compare("HB") == 0)
		{
			double hbDist = abs(pLocSetFld[i].x - rectLosCnt.x);
			if(hbDist < hbToLosDist)
				hbToLosDist = hbDist;
		}
		if(playersTypes[i].compare("QB") == 0)
		{
			 qbToLosDist = abs(pLocSetFld[i].x - rectLosCnt.x);
		}
	}
	if(qbToLosDist != INF && hbToLosDist != INF)
	{
		if(hbToLosDist >= 1.5 * qbToLosDist)
			formName = "pistol";
		else
			formName = "shotgun";
	}

	int fontFace = 0;
	double fontScale = 1;
	int thickness = 2;
	int len = 5;
	for(unsigned int i = 0; i < playersTypes.size(); ++i)
	{
		cout << playersTypes[i] << endl;
		putText(mosFrame, playersTypes[i], playersLocSet[i], fontFace, fontScale, CV_RGB(0, 0, 255), thickness,8);

		line(mosFrame, playersLocSet[i] - Point2d(len, 0), playersLocSet[i] + Point2d(len, 0), CV_RGB(0, 255, 0),2,8,0);
		line(mosFrame, playersLocSet[i] - Point2d(0, len), playersLocSet[i] + Point2d(0, len), CV_RGB(0, 255, 0),2,8,0);
	}

	line(mosFrame, losCnt - Point2d(len, 0), losCnt + Point2d(len, 0), CV_RGB(0, 255, 0),2,8,0);
	line(mosFrame, losCnt- Point2d(0, len), losCnt + Point2d(0, len), CV_RGB(0, 255, 0),2,8,0);

	for(unsigned int i = 0; i < playersTypes.size(); ++i)
	{
//		cout << playersTypes[i] << endl;
		putText(rectMosFrame, playersTypes[i], pLocSetFld[i], fontFace, fontScale, CV_RGB(0, 0, 255), thickness,8);

		line(rectMosFrame, pLocSetFld[i] - Point2d(len, 0), pLocSetFld[i] + Point2d(len, 0), CV_RGB(0, 255, 0),2,8,0);
		line(rectMosFrame, pLocSetFld[i] - Point2d(0, len), pLocSetFld[i] + Point2d(0, len), CV_RGB(0, 255, 0),2,8,0);
	}
	plotLos(rectMosFrame, rectLosBndBox);
	line(rectMosFrame, rectLosCnt - Point2d(len, 0), rectLosCnt + Point2d(len, 0), CV_RGB(0, 255, 0),2,8,0);
	line(rectMosFrame, rectLosCnt- Point2d(0, len), rectLosCnt + Point2d(0, len), CV_RGB(0, 255, 0),2,8,0);

	putText(mosFrame, formName, Point2d(10, 30), fontFace, fontScale, CV_RGB(0, 0, 255), thickness,8);
	putText(rectMosFrame, formName, Point2d(10, 30), fontFace, fontScale, CV_RGB(0, 0, 255), thickness,8);

	string formImagePath = "Results/Game" + gameIdStr + "/playersPlots/" + gameIdStr +"0" + vidIdxStr + ".jpg";
	imwrite(formImagePath, mosFrame);
	string formImageRectPath = "Results/Game" + gameIdStr + "/playersPlots/" + gameIdStr +"0" + vidIdxStr + "Rect.jpg";
	imwrite(formImageRectPath, rectMosFrame);

}

void play::labelPlayersAngle(direction offSide)
{
	Mat orgToFldHMat;
	rectification(orgToFldHMat);
	Mat fldToOrgHMat;
	getOverheadFieldHomo(fldToOrgHMat);

	vector<Point2d> srcLosVec, dstLosVec;
	srcLosVec.push_back(rectLosBndBox.a);
	srcLosVec.push_back(rectLosBndBox.b);
	srcLosVec.push_back(rectLosBndBox.c);
	srcLosVec.push_back(rectLosBndBox.d);
	srcLosVec.push_back(rectLosCnt);
	perspectiveTransform(srcLosVec, dstLosVec, fldToOrgHMat);
	losBndBox.a = dstLosVec[0];
	losBndBox.b = dstLosVec[1];
	losBndBox.c = dstLosVec[2];
	losBndBox.d = dstLosVec[3];
	losCnt = dstLosVec[4];
	plotRect(mosFrame, losBndBox, Scalar(255, 0, 0));

	string formsFile = "formModel/formations";
	vector<string> formations;
//	readFormsFile(formsFile, rightDir, formations);
	readFormsFile(formsFile, formations);

	string playersFilePath = "playerBndBoxes/Game" + gameIdStr + "/" + gameIdStr +"0" + vidIdxStr + ".players";
	vector<double> scores;
	vector<struct rect> players;
	vector<double> areas;
	readPlayerBndBoxes(playersFilePath, scores, players, areas);

	vector<Point2d> playersLocSet, pLocSetFld, offensePLocSetFld;
	for(unsigned int i = 0; i < players.size(); ++i)
	{
		Point2d p = Point2d(0.5 * (players[i].a.x + players[i].c.x),
				0.5 * (players[i].a.y + players[i].c.y) );
//		Point2d p = Point2d(0.5 * (players[i].b.x + players[i].c.x),
//				0.5 * (players[i].b.y + players[i].c.y) );
		playersLocSet.push_back(p);
		plotRect(mosFrame, players[i], Scalar(0, 0, 255));
	}
	perspectiveTransform(playersLocSet, pLocSetFld, orgToFldHMat);

	getOffensePlayers(playersLocSet, pLocSetFld, this, players, offSide);
//	getOffensePlayers(playersLocSet, pLocSetFld, this, players, offSide, scores);

	vector<string> playersTypes;
	Point2d vectVec(.0, 1);
//	double angThresh = 1 / sqrt(2);
	double angThresh = 0.5 * sqrt(3);
//	cout << "angThresh " << angThresh << endl;
	for(unsigned int i = 0; i < pLocSetFld.size(); ++i)
	{
		string pType = "non";
		Point2d vecLosToP = pLocSetFld[i] - rectLosCnt;
		double distToLos = norm(vecLosToP * (1.0 / 15.0));
//		if(distToLos <= 1)
//			continue;
		vecLosToP *= 1.0 / norm(vecLosToP);
		double dotPr = vecLosToP.ddot(vectVec);
//		cout << "dotPr " << dotPr << endl;
		if(abs(dotPr) >= angThresh)
		{
			if(distToLos > 5)
				pType = "WR";
		}
		else
		{
			if(distToLos > 1)
				pType = "HB";
		}
			playersTypes.push_back(pType);
	}

	double minDistToLos = INF;
	int qbIdx = -1;
	for(unsigned int i = 0; i < playersTypes.size(); ++i)
	{
//		cout << playersTypes[i] << endl;
		if(playersTypes[i].compare("HB") == 0)
		{
//			double yDistToLos = abs(pLocSetFld[i].y - rectLosCnt.y);
			double distToLos = norm(pLocSetFld[i] - rectLosCnt);
			if(distToLos < minDistToLos)
			{
				minDistToLos = distToLos;
				qbIdx = i;
			}
		}
	}
	if(qbIdx >= 0)
		playersTypes[qbIdx] = "QB";

	string formName = "unknown";
	double qbToLosDist = INF, hbToLosDist = INF;
	for(unsigned int i = 0; i < playersTypes.size(); ++i)
	{
		if(playersTypes[i].compare("HB") == 0)
		{
			double hbDist = abs(pLocSetFld[i].x - rectLosCnt.x);
			if(hbDist < hbToLosDist)
				hbToLosDist = hbDist;
		}
		if(playersTypes[i].compare("QB") == 0)
		{
			 qbToLosDist = abs(pLocSetFld[i].x - rectLosCnt.x);
		}
	}
	if(qbToLosDist != INF && hbToLosDist != INF)
	{
		if(hbToLosDist >= 1.5 * qbToLosDist)
			formName = "pistol";
		else
			formName = "shotgun";
	}

	int fontFace = 0;
	double fontScale = 1;
	int thickness = 2;
	int len = 5;
	for(unsigned int i = 0; i < playersTypes.size(); ++i)
	{
		if(playersTypes[i].compare("non") == 0)
			continue;
//		cout << playersTypes[i] << endl;
		putText(mosFrame, playersTypes[i], playersLocSet[i], fontFace, fontScale, CV_RGB(0, 0, 255), thickness,8);

		line(mosFrame, playersLocSet[i] - Point2d(len, 0), playersLocSet[i] + Point2d(len, 0), CV_RGB(0, 255, 0),2,8,0);
		line(mosFrame, playersLocSet[i] - Point2d(0, len), playersLocSet[i] + Point2d(0, len), CV_RGB(0, 255, 0),2,8,0);
	}

	line(mosFrame, losCnt - Point2d(len, 0), losCnt + Point2d(len, 0), CV_RGB(0, 255, 0),2,8,0);
	line(mosFrame, losCnt- Point2d(0, len), losCnt + Point2d(0, len), CV_RGB(0, 255, 0),2,8,0);

	for(unsigned int i = 0; i < playersTypes.size(); ++i)
	{
		if(playersTypes[i].compare("non") == 0)
			continue;
//		cout << playersTypes[i] << endl;
		putText(rectMosFrame, playersTypes[i], pLocSetFld[i], fontFace, fontScale, CV_RGB(0, 0, 255), thickness,8);

		line(rectMosFrame, pLocSetFld[i] - Point2d(len, 0), pLocSetFld[i] + Point2d(len, 0), CV_RGB(0, 255, 0),2,8,0);
		line(rectMosFrame, pLocSetFld[i] - Point2d(0, len), pLocSetFld[i] + Point2d(0, len), CV_RGB(0, 255, 0),2,8,0);
	}
	plotLos(rectMosFrame, rectLosBndBox);
	line(rectMosFrame, rectLosCnt - Point2d(len, 0), rectLosCnt + Point2d(len, 0), CV_RGB(0, 255, 0),2,8,0);
	line(rectMosFrame, rectLosCnt- Point2d(0, len), rectLosCnt + Point2d(0, len), CV_RGB(0, 255, 0),2,8,0);

	putText(mosFrame, formName, Point2d(10, 30), fontFace, fontScale, CV_RGB(0, 0, 255), thickness,8);
	putText(rectMosFrame, formName, Point2d(10, 30), fontFace, fontScale, CV_RGB(0, 0, 255), thickness,8);

	string formImagePath = "Results/Game" + gameIdStr + "/playersPlots/" + gameIdStr +"0" + vidIdxStr + ".jpg";
	imwrite(formImagePath, mosFrame);
	string formImageRectPath = "Results/Game" + gameIdStr + "/playersPlots/" + gameIdStr +"0" + vidIdxStr + "Rect.jpg";
	imwrite(formImageRectPath, rectMosFrame);

}

void play::getPlayersToLosVecs(vector<Point2d> &pToLosVec, vector<int> &pTypesId)
{
	Mat orgToFldHMat;
	rectification(orgToFldHMat);

	string formFilePath = "formAnnotations/Game" + gameIdStr + "/" + "vid" + vidIdxStr + ".form";
	vector<struct rect> players;
	vector<string> pTypes;
	readFormationGt(formFilePath, players, pTypes, losBndBox, losCnt);

	vector<Point2d> srcLosVec, dstLosVec;
	srcLosVec.push_back(losCnt);
	perspectiveTransform(srcLosVec, dstLosVec, orgToFldHMat);
	rectLosCnt = dstLosVec[0];

	vector<Point2d> playersLocSet, pLocSetFld, offensePLocSetFld;
	for(unsigned int i = 0; i < players.size(); ++i)
	{
		Point2d p = Point2d(0.5 * (players[i].a.x + players[i].c.x),
				0.5 * (players[i].a.y + players[i].c.y) );
//		Point2d p = Point2d(0.5 * (players[i].b.x + players[i].c.x),
//				0.5 * (players[i].b.y + players[i].c.y) );
		playersLocSet.push_back(p);
	}
	perspectiveTransform(playersLocSet, pLocSetFld, orgToFldHMat);

	for(unsigned int i = 0; i < pLocSetFld.size(); ++i)
	{
		pToLosVec.push_back(pLocSetFld[i] - rectLosCnt);
		int pTId = convertPTypeToPId(pTypes[i]);
//		cout << pTId << endl;
		pTypesId.push_back(pTId);
	}

//	int fontFace = 0;
//	double fontScale = 1;
//	int thickness = 2;
//	for(unsigned int i = 0; i < players.size(); ++i)
//	{
//		plotRect(mosFrame, players[i], CV_RGB(0, 0, 255));
//		putText(mosFrame, pTypes[i], playersLocSet[i], fontFace, fontScale, CV_RGB(0, 0, 255), thickness,8);
//	}
//	string gtFormImagePath = "formGtImgs/Game" + gameIdStr + "/" + gameIdStr +"0" + vidIdxStr + ".jpg";
//	imwrite(gtFormImagePath, mosFrame);
//	int len = 5;
//	for(unsigned int i = 0; i < players.size(); ++i)
//	{
////		plotRect(rectMosFrame, players[i], CV_RGB(0, 0, 255));
//		line(rectMosFrame, pLocSetFld[i] - Point2d(len, 0), pLocSetFld[i] + Point2d(len, 0), CV_RGB(0, 255, 0),2,8,0);
//		line(rectMosFrame, pLocSetFld[i]- Point2d(0, len), pLocSetFld[i] + Point2d(0, len), CV_RGB(0, 255, 0),2,8,0);
//		putText(rectMosFrame, pTypes[i], pLocSetFld[i], fontFace, fontScale, CV_RGB(0, 0, 255), thickness,8);
//	}
//	line(rectMosFrame, rectLosCnt - Point2d(len, 0), rectLosCnt + Point2d(len, 0), CV_RGB(255, 255, 0),2,8,0);
//	line(rectMosFrame, rectLosCnt- Point2d(0, len), rectLosCnt + Point2d(0, len), CV_RGB(255, 255, 0),2,8,0);
//	string gtRectFormImagePath = "formGtImgs/Game" + gameIdStr + "/" + gameIdStr +"0" + vidIdxStr + "Rect.jpg";
//	imwrite(gtRectFormImagePath, rectMosFrame);
}

void play::getPlayersLosPos(vector<Point2d> &positions, vector<string> &pTypes)
{
	Mat orgToFldHMat;
	rectification(orgToFldHMat);

	string formFilePath = "formAnnotations/Game" + gameIdStr + "/" + "vid" + vidIdxStr + ".form";
	vector<struct rect> players;
//	vector<string> pTypes;
	readFormationGt(formFilePath, players, pTypes, losBndBox, losCnt);

	vector<Point2d> srcLosVec, dstLosVec;
	srcLosVec.push_back(losCnt);
	perspectiveTransform(srcLosVec, dstLosVec, orgToFldHMat);
	rectLosCnt = dstLosVec[0];

	vector<Point2d> playersLocSet, pLocSetFld, offensePLocSetFld;
	for(unsigned int i = 0; i < players.size(); ++i)
	{
		Point2d p = Point2d(0.5 * (players[i].a.x + players[i].c.x),
				0.5 * (players[i].a.y + players[i].c.y) );
//		Point2d p = Point2d(0.5 * (players[i].b.x + players[i].c.x),
//				0.5 * (players[i].b.y + players[i].c.y) );
		playersLocSet.push_back(p);
	}
	perspectiveTransform(playersLocSet, pLocSetFld, orgToFldHMat);
	positions = pLocSetFld;
	positions.insert(positions.begin(), rectLosCnt);
	pTypes.insert(pTypes.begin(), "LOS");
//	for(unsigned int i = 0; i < pLocSetFld.size(); ++i)
//	{
//		positions.push_back(pLocSetFld[i]);
//		int pTId = convertPTypeToPId(pTypes[i]);
////		cout << pTId << endl;
//		pTypesId.push_back(pTId);
//	}
}

void play::savePlayersLosPos(const vector<Point2d> &positions, const vector<string> &pTypes)
{
//	string formFilePath = "formsExemplar/Game" + gameIdStr + "/" + "vid" + vidIdxStr + ".pos";
	string formFilePath = "detectedFormations/Game" + gameIdStr + "/" + "vid" + vidIdxStr + ".pos";
	ofstream fFormOut(formFilePath.c_str());
	for(unsigned int i = 0; i < positions.size(); ++i)
		fFormOut << positions[i].x << " " << positions[i].y << endl;
	fFormOut.close();

//	string pTypesFilePath = "formsExemplar/Game" + gameIdStr + "/" + "vid" + vidIdxStr + ".type";
	string pTypesFilePath = "detectedFormations/Game" + gameIdStr + "/" + "vid" + vidIdxStr + ".type";
	ofstream fTypesOut(pTypesFilePath.c_str());
	for(unsigned int i = 0; i < pTypes.size(); ++i)
		fTypesOut << pTypes[i]<< endl;
	fTypesOut.close();
}

void play::lablePTypesKnnFixedLosCnt(direction offSide,
		const Mat &trainFeaturesMat, const Mat &trainLabelsMat)
{
	Mat orgToFldHMat;
	rectification(orgToFldHMat);
	Mat fldToOrgHMat;
	getOverheadFieldHomo(fldToOrgHMat);

	vector<Point2d> srcLosVec, dstLosVec;
	srcLosVec.push_back(rectLosBndBox.a);
	srcLosVec.push_back(rectLosBndBox.b);
	srcLosVec.push_back(rectLosBndBox.c);
	srcLosVec.push_back(rectLosBndBox.d);
	srcLosVec.push_back(rectLosCnt);
	perspectiveTransform(srcLosVec, dstLosVec, fldToOrgHMat);
	losBndBox.a = dstLosVec[0];
	losBndBox.b = dstLosVec[1];
	losBndBox.c = dstLosVec[2];
	losBndBox.d = dstLosVec[3];
	losCnt = dstLosVec[4];
	plotRect(mosFrame, losBndBox, Scalar(255, 0, 0));

//	string formFilePath = "formAnnotations/Game" + gameIdStr + "/" + "vid" + vidIdxStr + ".form";
//	vector<struct rect> playersGt;
//	vector<string> pTypesGt;
//	readFormationGt(formFilePath, playersGt, pTypesGt, losBndBox, losCnt);
//	vector<Point2d> srcLosVec, dstLosVec;
//	srcLosVec.push_back(losBndBox.a);
//	srcLosVec.push_back(losBndBox.b);
//	srcLosVec.push_back(losBndBox.c);
//	srcLosVec.push_back(losBndBox.d);
//	srcLosVec.push_back(losCnt);
//	perspectiveTransform(srcLosVec, dstLosVec, orgToFldHMat);
//	rectLosBndBox.a = dstLosVec[0];
//	rectLosBndBox.b = dstLosVec[1];
//	rectLosBndBox.c = dstLosVec[2];
//	rectLosBndBox.d = dstLosVec[3];
//	rectLosCnt = dstLosVec[4];
//	plotRect(mosFrame, losBndBox, Scalar(255, 0, 0));


	string playersFilePath = "playerBndBoxes/Game" + gameIdStr + "/" + gameIdStr +"0" + vidIdxStr + ".players";
	vector<double> scores;
	vector<struct rect> players;
	vector<double> areas;
	readPlayerBndBoxes(playersFilePath, scores, players, areas);

	vector<Point2d> playersLocSet, pLocSetFld, pFeetLocSet, pFeetLocSetFld;
	for(unsigned int i = 0; i < players.size(); ++i)
	{
		Point2d p = Point2d(0.5 * (players[i].a.x + players[i].c.x),
				0.5 * (players[i].a.y + players[i].c.y) );
		Point2d feetP = Point2d(0.5 * (players[i].b.x + players[i].c.x),
				0.5 * (players[i].b.y + players[i].c.y) );
		playersLocSet.push_back(p);
		pFeetLocSet.push_back(feetP);
		plotRect(mosFrame, players[i], Scalar(0, 0, 255));
	}
	perspectiveTransform(playersLocSet, pLocSetFld, orgToFldHMat);
	perspectiveTransform(pFeetLocSet, pFeetLocSetFld, orgToFldHMat);

	//	getOffensePlayers(playersLocSet, pLocSetFld, this, players, offSide);
	//	getOffensePlayers(playersLocSet, pLocSetFld, this, players, offSide, scores);
	getOffsPlayersByLos(playersLocSet, pLocSetFld, pFeetLocSetFld, this, players, offSide);
	if(pLocSetFld.empty())
		return;

	Mat testFeaturesMat = Mat(pLocSetFld.size(), 2, CV_32FC1);
	for(unsigned int i = 0; i < pLocSetFld.size(); ++i)
	{
		Point2d pToLosVec = pLocSetFld[i] - rectLosCnt;
		testFeaturesMat.at<float>(i, 0) = pToLosVec.x;
		testFeaturesMat.at<float>(i, 1) = pToLosVec.y;
	}

	int K = 3;
    CvKNearest knn(trainFeaturesMat, trainLabelsMat, Mat(), false, K);
    Mat neighborResponses(pLocSetFld.size(), K, CV_32FC1);
    Mat results(pLocSetFld.size(), 1, CV_32FC1), dists(pLocSetFld.size(), K, CV_32FC1);
    knn.find_nearest(testFeaturesMat, K, results, neighborResponses, dists);

    vector<string> playersTypes;
    for(unsigned int i = 0; i < pLocSetFld.size(); ++i)
    {
    	string pType = convertPIdToPType(int(results.at<float>(i, 0)));
    	cout << pType << endl;
    	playersTypes.push_back(pType);
    }

	int fontFace = 0;
	double fontScale = 1;
	int thickness = 2;
	int len = 5;
	for(unsigned int i = 0; i < players.size(); ++i)
		plotRect(mosFrame, players[i], Scalar(255, 0, 0));

	for(unsigned int i = 0; i < playersTypes.size(); ++i)
	{
		if(playersTypes[i].compare("non") == 0)
			continue;
//		cout << playersTypes[i] << endl;
		putText(mosFrame, playersTypes[i], playersLocSet[i], fontFace, fontScale, CV_RGB(0, 0, 255), thickness,8);

		line(mosFrame, playersLocSet[i] - Point2d(len, 0), playersLocSet[i] + Point2d(len, 0), CV_RGB(0, 255, 0),2,8,0);
		line(mosFrame, playersLocSet[i] - Point2d(0, len), playersLocSet[i] + Point2d(0, len), CV_RGB(0, 255, 0),2,8,0);
	}

	line(mosFrame, losCnt - Point2d(len, 0), losCnt + Point2d(len, 0), CV_RGB(255, 255, 0),2,8,0);
	line(mosFrame, losCnt- Point2d(0, len), losCnt + Point2d(0, len), CV_RGB(255, 255, 0),2,8,0);

	for(unsigned int i = 0; i < playersTypes.size(); ++i)
	{
		if(playersTypes[i].compare("non") == 0)
			continue;
//		cout << playersTypes[i] << endl;
		putText(rectMosFrame, playersTypes[i], pLocSetFld[i], fontFace, fontScale, CV_RGB(0, 0, 255), thickness,8);

		line(rectMosFrame, pLocSetFld[i] - Point2d(len, 0), pLocSetFld[i] + Point2d(len, 0), CV_RGB(0, 255, 0),2,8,0);
		line(rectMosFrame, pLocSetFld[i] - Point2d(0, len), pLocSetFld[i] + Point2d(0, len), CV_RGB(0, 255, 0),2,8,0);
	}
	plotLos(rectMosFrame, rectLosBndBox);
	line(rectMosFrame, rectLosCnt - Point2d(len, 0), rectLosCnt + Point2d(len, 0), CV_RGB(0, 255, 0),2,8,0);
	line(rectMosFrame, rectLosCnt- Point2d(0, len), rectLosCnt + Point2d(0, len), CV_RGB(0, 255, 0),2,8,0);

	string formImagePath = "Results/Game" + gameIdStr + "/playersPlots/" + gameIdStr +"0" + vidIdxStr + ".jpg";
	imwrite(formImagePath, mosFrame);
	string formImageRectPath = "Results/Game" + gameIdStr + "/playersPlots/" + gameIdStr +"0" + vidIdxStr + "Rect.jpg";
	imwrite(formImageRectPath, rectMosFrame);
}

void play::lablePTypesKnnVarLosCnts(direction offSide,
		const Mat &trainFeaturesMat, const Mat &trainLabelsMat)
{
	vector<string> bestPlayersTypes;
	vector<struct rect> bestPlayers;
	vector<Point2d> bestPlayersLocSet, bestPLocSetFld;
	Mat fldToOrgHMat, orgToFldHMat;
	lablePTypesKnnVarLosCnts(offSide, trainFeaturesMat, trainLabelsMat,
			bestPlayersTypes, bestPlayers, bestPlayersLocSet, bestPLocSetFld, fldToOrgHMat, orgToFldHMat);
	bestPlayersTypes.insert(bestPlayersTypes.begin(), "LOS");
	bestPLocSetFld.insert(bestPLocSetFld.begin(), rectLosCnt);
	savePlayersLosPos(bestPLocSetFld, bestPlayersTypes);
}

void play::lablePTypesKnnVarLosCnts(direction offSide, const Mat &trainFeaturesMat,
		const Mat &trainLabelsMat, vector<string> &bestPlayersTypes, vector<struct rect> &bestPlayers,
		vector<Point2d> &bestPlayersLocSet, vector<Point2d> &bestPLocSetFld, Mat &fldToOrgHMat, Mat &orgToFldHMat)
{
//	Mat orgToFldHMat;
//	rectification(orgToFldHMat);
	rctfWithoutDetectLos(orgToFldHMat);
//	Mat fldToOrgHMat;
	getOverheadFieldHomo(fldToOrgHMat);

//	getLosBndBoxByUfmClr();
	getLosBndBoxByClrAndFg();

	string playersFilePath = "playerBndBoxes/Game" + gameIdStr + "/" + gameIdStr +"0" + vidIdxStr + ".players";
	vector<double> scores;
	vector<struct rect> players;
	vector<double> areas;
	readPlayerBndBoxes(playersFilePath, scores, players, areas);

	vector<Point2d> playersLocSet, pLocSetFld, pFeetLocSet, pFeetLocSetFld;
	for(unsigned int i = 0; i < players.size(); ++i)
	{
		Point2d p = Point2d(0.5 * (players[i].a.x + players[i].c.x),
				0.5 * (players[i].a.y + players[i].c.y) );
		Point2d feetP = Point2d(0.5 * (players[i].b.x + players[i].c.x),
				0.5 * (players[i].b.y + players[i].c.y) );
		playersLocSet.push_back(p);
		pFeetLocSet.push_back(feetP);
		plotRect(mosFrame, players[i], Scalar(0, 0, 255));
	}
	perspectiveTransform(playersLocSet, pLocSetFld, orgToFldHMat);
	perspectiveTransform(pFeetLocSet, pFeetLocSetFld, orgToFldHMat);

	vector<Point2d> olLocSet;
	getRectLosPnts(rectLosBndBox, olLocSet);
//	olLocSet.push_back(rectLosCnt);

	vector<vector<double> > costsAllLosCntPos;
	vector<vector<string> > plsTypesAllLosCntPos;
	vector<double> clrCostsAllLosCntPos;

//	vector<string> bestPlayersTypes;
//	vector<struct rect> bestPlayers;
//	vector<Point2d> bestPlayersLocSet, bestPLocSetFld;

	for(unsigned int lCntIdx = 0; lCntIdx < olLocSet.size(); ++lCntIdx)
	{
		rectLosCnt = olLocSet[lCntIdx];
		//	getOffensePlayers(playersLocSet, pLocSetFld, this, players, offSide);
		//	getOffensePlayers(playersLocSet, pLocSetFld, this, players, offSide, scores);
		vector<struct rect> offsPlayers = players;
		vector<Point2d> offsPlayersLocSet = playersLocSet,
				offsPLocSetFld = pLocSetFld;
		getOffsPlayersByLos(offsPlayersLocSet, offsPLocSetFld, pFeetLocSetFld, this, offsPlayers, offSide);
		if(offsPLocSetFld.empty())
			continue;
		//	return;
		vector<string> playersTypes;
		vector<double> costsOneLosCntPos;
		computeLosCntPosCost(this, offsPLocSetFld, trainFeaturesMat, trainLabelsMat, costsOneLosCntPos, playersTypes);
		costsAllLosCntPos.push_back(costsOneLosCntPos);
		plsTypesAllLosCntPos.push_back(playersTypes);

		double ufmClrCost = getUniformClrCost(offsPlayers, this);
		clrCostsAllLosCntPos.push_back(ufmClrCost);

//		bestPlayersTypes = playersTypes;
//		bestPlayers = offsPlayers;
//		bestPlayersLocSet = offsPlayersLocSet, bestPLocSetFld = offsPLocSetFld;
	}

	double maxPTypeCost = NEGINF;
	for(unsigned int i = 0; i < costsAllLosCntPos.size(); ++i)
	{
		for(unsigned int j = 0; j < costsAllLosCntPos[i].size(); ++j)
			if(costsAllLosCntPos[i][j] > maxPTypeCost)
				maxPTypeCost = costsAllLosCntPos[i][j];
	}

	double maxOffPsTypesCost = NEGINF, maxOffPsClrCost = NEGINF;
	vector<double> offPsCostAllLosPos;
	for(unsigned int i = 0; i < costsAllLosCntPos.size(); ++i)
	{
		//compensate the missing players (less than 6)
//		while(costsAllLosCntPos[i].size() < 6)
//			costsAllLosCntPos[i].push_back(maxPTypeCost);
		while(costsAllLosCntPos[i].size() < 8)
			costsAllLosCntPos[i].push_back(maxPTypeCost);
		double offPsTypesCost = 0;
		for(unsigned int j = 0; j < costsAllLosCntPos[i].size(); ++j)
			offPsTypesCost += costsAllLosCntPos[i][j];
		offPsCostAllLosPos.push_back(offPsTypesCost);
		if(offPsTypesCost > maxOffPsTypesCost)
			maxOffPsTypesCost = offPsTypesCost;

		if(clrCostsAllLosCntPos[i] > maxOffPsClrCost)
			maxOffPsClrCost = clrCostsAllLosCntPos[i];
	}

	if(offPsCostAllLosPos.size() != clrCostsAllLosCntPos.size())
	{
		cout << "offPsCostAllLosPos.size() != clrCostsAllLosCntPos.size()" << endl;
		return;
	}
	//normalization
	double minTotalCost = INF;
	int minCostLosCntIdx = -1;
	for(unsigned int i = 0; i < offPsCostAllLosPos.size(); ++i)
	{
		offPsCostAllLosPos[i] /= maxOffPsTypesCost;
		clrCostsAllLosCntPos[i] /= maxOffPsClrCost;
		double totalCost = offPsCostAllLosPos[i] + clrCostsAllLosCntPos[i];
//		double totalCost = clrCostsAllLosCntPos[i];
		if(totalCost < minTotalCost)
		{
			minTotalCost = totalCost;
			minCostLosCntIdx = i;
		}
	}

	if(minCostLosCntIdx == -1)
		return;
	rectLosCnt = olLocSet[minCostLosCntIdx];
//	vector<string> bestPlayersTypes = plsTypesAllLosCntPos[minCostLosCntIdx];
//	vector<struct rect> bestPlayers = players;
//	vector<Point2d> bestPlayersLocSet = playersLocSet, bestPLocSetFld = pLocSetFld;
	bestPlayersTypes = plsTypesAllLosCntPos[minCostLosCntIdx];
	bestPlayers = players;
	bestPlayersLocSet = playersLocSet, bestPLocSetFld = pLocSetFld;
	getOffsPlayersByLos(bestPlayersLocSet, bestPLocSetFld, pFeetLocSetFld, this, bestPlayers, offSide);

	vector<Point2d> srcLosVec, dstLosVec;
	srcLosVec.push_back(rectLosBndBox.a);
	srcLosVec.push_back(rectLosBndBox.b);
	srcLosVec.push_back(rectLosBndBox.c);
	srcLosVec.push_back(rectLosBndBox.d);
	srcLosVec.push_back(rectLosCnt);
	perspectiveTransform(srcLosVec, dstLosVec, fldToOrgHMat);
	losBndBox.a = dstLosVec[0];
	losBndBox.b = dstLosVec[1];
	losBndBox.c = dstLosVec[2];
	losBndBox.d = dstLosVec[3];
	losCnt = dstLosVec[4];
	plotRect(mosFrame, losBndBox, Scalar(0, 255, 0));


	for(unsigned int i = 0; i < bestPlayers.size(); ++i)
		plotRect(mosFrame, bestPlayers[i], Scalar(255, 0, 0));

	int fontFace = 0;
	double fontScale = 1;
	int thickness = 2;
	int len = 5;
	for(unsigned int i = 0; i < bestPlayersTypes.size(); ++i)
	{
		if(bestPlayersTypes[i].compare("non") == 0)
			continue;
//		cout << playersTypes[i] << endl;
		putText(mosFrame, bestPlayersTypes[i], bestPlayersLocSet[i], fontFace, fontScale, CV_RGB(0, 0, 255), thickness,8);

		line(mosFrame, bestPlayersLocSet[i] - Point2d(len, 0), bestPlayersLocSet[i] + Point2d(len, 0), CV_RGB(0, 255, 0),2,8,0);
		line(mosFrame, bestPlayersLocSet[i] - Point2d(0, len), bestPlayersLocSet[i] + Point2d(0, len), CV_RGB(0, 255, 0),2,8,0);
	}

	line(mosFrame, losCnt - Point2d(len, 0), losCnt + Point2d(len, 0), CV_RGB(255, 255, 0),2,8,0);
	line(mosFrame, losCnt- Point2d(0, len), losCnt + Point2d(0, len), CV_RGB(255, 255, 0),2,8,0);

	for(unsigned int i = 0; i < bestPlayersTypes.size(); ++i)
	{
		if(bestPlayersTypes[i].compare("non") == 0)
			continue;
//		cout << playersTypes[i] << endl;
		putText(rectMosFrame, bestPlayersTypes[i], bestPLocSetFld[i], fontFace, fontScale, CV_RGB(0, 0, 255), thickness,8);

		line(rectMosFrame, bestPLocSetFld[i] - Point2d(len, 0), bestPLocSetFld[i] + Point2d(len, 0), CV_RGB(0, 255, 0),2,8,0);
		line(rectMosFrame, bestPLocSetFld[i] - Point2d(0, len), bestPLocSetFld[i] + Point2d(0, len), CV_RGB(0, 255, 0),2,8,0);
	}
//	plotLos(rectMosFrame, rectLosBndBox);
	plotRect(rectMosFrame, rectLosBndBox, Scalar(0, 255, 0));
	line(rectMosFrame, rectLosCnt - Point2d(len, 0), rectLosCnt + Point2d(len, 0), CV_RGB(255, 255, 0),2,8,0);
	line(rectMosFrame, rectLosCnt- Point2d(0, len), rectLosCnt + Point2d(0, len), CV_RGB(255, 255, 0),2,8,0);

//	string formImagePath = "Results/Game" + gameIdStr + "/playersPlots/" + gameIdStr +"0" + vidIdxStr + ".jpg";
//	imwrite(formImagePath, mosFrame);
//	string formImageRectPath = "Results/Game" + gameIdStr + "/playersPlots/" + gameIdStr +"0" + vidIdxStr + "Rect.jpg";
//	imwrite(formImageRectPath, rectMosFrame);

}

void play::getLosBndBoxByUfmClr()
{
	Mat orgToFldHMat;
	rctfWithoutDetectLos(orgToFldHMat);
	Mat fldToOrgHMat;
	getOverheadFieldHomo(fldToOrgHMat);

	string playersFilePath = "playerBndBoxes/Game" + gameIdStr + "/" + gameIdStr +"0" + vidIdxStr + ".players";
	vector<double> scores;
	vector<struct rect> players;
	vector<double> areas;
	readPlayerBndBoxes(playersFilePath, scores, players, areas);

	vector<Point2d> playersLocSet, pLocSetFld;
	for(unsigned int i = 0; i < players.size(); ++i)
	{
		Point2d p = Point2d(0.5 * (players[i].a.x + players[i].c.x),
				0.5 * (players[i].a.y + players[i].c.y) );
		Point2d feetP = Point2d(0.5 * (players[i].b.x + players[i].c.x),
				0.5 * (players[i].b.y + players[i].c.y) );
		playersLocSet.push_back(p);
		plotRect(mosFrame, players[i], Scalar(0, 0, 255));
	}
	perspectiveTransform(playersLocSet, pLocSetFld, orgToFldHMat);

	vector<struct rect> insDPlayers = players;
	vector<Point2d> insDPlayersLocSet = playersLocSet,
			insDPLocSetFld = pLocSetFld;
	getPlayersInsdFld(insDPlayersLocSet, insDPLocSetFld, this, insDPlayers);
	if(insDPLocSetFld.empty())
	{
		cout << "No players inside field!" << endl;
		return;
	}

	Mat samples(insDPlayers.size(), 3, CV_32F);
	for(unsigned int i = 0; i < insDPlayers.size(); ++i)
	{
//		plotRect(mosFrame, players[i], Scalar(0, 0, 255));
		Point3d avgClr;
		getRectAvgClr(mosFrame, insDPlayers[i], avgClr);
		samples.at<float>(i, 0) = avgClr.x;
		samples.at<float>(i, 1) = avgClr.y;
		samples.at<float>(i, 2) = avgClr.z;
	}

	int K = 2;
	Mat labels;
	int attempts = 5;
	Mat centers;
	kmeans(samples, K, labels, TermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 10000, 0.0001), attempts, KMEANS_PP_CENTERS, centers);

	int fontFace = 0;
	double fontScale = 1;
	int thickness = 2;

	int step = 5;
	int maxUfmClrScore = NEGINF;
	vector<int> allUfmClrScores;
	for(int p = fld->endZoneWidth; p < (fld->fieldWidth - fld->endZoneWidth); p += step)
	{
		int leftClst1Num = 0, leftClst2Num = 0;
		int rightClst1Num = 0, rightClst2Num = 0;
		for(unsigned int i = 0; i < insDPLocSetFld.size(); ++i)
		{
			ostringstream convertLabel;
			convertLabel << labels.at<int>(i, 0);
			string labelStr = convertLabel.str();
			putText(mosFrame, labelStr, insDPlayers[i].a, fontFace, fontScale, CV_RGB(0, 0, 255), thickness,8);
			putText(rectMosFrame, labelStr, insDPLocSetFld[i], fontFace, fontScale, CV_RGB(0, 0, 255), thickness,8);

			if(labels.at<int>(i, 0) == 0)
			{
				if(insDPLocSetFld[i].x < p)
					++leftClst1Num;
				else if(insDPLocSetFld[i].x > p)
					++rightClst1Num;
			}
			else if(labels.at<int>(i, 0) == 1)
			{
				if(insDPLocSetFld[i].x < p)
					++leftClst2Num;
				else if(insDPLocSetFld[i].x > p)
					++rightClst2Num;
			}
			else
			{
				cout << "Non-existing cluster!" << endl;
			}
		}

		int clrScore = abs(leftClst1Num - leftClst2Num) + abs(rightClst1Num - rightClst2Num);
		allUfmClrScores.push_back(clrScore);
		if(clrScore > maxUfmClrScore)
		{
			maxUfmClrScore = clrScore;
		}
	}

//	vector<int> maxScoreXPos;
	int minX = exp(20) , maxX = -1 * exp(20);
	for(int p = fld->endZoneWidth; p < (fld->fieldWidth - fld->endZoneWidth); p += step)
	{
//		cout << allUfmClrScores[(p - fld->endZoneWidth) / 5] << endl;
		if(allUfmClrScores[(p - fld->endZoneWidth) / step] == maxUfmClrScore)
		{
//			cout << p << endl;
//			maxScoreXPos.push_back(p);

			line(rectMosFrame, Point2d(p, 0), Point2d(p, fld->fieldLength), Scalar(0, 255, 0),2,8,0);
			if(p < minX)
				minX = p;
			if(p > maxX)
				maxX = p;
		}
	}
//	cout << "minX: " << minX << endl;
//	cout << "INF: " << (10000 < INF) << endl;
	rectLosBndBox.a = Point2d(minX, fld->hashToSideLineDist);
	rectLosBndBox.b = Point2d(minX, fld->fieldLength - fld->hashToSideLineDist);
	rectLosBndBox.c = Point2d(maxX, fld->fieldLength - fld->hashToSideLineDist);
	rectLosBndBox.d = Point2d(maxX, fld->hashToSideLineDist);

	vector<Point2d> srcLosVec, dstLosVec;
	srcLosVec.push_back(rectLosBndBox.a);
	srcLosVec.push_back(rectLosBndBox.b);
	srcLosVec.push_back(rectLosBndBox.c);
	srcLosVec.push_back(rectLosBndBox.d);
	perspectiveTransform(srcLosVec, dstLosVec, fldToOrgHMat);
	losBndBox.a = dstLosVec[0];
	losBndBox.b = dstLosVec[1];
	losBndBox.c = dstLosVec[2];
	losBndBox.d = dstLosVec[3];

	plotRect(mosFrame, losBndBox, Scalar(255, 255, 0));
	plotRect(rectMosFrame, rectLosBndBox, Scalar(255, 255, 0));
	string formImagePath = "Results/Game" + gameIdStr + "/losPlots/" + gameIdStr +"0" + vidIdxStr + ".jpg";
	imwrite(formImagePath, mosFrame);
	string formImageRectPath = "Results/Game" + gameIdStr + "/losPlots/" + gameIdStr +"0" + vidIdxStr + "Rect.jpg";
	imwrite(formImageRectPath, rectMosFrame);

}

void play::getLosBndBoxByClrAndFg()
{
	Mat orgToFldHMat;
	rctfWithoutDetectLos(orgToFldHMat);
	Mat fldToOrgHMat;
	getOverheadFieldHomo(fldToOrgHMat);

//	string playersFilePath = "playerBndBoxes/Game" + gameIdStr + "/" + gameIdStr +"0" + vidIdxStr + ".players";
//	vector<double> scores;
//	vector<struct rect> players;
//	vector<double> areas;
//	readPlayerBndBoxes(playersFilePath, scores, players, areas);
//
//	vector<Point2d> playersLocSet, pLocSetFld;
//	for(unsigned int i = 0; i < players.size(); ++i)
//	{
//		Point2d p = Point2d(0.5 * (players[i].a.x + players[i].c.x),
//				0.5 * (players[i].a.y + players[i].c.y) );
//		Point2d feetP = Point2d(0.5 * (players[i].b.x + players[i].c.x),
//				0.5 * (players[i].b.y + players[i].c.y) );
//		playersLocSet.push_back(p);
//		plotRect(mosFrame, players[i], Scalar(0, 0, 255));
//	}
//	perspectiveTransform(playersLocSet, pLocSetFld, orgToFldHMat);
//
//	vector<struct rect> insDPlayers = players;
//	vector<Point2d> insDPlayersLocSet = playersLocSet,
//			insDPLocSetFld = pLocSetFld;
//	getPlayersInsdFld(insDPlayersLocSet, insDPLocSetFld, this, insDPlayers);
//	if(insDPLocSetFld.empty())
//	{
//		cout << "No players inside field!" << endl;
//		return;
//	}
//
//	Mat samples(insDPlayers.size(), 3, CV_32F);
//	for(unsigned int i = 0; i < insDPlayers.size(); ++i)
//	{
////		plotRect(mosFrame, players[i], Scalar(0, 0, 255));
//		Point3d avgClr;
//		getRectAvgClr(mosFrame, insDPlayers[i], avgClr);
//		samples.at<float>(i, 0) = avgClr.x;
//		samples.at<float>(i, 1) = avgClr.y;
//		samples.at<float>(i, 2) = avgClr.z;
//	}
//
//	int K = 2;
//	Mat labels;
//	int attempts = 5;
//	Mat centers;
//	kmeans(samples, K, labels, TermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 10000, 0.0001), attempts, KMEANS_PP_CENTERS, centers);
//
//	int fontFace = 0;
//	double fontScale = 1;
//	int thickness = 2;
//
	int step = 5;
//	int maxUfmClrScore = NEGINF;
//	vector<int> allUfmClrScores;
//	for(int p = fld->endZoneWidth; p < (fld->fieldWidth - fld->endZoneWidth); p += step)
//	{
//		int leftClst1Num = 0, leftClst2Num = 0;
//		int rightClst1Num = 0, rightClst2Num = 0;
//		for(unsigned int i = 0; i < insDPLocSetFld.size(); ++i)
//		{
//			ostringstream convertLabel;
//			convertLabel << labels.at<int>(i, 0);
//			string labelStr = convertLabel.str();
//			putText(mosFrame, labelStr, insDPlayers[i].a, fontFace, fontScale, CV_RGB(0, 0, 255), thickness,8);
//			putText(rectMosFrame, labelStr, insDPLocSetFld[i], fontFace, fontScale, CV_RGB(0, 0, 255), thickness,8);
//
//			if(labels.at<int>(i, 0) == 0)
//			{
//				if(insDPLocSetFld[i].x < p)
//					++leftClst1Num;
//				else if(insDPLocSetFld[i].x > p)
//					++rightClst1Num;
//			}
//			else if(labels.at<int>(i, 0) == 1)
//			{
//				if(insDPLocSetFld[i].x < p)
//					++leftClst2Num;
//				else if(insDPLocSetFld[i].x > p)
//					++rightClst2Num;
//			}
//			else
//			{
//				cout << "Non-existing cluster!" << endl;
//			}
//		}
//
//		int clrScore = abs(leftClst1Num - leftClst2Num) + abs(rightClst1Num - rightClst2Num);
//		allUfmClrScores.push_back(clrScore);
//		if(clrScore > maxUfmClrScore)
//		{
//			maxUfmClrScore = clrScore;
//		}
//	}
//
////	vector<int> maxScoreXPos;
//	int minX = exp(20) , maxX = -1 * exp(20);
//	for(int p = fld->endZoneWidth; p < (fld->fieldWidth - fld->endZoneWidth); p += step)
//	{
////		cout << allUfmClrScores[(p - fld->endZoneWidth) / 5] << endl;
//		if(allUfmClrScores[(p - fld->endZoneWidth) / step] == maxUfmClrScore)
//		{
////			cout << p << endl;
////			maxScoreXPos.push_back(p);
//
//			line(rectMosFrame, Point2d(p, 0), Point2d(p, fld->fieldLength), Scalar(255, 0, 0),2,8,0);
//			if(p < minX)
//				minX = p;
//			if(p > maxX)
//				maxX = p;
//		}
//	}
	int midX = 0.5 * fld->fieldWidth;
	int searchWidth = 0.5 * fld->fieldWidth - fld->endZoneWidth;
//	int midX = (minX + maxX) * 0.5;
//	int searchWidth = fld->yardLinesDist * 0.5;
	vector<double> clrDifVec, areaRatioVec;
	for(int w = midX - searchWidth; w <= midX + searchWidth; w += step)
		for(int l = fld->hashToSideLineDist; l <= fld->fieldLength - fld->hashToSideLineDist; l += step)
		{
			//color difference
//			Rect r(w - fld->yardLinesDist, l - fld->yardLinesDist,
//					2 * fld->yardLinesDist, 2 * fld->yardLinesDist);
			Rect r(w - 0.25 * fld->yardLinesDist, l - fld->yardLinesDist,
					0.5 * fld->yardLinesDist, 2 * fld->yardLinesDist);
//			double clrDif = getColorDif(r, rectMosFrame);
			double clrDif = getOrgImgColorDif(r, mosFrame, fldToOrgHMat);
	//		cout << "clrDif: " << clrDif << endl;
			clrDifVec.push_back(clrDif);
			double areaRatio = getFgAreaRatio(r, rectImage);
			areaRatioVec.push_back(areaRatio);
//			if(areaRatio != 0)
//				cout << "areaRatio: " << areaRatio << endl;
		}
//	imshow("rectImg", rectImage);
//	cvWaitKey(15);
	double maxClr = NEGINF, maxArea = NEGINF;
	for(unsigned int i = 0; i < clrDifVec.size(); ++i)
	{
		if(clrDifVec[i] > maxClr)
			maxClr = clrDifVec[i];
		if(areaRatioVec[i] > maxArea)
			maxArea = areaRatioVec[i];
	}
	double maxClrArea = NEGINF;
	int idx = 0;
	for(int w = midX - searchWidth; w <= midX + searchWidth; w += step)
		for(int l = fld->hashToSideLineDist; l <= fld->fieldLength - fld->hashToSideLineDist; l += step)
	{
		double scr = clrDifVec[idx] / maxClr + areaRatioVec[idx] / maxArea;
//		double scr = areaRatioVec[idx] / maxArea;
		if(scr > maxClrArea)
		{
//			rectLosBndBox.a = Point2d(w - 0.25 * fld->yardLinesDist, l - fld->yardLinesDist);
//			rectLosBndBox.b = Point2d(w - 0.25 * fld->yardLinesDist, l + fld->yardLinesDist);
//			rectLosBndBox.c = Point2d(w + 0.25 * fld->yardLinesDist, l + fld->yardLinesDist);
//			rectLosBndBox.d = Point2d(w + 0.25 * fld->yardLinesDist, l - fld->yardLinesDist);
			rectLosBndBox.a = Point2d(w, l - fld->yardLinesDist);
			rectLosBndBox.b = Point2d(w, l + fld->yardLinesDist);
			rectLosBndBox.c = Point2d(w, l + fld->yardLinesDist);
			rectLosBndBox.d = Point2d(w, l - fld->yardLinesDist);
			maxClrArea = scr;
		}
		++idx;
	}

//	rectLosBndBox.a = Point2d(minX, fld->hashToSideLineDist);
//	rectLosBndBox.b = Point2d(minX, fld->fieldLength - fld->hashToSideLineDist);
//	rectLosBndBox.c = Point2d(maxX, fld->fieldLength - fld->hashToSideLineDist);
//	rectLosBndBox.d = Point2d(maxX, fld->hashToSideLineDist);


	vector<Point2d> srcLosVec, dstLosVec;
	srcLosVec.push_back(rectLosBndBox.a);
	srcLosVec.push_back(rectLosBndBox.b);
	srcLosVec.push_back(rectLosBndBox.c);
	srcLosVec.push_back(rectLosBndBox.d);
	perspectiveTransform(srcLosVec, dstLosVec, fldToOrgHMat);
	losBndBox.a = dstLosVec[0];
	losBndBox.b = dstLosVec[1];
	losBndBox.c = dstLosVec[2];
	losBndBox.d = dstLosVec[3];

	plotRect(mosFrame, losBndBox, Scalar(0, 255, 0));
	plotRect(rectMosFrame, rectLosBndBox, Scalar(0, 255, 0));
	string formImagePath = "Results/Game" + gameIdStr + "/losPlots/" + gameIdStr +"0" + vidIdxStr + ".jpg";
	imwrite(formImagePath, mosFrame);
	string formImageRectPath = "Results/Game" + gameIdStr + "/losPlots/" + gameIdStr +"0" + vidIdxStr + "Rect.jpg";
	imwrite(formImageRectPath, rectMosFrame);
}

void play::inferMissingPlayers(direction offSide, const Mat &trainFeaturesMat, const Mat &trainLabelsMat,
		const vector<vector<Point2d> > &pToLosVecAllPlays, const vector<vector<int> > &pTypesIdAllPlays)
{
	vector<string> bestPlayersTypes;
	vector<struct rect> bestPlayers;
	vector<Point2d> bestPlayersLocSet, bestPLocSetFld;
	Mat fldToOrgHMat, orgToFldHMat;
	lablePTypesKnnVarLosCnts(offSide, trainFeaturesMat, trainLabelsMat,
			bestPlayersTypes, bestPlayers, bestPlayersLocSet, bestPLocSetFld, fldToOrgHMat, orgToFldHMat);
	vector<int> bestPTypesId;
	for(unsigned int i = 0; i < bestPlayersTypes.size(); ++i)
	{
		int pTId = convertPTypeToPId(bestPlayersTypes[i]);
		bestPTypesId.push_back(pTId);
	}

	double minFormCost = INF;
	int closestFormId = -1;
	vector<bool> missIndicator;
	for(unsigned int i = 0; i < pTypesIdAllPlays.size(); ++i)
	{
		double formCost = 0;
		vector<bool> ms(pTypesIdAllPlays[i].size(), true);

		for(unsigned int j = 0; j < bestPTypesId.size(); ++j)
		{
			double minPlayerCost = exp(20);
			int closestPId = -1;
			for(unsigned int k = 0; k < pTypesIdAllPlays[i].size(); ++k)
			{
//				cout << "pTypesIdAllPlays[i][k] " << pTypesIdAllPlays[i][k] << endl;
//				cout << "bestPTypesId[j] " << bestPTypesId[j] << endl;
				if(pTypesIdAllPlays[i][k] != bestPTypesId[j])
					continue;
//				cout << "##################################################" << endl;
				double pCost = norm(pToLosVecAllPlays[i][k] - (bestPLocSetFld[j] - rectLosCnt));
//				cout << "pCost" << pCost << endl;
				if(pCost < minPlayerCost)
				{
					minPlayerCost = pCost;
					closestPId = k;
				}
//				ms[k] = false;
				//closestPId = k;
			}
			if(closestPId != -1)
			{
				ms[closestPId] = false;
				//cout << "##################################################" << endl;
			}
//			else
//				cout << "##################################################" << endl;
			formCost += minPlayerCost;
		}

		if(formCost < minFormCost)
		{
			minFormCost = formCost;
			closestFormId = i;
			missIndicator = ms;
		}
	}

	vector<Point2d> srcVec, dstVec;
	for(unsigned int i = 0; i < pToLosVecAllPlays[closestFormId].size(); ++i)
	{
		Point2d playerPosFld = rectLosCnt + pToLosVecAllPlays[closestFormId][i];
		srcVec.push_back(playerPosFld);
	}
	perspectiveTransform(srcVec, dstVec, fldToOrgHMat);

	int fontFace = 0;
	double fontScale = 1;
	int thickness = 2;

	string kltFilePath = "Tracks/Game" + gameIdStr + "/video" + vidIdxStr + ".tracks";
	vector<track> trks;
	readKltTracks(kltFilePath, trks);

	for(unsigned int i = 0; i < missIndicator.size(); ++i)
	{
//		Point2d playerPosFld = rectLosCnt + pToLosVecAllPlays[closestFormId][i];
		string pTypeStr = convertPIdToPType(pTypesIdAllPlays[closestFormId][i]);
		Scalar clr;
		bool isPlayerByTrks = false;
		if(!missIndicator[i])
			clr = CV_RGB(255, 0, 0);
		else
		{
			clr = CV_RGB(0, 255, 0);
			isPlayerByTrks = isPlayerByKltTracks(trks, srcVec[i], orgToFldHMat);
		}

		if(missIndicator[i])
		{
			putText(rectMosFrame, pTypeStr, srcVec[i], fontFace, fontScale, clr, thickness,8);
			circle(rectMosFrame, srcVec[i], 30, clr, thickness);
			putText(mosFrame, pTypeStr, dstVec[i], fontFace, fontScale, clr, thickness,8);
			circle(mosFrame, dstVec[i], 30, clr, thickness);
		}
		if(isPlayerByTrks)
		{
//			circle(rectMosFrame, srcVec[i], 50, clr, thickness);
//			circle(mosFrame, dstVec[i], 50, clr, thickness);
			rectangle(rectMosFrame, srcVec[i] - Point2d(30, 30), srcVec[i] + Point2d(30, 30), clr, thickness);
			rectangle(mosFrame, dstVec[i] - Point2d(30, 30), dstVec[i] + Point2d(30, 30), clr, thickness);
		}

	}
	ostringstream convertVidId;
	convertVidId << closestFormId + 1 ;
	string vidId = convertVidId.str();

	putText(rectMosFrame, vidId, Point2d(30, 30), fontFace, fontScale, CV_RGB(255, 0, 0), thickness,8);
	putText(mosFrame, vidId, Point2d(30, 30), fontFace, fontScale, CV_RGB(255, 0, 0), thickness,8);

	drawKlt(orgToFldHMat);
	string formImagePath = "Results/Game" + gameIdStr + "/playersPlots/" + gameIdStr +"0" + vidIdxStr + ".jpg";
	imwrite(formImagePath, mosFrame);
	string formImageRectPath = "Results/Game" + gameIdStr + "/playersPlots/" + gameIdStr +"0" + vidIdxStr + "Rect.jpg";
	imwrite(formImageRectPath, rectMosFrame);
}

void play::drawKlt(Mat &orgToFldHMat)
{
	string kltFilePath = "Tracks/Game" + gameIdStr + "/video" + vidIdxStr + ".tracks";
	vector<track> trks;
	readKltTracks(kltFilePath, trks);
	drawKltTracks(this, trks, orgToFldHMat);
}

