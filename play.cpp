#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <limits>


#include "topOrdShortestP.h"
#include "play.h"
#include "wrPicStrModel.h"
#include "playerBndBox.h"
#include "formation.h"
#include "blob_HUDL.h"
#include "ellipse.h"
#include "imgRectification.h"


play::play(struct playId p)
{
	pId.gameId = p.gameId;
	pId.vidId = p.vidId;
	mos = -1;
	yardLnsDist = -1.0;

	preDir = nonDir;
	trueDir = nonDir;

	vpExist = false;

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
	scrimLnsKltFilePath = "scrimLine/Game" + gameIdStr + "/scrimLinesPredMos";
	scrimLnsGradFilePath = "scrimLine/Game" + gameIdStr + "/losPredMos/video0" + vidIdxStr + ".glos4";
	trksFilePath = "TracksPredMos/Game" + gameIdStr + "/Game" + gameIdStr + "_video0" + vidIdxStr;
#elif predMos == 0
	mosFilePath = "Mos/Game" + gameIdStr  + "/mos_id_new.txt";
	scrimLnsKltFilePath = "scrimLine/Game" + gameIdStr + "/scrimLines";
	scrimLnsGradFilePath = "scrimLine/Game" + gameIdStr + "/losTrueMos/video0" + vidIdxStr + ".glos4";
	trksFilePath = "TracksCamFlt/Game" + gameIdStr + "/Game" + gameIdStr + "_video0" + vidIdxStr;
#endif


#if yardLinesMethod == 1
	yardLnsFilePath = "yardLines/Game" +  gameIdStr + "/LinesAllFrames/video0" +  vidIdxStr +".line_new";
#elif yardLinesMethod == 2
	yardLnsFilePath = "yardLines/Game" +  gameIdStr + "/newLinesAllFrames/video0" +  vidIdxStr +".line_new";
#endif

	//plotPath = "Results/Game" + gameIdStr + "/crtPlots/vid" + vidIdxStr + ".jpg";

	vpFilePath = "yardLines/Game" + gameIdStr + "/LinesAllFrames/video0" + vidIdxStr +".lineVp_new";
	trueDirFilePath = "ODKGt/Game" + gameIdStr + "/game" + gameIdStr + "_wr_new";

	//Game02_video0003.bmp
	//randTreesTrainData/
	fgImgPath = "fgMosImages/Game" + gameIdStr + "/video0" + vidIdxStr +".bmp";
	//fgImgPath = "randTreesTrainData/Game" + gameIdStr + "/annotMosFgImages/video0" + vidIdxStr +".bmp";
	videoPath = "../videos/Game" + gameIdStr + "/Game" + gameIdStr + "_video0" + vidIdxStr +".avi";
	mosImgPath = "mosImages/Game" + gameIdStr + "/vid" + vidIdxStr + ".jpg";
	image = imread(fgImgPath.c_str(), CV_LOAD_IMAGE_COLOR);
	mosFrame = imread(mosImgPath.c_str(), CV_LOAD_IMAGE_COLOR);
//	image.create(imgYLen, imgXLen, CV_32FC3 );
//	image = cv::Scalar(0,0,0);

	form = NULL;

	return;
}

