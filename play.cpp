#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <limits>

#include "play.h"
#include "blob_HUDL.h"
#include "ellipse.h"
#include "imgRectification.h"
#include "playAuxFunc.h"
#include "matchImgs.h"
#include "fieldModel.h"
#include "playerType.h"
#include "formTree.h"

//int fld->fieldLength;
//int fld->fieldWidth;
//int fld->endZoneWidth;
//int fld->yardLinesDist;
//int fld->hashLinesDist;
//int fld->hashToSideLineDist;
//int fld->hashLinesLen;


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

//	plotYardLnsAndLos(fgImage, losBndBox, yardLines, losLine);
//
//	plotPath = "Results/Game" + gameIdStr + "/plots/vid" + vidIdxStr + ".jpg";
//
//	imwrite(plotPath, fgImage);
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

//	RNG rng(12345);
//	for(unsigned int i = 0; i < ellipses.size(); ++i)
//	{
//		Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
//		ellipse(fgImage, ellipses[i], color, 2, 8 );
//	}
//
//	string eliOutputImg = "ellipses/Game" + gameIdStr + "/video0" + vidIdxStr +".bmp";
//	imwrite(eliOutputImg, fgImage);

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
	string fgImgPath = "rectImages/Game" + gameIdStr + "/vid" + vidIdxStr + "RectFg.jpg";
	imwrite(fgImgPath, rectFgImgTmp);
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

//	string panoPath = "panorama/panoGame" + gameIdStr + ".jpg";
//	Mat pano = imread(panoPath.c_str(), CV_LOAD_IMAGE_COLOR);
//	string homMatPath = "homog/Game" + gameIdStr + "/" + vidIdxStr + ".txt";
//	vector<Mat> homoList = readHomographs(homMatPath);
//	Mat bgOrig;
//	warpPerspective(pano, bgOrig, homoList[mos - 1].inv(), mosFrame.size());

//	string bgStr;
//	int bgVidId = pId.vidId;
//	ostringstream convertVidId;
//	convertVidId << bgVidId;
//	bgStr = convertVidId.str();
//	if(bgVidId < 10)
//		bgStr = "00" + bgStr;
//	else if(pId.vidId < 100 )
//		bgStr = "0" + bgStr;

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

	string formsFile = "formModel/formations";
	vector<string> formations;
	readFormsFile(formsFile, offSide, formations);

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

	getOffensePlayers(playersLocSet, pLocSetFld, this, players, offSide);

//	for(unsigned int i = 0; i < players.size(); ++i)
//		plotRect(mosFrame, players[i], Scalar(255, 0, 0));

//	vector<double> offenseScores;
//	for(unsigned int i = 0; i < pLocSetFld.size(); ++i)
//	{
//		bool offense = false;
//		if(offSide == leftDir && pLocSetFld[i].x <= rectLosCnt.x)
//			offense = true;
//		else if(offSide == rightDir && pLocSetFld[i].x >= rectLosCnt.x)
//			offense = true;
//
//		if(offense && fld->isPointInsideFld(pLocSetFld[i]))
//		{
//			offensePLocSetFld.push_back(pLocSetFld[i]);
//			offenseScores.push_back(scores[i]);
//		}
//	}
//	cout << "pLocSetFld " << pLocSetFld.size() << endl;
//	cout << "offensePLocSetFld " << offensePLocSetFld.size() << endl;
//	cout << "offenseScores " << offenseScores.size() << endl;
	double bestFormScore = NEGINF;
	formTree* bestForm;
//	for(unsigned int i = 0; i < formations.size(); ++i)
//	{
//		formTree fTree(formations[i]);
//		fTree.setupPartsLocSetStarModel(rectLosCnt, offensePLocSetFld, offenseScores);
//		fTree.findBestFormStarModel();
//		if(fTree.formBestScore >= bestFormScore)
//		{
//			bestFormScore = fTree.formBestScore;
//			bestForm = &fTree;
////			bestForm->plotFormOrigImg(mosFrame, fldToOrgHMat);
////			drawPlayerBndBoxes();
//		}
//	}
	vector<formTree*> fTrees;
	for(unsigned int i = 0; i < formations.size(); ++i)
	{
		formTree* f = new formTree(formations[i]);
//		f->setupPartsLocSetStarModel(rectLosCnt, offensePLocSetFld, offenseScores);
		f->setupPartsLocSetStarModel(rectLosCnt, pLocSetFld, scores);
		f->findBestFormStarModel();
		if(f->formBestScore >= bestFormScore)
		{
			bestFormScore = f->formBestScore;
			bestForm = f;
			fTrees.push_back(f);
//			bestForm->plotFormOrigImg(mosFrame, fldToOrgHMat);
//			drawPlayerBndBoxes();
		}
	}


	cout << bestForm->formName << bestForm->formBestScore << endl;

	bestForm->plotFormOrigImg(mosFrame, fldToOrgHMat);
//	drawPlayerBndBoxes();
	string playersImagePath = "Results/Game" + gameIdStr + "/playersPlots/" + gameIdStr +"0" + vidIdxStr + ".jpg";
	imwrite(playersImagePath, mosFrame);

}