play::~play()
{
	//imwrite(plotPath, image);
	image.release();
	if(form != NULL)
		delete form;
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


double distFromPntToLine(Point2d pt0, struct yardLine yLine)
{
	float rho = yLine.rho;
	float theta = yLine.theta;
	Point2d pt1, pt2;
	double cosTheta = cos(theta), sinTheta = sin(theta);
	//double a = -cosTheta / sinTheta, b = rho / sinTheta;
	double x0 = cosTheta*rho, y0 = sinTheta*rho;
	pt1.x = x0 + 1000*(-sinTheta);
	pt1.y = y0 + 1000*(cosTheta);
	pt2.x = x0 - 1000*(-sinTheta);
	pt2.y = y0 - 1000*(cosTheta);
	double dist = ((pt2.x - pt1.x) * (pt1.y - pt0.y) * 1.0 - (pt1.x - pt0.x) * (pt2.y - pt1.y) * 1.0)
			/ sqrt((pt2.x - pt1.x) * (pt2.x - pt1.x) * 1.0 + (pt2.y - pt1.y) * (pt2.y - pt1.y) * 1.0);
	//cout<<"dist: "<<dist<<endl;
	return dist;
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

double closestLnDist(vector<struct yardLine> lines)
{
	sort(lines.begin(), lines.end(), compLns);
	double minDist = 100000;
	for(unsigned int i = 0; i < lines.size() - 1; ++i)
	{
		float rho = lines[i].rho;
		float theta = lines[i].theta;
		double cosTheta = cos(theta), sinTheta = sin(theta);

		float rhoNextLn = lines[i + 1].rho;
		float thetaNextLn = lines[i + 1].theta;
		double cosThetaNextLn = cos(thetaNextLn), sinThetaNextLn = sin(thetaNextLn);

		if(cosTheta != 0)
		{
			double x = (rho - 240 * sinTheta) / cosTheta;
			double xNextLn = (rhoNextLn - 240 * sinThetaNextLn) / cosThetaNextLn;
//			double dist = abs(distFromPntToLine(Point2d(x, 240), lines[i+1]));
			double dist = norm(Point2d(x, 240) - Point2d(xNextLn, 240));
			if(dist < minDist)
				minDist = dist;
		}
	}
	if(minDist > 150)
		minDist = 150;
	if(minDist < 90)
		minDist = 90;
	return minDist;
}


double avgLnDist(vector<struct yardLine> lines)
{
	sort(lines.begin(), lines.end(), compLns);
	double minDist = 100000;
	for(unsigned int i = 0; i < lines.size() - 1; ++i)
	{
		float rho = lines[i].rho;
		float theta = lines[i].theta;
		double cosTheta = cos(theta), sinTheta = sin(theta);

		float rhoNextLn = lines[i + 1].rho;
		float thetaNextLn = lines[i + 1].theta;
		double cosThetaNextLn = cos(thetaNextLn), sinThetaNextLn = sin(thetaNextLn);

		if(cosTheta != 0)
		{
			double x = (rho - 240 * sinTheta) / cosTheta;
			double xNextLn = (rhoNextLn - 240 * sinThetaNextLn) / cosThetaNextLn;
//			double dist = abs(distFromPntToLine(Point2d(x, 240), lines[i+1]));
			double dist = norm(Point2d(x, 240) - Point2d(xNextLn, 240));
			if(dist < minDist)
				minDist = dist;
		}
	}
//	if(minDist > 150)
//		minDist = 150;
//	if(minDist < 90)
//		minDist = 90;

	double avgDist = 0.0;
	int distNum = 0;
	for(unsigned int i = 0; i < lines.size() - 1; ++i)
	{
		float rho = lines[i].rho;
		float theta = lines[i].theta;
		double cosTheta = cos(theta), sinTheta = sin(theta);

		float rhoNextLn = lines[i + 1].rho;
		float thetaNextLn = lines[i + 1].theta;
		double cosThetaNextLn = cos(thetaNextLn), sinThetaNextLn = sin(thetaNextLn);

		if(cosTheta != 0)
		{
			double x = (rho - 240 * sinTheta) / cosTheta;
			double xNextLn = (rhoNextLn - 240 * sinThetaNextLn) / cosThetaNextLn;
//			double dist = abs(distFromPntToLine(Point2d(x, 240), lines[i+1]));
			double dist = norm(Point2d(x, 240) - Point2d(xNextLn, 240));
			if(dist <= 3.0 * minDist)
			{
				avgDist += dist;
				++distNum;
			}
		}
	}

	avgDist /= distNum;
	if(avgDist > 250)
		avgDist = 250;
//	if(avgDist < 50)
//		avgDist = 50;
	if(avgDist < 90)
		avgDist = 90;

	//cout<<"avgDist "<<avgDist<<endl;
	return avgDist;
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

bool readScrimLines(string filePath, vector<rect>& scrimLns)
{
	ifstream fin(filePath.c_str());
	if(!fin.is_open())
	{
		cout<<"Can't open file "<<filePath<<endl;
		return false;
	}

	fin.seekg(0, ios::end);
	if (fin.tellg() == 0) {
		cout<<"Empty line file"<<filePath<<endl;
		return false;
	}

	fin.seekg(0, ios::beg);

	while(!fin.eof())
	{
		struct rect r;
		fin >> r.vidId;
		fin >> r.a.x >> r.a.y;
		fin >> r.b.x >> r.b.y;
		fin >> r.c.x >> r.c.y;
		fin >> r.d.x >> r.d.y;

		scrimLns.push_back(r);
	}

	fin.close();

	return true;
}

int findScrimLine(vector<rect> srimLns, int vidIndex)
{
	for(unsigned int i = 0; i < srimLns.size(); ++i)
	{
		if(srimLns[i].vidId == vidIndex)
			return i;
		if(srimLns[i].vidId > vidIndex)
			return -1;
	}
	return -1;
}

void play::getScrimLn()
{
	vector<rect> scrimLns;
	if(readScrimLines(scrimLnsKltFilePath, scrimLns))
	{
		int sLnIdx = findScrimLine(scrimLns, pId.vidId);
		if(sLnIdx == -1)
		{
			cout << "Cann't find scrimmage line" << endl;
			scrimLn.a = Point2d(double(imgXLen) / 2, double(imgYLen) / 2);
			scrimLn.b = Point2d(double(imgXLen) / 2, double(imgYLen) / 2);
			scrimLn.c = Point2d(double(imgXLen) / 2, double(imgYLen) / 2);
			scrimLn.d = Point2d(double(imgXLen) / 2, double(imgYLen) / 2);
			//return;
		}
		else
			scrimLn = scrimLns[sLnIdx];
	}
	else
	{
		scrimLn.a = Point2d(double(imgXLen) / 2, double(imgYLen) / 2);
		scrimLn.b = Point2d(double(imgXLen) / 2, double(imgYLen) / 2);
		scrimLn.c = Point2d(double(imgXLen) / 2, double(imgYLen) / 2);
		scrimLn.d = Point2d(double(imgXLen) / 2, double(imgYLen) / 2);
	}

	scrimCnt = (scrimLn.a + scrimLn.b + scrimLn.c + scrimLn.d) * 0.25;

	if(scrimCnt.y < imgYLen * 0.3)
		scrimCnt.y = imgYLen * 0.3;
	else if(scrimCnt.y > imgYLen * 0.7)
		scrimCnt.y = imgYLen * 0.7;

	return;
}


void play::getGradientScrimLn()
{
	bool losExist = readGradientLos(scrimLnsGradFilePath, los);

	if(!losExist)
	{
		los[0].x = imgXLen * 0.5;
		los[0].y = 0.0;
		los[1].x = imgXLen * 0.5;;
		los[1].y = imgYLen;
	}

	findLosBndBox();

//	plotYardLnsAndLos(image, scrimLn, yardLines, los);
//
//	plotPath = "Results/Game" + gameIdStr + "/plots/vid" + vidIdxStr + ".jpg";
//
//	imwrite(plotPath, image);
}

void play::findLosBndBox()
{
	Point2d vecLos = los[1] - los[0];
	double vecLosLen = norm(vecLos);
//	cout << los[0].x << " " << los[0].y << endl;
//	cout << los[1].x << " " << los[1].y << endl;
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
		Point2d boxCnt = los[0] - i * scanStep * vecLos;
		struct rect bndBox;
		bndBox.a = boxCnt + 0.5 * vecLosBoxLen * vecLos + 0.5 * vecPerpLosBoxLen * vecPerpLos;
		bndBox.b = boxCnt - 0.5 * vecLosBoxLen * vecLos + 0.5 * vecPerpLosBoxLen * vecPerpLos;
		bndBox.c = boxCnt - 0.5 * vecLosBoxLen * vecLos - 0.5 * vecPerpLosBoxLen * vecPerpLos;
		bndBox.d = boxCnt + 0.5 * vecLosBoxLen * vecLos - 0.5 * vecPerpLosBoxLen * vecPerpLos;

		int fgPixNum = fgPixelsInsideBox(image, bndBox);

		if(fgPixNum >= maxFgPixNum)
		{
			scrimLn = bndBox;
			maxFgPixNum = fgPixNum;
		}

//		scrimLn = bndBox;

//		plotYardLnsAndLos(image, scrimLn, yardLines, los);
//
//		plotPath = "Results/Game" + gameIdStr + "/plots/vid" + vidIdxStr + ".jpg";
//
//		imwrite(plotPath, image);

		//cout << i << " " << fgPixNum << endl;

	}

	scrimCnt = (scrimLn.a + scrimLn.b + scrimLn.c + scrimLn.d) * 0.25;

	if(scrimCnt.y < imgYLen * 0.3)
		scrimCnt.y = imgYLen * 0.3;
	else if(scrimCnt.y > imgYLen * 0.7)
		scrimCnt.y = imgYLen * 0.7;

}

void play::findLosOnRectFg(const Mat &homoMat)
{
	struct rect imgBnd, imgBndRect;
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
	for(int xCnt = EndZoneWidth - 1; xCnt < (FieldWidth - EndZoneWidth); xCnt += 5)
		for(int yCnt = HashToSideLineDist - 1; yCnt < (FieldLength - HashToSideLineDist); yCnt += 5)
		{
			Point2d scanRectCnt = Point2d(xCnt, yCnt);
			if(!isPntInsideRect(scanRectCnt, imgBndRect))
				continue;
			double lowY = yCnt - 2 * YardLinesDist;
			double upY = yCnt + 2 * YardLinesDist;
			double leftX = xCnt - 0.5 * YardLinesDist;
			double rightX = xCnt + 0.5 * YardLinesDist;
			int fgPixelsNum = 0;
			for(int y = lowY; y < upY; ++y)
				for(int x = leftX; x < rightX; ++x)
				{
					if( y < 0 || y >= FieldLength)
						continue;
					if( x < 0 || x >= FieldWidth)
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
				rectScrimCnt = Point2d(xCnt, yCnt);
				rectScrimLn.a = Point2d(leftX, lowY);
				rectScrimLn.b = Point2d(leftX, upY);
				rectScrimLn.c = Point2d(rightX, upY);
				rectScrimLn.d = Point2d(rightX, lowY);
			}
		}
}

bool readTrks(string filePath, vector<track>& trks)
{
	ifstream fin(filePath.c_str());
	if(!fin.is_open())
	{
		cout<<"Can't open file "<<filePath<<endl;
		return false;
	}

	fin.seekg(0, ios::end);
	if (fin.tellg() == 0) {
		cout<<"Empty line file"<<filePath<<endl;
		return false;
	}

	fin.seekg(0, ios::beg);
	//int frame = 1;
	while(!fin.eof())
	{
		struct track trk;
		int s, zero, col;
		//double dist, ydist;
		fin>>s>>trk.start.x>>trk.start.y>>trk.end.x>>trk.end.y
				>>trk.frameStart>>trk.frameEnd>>trk.dist>>zero
				>>trk.ydist>>col;
		trks.push_back(trk);
	}

	fin.close();

	return true;
}

void play::getTracks()
{
	bool trksExist = readTrks(trksFilePath, tracks);
	if(!trksExist)
		cout << "No tracks!" << endl;

	for(unsigned int i = 0; i < tracks.size(); ++i)
	{
		tracks[i].lowRec = false;
		tracks[i].upRec = false;
	}
	return;
}

bool isTrkInsideRect(const struct track &trk, const struct rect &scanRect)
{
	Point2d vecAB2d =  Point2d( scanRect.b.x - scanRect.a.x, scanRect.b.y - scanRect.a.y);
	Point2d vecBC2d =  Point2d( scanRect.c.x - scanRect.b.x, scanRect.c.y - scanRect.b.y);
	Point2d vecCD2d =  Point2d( scanRect.d.x - scanRect.c.x, scanRect.d.y - scanRect.c.y);
	Point2d vecDA2d =  Point2d( scanRect.a.x - scanRect.d.x, scanRect.a.y - scanRect.d.y);

	Point3d vecAB3d = Point3d(vecAB2d.x, vecAB2d.y, 0.0);
	Point3d vecBC3d = Point3d(vecBC2d.x, vecBC2d.y, 0.0);
	Point3d vecCD3d = Point3d(vecCD2d.x, vecCD2d.y, 0.0);
	Point3d vecDA3d = Point3d(vecDA2d.x, vecDA2d.y, 0.0);


	Point2d midPnt = Point2d( trk.start.x + trk.end.x + 0.0, trk.start.y + trk.end.y + 0.0);
	midPnt *= 0.5;

	Point3d crsABAMid =  vecAB3d.cross(Point3d( (midPnt.x - scanRect.a.x), (midPnt.y - scanRect.a.y), 0.0 ));
	Point3d crsBCBMid =  vecBC3d.cross(Point3d( (midPnt.x - scanRect.b.x), (midPnt.y - scanRect.b.y), 0.0 ));
	Point3d crsCDCMid =  vecCD3d.cross(Point3d( (midPnt.x - scanRect.c.x), (midPnt.y - scanRect.c.y), 0.0 ));
	Point3d crsDADMid =  vecDA3d.cross(Point3d( (midPnt.x - scanRect.d.x), (midPnt.y - scanRect.d.y), 0.0 ));

	if(max(max(crsABAMid.z, crsBCBMid.z), max(crsCDCMid.z, crsDADMid.z) ) <= 0)
	{
		//++scanRect.trksNum;
		//cout<<"Inside"<<endl;
		return true;
	}

	return false;
}

bool isTrkInsideBndBox(const struct track &trk, const struct bndBox &bBox, double yardLnsDist)
{
	struct rect scanRect;
	scanRect.a = bBox.leftUpVert;
	scanRect.b = bBox.leftUpVert + Point2d(0.0, bBox.yLength * yardLnsDist);
	scanRect.c = bBox.leftUpVert + Point2d(bBox.xLength * yardLnsDist, bBox.yLength * yardLnsDist);
	scanRect.d = bBox.leftUpVert + Point2d(bBox.xLength * yardLnsDist);

	Point2d vecAB2d =  Point2d( scanRect.b.x - scanRect.a.x, scanRect.b.y - scanRect.a.y);
	Point2d vecBC2d =  Point2d( scanRect.c.x - scanRect.b.x, scanRect.c.y - scanRect.b.y);
	Point2d vecCD2d =  Point2d( scanRect.d.x - scanRect.c.x, scanRect.d.y - scanRect.c.y);
	Point2d vecDA2d =  Point2d( scanRect.a.x - scanRect.d.x, scanRect.a.y - scanRect.d.y);

	Point3d vecAB3d = Point3d(vecAB2d.x, vecAB2d.y, 0.0);
	Point3d vecBC3d = Point3d(vecBC2d.x, vecBC2d.y, 0.0);
	Point3d vecCD3d = Point3d(vecCD2d.x, vecCD2d.y, 0.0);
	Point3d vecDA3d = Point3d(vecDA2d.x, vecDA2d.y, 0.0);


	Point2d midPnt = Point2d( trk.start.x + trk.end.x + 0.0, trk.start.y + trk.end.y + 0.0);
	midPnt *= 0.5;

	Point3d crsABAMid =  vecAB3d.cross(Point3d( (midPnt.x - scanRect.a.x), (midPnt.y - scanRect.a.y), 0.0 ));
	Point3d crsBCBMid =  vecBC3d.cross(Point3d( (midPnt.x - scanRect.b.x), (midPnt.y - scanRect.b.y), 0.0 ));
	Point3d crsCDCMid =  vecCD3d.cross(Point3d( (midPnt.x - scanRect.c.x), (midPnt.y - scanRect.c.y), 0.0 ));
	Point3d crsDADMid =  vecDA3d.cross(Point3d( (midPnt.x - scanRect.d.x), (midPnt.y - scanRect.d.y), 0.0 ));

	if(max(max(crsABAMid.z, crsBCBMid.z), max(crsCDCMid.z, crsDADMid.z) ) <= 0)
	{
		//++scanRect.trksNum;
		//cout<<"Inside"<<endl;
		return true;
	}

	return false;
}

bool isScanRectInsdRngRect(const struct rect &scanRect, const struct rect &rngRect)
{
	Point2d vecAB2d =  Point2d( rngRect.b.x - rngRect.a.x, rngRect.b.y - rngRect.a.y);
	Point2d vecBC2d =  Point2d( rngRect.c.x - rngRect.b.x, rngRect.c.y - rngRect.b.y);
	Point2d vecCD2d =  Point2d( rngRect.d.x - rngRect.c.x, rngRect.d.y - rngRect.c.y);
	Point2d vecDA2d =  Point2d( rngRect.a.x - rngRect.d.x, rngRect.a.y - rngRect.d.y);

	Point3d vecAB3d = Point3d(vecAB2d.x, vecAB2d.y, 0.0);
	Point3d vecBC3d = Point3d(vecBC2d.x, vecBC2d.y, 0.0);
	Point3d vecCD3d = Point3d(vecCD2d.x, vecCD2d.y, 0.0);
	Point3d vecDA3d = Point3d(vecDA2d.x, vecDA2d.y, 0.0);


	Point2d cntPnt = (scanRect.a + scanRect.b + scanRect.c + scanRect.d) * 0.25;

	Point3d crsABAMid =  vecAB3d.cross(Point3d( (cntPnt.x - rngRect.a.x), (cntPnt.y - rngRect.a.y), 0.0 ));
	Point3d crsBCBMid =  vecBC3d.cross(Point3d( (cntPnt.x - rngRect.b.x), (cntPnt.y - rngRect.b.y), 0.0 ));
	Point3d crsCDCMid =  vecCD3d.cross(Point3d( (cntPnt.x - rngRect.c.x), (cntPnt.y - rngRect.c.y), 0.0 ));
	Point3d crsDADMid =  vecDA3d.cross(Point3d( (cntPnt.x - rngRect.d.x), (cntPnt.y - rngRect.d.y), 0.0 ));

	if(max(max(crsABAMid.z, crsBCBMid.z), max(crsCDCMid.z, crsDADMid.z) ) <= 0)
	{
		return true;
	}

	return false;
}
bool readVanishPnts(string filePath, int mos, Point2d& vp)
{
	ifstream fin(filePath.c_str());
	if(!fin.is_open())
	{
		cout<<"Can't open file "<<filePath<<endl;
		return false;
	}

	fin.seekg(0, ios::end);
	if (fin.tellg() == 0) {
		cout<<"Empty line file"<<filePath<<endl;
		return false;
	}

	fin.seekg(0, ios::beg);
	//int frame = 1;
	while(!fin.eof())
	{
		int frame, vpX, vpY;
		fin >> frame >> vpX >> vpY;
		if(frame >= mos)
		{
			vp.x = vpX;
			vp.y = vpY;
//			cout<< "vp: " << vp.x <<" " << vp.y <<endl;
//			cout<<"frame: " <<frame<< " mos: " << mos << endl;
			return true;
		}

		if(frame > mos + 45)
			break;
	}

	fin.close();

	return false;
}

inline direction trackDir(struct track trk)
{
	if(trk.start.x > trk.end.x)
		return leftDir;
	else if(trk.start.x < trk.end.x)
		return rightDir;
	else
		return nonDir;
}

double computeTrksConsistScr(const vector<track> &trks)
{
	if(trks.empty())
	{
		//cout << "Empty tracks to compute score!" << endl;
		return 0.0;
	}
	//dijGraph G;
	Graph G(trks.size() + 2);
	int source, destination;
	G.constructGraphForTrks(trks, source, destination);
	//constructdijGraphForTrks(trks, G, source, destination);
	vector<int> path;
	//dijkstra(G, source, destination, path);
	G.shortestPath(source, destination, path);
//	cout << "source:" << source << endl;
//	cout << "destination:" << destination << endl;
	double score = 0.0;
	for(unsigned i = 1; i < path.size() - 1; ++i)
	{
		score += norm(Point2d(trks[path[i] - 1].end.x - trks[path[i] - 1].start.x, trks[path[i] - 1].end.y - trks[path[i] - 1].start.y));
	}
	//cout << "score " << score << endl;
	return score;
}

bool computeTrksConsistPath(const vector<track> &trks, vector<int> &path)
{
	if(trks.empty())
	{
		//cout << "Empty tracks to compute score!" << endl;
		return false;
	}
//	dijGraph G;
//	int source, destination;
//	constructdijGraphForTrks(trks, G, source, destination);
//	//vector<int> path;
//	dijkstra(G, source, destination, path);

	Graph G(trks.size() + 2);
	int source, destination;
	G.constructGraphForTrks(trks, source, destination);
	G.shortestPath(source, destination, path);

	return true;
}


void play::computeTopBoxes(unsigned int n, double boxXLen, double boxYLen, vector<playerBndBox> &topBoxes)
{
	vector<playerBndBox> boxes;
//	for(unsigned int y = 1; y <= (imgYLen - int(boxXLen * yardLnsDist)); y += 1)
//	{
//		for(unsigned int x = 1; x <= (imgXLen - int(boxYLen * yardLnsDist)); x += 5)
	for(unsigned int y = 1; y <= imgYLen; y += 5)
	{
		for(unsigned int x = 1; x <= imgXLen; x += 5)
		{
			struct rect scanRect = {};
			scanRect.trksNum = 0;

			scanRect.a = Point2d(x, y);
			scanRect.b = Point2d(x, y + boxYLen * yardLnsDist);
			scanRect.c = Point2d(x + boxXLen * yardLnsDist, y + boxYLen * yardLnsDist);
			scanRect.d = Point2d(x + boxXLen * yardLnsDist, y);

			//Point2d boxCnt = (scanRect.a + scanRect.b + scanRect.c + scanRect.d) * 0.25;
			//double distBoxToLos = norm(boxCnt - scrimCnt) / yardLnsDist;

//			if(!isScanRectInsdRngRect(scanRect, recSearchRng))
//				continue;
			playerBndBox leftBox(scanRect);
			leftBox.setDir(leftDir);
			vector<struct track> leftTrksInside;
			for(unsigned int i = 0; i < tracks.size(); ++i)
			{
//				if(isTrkInsideRect(tracks[i], scrimLn))
//					continue;

				if(isTrkInsideRect(tracks[i], scanRect) && (trackDir(tracks[i]) == leftDir))
				{
					leftTrksInside.push_back(tracks[i]);
					leftBox.addTrk(i);
				}
			}

			double leftLongestPath = computeTrksConsistScr(leftTrksInside) / yardLnsDist;
			leftBox.setLongestPath(leftLongestPath);
			//leftBox.setLongestPath(distBoxToLos * leftLongestPath);

			playerBndBox rightBox(scanRect);
			rightBox.setDir(rightDir);
			vector<struct track> rightTrksInside;
			for(unsigned int i = 0; i < tracks.size(); ++i)
			{
				if(isTrkInsideRect(tracks[i], scrimLn))
					continue;

				if(isTrkInsideRect(tracks[i], scanRect) && (trackDir(tracks[i]) == rightDir))
				{
					rightTrksInside.push_back(tracks[i]);
					rightBox.addTrk(i);
				}
			}

			double rightLongestPath = computeTrksConsistScr(rightTrksInside) / yardLnsDist;
			rightBox.setLongestPath(rightLongestPath);
			//rightBox.setLongestPath(distBoxToLos * rightLongestPath);


			if(leftLongestPath > rightLongestPath)
				boxes.push_back(leftBox);
			else
				boxes.push_back(rightBox);
		}
	}

	sort(boxes.begin(), boxes.end(), compBndBoxes);
	while( (topBoxes.size() < n) && (!boxes.empty()) )
	{
		bool intersect = false;
		playerBndBox b = boxes.front();
		boxes.erase(boxes.begin());
		for(unsigned int i = 0; i < topBoxes.size(); ++i)
			if(b.trksIntersect(topBoxes[i]))
			{
				intersect = true;
				break;
			}
		if(!intersect)
			topBoxes.push_back(b);
	}

}

void play::computeTopBoxesDir(const wrPicStrModel& leftModel, const wrPicStrModel& rightModel)
{
	computeYardLnsDist();
	getScrimLn();
	getTracks();
	getTrueDir();
	//getVp();
	//computeRecSearchRng();

	unsigned int  n = 7;
	double boxXLen = (leftModel.lowRecBndBox.xLength + rightModel.lowRecBndBox.xLength
			+ leftModel.upRecBndBox.xLength + rightModel.upRecBndBox.xLength) / 4.0;
	//cout << "boxXLen: " << boxXLen << endl;
	double boxYLen = (leftModel.lowRecBndBox.yLength + rightModel.lowRecBndBox.yLength
			+ leftModel.upRecBndBox.yLength + rightModel.upRecBndBox.yLength) / 4.0;
	//cout << "boxYLen: " << boxYLen << endl;
	vector<playerBndBox> topBoxes;
	computeTopBoxes(n, boxXLen, boxYLen, topBoxes);
	int leftNum = 0, rightNum = 0;
	for(unsigned int i = 0; i < topBoxes.size(); ++i)
		if(topBoxes[i].dir == leftDir)
			++leftNum;
		else if(topBoxes[i].dir == rightDir)
			++rightNum;

	if(leftNum > rightNum)
		preDir = leftDir;
	else if(rightNum > leftNum)
		preDir = rightDir;

	plotTmVisTracks(image, tracks, scrimLn, yardLines);

	plotTopBoxes(image, topBoxes, tracks);

	if((trueDir == preDir) && (trueDir != nonDir))
		plotPath = "Results/Game" + gameIdStr + "/crtPlots/vid" + vidIdxStr + ".jpg";
	else if(trueDir != nonDir)
		plotPath = "Results/Game" + gameIdStr + "/errPlots/vid" + vidIdxStr + ".jpg";
	else
		plotPath = "Results/Game" + gameIdStr + "/nonOdPlots/vid" + vidIdxStr + ".jpg";

//	plot();
//
	imwrite(plotPath, image);

}

void play::computeLosSideDir()
{
	computeYardLnsDist();
	getScrimLn();
	getTracks();
	getTrueDir();
	getVp();
	//computeRecSearchRng();

//	Point2d leftSideVec = Point2d(0.0, 0.0);
//	Point2d rightSideVec = Point2d(0.0, 0.0);

	double leftSideTrksLen = 0.0, rightSideTrksLen = 0.0;

	Point2d vecVpToScrimCnt;
	if(vpExist)
	{
		vecVpToScrimCnt = scrimCnt - vp;
		vecVpToScrimCnt *= 1.0 / norm(vecVpToScrimCnt);
	}
	else
	{
		vecVpToScrimCnt = Point2d(.0, 1.0);
	}

	Point2d perpVecVpToScrimCnt = Point2d(-1.0 * vecVpToScrimCnt.y, vecVpToScrimCnt.x);
	if(perpVecVpToScrimCnt.x < 0.0)
		perpVecVpToScrimCnt = -1.0 * perpVecVpToScrimCnt;

	for(unsigned int i = 0; i < tracks.size(); ++i)
	{
		//if(isTrkInsideRect(tracks[i], recSearchRng))
//		if(isTrkInsideRect(tracks[i], scrimLn))
//			continue;

		Point2d trkIMid = Point2d(tracks[i].start.x + tracks[i].end.x, tracks[i].start.y + tracks[i].end.y) * 0.5;
		Point2d vecScrimCntToTrkMid = trkIMid - scrimCnt;
		double dotPro = perpVecVpToScrimCnt.dot(vecScrimCntToTrkMid);
		Point2d trkIVec = Point2d(tracks[i].end.x - tracks[i].start.x, tracks[i].end.y - tracks[i].start.y);
		if(dotPro > 0.0)//right side of LOS
			rightSideTrksLen += norm(trkIVec);
		else if(dotPro < 0.0)
			leftSideTrksLen += norm(trkIVec);
	}

	if(leftSideTrksLen > rightSideTrksLen)
		preDir = leftDir;
	else if(leftSideTrksLen < rightSideTrksLen)
		preDir = rightDir;
	else
		preDir = nonDir;

}

void play::computeAllTracksDir()
{
	computeYardLnsDist();
	getScrimLn();
	getTracks();
	getTrueDir();
	getVp();

	Point2d totalVec = Point2d(0.0, 0.0);
	for(unsigned int i = 0; i < tracks.size(); ++i)
	{
		totalVec += Point2d(tracks[i].end.x - tracks[i].start.x, tracks[i].end.y - tracks[i].start.y);
	}

	if(totalVec.x < 0.0)
		preDir = leftDir;
	else if(totalVec.x > 0.0)
		preDir = rightDir;
	else
		preDir = nonDir;
}
void drawLines(Mat& dst, struct yardLine yLine, CvScalar color)
{
	float rho = yLine.rho;
	float theta = yLine.theta;
	CvPoint pt1, pt2;
	double cosTheta = cos(theta), sinTheta = sin(theta);
	//double a = -cosTheta / sinTheta, b = rho / sinTheta;
	double x0 = cosTheta*rho, y0 = sinTheta*rho;
	pt1.x = cvRound(x0 + 1000*(-sinTheta));
	pt1.y = cvRound(y0 + 1000*(cosTheta));
	pt2.x = cvRound(x0 - 1000*(-sinTheta));
	pt2.y = cvRound(y0 - 1000*(cosTheta));
	//cvLine(dst, pt1, pt2, color, 3, 8);
	line(dst, pt1, pt2, color, 2, 8, 0);
	//circle(dst, pt1, 8,CV_RGB(255, 0, 0), -1);
}

// the HSV color model will be as follows
// h : [0 - 360]
// s : [0 - 1]
// v : [0 - 1]
// If you want it differently (in a 2 * pi scale, 256 instead of 1, etc,
// you'll have to change it yourself.
// rgb is returned in 0-1 scale (ready for color3f)
void HSVtoRGB(float hsv[3], float rgb[3]) {
	float tmp1 = hsv[2] * (1-hsv[1]);
	float tmp2 = hsv[2] * (1-hsv[1] * (hsv[0] / 60.0f - (int) (hsv[0]/60.0f) ));
	float tmp3 = hsv[2] * (1-hsv[1] * (1 - (hsv[0] / 60.0f - (int) (hsv[0]/60.0f) )));
	switch((int)(hsv[0] / 60)) {
		case 0:
			rgb[0] = hsv[2] ;
			rgb[1] = tmp3 ;
			rgb[2] = tmp1 ;
			break;
		case 1:
			rgb[0] = tmp2 ;
			rgb[1] = hsv[2] ;
			rgb[2] = tmp1 ;
			break;
		case 2:
			rgb[0] = tmp1 ;
			rgb[1] = hsv[2] ;
			rgb[2] = tmp3 ;
			break;
		case 3:
			rgb[0] = tmp1 ;
			rgb[1] = tmp2 ;
			rgb[2] = hsv[2] ;
			break;
		case 4:
			rgb[0] = tmp3 ;
			rgb[1] = tmp1 ;
			rgb[2] = hsv[2] ;
			break;
		case 5:
			rgb[0] = hsv[2] ;
			rgb[1] = tmp1 ;
			rgb[2] = tmp2 ;
			break;
		default:
			cout << "What!? Inconceivable!\n";
	}

}

void plotTracks(Mat& img, vector<track> trks, struct rect& scrimLnRect, vector<yardLine> yardlns, struct rect& lowRecRect, struct rect& upRecRect)
{
//	  img = cvCreateImage(cvSize(852,480),IPL_DEPTH_32F,3);
//	  img = cv::Scalar(0,0,0);


	  for(unsigned int i = 0; i < trks.size(); ++i)
	  {
		  if(trks[i].lowRec || trks[i].upRec)
		  {
			  line(img, trks[i].start, trks[i].end, CV_RGB(0, 0, 255),1,8,0);
			  circle(img, trks[i].end, 2,CV_RGB(0, 0, 255), 1, 8, 0);
		  }
		  else
		  {
			  line(img, trks[i].start, trks[i].end, CV_RGB(255, 0, 0),1,8,0);
			  circle(img, trks[i].end, 2,CV_RGB(255, 0, 0), 1, 8, 0);
		  }
	  }

	  for(unsigned int i = 0; i < yardlns.size(); ++i)
		  drawLines(img, yardlns[i], CV_RGB(255, 255, 255));

	  line(img, scrimLnRect.a, scrimLnRect.b, CV_RGB(255, 255, 255),1,8,0);
	  line(img, scrimLnRect.b, scrimLnRect.c, CV_RGB(255, 255, 255),1,8,0);
	  line(img, scrimLnRect.c, scrimLnRect.d, CV_RGB(255, 255, 255),1,8,0);
	  line(img, scrimLnRect.d, scrimLnRect.a, CV_RGB(255, 255, 255),1,8,0);

	  line(img, lowRecRect.a, lowRecRect.b, CV_RGB(0, 255, 0),1,8,0);
	  line(img, lowRecRect.b, lowRecRect.c, CV_RGB(0, 255, 0),1,8,0);
	  line(img, lowRecRect.c, lowRecRect.d, CV_RGB(0, 255, 0),1,8,0);
	  line(img, lowRecRect.d, lowRecRect.a, CV_RGB(0, 255, 0),1,8,0);

	  line(img, upRecRect.a, upRecRect.b, CV_RGB(0, 255, 0),1,8,0);
	  line(img, upRecRect.b, upRecRect.c, CV_RGB(0, 255, 0),1,8,0);
	  line(img, upRecRect.c, upRecRect.d, CV_RGB(0, 255, 0),1,8,0);
	  line(img, upRecRect.d, upRecRect.a, CV_RGB(0, 255, 0),1,8,0);

	  return;
}

void plotRect(Mat& img, struct rect& rct, Scalar clr)
{
	  line(img, rct.a, rct.b, clr,2,8,0);
	  line(img, rct.b, rct.c, clr,2,8,0);
	  line(img, rct.c, rct.d, clr,2,8,0);
	  line(img, rct.d, rct.a, clr,2,8,0);
}

void plotTmVisTracks(Mat& img, vector<track> trks, struct rect& scrimLnRect, vector<yardLine> yardlns, struct rect& lowRecRect, struct rect& upRecRect)
{
//	  img = cvCreateImage(cvSize(852,480),IPL_DEPTH_32F,3);
//	  img = cv::Scalar(0,0,0);

	int minTm = 1000000, maxTm = -1;
	for(unsigned int i = 0; i < trks.size(); ++i)
	{
		if(trks[i].frameStart < minTm)
			minTm = trks[i].frameStart;
		if(trks[i].frameStart > maxTm)
			maxTm = trks[i].frameStart;
	}

	for(unsigned int i = 0; i < trks.size(); ++i)
	{
	  if(trks[i].lowRec || trks[i].upRec)
	  {
		  line(img, trks[i].start, trks[i].end, CV_RGB(255, 255, 255),1,8,0);
		  circle(img, trks[i].end, 2,CV_RGB(255, 255, 255), 1, 8, 0);
	  }
	  else
	  {
		  float hsv[3], rgb[3];
		  hsv[0] = 240.0 * (1.0 - float(trks[i].frameStart - minTm) / float(maxTm - minTm) );
		  hsv[1] = 1.0;
		  hsv[2] = 1.0;
		  HSVtoRGB(hsv, rgb);
		  line(img, trks[i].start, trks[i].end, CV_RGB(rgb[0] * 255, rgb[1] * 255, rgb[2] * 255),1,8,0);
		  circle(img, trks[i].end, 2,CV_RGB(rgb[0] * 255, rgb[1] * 255, rgb[2] * 255), 1, 8, 0);
	  }
	}

	for(unsigned int i = 0; i < yardlns.size(); ++i)
	  drawLines(img, yardlns[i], CV_RGB(255, 255, 255));

	line(img, scrimLnRect.a, scrimLnRect.b, CV_RGB(255, 255, 255),1,8,0);
	line(img, scrimLnRect.b, scrimLnRect.c, CV_RGB(255, 255, 255),1,8,0);
	line(img, scrimLnRect.c, scrimLnRect.d, CV_RGB(255, 255, 255),1,8,0);
	line(img, scrimLnRect.d, scrimLnRect.a, CV_RGB(255, 255, 255),1,8,0);

	line(img, lowRecRect.a, lowRecRect.b, CV_RGB(0, 255, 0),1,8,0);
	line(img, lowRecRect.b, lowRecRect.c, CV_RGB(0, 255, 0),1,8,0);
	line(img, lowRecRect.c, lowRecRect.d, CV_RGB(0, 255, 0),1,8,0);
	line(img, lowRecRect.d, lowRecRect.a, CV_RGB(0, 255, 0),1,8,0);

	line(img, upRecRect.a, upRecRect.b, CV_RGB(0, 255, 0),1,8,0);
	line(img, upRecRect.b, upRecRect.c, CV_RGB(0, 255, 0),1,8,0);
	line(img, upRecRect.c, upRecRect.d, CV_RGB(0, 255, 0),1,8,0);
	line(img, upRecRect.d, upRecRect.a, CV_RGB(0, 255, 0),1,8,0);

	return;
}


void plotTmVisTracks(Mat& img, vector<track> trks, struct rect& scrimLnRect, vector<yardLine> yardlns)
{
//	  img = cvCreateImage(cvSize(852,480),IPL_DEPTH_32F,3);
//	  img = cv::Scalar(0,0,0);

	int minTm = 1000000, maxTm = -1;
	for(unsigned int i = 0; i < trks.size(); ++i)
	{
		if(trks[i].frameStart < minTm)
			minTm = trks[i].frameStart;
		if(trks[i].frameStart > maxTm)
			maxTm = trks[i].frameStart;
	}

	for(unsigned int i = 0; i < trks.size(); ++i)
	{

	  float hsv[3], rgb[3];
	  hsv[0] = 240.0 * (1.0 - float(trks[i].frameStart - minTm) / float(maxTm - minTm) );
	  hsv[1] = 1.0;
	  hsv[2] = 1.0;
	  HSVtoRGB(hsv, rgb);
	  line(img, trks[i].start, trks[i].end, CV_RGB(rgb[0] * 255, rgb[1] * 255, rgb[2] * 255),2,8,0);
	  circle(img, trks[i].end, 2,CV_RGB(rgb[0] * 255, rgb[1] * 255, rgb[2] * 255), 2, 8, 0);


//	  line(img, trks[i].start, trks[i].end, CV_RGB(255, 0, 0),2,8,0);
//	  circle(img, trks[i].end, 2,CV_RGB(255, 0, 0), 2, 8, 0);

	}

	for(unsigned int i = 0; i < yardlns.size(); ++i)
	  drawLines(img, yardlns[i], CV_RGB(255, 255, 255));

	line(img, scrimLnRect.a, scrimLnRect.b, CV_RGB(255, 255, 255),2,8,0);
	line(img, scrimLnRect.b, scrimLnRect.c, CV_RGB(255, 255, 255),2,8,0);
	line(img, scrimLnRect.c, scrimLnRect.d, CV_RGB(255, 255, 255),2,8,0);
	line(img, scrimLnRect.d, scrimLnRect.a, CV_RGB(255, 255, 255),2,8,0);

//	int fontFace = FONT_HERSHEY_SIMPLEX;
	int fontFace = 0;
	double fontScale = 1;
	int thickness = 1;

	putText(img, "a", scrimLnRect.a, fontFace, fontScale, CV_RGB(255, 255, 0), thickness,8);
	putText(img, "b", scrimLnRect.b, fontFace, fontScale, CV_RGB(255, 255, 0), thickness,8);
	putText(img, "c", scrimLnRect.c, fontFace, fontScale, CV_RGB(255, 255, 0), thickness,8);
	putText(img, "d", scrimLnRect.d, fontFace, fontScale, CV_RGB(255, 255, 0), thickness,8);

	return;
}



void plotScrimToRecVecs(Mat& img, Point2d scrimCnt, Point2d vecScrimToUpRec, Point2d vecScrimToLowRec)
{
	  line(img, scrimCnt, scrimCnt + vecScrimToUpRec, CV_RGB(0, 255, 0),2,8,0);
	  line(img, scrimCnt, scrimCnt + vecScrimToLowRec, CV_RGB(0, 255, 0),2,8,0);

	  return;
}

void plotTopBoxes(Mat& img, const vector<playerBndBox> &topBoxes, const vector<struct track> tracks)
{
//	int fontFace = FONT_HERSHEY_SIMPLEX;
	int fontFace = 0;
	double fontScale = 1;
	int thickness = 2;

	for(unsigned int i = 0; i < topBoxes.size(); ++i)
	{
		//draw in purple color
		line(img, topBoxes[i].bndRect.a, topBoxes[i].bndRect.b, CV_RGB(128, 0, 128),1,8,0);
		line(img, topBoxes[i].bndRect.b, topBoxes[i].bndRect.c, CV_RGB(128, 0, 128),1,8,0);
		line(img, topBoxes[i].bndRect.c, topBoxes[i].bndRect.d, CV_RGB(128, 0, 128),1,8,0);
		line(img, topBoxes[i].bndRect.d, topBoxes[i].bndRect.a, CV_RGB(128, 0, 128),1,8,0);

		vector<struct track> trksInside;
		for(unsigned int j = 0; j < topBoxes[i].trksInsideIds.size(); ++j)
			trksInside.push_back(tracks[topBoxes[i].trksInsideIds[j]]);
//		{
//			line(img, tracks[topBoxes[i].trksInsideIds[j]].start, tracks[topBoxes[i].trksInsideIds[j]].end, CV_RGB(255, 255, 255),1,8,0);
//			circle(img, tracks[topBoxes[i].trksInsideIds[j]].end, 2,CV_RGB(255, 255, 255), 1, 8, 0);
//		}

		vector<int> longestPath;
		if(computeTrksConsistPath(trksInside, longestPath))
			for(unsigned int k = 1; k < longestPath.size() - 1; ++k)
			{
				//lowTrksInside[lowRecPath[i] - 1].lowRec = true;
				line(img, trksInside[longestPath[k] - 1].start, trksInside[longestPath[k] - 1].end, CV_RGB(255, 255, 255),1,8,0);
				circle(img, trksInside[longestPath[k] - 1].end, 2,CV_RGB(255, 255, 255), 1, 8, 0);
			}

		ostringstream cvtLongestPath;
		cvtLongestPath << double(int(topBoxes[i].longestTrksPath * 1000.0)) / 1000.0;
		string longestPathStr = "len: " + cvtLongestPath.str();
		//putText(img, longestPathStr, topBoxes[i].bndRect.a, fontFace, fontScale, Scalar::all(255), thickness,8);
		putText(img, longestPathStr, topBoxes[i].bndRect.a, fontFace, fontScale, CV_RGB(128, 0, 128), thickness,8);

		string dirOrderStr;
		ostringstream cvtOrder;
		cvtOrder << (i + 1);
		if(topBoxes[i].dir == leftDir)
			dirOrderStr = cvtOrder.str() + " L";
		else if(topBoxes[i].dir == rightDir)
			dirOrderStr = cvtOrder.str() + " R";

		//putText(img, dirOrderStr, topBoxes[i].bndRect.c + Point2d(.0, 5.0), fontFace, fontScale, Scalar::all(255), thickness,8);
		putText(img, dirOrderStr, topBoxes[i].bndRect.b + Point2d(.0, 10.0), fontFace, fontScale, CV_RGB(128, 0, 128), thickness,8);

	}


}

void plotFormation(Mat& img, const formation *f, const vector<struct track> tracks, const struct rect& scrimLnRect)
{
//	int fontFace = FONT_HERSHEY_SIMPLEX;
	int fontFace = 0;
	double fontScale = 1;
	int thickness = 2;

	struct rect lowWrRect, lowCbRect, upWrRect, upCbRect;
	double lowWrCbScr = NEGINF, upWrCbScr = NEGINF;
	//double totalScore = 0.0;

	for(unsigned int i = 0; i < f->players.size(); ++i)
	{
		//draw in purple color
		line(img, f->players[i].pBox->bndRect.a, f->players[i].pBox->bndRect.b, CV_RGB(0, 0, 255),2,8,0);
		line(img, f->players[i].pBox->bndRect.b, f->players[i].pBox->bndRect.c, CV_RGB(0, 0, 255),2,8,0);
		line(img, f->players[i].pBox->bndRect.c, f->players[i].pBox->bndRect.d, CV_RGB(0, 0, 255),2,8,0);
		line(img, f->players[i].pBox->bndRect.d, f->players[i].pBox->bndRect.a, CV_RGB(0, 0, 255),2,8,0);

		vector<struct track> trksInside;
		for(unsigned int j = 0; j < f->players[i].pBox->trksInsideIds.size(); ++j)
			trksInside.push_back(tracks[f->players[i].pBox->trksInsideIds[j]]);
//		{
//			line(img, tracks[f->players[i].pBox->trksInsideIds[j]].start, tracks[f->players[i].pBox->trksInsideIds[j]].end, CV_RGB(255, 255, 255),1,8,0);
//			circle(img, tracks[f->players[i].pBox->trksInsideIds[j]].end, 2,CV_RGB(255, 255, 255), 1, 8, 0);
//		}

		vector<int> longestPath;
		if(computeTrksConsistPath(trksInside, longestPath))
			for(unsigned int k = 1; k < longestPath.size() - 1; ++k)
			{
				//lowTrksInside[lowRecPath[i] - 1].lowRec = true;
				line(img, trksInside[longestPath[k] - 1].start, trksInside[longestPath[k] - 1].end, CV_RGB(255, 255, 255),1,8,0);
				circle(img, trksInside[longestPath[k] - 1].end, 2,CV_RGB(255, 255, 255), 2, 8, 0);
			}

		string playerType;
		switch(f->players[i].pType)
		{
		case lowWR:
			playerType = "lowWR";
			lowWrCbScr = f->players[i].wrCbScore;
			//cout << " f->players[i].wrCbScore: " <<  f->players[i].wrCbScore << endl;
			lowWrRect = f->players[i].pBox->bndRect;
			break;
		case lowCB:
			playerType = "lowCB";
			lowCbRect = f->players[i].pBox->bndRect;
			break;
		case upWR:
			playerType = "upWR";
			upWrCbScr = f->players[i].wrCbScore;
			upWrRect = f->players[i].pBox->bndRect;
			break;
		case upCB:
			playerType = "upCB";
			upCbRect = f->players[i].pBox->bndRect;
			break;
		default:
			playerType = "Wrong player type!";
			break;
		}

		//totalScore += f->players[i].score;

		putText(img, playerType, f->players[i].pBox->bndRect.a, fontFace, fontScale, CV_RGB(0, 0, 255), thickness,8);

		ostringstream cvtScore;
		cvtScore << double(int(f->players[i].score * 1000.0)) / 1000.0;
		string scoreStr = "S: " + cvtScore.str();
		putText(img, scoreStr, f->players[i].pBox->bndRect.b + Point2d(.0, 15.0), fontFace, fontScale, CV_RGB(0, 0, 255), thickness,8);

	}

	ostringstream cvtTotalScore;
	cvtTotalScore << double(int(f->totalScore* 1000.0)) / 1000.0;
	string totalScoreStr = "TS: " + cvtTotalScore.str();
	Point2d scrimLnCnt = (scrimLnRect.a + scrimLnRect.b + scrimLnRect.c + scrimLnRect.d) * 0.25;
	putText(img, totalScoreStr, scrimLnCnt, fontFace, fontScale, CV_RGB(0, 0, 255), thickness,8);

	if(lowWrCbScr != NEGINF)
	{
		Point2d lowWrCnt, lowCbCnt;
		lowWrCnt = (lowWrRect.a + lowWrRect.b + lowWrRect.c + lowWrRect.d) * 0.25;
		lowCbCnt = (lowCbRect.a + lowCbRect.b + lowCbRect.c + lowCbRect.d) * 0.25;
		Point2d upWrCnt, upCbCnt;
		upWrCnt = (upWrRect.a + upWrRect.b + upWrRect.c + upWrRect.d) * 0.25;
		upCbCnt = (upCbRect.a + upCbRect.b + upCbRect.c + upCbRect.d) * 0.25;
		line(img, lowWrCnt, lowCbCnt, CV_RGB(255, 255, 0),2,8,0);
		ostringstream cvtLowScr;
		cvtLowScr << double(int(lowWrCbScr* 1000.0)) / 1000.0;
		string lowScrStr = "S: " + cvtLowScr.str();
		putText(img, lowScrStr, (lowWrCnt + lowCbCnt) * 0.5, fontFace, fontScale, CV_RGB(255, 255, 0), thickness, 8);

		line(img, upWrCnt, upCbCnt, CV_RGB(255, 255, 0),2,8,0);
		ostringstream cvtUpScr;
		cvtUpScr << double(int(upWrCbScr* 1000.0)) / 1000.0;
		string upScrStr = "S: " + cvtUpScr.str();
		putText(img, upScrStr, (upWrCnt + upCbCnt) * 0.5, fontFace, fontScale, CV_RGB(255, 255, 0), thickness, 8);
	}
}


void plotGtForm(Mat& img, const vector<player> &gtPlayers)
{
//	int fontFace = FONT_HERSHEY_SIMPLEX;
	int fontFace = 0;
	double fontScale = 1;
	int thickness = 2;

	for(unsigned int i = 0; i < gtPlayers.size(); ++i)
	{
		//draw in purple color
		line(img, gtPlayers[i].pBox->bndRect.a, gtPlayers[i].pBox->bndRect.b, CV_RGB(255, 0, 0),2,8,0);
		line(img, gtPlayers[i].pBox->bndRect.b, gtPlayers[i].pBox->bndRect.c, CV_RGB(255, 0, 0),2,8,0);
		line(img, gtPlayers[i].pBox->bndRect.c, gtPlayers[i].pBox->bndRect.d, CV_RGB(255, 0, 0),2,8,0);
		line(img, gtPlayers[i].pBox->bndRect.d, gtPlayers[i].pBox->bndRect.a, CV_RGB(255, 0, 0),2,8,0);

		string playerType;
		switch(gtPlayers[i].pType)
		{
		case lowWR:
			playerType = "lowWR";
			break;
		case lowCB:
			playerType = "lowCB";
			break;
		case upWR:
			playerType = "upWR";
			break;
		case upCB:
			playerType = "upCB";
			break;
		default:
			playerType = "Wrong player type!";
			break;
		}

		putText(img, playerType, gtPlayers[i].pBox->bndRect.a, fontFace, fontScale, CV_RGB(255, 0, 0), thickness,8);

	}

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

void play::getVp()
{
	vpExist = readVanishPnts(vpFilePath, mos, vp);
}

void play::computeRecSearchRng()
{
	if(vpExist)
	{
		Point2d vecVpToScrimCnt = scrimCnt - vp;
		vecVpToScrimCnt *= 1.0 / norm(vecVpToScrimCnt);
		Point2d perpVecVpToScrimCnt = Point2d(-1.0 * vecVpToScrimCnt.y, vecVpToScrimCnt.x);
		if(perpVecVpToScrimCnt.x < 0.0)
			perpVecVpToScrimCnt *= -1.0;
//		Point2d E = scrimCnt - yardLnsDist * 1.5 * perpVecVpToScrimCnt;
//		Point2d F = scrimCnt + yardLnsDist * 1.5 * perpVecVpToScrimCnt;

		Point2d E = scrimCnt - yardLnsDist * perpVecVpToScrimCnt;
		Point2d F = scrimCnt + yardLnsDist * perpVecVpToScrimCnt;

		recSearchRng.a.y = 0.0;
		recSearchRng.a.x = (E.x - vp.x) / (E.y - vp.y) * (recSearchRng.a.y - vp.y) + vp.x;
		recSearchRng.b.y = imgYLen;
		recSearchRng.b.x = (E.x - vp.x) / (E.y - vp.y) * (recSearchRng.b.y - vp.y) + vp.x;
		recSearchRng.c.y = imgYLen;
		recSearchRng.c.x = (F.x - vp.x) / (F.y - vp.y) * (recSearchRng.c.y - vp.y) + vp.x;
		recSearchRng.d.y = 0.0;
		recSearchRng.d.x = (F.x - vp.x) / (F.y - vp.y) * (recSearchRng.d.y - vp.y) + vp.x;
	}
	else
	{
//		recSearchRng.a = Point2d(scrimCnt.x - yardLnsDist * 1.5, 0.0);
//		recSearchRng.b = Point2d(scrimCnt.x - yardLnsDist * 1.5, imgYLen);
//		recSearchRng.c = Point2d(scrimCnt.x + yardLnsDist * 1.5, imgYLen);
//		recSearchRng.d = Point2d(scrimCnt.x + yardLnsDist * 1.5, 0.0);

		recSearchRng.a = Point2d(scrimCnt.x - yardLnsDist, 0.0);
		recSearchRng.b = Point2d(scrimCnt.x - yardLnsDist, imgYLen);
		recSearchRng.c = Point2d(scrimCnt.x + yardLnsDist, imgYLen);
		recSearchRng.d = Point2d(scrimCnt.x + yardLnsDist, 0.0);

	}

}
//bool play::insideRecRange(struct track &t)
//{
//	if(vpExist)
//	{
//
//	}
//	else
//	{
//
//	}
//}

void play::computeFormation(const vector<playerType> &pTypes)
{
	form = new formation(this, pTypes);
	form->compForm();
}

void play::computeFormation(const vector<playerType> &pTypes, vector<double> &weight, vector<double> &feature)
{
	if(form != NULL)
		delete form;
	form = new formation(this, pTypes);
	form->compForm(weight, feature);
	//cout << "feature[0] " << feature[0] << endl;
}



void play::computeTopBoxesForm(const vector<playerType> &pTypes)
{
	form = new formation(this, pTypes);
	vector<playerBndBox> topBoxes;
	int n = 10;
	double boxXLen = 1.95752 * 2.0 / 3.0;
	double boxYLen = 0.603402 / 2.0;
	computeTopBoxes(n, boxXLen, boxYLen, topBoxes);
	//plotTopBoxes(image, topBoxes, tracks);
	form->compForm(topBoxes);
}

void play::computeCBDir(const vector<playerType> &pTypes)
{
	computeYardLnsDist();
	getScrimLn();
	getTracks();
	getTrueDir();
	getVp();

	computeFormation(pTypes);

	vector<direction> dirs;

	Point2d vecVpToScrimCnt;
	if(vpExist)
	{
		vecVpToScrimCnt = scrimCnt - vp;
		vecVpToScrimCnt *= 1.0 / norm(vecVpToScrimCnt);
	}
	else
	{
		vecVpToScrimCnt = Point2d(.0, 1.0);
	}

	Point2d perpVecVpToScrimCnt = Point2d(-1.0 * vecVpToScrimCnt.y, vecVpToScrimCnt.x);
	if(perpVecVpToScrimCnt.x < 0.0)
		perpVecVpToScrimCnt = -1.0 * perpVecVpToScrimCnt;

	for(unsigned int i = 0; i < form->players.size(); ++i)
	{
		if( (form->players[i].pType == lowCB) || (form->players[i].pType == upCB) )
		{
			Point2d boxCnt = (form->players[i].pBox->bndRect.a + form->players[i].pBox->bndRect.b +
					form->players[i].pBox->bndRect.c + form->players[i].pBox->bndRect.d) * 0.25;
			Point2d vecScrimCntToBoxCnt = boxCnt - scrimCnt;
			double dotPro = vecScrimCntToBoxCnt.dot(perpVecVpToScrimCnt);
			if(dotPro > 0.0)
				dirs.push_back(rightDir);
			else if(dotPro < 0.0)
				dirs.push_back(leftDir);
		}
	}

	bool dirsAgree = true;
	if(dirs.size() == 0)
		dirsAgree = false;
	else if(dirs.size() == 1)
		dirsAgree = true;
	else
		for(unsigned int i = 1; i < dirs.size(); ++i)
			if(dirs[i] != dirs[0])
				dirsAgree = false;
	if(dirsAgree)
		preDir = dirs[0];
	else
		preDir = nonDir;

	plotTmVisTracks(image, tracks, scrimLn, yardLines);

	plotFormation(image, form, tracks, scrimLn);

	if((trueDir == preDir) && (trueDir != nonDir))
		plotPath = "Results/Game" + gameIdStr + "/crtPlots/vid" + vidIdxStr + ".jpg";
	else if(trueDir != nonDir)
	{
		if(preDir != nonDir)
			plotPath = "Results/Game" + gameIdStr + "/errPlots/vid" + vidIdxStr + ".jpg";
		else
			plotPath = "Results/Game" + gameIdStr + "/nonDirPlots/vid" + vidIdxStr + ".jpg";
	}
	else
		plotPath = "Results/Game" + gameIdStr + "/nonOdPlots/vid" + vidIdxStr + ".jpg";

	imwrite(plotPath, image);

}

void play::computeFormDir(const vector<playerType> &pTypes, int iterateIdx, vector<double> &weight, vector<double> &feature, const vector<player> &gtPs)
{
//	computeYardLnsDist();
//	getScrimLn();
//	getTracks();
//	getTrueDir();
//	getVp();
	computeFormation(pTypes, weight, feature);
	//computeTopBoxesForm(pTypes);

	preDir = form->dir;

//	plotRect(image, form->lowRange);
//	plotRect(image, form->upRange);
	plotTmVisTracks(image, tracks, scrimLn, yardLines);

	plotFormation(image, form, tracks, scrimLn);

	plotGtForm(image, gtPs);

	ostringstream convertIdx;
	convertIdx << iterateIdx ;
	string idxStr = convertIdx.str();
	//plotPath = "percepProcess/Game" + gameIdStr + "/crtPlots/vid" + vidIdxStr + "_" + idxStr + ".jpg";
	plotPath = "multiModelsTrainProcess/Game" + gameIdStr + "/crtPlots/vid" + vidIdxStr + "_" + idxStr + ".jpg";
	//	if((trueDir == preDir) && (trueDir != nonDir))
//		plotPath = "Results/Game" + gameIdStr + "/crtPlots/vid" + vidIdxStr + ".jpg";
//	else if(trueDir != nonDir)
//	{
//		if(preDir != nonDir)
//			plotPath = "Results/Game" + gameIdStr + "/errPlots/vid" + vidIdxStr + ".jpg";
//		else
//			plotPath = "Results/Game" + gameIdStr + "/nonDirPlots/vid" + vidIdxStr + ".jpg";
//	}
//	else
//		plotPath = "Results/Game" + gameIdStr + "/nonOdPlots/vid" + vidIdxStr + ".jpg";

	imwrite(plotPath, image);
}

void play::computeFormDir(const vector<playerType> &pTypes, vector<double> &weight, int featureNum)
{
//	computeYardLnsDist();
//	getScrimLn();
//	getTracks();
//	getTrueDir();
//	getVp();

	vector<double> feature(featureNum, 0.0);
	computeFormation(pTypes, weight, feature);
	//computeTopBoxesForm(pTypes);

	preDir = form->dir;

//	plotRect(image, form->lowRange);
//	plotRect(image, form->upRange);
	plotTmVisTracks(image, tracks, scrimLn, yardLines);

	plotFormation(image, form, tracks, scrimLn);

//	ostringstream convertIdx;
//	convertIdx << iterateIdx ;
//	string idxStr = convertIdx.str();
	//plotPath = "Results/Game" + gameIdStr + "/crtPlots/vid" + vidIdxStr + idxStr + ".jpg";
	if((trueDir == preDir) && (trueDir != nonDir))
		plotPath = "Results/Game" + gameIdStr + "/crtPlots/vid" + vidIdxStr + ".jpg";
	else if(trueDir != nonDir)
	{
		if(preDir != nonDir)
			plotPath = "Results/Game" + gameIdStr + "/errPlots/vid" + vidIdxStr + ".jpg";
		else
			plotPath = "Results/Game" + gameIdStr + "/nonDirPlots/vid" + vidIdxStr + ".jpg";
	}
	else
		plotPath = "Results/Game" + gameIdStr + "/nonOdPlots/vid" + vidIdxStr + ".jpg";

	imwrite(plotPath, image);
}

void play::computeFormDir(const vector<playerType> &pTypes, vector<double> &weight, int featureNum, int model)
{
	vector<double> feature(featureNum, 0.0);
	computeFormation(pTypes, weight, feature);

	preDir = form->dir;

	plotTmVisTracks(image, tracks, scrimLn, yardLines);

	plotFormation(image, form, tracks, scrimLn);

	ostringstream convertModel;
	convertModel << model ;
	string modelStr = convertModel.str();

	plotPath = "Results/Game" + gameIdStr + "/plots/vid" + vidIdxStr + "M" + modelStr + ".jpg";

	imwrite(plotPath, image);
}

void play::computeDirLosArea()
{
	Point2d vecVpToScrimCnt;
	if(vpExist)
	{
		vecVpToScrimCnt = scrimCnt - vp;
		vecVpToScrimCnt *= 1.0 / norm(vecVpToScrimCnt);
	}
	else
	{
		vecVpToScrimCnt = Point2d(.0, 1.0);
	}

	Point2d upIntersectPnt, lowIntersectPnt;
	upIntersectPnt.y = .0;
	upIntersectPnt.x = vecVpToScrimCnt.x * (upIntersectPnt.y - vp.y) / vecVpToScrimCnt.y + vp.x;

	lowIntersectPnt.y = imgYLen;
	lowIntersectPnt.x = vecVpToScrimCnt.x * (lowIntersectPnt.y - vp.y) / vecVpToScrimCnt.y + vp.x;

	double leftArea = 0.5 * (upIntersectPnt.x + lowIntersectPnt.x) * imgYLen;
	double rightArea = imgXLen * imgYLen - leftArea;

	if(leftArea >= rightArea)
		preDir = leftDir;
	else
		preDir = rightDir;

	cout << "leftArea: " << leftArea << " rightArea:" << rightArea << " totalArea:" << (leftArea + rightArea) << endl;

}

void play::computeFormDir(const vector<playerType> &pTypes)
{
//	computeYardLnsDist();
//	getScrimLn();
//	getTracks();
//	getTrueDir();
//	getVp();

	computeFormation(pTypes);
	//computeTopBoxesForm(pTypes);

	preDir = form->dir;

//	plotRect(image, form->lowRange);
//	plotRect(image, form->upRange);
	plotTmVisTracks(image, tracks, scrimLn, yardLines);
	//plotTracks(image, tracks, scrimLn, yardLines);

	plotFormation(image, form, tracks, scrimLn);

	if((trueDir == preDir) && (trueDir != nonDir))
		plotPath = "Results/Game" + gameIdStr + "/crtPlots/vid" + vidIdxStr + ".jpg";
	else if(trueDir != nonDir)
	{
		if(preDir != nonDir)
			plotPath = "Results/Game" + gameIdStr + "/errPlots/vid" + vidIdxStr + ".jpg";
		else
			plotPath = "Results/Game" + gameIdStr + "/nonDirPlots/vid" + vidIdxStr + ".jpg";
	}
	else
		plotPath = "Results/Game" + gameIdStr + "/nonOdPlots/vid" + vidIdxStr + ".jpg";

	imwrite(plotPath, image);
}


bool isLosCloserToLowEnd(const play *p)
{
	Point2d vecVpToScrimCnt;
	if(p->vpExist)
	{
		vecVpToScrimCnt = p->scrimCnt - p->vp;
		vecVpToScrimCnt *= 1.0 / norm(vecVpToScrimCnt);
	}
	else
	{
		vecVpToScrimCnt = Point2d(.0, 1.0);
	}

	Point2d lowEnd;
	lowEnd.y = imgYLen;
	lowEnd.x = vecVpToScrimCnt.x / vecVpToScrimCnt.y * (lowEnd.y - p->scrimCnt.y) + p->scrimCnt.x;

	if(lowEnd.x < 0.0)
	{
		lowEnd.x = 0.0;
		lowEnd.y = vecVpToScrimCnt.y / vecVpToScrimCnt.x * (lowEnd.x - p->scrimCnt.x) + p->scrimCnt.y;
	}
	else if(lowEnd.x > imgXLen)
	{
		lowEnd.x = imgXLen;
		lowEnd.y = vecVpToScrimCnt.y / vecVpToScrimCnt.x * (lowEnd.x - p->scrimCnt.x) + p->scrimCnt.y;
	}

	Point2d upEnd;
	upEnd.y = 0.0;
	upEnd.x = vecVpToScrimCnt.x / vecVpToScrimCnt.y * (upEnd.y - p->scrimCnt.y) + p->scrimCnt.x;

	if(upEnd.x < 0.0)
	{
		upEnd.x = 0.0;
		upEnd.y = vecVpToScrimCnt.y / vecVpToScrimCnt.x * (upEnd.x - p->scrimCnt.x) + p->scrimCnt.y;
	}
	else if(upEnd.x > imgXLen)
	{
		upEnd.x = imgXLen;
		upEnd.y = vecVpToScrimCnt.y / vecVpToScrimCnt.x * (upEnd.x - p->scrimCnt.x) + p->scrimCnt.y;
	}

	return (norm(p->scrimCnt - lowEnd) < norm(upEnd - p->scrimCnt));

}

void play::setUp()
{
	computeYardLnsDist();
#if losMethod == 1
	getScrimLn();
#elif losMethod == 2
	getGradientScrimLn();
#endif
	getTracks();
	getTrueDir();
	getVp();
}

void play::saveMosFrm()
{
	VideoCapture capture(videoPath);
	if (!capture.isOpened()) {
		cout << "Error in opening the video";
		return;
	}

	Mat frame;
	for(int i = 1; i <= mos; ++i)
		capture >> frame;
//
//	plotPath = "mosImages/Game" + gameIdStr + "/vid" + vidIdxStr + ".jpg";


//	ostringstream convertVidId;
//	int mosImgsVidIdx = pId.vidId;
//	convertVidId << mosImgsVidIdx;
//	string mosImgsVidIdxStr = convertVidId.str();
//	if(mosImgsVidIdx < 10)
//		mosImgsVidIdxStr = "00" + mosImgsVidIdxStr;
//	else if(mosImgsVidIdx < 100 )
//		mosImgsVidIdxStr = "0" + mosImgsVidIdxStr;

//	plotPath = "mosImages/Game" + gameIdStr + "/od/vid" + mosImgsVidIdxStr + ".jpg";
//	string nonOdPlotPath = "mosImages/Game" + gameIdStr + "/nonOd/vid" + mosImgsVidIdxStr + ".jpg";
//	if(trueDir != nonDir)
//		imwrite(plotPath, frame);
//	else
//		imwrite(nonOdPlotPath, frame);

//	plotPath = "mosImages/Game" + gameIdStr + "/000" + mosImgsVidIdxStr + ".jpg";
	imwrite(mosImgPath, frame);

}

void play::cvtFgImgFrmBmpToJpg()
{
	string jpgImgPath = "fgMosJpgImages/Game" + gameIdStr + "/000" + vidIdxStr +".jpg";
	imwrite(jpgImgPath, image);
}

void play::extractOdGridsFeature(direction dir, vector<int> &featureVec)
{
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
	vector<rect> scanLinesPerp;
	if(vpExist)
	{
		Point2d pLos;
		pLos.y = imgYLen / 2;
		pLos.x = (pLos.y - vp.y) * (scrimCnt.x - vp.x) / (scrimCnt.y - vp.y) + vp.x;
		Point2d vecVpToScrim = scrimCnt - vp;
		vecVpToScrim *= (1.0 / norm(vecVpToScrim));
		Point2d vecVpToScrimPerp = Point2d(-1.0 * vecVpToScrim.y, vecVpToScrim.x);
		if(vecVpToScrimPerp.x > 0)
			vecVpToScrimPerp *= -1.0;
		cout << "vecVpToScrimPerp.x " << vecVpToScrimPerp.x << endl;
		for(int i = 0; i < 10; ++i)
		{
			int fgPixelsNum = 0;
			struct rect rectLines;
			if(dir == rightDir)
			{
				rectLines.a = vp;
				rectLines.b = Point2d(pLos.x + d * i * (yardLnsDist / 2.0), pLos.y);
				rectLines.c = Point2d(pLos.x + d * (i + 1) * (yardLnsDist / 2.0), pLos.y);
				rectLines.d = vp;
			}
			else
			{
				rectLines.d = vp;
				rectLines.c = Point2d(pLos.x + d * i * (yardLnsDist / 2.0), pLos.y);
				rectLines.b = Point2d(pLos.x + d * (i + 1) * (yardLnsDist / 2.0), pLos.y);
				rectLines.a = vp;
			}
			for(int j = -5; j < 5; ++j)
			{
				struct rect rectLinesPerp;
				rectLinesPerp.a = scrimCnt + j * (yardLnsDist / 2.0) * vecVpToScrim;
				rectLinesPerp.b = rectLinesPerp.a + vecVpToScrimPerp * 5;
				rectLinesPerp.d = scrimCnt + (j + 1) * (yardLnsDist / 2.0) * vecVpToScrim;
				rectLinesPerp.c = rectLinesPerp.d + vecVpToScrimPerp * 5;
				for(int y = 0; y < imgYLen; ++y)
					for(int x = 0; x < imgXLen; ++x)
					{
						const Point3_<uchar>* p = image.ptr<Point3_<uchar> >(y, x);
						if(int(p->z) == 255)
						{
							Point2d pnt = Point2d(x + 1, y + 1);
							if(isPntInsideTwoLines(pnt, rectLines) && isPntInsideTwoLines(pnt, rectLinesPerp))
								++fgPixelsNum;
						}
					}
				featureVec.push_back(fgPixelsNum);
				if(scanLinesPerp.size() < 10)
					scanLinesPerp.push_back(rectLinesPerp);
			}
			scanLines.push_back(rectLines);

		}

		for(unsigned int i = 0; i < scanLinesPerp.size(); ++i)
			scanLines.push_back(scanLinesPerp[i]);
	}
	else
	{
//		Point2d pLos;
//		pLos.y = imgYLen / 2;
//		pLos.x = imgXLen / 2;
//
//		for(int i = 0; i < 10; ++i)
//		{
//			int fgPixelsNum = 0;
//			struct rect rectLines;
//
//			if(dir == rightDir)
//			{
//				rectLines.a = Point2d(pLos.x + d * i * (yardLnsDist / 2.0), 0.0);
//				rectLines.b = Point2d(pLos.x + d * i * (yardLnsDist / 2.0), pLos.y);
//				rectLines.c = Point2d(pLos.x + d * (i + 1) * (yardLnsDist / 2.0), pLos.y);
//				rectLines.d = Point2d(pLos.x + d * (i + 1) * (yardLnsDist / 2.0), 0.0);
//			}
//			else
//			{
//				rectLines.d = Point2d(pLos.x + d * i * (yardLnsDist / 2.0), 0.0);
//				rectLines.c = Point2d(pLos.x + d * i * (yardLnsDist / 2.0), pLos.y);
//				rectLines.b = Point2d(pLos.x + d * (i + 1) * (yardLnsDist / 2.0), pLos.y);
//				rectLines.a = Point2d(pLos.x + d * (i + 1) * (yardLnsDist / 2.0), 0.0);
//			}
//
//			for(int y = 0; y < imgYLen; ++y)
//				for(int x = 0; x < imgXLen; ++x)
//				{
//					const Point3_<uchar>* p = image.ptr<Point3_<uchar> >(y, x);
//					if(int(p->z) == 255)
//					{
//						Point2d pnt = Point2d(x, y);
//						if(isPntInsideTwoLines(pnt, rectLines))
//							++fgPixelsNum;
//					}
//				}
//			scanLines.push_back(rectLines);
//			featureVec.push_back(fgPixelsNum);

//		}
	}

	plotYardLnsAndLos(image, scrimLn, yardLines);
	plotScanLines(image, scanLines, featureVec);

	plotPath = "Results/Game" + gameIdStr + "/plots/vid" + vidIdxStr + dirStr + ".jpg";

	imwrite(plotPath, image);

}

void play::extractOdStripsFeature(direction dir, vector<int> &featureVec)
{
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
	vector<rect> scanLinesPerp;
	if(vpExist)
	{
		Point2d pLos;
		pLos.y = imgYLen / 2;
		pLos.x = (pLos.y - vp.y) * (scrimCnt.x - vp.x) / (scrimCnt.y - vp.y) + vp.x;
//		cout << "pLos.x: " << pLos.x << endl;
		for(int i = 0; i < 5; ++i)
		{
			int fgPixelsNum = 0;
			struct rect rectLines;
			if(dir == rightDir)
			{
				rectLines.a = vp;
				rectLines.b = Point2d(pLos.x + d * i * yardLnsDist, pLos.y);
				rectLines.c = Point2d(pLos.x + d * (i + 1) * yardLnsDist, pLos.y);
				rectLines.d = vp;
			}
			else
			{
				rectLines.d = vp;
				rectLines.c = Point2d(pLos.x + d * i * yardLnsDist, pLos.y);
//				cout << "rectLines.c.y "  << rectLines.c.y << " " << "rectLines.c.x " << rectLines.c.x << endl;
				rectLines.b = Point2d(pLos.x + d * (i + 1) * yardLnsDist, pLos.y);
				rectLines.a = vp;
			}

			for(int y = 0; y < imgYLen; ++y)
				for(int x = 0; x < imgXLen; ++x)
				{
					const Point3_<uchar>* p = image.ptr<Point3_<uchar> >(y, x);
					if(int(p->z) == 255)
					{
						Point2d pnt = Point2d(x + 1, y + 1);
						if(isPntInsideTwoLines(pnt, rectLines))
							++fgPixelsNum;
					}
				}
			scanLines.push_back(rectLines);
			featureVec.push_back(fgPixelsNum);

		}
	}
	else
	{
		Point2d pLos;
		pLos.y = imgYLen / 2;
		pLos.x = imgXLen / 2;

		for(int i = 0; i < 5; ++i)
		{
			int fgPixelsNum = 0;
			struct rect rectLines;

			if(dir == rightDir)
			{
				rectLines.a = vp;
				rectLines.b = Point2d(pLos.x + d * i * yardLnsDist, pLos.y);
				rectLines.c = Point2d(pLos.x + d * (i + 1) * yardLnsDist, pLos.y);
				rectLines.d = vp;
			}
			else
			{
				rectLines.d = vp;
				rectLines.c = Point2d(pLos.x + d * i * yardLnsDist, pLos.y);
				rectLines.b = Point2d(pLos.x + d * (i + 1) * yardLnsDist, pLos.y);
				rectLines.a = vp;
			}


			for(int y = 0; y < imgYLen; ++y)
				for(int x = 0; x < imgXLen; ++x)
				{
					const Point3_<uchar>* p = image.ptr<Point3_<uchar> >(y, x);
					if(int(p->z) == 255)
					{
						Point2d pnt = Point2d(x + 1, y + 1);
						if(isPntInsideTwoLines(pnt, rectLines))
							++fgPixelsNum;
					}
				}
			scanLines.push_back(rectLines);
			featureVec.push_back(fgPixelsNum);

		}
	}

//	plotYardLnsAndLos(image, scrimLn, yardLines);

#if losMethod == 1
	plotYardLnsAndLos(image, scrimLn, yardLines);
#elif losMethod == 2
	plotYardLnsAndLos(image, scrimLn, yardLines, los);
#endif

	plotScanLines(image, scanLines, featureVec);

	plotPath = "Results/Game" + gameIdStr + "/plots/vid" + vidIdxStr + dirStr + ".jpg";

	imwrite(plotPath, image);
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
			maxYardLnXCoord = rectScrimCnt.x + d * (i + 1) * YardLinesDist;
			minYardLnXCoord = rectScrimCnt.x + d * i * YardLinesDist;
		}
		else
		{
			maxYardLnXCoord = rectScrimCnt.x + d * i * YardLinesDist;
			minYardLnXCoord = rectScrimCnt.x + d * (i + 1) * YardLinesDist;
		}

		rectLines.a = Point2d(minYardLnXCoord, .0);
		rectLines.b = Point2d(minYardLnXCoord, FieldLength);
		rectLines.c = Point2d(maxYardLnXCoord, FieldLength);
		rectLines.d = Point2d(maxYardLnXCoord, .0);


		for(int y = 0; y < FieldLength; ++y)
			for(int x = minYardLnXCoord - 1; x < maxYardLnXCoord - 1; ++x)
			{
				if( x < 0 || x >= FieldWidth)
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
//	plotYardLnsAndLos(image, scrimLn, yardLines);

#if losMethod == 1
	plotLos(rectImage, rectScrimLn);
#elif losMethod == 2
	plotLos(rectImage, rectScrimLn, rectLos);
#endif

	circle(rectImage, rectScrimCnt, 3, CV_RGB(0, 0, 250), 3);

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
			maxYardLnXCoord = rectScrimCnt.x + d * (i + 1) * YardLinesDist;
			minYardLnXCoord = rectScrimCnt.x + d * i * YardLinesDist;
		}
		else
		{
			maxYardLnXCoord = rectScrimCnt.x + d * i * YardLinesDist;
			minYardLnXCoord = rectScrimCnt.x + d * (i + 1) * YardLinesDist;
		}

		for(int j = -5; j < 5; ++j)
		{
			int fgPixelsNum = 0;
			struct rect scanR;

//			double lowY = rectScrimCnt.y + j * (FieldLength / 10.0);
//			double upY = rectScrimCnt.y + (j + 1) * (FieldLength / 10.0);
			double lowY = rectScrimCnt.y + j * (YardLinesDist * 2.0);
			double upY = rectScrimCnt.y + (j + 1) * (YardLinesDist * 2.0);
//			cout << "lowY: " << lowY << endl;
//			cout << "upY: " << upY << endl;
			scanR.a = Point2d(minYardLnXCoord, lowY);
			scanR.b = Point2d(minYardLnXCoord, upY);
			scanR.c = Point2d(maxYardLnXCoord, upY);
			scanR.d = Point2d(maxYardLnXCoord, lowY);


			for(int y = lowY; y < upY; ++y)
				for(int x = minYardLnXCoord; x < maxYardLnXCoord; ++x)
				{
					if( y < 0 || y >= FieldLength)
						continue;
					if( x < 0 || x >= FieldWidth)
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
//	plotYardLnsAndLos(image, scrimLn, yardLines);

#if losMethod == 1
	plotLos(rectImage, rectScrimLn);
	plotLos(rectMosFrame, rectScrimLn);
#elif losMethod == 2
	plotLos(rectImage, rectScrimLn, rectLos);
#endif

	circle(rectImage, rectScrimCnt, 3, CV_RGB(0, 0, 250), 3);
	circle(rectMosFrame, rectScrimCnt, 3, CV_RGB(0, 0, 250), 3);

	plotScanLines(rectImage, scanRects, featureVec);
	plotScanLines(rectMosFrame, scanRects, featureVec);

	plotPath = "Results/Game" + gameIdStr + "/plots/vid" + vidIdxStr + dirStr + ".jpg";
	string mosPlotPath = "Results/Game" + gameIdStr + "/plots/vidMos" + vidIdxStr + dirStr + ".jpg";

	imwrite(plotPath, rectImage);
	imwrite(mosPlotPath, rectMosFrame);
}

void play::extractOdGridsFeatRect(direction dir, vector<int> &featureVec,
		const vector<CvSize> &gridSizes, const vector<Point2i> gridsNum)
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
				maxYardLnXCoord = rectScrimCnt.x + d * (i + 1) * gridSizes[k].width;
				minYardLnXCoord = rectScrimCnt.x + d * i * gridSizes[k].width;
			}
			else
			{
				maxYardLnXCoord = rectScrimCnt.x + d * i * gridSizes[k].width;
				minYardLnXCoord = rectScrimCnt.x + d * (i + 1) * gridSizes[k].width;
			}

			for(int j = -0.5 * gridsNum[k].y; j < gridsNum[k].y * 0.5; ++j)
			{
				int fgPixelsNum = 0;
				struct rect scanR;

	//			double lowY = rectScrimCnt.y + j * (FieldLength / 10.0);
	//			double upY = rectScrimCnt.y + (j + 1) * (FieldLength / 10.0);
				double lowY = rectScrimCnt.y + j * gridSizes[k].height;
				double upY = rectScrimCnt.y + (j + 1) * gridSizes[k].height;
	//			cout << "lowY: " << lowY << endl;
	//			cout << "upY: " << upY << endl;
				scanR.a = Point2d(minYardLnXCoord, lowY);
				scanR.b = Point2d(minYardLnXCoord, upY);
				scanR.c = Point2d(maxYardLnXCoord, upY);
				scanR.d = Point2d(maxYardLnXCoord, lowY);


				for(int y = lowY; y < upY; ++y)
					for(int x = minYardLnXCoord; x < maxYardLnXCoord; ++x)
					{
						if( y < 0 || y >= FieldLength)
							continue;
						if( x < 0 || x >= FieldWidth)
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

				scanRectsOneLevel.push_back(scanR);
				featureVecLevel.push_back(fgPixelsNum);

			}
		}

	Mat fgImageLevel, mosImageLevel;
	rectImage.copyTo(fgImageLevel);
	rectMosFrame.copyTo(mosImageLevel);
#if losMethod == 1
	plotLos(fgImageLevel, rectScrimLn);
	plotLos(mosImageLevel, rectScrimLn);
#elif losMethod == 2
	plotLos(fgImageLevel, rectScrimLn, rectLos);
#endif

	circle(fgImageLevel, rectScrimCnt, 3, CV_RGB(0, 0, 250), 3);
	circle(mosImageLevel, rectScrimCnt, 3, CV_RGB(0, 0, 250), 3);

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

//	plotYardLnsAndLos(image, scrimLn, yardLines);

#if losMethod == 1
	plotLos(rectImage, rectScrimLn);
	plotLos(rectMosFrame, rectScrimLn);
#elif losMethod == 2
	plotLos(rectImage, rectScrimLn, rectLos);
#endif

	circle(rectImage, rectScrimCnt, 3, CV_RGB(0, 0, 250), 3);
	circle(rectMosFrame, rectScrimCnt, 3, CV_RGB(0, 0, 250), 3);

	plotScanLines(rectImage, scanRects, featureVec);
	plotScanLines(rectMosFrame, scanRects, featureVec);

	plotPath = "Results/Game" + gameIdStr + "/plots/vid" + vidIdxStr + dirStr + ".jpg";
	string mosPlotPath = "Results/Game" + gameIdStr + "/plots/vidMos" + vidIdxStr + dirStr + ".jpg";

	imwrite(plotPath, rectImage);
	imwrite(mosPlotPath, rectMosFrame);
}

void play::extractOdStripsFeatFldCrd(direction dir, vector<int> &featureVec)
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
			rectLines.a = Point2d(rectScrimCnt.x + d * i * YardLinesDist, .0);
			rectLines.b = Point2d(rectScrimCnt.x + d * i * YardLinesDist, FieldLength);
			rectLines.c = Point2d(rectScrimCnt.x + d * (i + 1) * YardLinesDist, FieldLength);
			rectLines.d = Point2d(rectScrimCnt.x + d * (i + 1) * YardLinesDist, .0);
		}
		else
		{
			rectLines.a = Point2d(rectScrimCnt.x + d * (i + 1) * YardLinesDist, .0);
			rectLines.b = Point2d(rectScrimCnt.x + d * (i + 1) * YardLinesDist, FieldLength);
			rectLines.c = Point2d(rectScrimCnt.x + d * i * YardLinesDist, FieldLength);
			rectLines.d = Point2d(rectScrimCnt.x + d * i * YardLinesDist, .0);
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
				const Point3_<uchar>* p = image.ptr<Point3_<uchar> >(y, x);
				if(int(p->z) == 255)
				{
					Point2d pnt = Point2d(x + 1, y + 1);
					if(isPntInsideTwoLines(pnt, rectLines))
						++fgPixelsNum;
				}
			}
		scanLines.push_back(rectLines);
		featureVec.push_back(fgPixelsNum);

	}

	getFieldYardLines(yardLnsFldModel);
	for(unsigned int ln = 0; ln < yardLnsFldModel.size(); ++ln)
	{
		vector<Point2d> yardLnFld;
		perspectiveTransform(yardLnsFldModel[ln], yardLnFld, homoMat);
		yardLnsFldModel[ln] = yardLnFld;
//		yardLnsFldModel[ln][0] = yardLnFld[0];
//		yardLnsFldModel[ln][1] = yardLnFld[1];
	}
//	plotYardLnsAndLos(image, scrimLn, yardLines);

#if losMethod == 1
	plotYardLnsAndLos(mosFrame, scrimLn, yardLnsFldModel);
	plotYardLnsAndLos(image, scrimLn, yardLnsFldModel);
	//plotLos(image, scrimLn);
#elif losMethod == 2
	plotYardLnsAndLos(image, scrimLn, yardLnsFldModel, los);
	//plotLos(image, scrimLn, los);
#endif

	circle(mosFrame, scrimCnt, 3, CV_RGB(0, 0, 250), 3);
	circle(image, scrimCnt, 3, CV_RGB(0, 0, 250), 3);

	plotScanLines(mosFrame, scanLines, featureVec);
	plotScanLines(image, scanLines, featureVec);

	plotPath = "Results/Game" + gameIdStr + "/plots/vid" + vidIdxStr + dirStr + ".jpg";
	string mosPlotPath = "Results/Game" + gameIdStr + "/plots/vidMos" + vidIdxStr + dirStr + ".jpg";

	imwrite(plotPath, image);
	imwrite(mosPlotPath, mosFrame);

}

void play::extractOdGridsFeatFldCrd(direction dir, vector<int> &featureVec)
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
	fieldRange.a = Point2d(EndZoneWidth, .0);
	fieldRange.b = Point2d(EndZoneWidth, FieldLength);
	fieldRange.c = Point2d(FieldWidth - EndZoneWidth, FieldLength);
	fieldRange.d = Point2d(FieldWidth - EndZoneWidth, .0);

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
			maxYardLnXCoord = rectScrimCnt.x + d * (i + 1) * YardLinesDist;
			minYardLnXCoord = rectScrimCnt.x + d * i * YardLinesDist;
		}
		else
		{
			maxYardLnXCoord = rectScrimCnt.x + d * i * YardLinesDist;
			minYardLnXCoord = rectScrimCnt.x + d * (i + 1) * YardLinesDist;
		}

		for(int j = -10; j < 10; ++j)
		{
			int fgPixelsNum = 0;
			struct rect scanR;

			double lowY = rectScrimCnt.y + j * (FieldLength / 10.0);
			double upY = rectScrimCnt.y + (j + 1) * (FieldLength / 10.0);
			scanR.a = Point2d(minYardLnXCoord, lowY);
			scanR.b = Point2d(minYardLnXCoord, upY);
			scanR.c = Point2d(maxYardLnXCoord, upY);
			scanR.d = Point2d(maxYardLnXCoord, lowY);

//			bool outsideField = false;
//			if( upY < 0 || lowY > FieldLength)
//				outsideField = true;
//			if( maxYardLnXCoord < 0 || minYardLnXCoord > FieldWidth)
//				outsideField = true;

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
					const Point3_<uchar>* p = image.ptr<Point3_<uchar> >(y, x);
					if(int(p->z) == 255)
					{
						Point2d pnt = Point2d(x + 1, y + 1);
						if(isPntInsideRect(pnt, scanR) && isPntInsideRect(pnt, fieldRange))
							++fgPixelsNum;
					}
				}

			scanRects.push_back(scanR);
			featureVec.push_back(fgPixelsNum);

		}
	}

	getFieldYardLines(yardLnsFldModel);
	for(unsigned int ln = 0; ln < yardLnsFldModel.size(); ++ln)
	{
		vector<Point2d> yardLnFld;
		perspectiveTransform(yardLnsFldModel[ln], yardLnFld, homoMat);
		yardLnsFldModel[ln] = yardLnFld;
//		yardLnsFldModel[ln][0] = yardLnFld[0];
//		yardLnsFldModel[ln][1] = yardLnFld[1];
	}
//	plotYardLnsAndLos(image, scrimLn, yardLines);

#if losMethod == 1
	plotYardLnsAndLos(image, scrimLn, yardLnsFldModel);
	plotYardLnsAndLos(mosFrame, scrimLn, yardLnsFldModel);
	//plotLos(image, scrimLn);
#elif losMethod == 2
	plotYardLnsAndLos(image, scrimLn, yardLnsFldModel, los);
	//plotLos(image, scrimLn, los);
#endif

	circle(image, scrimCnt, 3, CV_RGB(0, 0, 250), 3);
	circle(mosFrame, scrimCnt, 3, CV_RGB(0, 0, 250), 3);

	plotScanLines(image, scanRects, featureVec);
	plotScanLines(mosFrame, scanRects, featureVec);

	plotPath = "Results/Game" + gameIdStr + "/plots/vid" + vidIdxStr + dirStr + ".jpg";
	string mosPlotPath = "Results/Game" + gameIdStr + "/plots/vidMos" + vidIdxStr + dirStr + ".jpg";

	imwrite(plotPath, image);
	imwrite(mosPlotPath, mosFrame);
}
int fgPixelsInsideBoxXY(const play *pl, const struct rect &box)
{
	int fgPixelsNum = 0;
	for(int y = box.a.y; y < box.c.y; ++y)
		for(int x = box.a.x; x < box.c.x; ++x)
		{
			const Point3_<uchar>* p = pl->image.ptr<Point3_<uchar> >(y, x);
			if(int(p->z) == 255)
				++fgPixelsNum;
		}

	return fgPixelsNum;
}


bool isPntInsideTwoLines(Point2d pnt, const struct rect &rectLines)
{
	Point2d vecAB2d =  Point2d( rectLines.b.x - rectLines.a.x, rectLines.b.y - rectLines.a.y);
	Point2d vecCD2d =  Point2d( rectLines.d.x - rectLines.c.x, rectLines.d.y - rectLines.c.y);

	Point3d vecAB3d = Point3d(vecAB2d.x, vecAB2d.y, 0.0);
	Point3d vecCD3d = Point3d(vecCD2d.x, vecCD2d.y, 0.0);


	Point3d crsABAPnt =  vecAB3d.cross(Point3d( (pnt.x - rectLines.a.x), (pnt.y - rectLines.a.y), 0.0 ));
	Point3d crsCDCPnt =  vecCD3d.cross(Point3d( (pnt.x - rectLines.c.x), (pnt.y - rectLines.c.y), 0.0 ));

	if(max(crsABAPnt.z, crsCDCPnt.z) <= 0)
		return true;

	return false;
}

bool isPntInsideRect(Point2d pnt, const struct rect &rect)
{
	Point2d vecAB2d = Point2d(rect.b.x - rect.a.x, rect.b.y - rect.a.y);
	Point2d vecBC2d = Point2d(rect.c.x - rect.b.x, rect.c.y - rect.b.y);
	Point2d vecCD2d = Point2d(rect.d.x - rect.c.x, rect.d.y - rect.c.y);
	Point2d vecDA2d = Point2d(rect.a.x - rect.d.x, rect.a.y - rect.d.y);

	Point3d vecAB3d = Point3d(vecAB2d.x, vecAB2d.y, 0.0);
	Point3d vecBC3d = Point3d(vecBC2d.x, vecBC2d.y, 0.0);
	Point3d vecCD3d = Point3d(vecCD2d.x, vecCD2d.y, 0.0);
	Point3d vecDA3d = Point3d(vecDA2d.x, vecDA2d.y, 0.0);


	Point3d crsABAPnt = vecAB3d.cross(Point3d( (pnt.x - rect.a.x), (pnt.y - rect.a.y), 0.0 ));
	Point3d crsBCBPnt = vecBC3d.cross(Point3d( (pnt.x - rect.b.x), (pnt.y - rect.b.y), 0.0 ));
	Point3d crsCDCPnt = vecCD3d.cross(Point3d( (pnt.x - rect.c.x), (pnt.y - rect.c.y), 0.0 ));
	Point3d crsDADPnt = vecDA3d.cross(Point3d( (pnt.x - rect.d.x), (pnt.y - rect.d.y), 0.0 ));

	if(max(max(crsABAPnt.z, crsCDCPnt.z), max(crsBCBPnt.z, crsDADPnt.z)) <= 0)
		return true;

	return false;
}


int fgPixelsInsideBox(const Mat &fgImg, const struct rect &box)
{
	int fgPixelsNum = 0;
	for(int y = 0; y < imgYLen; ++y)
		for(int x = 0; x < imgXLen; ++x)
		{
			const Point3_<uchar>* p = fgImg.ptr<Point3_<uchar> >(y, x);
			if(int(p->z) == 255)
			{
				Point2d pnt = Point2d(x, y);
				if(isPntInsideRect(pnt, box))
					++fgPixelsNum;
			}
		}
	return fgPixelsNum;
}

void plotYardLnsAndLos(Mat& img, struct rect& scrimLnRect, vector<yardLine> yardlns)
{
	for(unsigned int i = 0; i < yardlns.size(); ++i)
	  drawLines(img, yardlns[i], CV_RGB(255, 255, 255));

	line(img, scrimLnRect.a, scrimLnRect.b, CV_RGB(255, 255, 255),2,8,0);
	line(img, scrimLnRect.b, scrimLnRect.c, CV_RGB(255, 255, 255),2,8,0);
	line(img, scrimLnRect.c, scrimLnRect.d, CV_RGB(255, 255, 255),2,8,0);
	line(img, scrimLnRect.d, scrimLnRect.a, CV_RGB(255, 255, 255),2,8,0);

//	int fontFace = FONT_HERSHEY_SIMPLEX;
	int fontFace = 0;
	double fontScale = 1;
	int thickness = 2;

	putText(img, "a", scrimLnRect.a, fontFace, fontScale, CV_RGB(255, 255, 255), thickness,8);
	putText(img, "b", scrimLnRect.b, fontFace, fontScale, CV_RGB(255, 255, 255), thickness,8);
	putText(img, "c", scrimLnRect.c, fontFace, fontScale, CV_RGB(255, 255, 255), thickness,8);
	putText(img, "d", scrimLnRect.d, fontFace, fontScale, CV_RGB(255, 255, 255), thickness,8);

	return;
}

void plotYardLnsAndLos(Mat& img, struct rect& scrimLnRect, vector<vector<Point2d> > yardLns)
{
	for(unsigned int i = 0; i < yardLns.size(); ++i)
		line(img, yardLns[i][0], yardLns[i][1], CV_RGB(250, 250, 250),2,8,0);

	line(img, scrimLnRect.a, scrimLnRect.b, CV_RGB(0, 0, 250),2,8,0);
	line(img, scrimLnRect.b, scrimLnRect.c, CV_RGB(0, 0, 250),2,8,0);
	line(img, scrimLnRect.c, scrimLnRect.d, CV_RGB(0, 0, 250),2,8,0);
	line(img, scrimLnRect.d, scrimLnRect.a, CV_RGB(0, 0, 250),2,8,0);

//	int fontFace = FONT_HERSHEY_SIMPLEX;
	int fontFace = 0;
	double fontScale = 1;
	int thickness = 2;

	putText(img, "a", scrimLnRect.a, fontFace, fontScale, CV_RGB(0, 0, 250), thickness,8);
	putText(img, "b", scrimLnRect.b, fontFace, fontScale, CV_RGB(0, 0, 250), thickness,8);
	putText(img, "c", scrimLnRect.c, fontFace, fontScale, CV_RGB(0, 0, 250), thickness,8);
	putText(img, "d", scrimLnRect.d, fontFace, fontScale, CV_RGB(0, 0, 250), thickness,8);

	return;
}

void plotYardLnsAndLos(Mat& img, struct rect& scrimLnRect, vector<yardLine> yardlns, Point2d los[2])
{
	for(unsigned int i = 0; i < yardlns.size(); ++i)
	  drawLines(img, yardlns[i], CV_RGB(255, 255, 255));

	line(img, los[0], los[1], CV_RGB(255, 0, 255),2,8,0);

	line(img, scrimLnRect.a, scrimLnRect.b, CV_RGB(255, 0, 255),2,8,0);
	line(img, scrimLnRect.b, scrimLnRect.c, CV_RGB(255, 0, 255),2,8,0);
	line(img, scrimLnRect.c, scrimLnRect.d, CV_RGB(255, 0, 255),2,8,0);
	line(img, scrimLnRect.d, scrimLnRect.a, CV_RGB(255, 0, 255),2,8,0);

//	int fontFace = FONT_HERSHEY_SIMPLEX;
	int fontFace = 0;
	double fontScale = 1;
	int thickness = 2;

	putText(img, "a", scrimLnRect.a, fontFace, fontScale, CV_RGB(255, 0, 255), thickness,8);
	putText(img, "b", scrimLnRect.b, fontFace, fontScale, CV_RGB(255, 0, 255), thickness,8);
	putText(img, "c", scrimLnRect.c, fontFace, fontScale, CV_RGB(255, 0, 255), thickness,8);
	putText(img, "d", scrimLnRect.d, fontFace, fontScale, CV_RGB(255, 0, 255), thickness,8);

	return;
}

void plotYardLnsAndLos(Mat& img, struct rect& scrimLnRect, vector<vector<Point2d> > yardLns, Point2d los[2])
{
	for(unsigned int i = 0; i < yardLns.size(); ++i)
		line(img, yardLns[i][0], yardLns[i][1], CV_RGB(200, 200, 200),2,8,0);

	line(img, los[0], los[1], CV_RGB(255, 0, 255),2,8,0);

	line(img, scrimLnRect.a, scrimLnRect.b, CV_RGB(255, 0, 255),2,8,0);
	line(img, scrimLnRect.b, scrimLnRect.c, CV_RGB(255, 0, 255),2,8,0);
	line(img, scrimLnRect.c, scrimLnRect.d, CV_RGB(255, 0, 255),2,8,0);
	line(img, scrimLnRect.d, scrimLnRect.a, CV_RGB(255, 0, 255),2,8,0);

//	int fontFace = FONT_HERSHEY_SIMPLEX;
	int fontFace = 0;
	double fontScale = 1;
	int thickness = 2;

	putText(img, "a", scrimLnRect.a, fontFace, fontScale, CV_RGB(255, 0, 255), thickness,8);
	putText(img, "b", scrimLnRect.b, fontFace, fontScale, CV_RGB(255, 0, 255), thickness,8);
	putText(img, "c", scrimLnRect.c, fontFace, fontScale, CV_RGB(255, 0, 255), thickness,8);
	putText(img, "d", scrimLnRect.d, fontFace, fontScale, CV_RGB(255, 0, 255), thickness,8);

	return;
}


void plotLos(Mat& img, struct rect& scrimLnRect)
{
	plotRect(img, scrimLnRect, CV_RGB(0, 0, 250));

	int fontFace = 0;
	double fontScale = 1;
	int thickness = 2;

	putText(img, "a", scrimLnRect.a, fontFace, fontScale, CV_RGB(0, 0, 250), thickness,8);
	putText(img, "b", scrimLnRect.b, fontFace, fontScale, CV_RGB(0, 0, 250), thickness,8);
	putText(img, "c", scrimLnRect.c, fontFace, fontScale, CV_RGB(0, 0, 250), thickness,8);
	putText(img, "d", scrimLnRect.d, fontFace, fontScale, CV_RGB(0, 0, 250), thickness,8);
}


void plotLos(Mat& img, struct rect& scrimLnRect, Point2d los[2])
{
	line(img, los[0], los[1], CV_RGB(255, 0, 200),2,8,0);
	plotRect(img, scrimLnRect, CV_RGB(0, 0, 250));
}

void plotScanLines(Mat& img, vector<rect> &scanLines, const vector<int> &featureVec)
{
//	int fontFace = FONT_HERSHEY_SIMPLEX;
	int fontFace = 0;
	double fontScale = 1;
	int thickness = 2;

	for(unsigned int i = 0; i < scanLines.size(); ++i)
	{
//		Point2d E = scanLines[i].b + 10 * (scanLines[i].b - scanLines[i].a);
//		Point2d F = scanLines[i].d + 10 * (scanLines[i].c - scanLines[i].d);
//		line(img, scanLines[i].a, E, CV_RGB(200, 0, 0),2,8,0);
//		line(img, scanLines[i].d, F, CV_RGB(200, 0, 0),2,8,0);

//		line(img, scanLines[i].a, scanLines[i].b, CV_RGB(200, 0, 0),2,8,0);
//		line(img, scanLines[i].c, scanLines[i].d, CV_RGB(200, 0, 0),2,8,0);
		plotRect(img, scanLines[i], CV_RGB(200, 0, 0));

//		if(i == 10)
//		{
//			putText(img, "a", scanLines[i].a, fontFace, fontScale, CV_RGB(0, 0, 250), thickness,8);
//			putText(img, "b", scanLines[i].b, fontFace, fontScale, CV_RGB(0, 0, 250), thickness,8);
//			putText(img, "c", scanLines[i].c, fontFace, fontScale, CV_RGB(0, 0, 250), thickness,8);
//			putText(img, "d", scanLines[i].d, fontFace, fontScale, CV_RGB(0, 0, 250), thickness,8);
//		}


		ostringstream convertF;
		convertF << featureVec[i] ;
		string featStr = convertF.str();

		Point2d p = (scanLines[i].a + scanLines[i].b + scanLines[i].c + scanLines[i].d) * 0.25;

		putText(img, featStr, p , fontFace, fontScale, CV_RGB(200, 0, 0), thickness,8);

		ostringstream convertIdx;
		convertIdx << i ;
		string idx = convertIdx.str();

		putText(img, idx, scanLines[i].a * 0.7 + scanLines[i].b * 0.3, fontFace, fontScale, CV_RGB(0, 0, 200), thickness,8);

//		if((i % 2) == 0)
//			putText(img, featStr, p , fontFace, fontScale, CV_RGB(200, 0, 0), thickness,8);
//		else
//			putText(img, featStr, p , fontFace, fontScale, CV_RGB(0, 0, 200), thickness,8);

//		line(img, scanLines[i].a,  scanLines[i].b, CV_RGB(255, 0, 0),2,8,0);
//		line(img, scanLines[i].d,  scanLines[i].c, CV_RGB(255, 0, 0),2,8,0);
	}
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
//		ellipse(image, ellipses[i], color, 2, 8 );
//	}
//
//	string eliOutputImg = "ellipses/Game" + gameIdStr + "/video0" + vidIdxStr +".bmp";
//	imwrite(eliOutputImg, image);

}

void play::ellipseWrClassifier()
{
	vector<RotatedRect> ellipses;
	detectEllipsesFromFg(ellipses);

//	int fontFace = FONT_HERSHEY_SIMPLEX;
	int fontFace = 0;
	double fontScale = 1;
	int thickness = 2;

	Point2d vecVpToScrimCnt;
	if(vpExist)
	{
		vecVpToScrimCnt = scrimCnt - vp;
		vecVpToScrimCnt *= 1.0 / norm(vecVpToScrimCnt);
	}
	else
	{
		vecVpToScrimCnt = Point2d(.0, 1.0);
	}

	Point2d perpVecVpToScrimCnt = Point2d(-1.0 * vecVpToScrimCnt.y, vecVpToScrimCnt.x);
	if(perpVecVpToScrimCnt.x < 0.0)
		perpVecVpToScrimCnt = -1.0 * perpVecVpToScrimCnt;

	RNG rng(12345);
	for(unsigned int i = 0; i < ellipses.size(); ++i)
	{
		Point2f vecScrimCntToElp = Point2d((double)ellipses[i].center.x - scrimCnt.x, (double)ellipses[i].center.y - scrimCnt.y);

		double dotPro = vecScrimCntToElp.dot(vecVpToScrimCnt);
		double dotProPerp = vecScrimCntToElp.dot(perpVecVpToScrimCnt);
		double angScrimCntToElpCnt = atan(dotProPerp / dotPro);
		double wrScore = (norm(vecScrimCntToElp) / yardLnsDist) / (abs(angScrimCntToElpCnt) + 0.1);
		Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
		ellipse(image, ellipses[i], color, 2, 8 );

		ostringstream cvtScore;
//		cvtScore << double(int(wrScore * 1000.0)) / 1000.0;
		cvtScore << int(wrScore);
		string scoreStr = "S: " + cvtScore.str();
		putText(image, scoreStr, ellipses[i].center , fontFace, fontScale, CV_RGB(0, 0, 255), thickness,8);

	}

	plotYardLnsAndLos(image, scrimLn, yardLines);
	string eliOutputImg = "ellipses/Game" + gameIdStr + "/video0" + vidIdxStr +".bmp";
	imwrite(eliOutputImg, image);
}


void play::rectification()
{
	string matchesFile = "imgRectMatches/Game" + gameIdStr + "/vid" + vidIdxStr + "RectMatches";
	Mat dstImg, homoMat;
	//rectify mos frame
	rectifyImageToField(matchesFile, mosFrame, dstImg, homoMat);
	//rectMosFrame = dstImg.clone();
	rectMosFrame.create(FieldLength, FieldWidth, CV_32FC3);
	dstImg.copyTo(rectMosFrame);

	//rectfy foreground image
	rectifyImageToField(matchesFile, image, rectImage, homoMat);

	string dstImgPath = "rectImages/Game" + gameIdStr + "/vid" + vidIdxStr + "Rect.jpg";

//	struct rect rectScrimLn;
//	Point2d dstscrimCnt;
//	homoTransPoint(scrimLn.a, homoMat, rectScrimLn.a);
//	homoTransPoint(scrimLn.b, homoMat, rectScrimLn.b);
//	homoTransPoint(scrimLn.c, homoMat, rectScrimLn.c);
//	homoTransPoint(scrimLn.d, homoMat, rectScrimLn.d);

#if rectLosMethod == 1
	vector<Point2d> srcLosVec, dstLosVec;
	srcLosVec.push_back(scrimLn.a);
	srcLosVec.push_back(scrimLn.b);
	srcLosVec.push_back(scrimLn.c);
	srcLosVec.push_back(scrimLn.d);
	srcLosVec.push_back(scrimCnt);
	perspectiveTransform(srcLosVec, dstLosVec, homoMat);
	rectScrimLn.a = dstLosVec[0];
	rectScrimLn.b = dstLosVec[1];
	rectScrimLn.c = dstLosVec[2];
	rectScrimLn.d = dstLosVec[3];
	rectScrimCnt = dstLosVec[4];
#elif rectLosMethod == 2
	findLosOnRectFg(homoMat);

#endif

	//plotLos(dstImg, rectScrimLn);

	//homoTransPoint(scrimCnt, homoMat, dstscrimCnt);
	//circle(dstImg, rectScrimCnt, 3, CV_RGB(0, 0, 250), 3);

#if losMethod == 2
	vector<Point2d> srcRectLosVec, dstRectLosVec;
	srcRectLosVec.push_back(los[0]);
	srcRectLosVec.push_back(los[1]);
	perspectiveTransform(srcRectLosVec, dstRectLosVec, homoMat);
	rectLos[0] = dstRectLosVec[0];
	rectLos[1] = dstRectLosVec[1];
#endif

	imwrite(dstImgPath, dstImg);
}

void play::getOverheadFieldHomo(Mat &homoMat)
{
	string matchesFile = "imgRectMatches/Game" + gameIdStr + "/vid" + vidIdxStr + "RectMatches";
	Mat dstImg;
	transFieldToImage(matchesFile, dstImg, homoMat);

//	Mat blendImg, mosFCvtImg;
//	mosFrame.convertTo(mosFCvtImg, CV_32FC3);
//	addWeighted(mosFCvtImg, 0.5, dstImg, 0.5, 0.0, blendImg);
//	string dstImgPath = "rectImages/Game" + gameIdStr + "/vid" + vidIdxStr + "FldCoord.jpg";
//
//	imwrite(dstImgPath, blendImg);

}

//idx a b score ****
//****
//****
//
//Every three lines in the output is for one los candidate,
//the line is parameterized as y=ax+b,
//the score of this line is the 'score'.
bool readGradientLos(const string &filePath, Point2d los[2])
{
	ifstream fin(filePath.c_str());
	if(!fin.is_open())
	{
		cout << "Can't open file " << filePath << endl;
		return false;
	}

	fin.seekg(0, ios::end);
	if (fin.tellg() == 0) {
		cout << "Empty file" << filePath << endl;
		return false;
	}

	fin.seekg(0, ios::beg);

	double maxScore = NEGINF, maxA = NEGINF, maxB = NEGINF;
	while(!fin.eof())
	{
		int idx = NEGINF;
		double a = NEGINF, b = NEGINF, score = NEGINF;
		double tmp;
		fin >> idx >> a >> b >> score;
		if((idx == NEGINF) || (a == NEGINF)|| (b == NEGINF)|| (score == NEGINF))
			break;
		if(score >= maxScore)
		{
			maxScore = score;
			maxA = a;
			maxB = b;
		}
		//cout << idx << " " << a << " " << b << " " << score << endl;
		for(int i = 5; i <= 81; ++i)
			fin >> tmp;
	}

	fin.close();

	los[0].x = 0.0;
	los[0].y = maxB;
	if(los[0].y > imgYLen)
	{
		los[0].y = imgYLen;
		if(maxA != 0.0)
			los[0].x = (los[0].y - maxB) / maxA;
	}

	if(los[0].y < 0.0)
	{
		los[0].y = 0.0;
		if(maxA != 0.0)
			los[0].x = (los[0].y - maxB) / maxA;
	}

	los[1].x = imgXLen;
	los[1].y = maxA * los[1].x + maxB;

	if(los[1].y > imgYLen)
	{
		los[1].y = imgYLen;
		if(maxA != 0.0)
			los[1].x = (los[1].y - maxB) / maxA;
	}

	if(los[1].y < 0.0)
	{
		los[1].y = 0.0;
		if(maxA != 0.0)
			los[1].x = (los[1].y - maxB) / maxA;
	}

	if(los[0].y > los[1].y)
	{
		swap(los[0].x, los[1].x);
		swap(los[0].y, los[1].y);
	}

	return true;
}
