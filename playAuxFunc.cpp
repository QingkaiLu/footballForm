#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <limits>
#include <cv.h>
#include <ml.h>
#include "playAuxFunc.h"
#include "play.h"
#include "fieldModel.h"

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


void plotRect(Mat& img, struct rect& rct, Scalar clr)
{
	  line(img, rct.a, rct.b, clr,2,8,0);
	  line(img, rct.b, rct.c, clr,2,8,0);
	  line(img, rct.c, rct.d, clr,2,8,0);
	  line(img, rct.d, rct.a, clr,2,8,0);
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


int fgPixelsInsideBoxXY(const play *pl, const struct rect &box)
{
	int fgPixelsNum = 0;
	for(int y = box.a.y; y < box.c.y; ++y)
		for(int x = box.a.x; x < box.c.x; ++x)
		{
			const Point3_<uchar>* p = pl->fgImage.ptr<Point3_<uchar> >(y, x);
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

//idx a b score ****
//****
//****
//
//Every three lines in the output is for one losLine candidate,
//the line is parameterized as y=ax+b,
//the score of this line is the 'score'.
bool readGradientLos(const string &filePath, Point2d losLine[2])
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

	losLine[0].x = 0.0;
	losLine[0].y = maxB;
	if(losLine[0].y > imgYLen)
	{
		losLine[0].y = imgYLen;
		if(maxA != 0.0)
			losLine[0].x = (losLine[0].y - maxB) / maxA;
	}

	if(losLine[0].y < 0.0)
	{
		losLine[0].y = 0.0;
		if(maxA != 0.0)
			losLine[0].x = (losLine[0].y - maxB) / maxA;
	}

	losLine[1].x = imgXLen;
	losLine[1].y = maxA * losLine[1].x + maxB;

	if(losLine[1].y > imgYLen)
	{
		losLine[1].y = imgYLen;
		if(maxA != 0.0)
			losLine[1].x = (losLine[1].y - maxB) / maxA;
	}

	if(losLine[1].y < 0.0)
	{
		losLine[1].y = 0.0;
		if(maxA != 0.0)
			losLine[1].x = (losLine[1].y - maxB) / maxA;
	}

	if(losLine[0].y > losLine[1].y)
	{
		swap(losLine[0].x, losLine[1].x);
		swap(losLine[0].y, losLine[1].y);
	}

	return true;
}

void plotYardLnsAndLos(Mat& img, struct rect& scrimLnRect, vector<yardLine> yardlns, Point2d losLine[2])
{
	for(unsigned int i = 0; i < yardlns.size(); ++i)
	  drawLines(img, yardlns[i], CV_RGB(255, 255, 255));

	line(img, losLine[0], losLine[1], CV_RGB(255, 0, 255),2,8,0);

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


void plotLos(Mat& img, struct rect& scrimLnRect, Point2d losLine[2])
{
	line(img, losLine[0], losLine[1], CV_RGB(255, 0, 200),2,8,0);
	plotRect(img, scrimLnRect, CV_RGB(0, 0, 250));
}

void plotYardLnsAndLos(Mat& img, struct rect& scrimLnRect, vector<vector<Point2d> > yardLns)
{
	for(unsigned int i = 0; i < yardLns.size(); ++i)
		line(img, yardLns[i][0], yardLns[i][1], CV_RGB(0, 250, 0),2,8,0);

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

void plotYardLnsAndLos(Mat& img, struct rect& scrimLnRect, vector<vector<Point2d> > yardLns, Point2d losLine[2])
{
	for(unsigned int i = 0; i < yardLns.size(); ++i)
		line(img, yardLns[i][0], yardLns[i][1], CV_RGB(200, 200, 200),2,8,0);

	line(img, losLine[0], losLine[1], CV_RGB(255, 0, 255),2,8,0);

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

void plotPlayerPosBox(Mat& img, struct rect& playerBox, string pTypeString)
{
	int fontFace = 0;
	double fontScale = 1;
	int thickness = 2;
	plotRect(img, playerBox, CV_RGB(0, 0, 255));
	putText(img, pTypeString, playerBox.a, fontFace, fontScale, CV_RGB(255, 0, 255), thickness,8);

}

Mat subtractEdgeImg(const Mat &img, const Mat &bg) {
//Mat subtractEdgeImg(Mat img, Mat bg, Mat originImg, Mat originBg) {
	Mat result = Mat::zeros(img.size(), img.type());
//	Mat result2 = Mat::zeros(img.size(), img.type());
	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			uchar imgTmp = img.at<uchar>(i, j);
			uchar minDiff = imgTmp - bg.at<uchar>(i, j);
//			int minR = i, minC = j;
			for (int r = MAX(0, i-5); r < MIN(i+5, img.rows); r++) {
				for (int c = MAX(0, j-5); c < MIN(j+5, img.cols); c++) {
					if (imgTmp - bg.at<uchar>(r, c) < minDiff) {
						minDiff = imgTmp - bg.at<uchar>(r, c);
					}
				}
			}
			result.at<uchar>(i, j) = minDiff;
//			result2.at<uchar>(i, j) = originImg.at<uchar>(i, j) - originBg.at<uchar>(i, j);
		}
	}
	return result;
}

vector<Mat> readHomographs(const string &fileName) {
	ifstream fin(fileName.c_str());
	vector<Mat> results;
	int frameNum;
	fin >> frameNum;

	for (int i = 0; i < frameNum; i++) {
		int row, col;
		fin >> row >> col;
		Mat A(row, col, CV_64F);
		for (int r = 0; r < 3; r++) {
			for (int c = 0; c < 3; c++) {
				double tmp;
				fin >> tmp;
				A.at<double>(r, c) = tmp;
			}
		}
		results.push_back(A);
	}
	return results;
}

void readFormsFile(const string &formsFile, direction offSide, vector<string> &formations)
{
	string dir;
	if(offSide == leftDir)
		dir = "Left";
	else if(offSide == rightDir)
		dir = "Right";
	else
	{
		cout << "Wrong direction in detectForms()." << endl;
		return;
	}
	ifstream fin(formsFile.c_str());

	if(!fin.is_open())
	{
		cout << "Can't open file " << formsFile << endl;
		return;
	}

	fin.seekg(0, ios::end);
	if (fin.tellg() == 0) {
		cout << "Empty file " << formsFile << endl;
		return;
	}
	fin.seekg(0, ios::beg);
	while(!fin.eof())
	{
		string form;
		fin >> form;
		if(form.empty())
			break;
		form = form + dir;
		formations.push_back(form);
//		cout << form << endl;
	}
	fin.close();
}

void readFormsFile(const string &formsFile, vector<string> &formations)
{
	ifstream fin(formsFile.c_str());

	if(!fin.is_open())
	{
		cout << "Can't open file " << formsFile << endl;
		return;
	}

	fin.seekg(0, ios::end);
	if (fin.tellg() == 0) {
		cout << "Empty file " << formsFile << endl;
		return;
	}
	fin.seekg(0, ios::beg);
	while(!fin.eof())
	{
		string form;
		fin >> form;
		if(form.empty())
			break;
		formations.push_back(form);
//		cout << form << endl;
	}
	fin.close();
}


void readPlayerBndBoxes(const string &playersFilePath, 	vector<double> &scores,
		vector<struct rect> &players, vector<double> &areas)
{
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
//	vector<double> scores;
//	vector<struct rect> players;
//	vector<double> areas;
	while(!fin.eof())
	{
		int playId = NEGINF;
		double score = NEGINF;
		double xMin, yMin, xMax, yMax;
		fin >> playId >> score >> xMin >> yMin >> xMax >> yMax;
		if(score == NEGINF)
			break;
//		cout << playId << score << endl; // >> xMin >> yMin >> xMax >> yMax;
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

	fin.close();
}

void readFormationGt(const string &formFilePath, vector<struct rect> &players,
	rect &losBndBox, Point2d &losCnt)
{
	ifstream fin(formFilePath.c_str());

	if(!fin.is_open())
	{
		cout << "Can't open file " << formFilePath << endl;
		return;
	}

	fin.seekg(0, ios::end);
	if (fin.tellg() == 0) {
		cout << "Empty file " << formFilePath << endl;
		return;
	}
	fin.seekg(0, ios::beg);
	string tmp, formName;
	fin >> tmp >> formName;
	fin >> tmp >> losCnt.x >> losCnt.y;
//	cout << losCnt.x << " " << losCnt.y << endl;
	int itr = 0;
	while(!fin.eof())
	{
		string pType;
		double xMin = NEGINF, yMin, xMax, yMax;
		fin >> pType >> xMin >> yMin >> xMax >> yMax;
		if(xMin == NEGINF)
			break;
//		cout << playId << score << endl; // >> xMin >> yMin >> xMax >> yMax;
		struct rect player;
		player.a = Point2d(xMin, yMin);
		player.b = Point2d(xMin, yMax);
		player.c = Point2d(xMax, yMax);
		player.d = Point2d(xMax, yMin);
		if(itr == 0)//los bounding box
			losBndBox = player;
		else
			players.push_back(player);
		++itr;
	}

	fin.close();
}


void readFormationGt(const string &formFilePath, vector<struct rect> &players,
		vector<string> &pTypes, rect &losBndBox, Point2d &losCnt)
{
	ifstream fin(formFilePath.c_str());

	if(!fin.is_open())
	{
		cout << "Can't open file " << formFilePath << endl;
		return;
	}

	fin.seekg(0, ios::end);
	if (fin.tellg() == 0) {
		cout << "Empty file " << formFilePath << endl;
		return;
	}
	fin.seekg(0, ios::beg);
	string tmp, formName;
	fin >> tmp >> formName;
	fin >> tmp >> losCnt.x >> losCnt.y;
//	cout << losCnt.x << " " << losCnt.y << endl;
	int itr = 0;
	while(!fin.eof())
	{
		string pType;
		double xMin = NEGINF, yMin, xMax, yMax;
		fin >> pType >> xMin >> yMin >> xMax >> yMax;
		if(xMin == NEGINF)
			break;
//		cout << playId << score << endl; // >> xMin >> yMin >> xMax >> yMax;
		struct rect player;
		player.a = Point2d(xMin, yMin);
		player.b = Point2d(xMin, yMax);
		player.c = Point2d(xMax, yMax);
		player.d = Point2d(xMax, yMin);
		if(itr == 0)//los bounding box
			losBndBox = player;
		else
		{
			players.push_back(player);
			pTypes.push_back(pType);
//			cout << pType << endl;
//			cout << player.a.x << ", " << player.a.y << endl;
		}
		++itr;
	}

	fin.close();
}

void plotRectAvgClr(Mat& img, const struct rect& rct, Scalar clr, Point3d &avgClr)
{
	line(img, rct.a, rct.b, clr,2,8,0);
	line(img, rct.b, rct.c, clr,2,8,0);
	line(img, rct.c, rct.d, clr,2,8,0);
	line(img, rct.d, rct.a, clr,2,8,0);

//	Point3d avgClr(.0, .0, .0);
//	for(int x = rct.a.x - 1; x < rct.c.x; ++x)
//	  for(int y = rct.a.y - 1; y < rct.c.y; ++y)
//		  {
//			  Point3_<uchar>* p = img.ptr<Point3_<uchar> >(y,x);
//			  avgClr.x += p->x;//B
//			  avgClr.y += p->y;//G
//			  avgClr.z += p->z;//R
//		  }
//	avgClr *= 1.0 / ((rct.c.x - rct.a.x) * (rct.c.y - rct.a.y));

	avgClr = Point3d(.0, .0, .0);
	Point2d cnt = 0.5 * (rct.a + rct.c);
	int w = 10;
	for(int x = -1.0 * w; x <= w; ++x)
	  for(int y = -1.0 * w; y <= w; ++y)
		  {
			  Point3_<uchar>* p = img.ptr<Point3_<uchar> >(cnt.y - 1 + y, cnt.x - 1 + x);
			  avgClr.x += p->x;//B
			  avgClr.y += p->y;//G
			  avgClr.z += p->z;//R
		  }
	avgClr *= 1.0 / ((2 * w + 1) * (2 * w + 1));
	line(img, cnt + Point2d(-1.0 * w, -1.0 * w), cnt + Point2d(-1.0 * w, w), clr,2,8,0);
	line(img, cnt + Point2d(-1.0 * w, w), cnt + Point2d(w, w), clr,2,8,0);
	line(img, cnt + Point2d(w, w), cnt + Point2d(w, -1.0 * w), clr,2,8,0);
	line(img, cnt + Point2d(w, -1.0 * w), cnt + Point2d(-1.0 * w, -1.0 * w), clr,2,8,0);


//	cout << int(avgClr.x) << " " << int(avgClr.y)  << " " << int(avgClr.z) << endl;

//	int fontFace = 0;
//	double fontScale = 1;
//	int thickness = 2;
//
//	ostringstream convertClrB;
//	convertClrB << int(avgClr.x);
//	string B = convertClrB.str();
//
//	ostringstream convertClrG;
//	convertClrG << int(avgClr.y);
//	string G = convertClrG.str();
//
//	ostringstream convertClrR;
//	convertClrR << int(avgClr.z);
//	string R = convertClrR.str();
//
//	string RGB = "(" + R + "," + G + "," + B + ")";
//
//	putText(img, RGB, rct.b, fontFace, fontScale, CV_RGB(0, 0, 255), thickness,8);

}

void getRectAvgClr(Mat& img, const struct rect& rct, Point3d &avgClr)
{
	avgClr = Point3d(.0, .0, .0);
	for(int x = rct.a.x; x <= rct.c.x; ++x)
	  for(int y = rct.a.y; y <= rct.c.y; ++y)
		  {
			  Point3_<uchar>* p = img.ptr<Point3_<uchar> >(y, x);
			  avgClr.x += p->x;//B
			  avgClr.y += p->y;//G
			  avgClr.z += p->z;//R
		  }
	avgClr *= 1.0 / (double)((rct.c.x - rct.a.x) * (rct.c.y - rct.a.y));

//	Point2d cnt = 0.5 * (rct.a + rct.c);
//	int w = 10;
//	for(int x = -1.0 * w; x <= w; ++x)
//	  for(int y = -1.0 * w; y <= w; ++y)
//		  {
//			  Point3_<uchar>* p = img.ptr<Point3_<uchar> >(cnt.y - 1 + y, cnt.x - 1 + x);
//			  avgClr.x += p->x;//B
//			  avgClr.y += p->y;//G
//			  avgClr.z += p->z;//R
//		  }
//	avgClr *= 1.0 / ((2 * w + 1) * (2 * w + 1));
//	line(img, cnt + Point2d(-1.0 * w, -1.0 * w), cnt + Point2d(-1.0 * w, w), clr,2,8,0);
//	line(img, cnt + Point2d(-1.0 * w, w), cnt + Point2d(w, w), clr,2,8,0);
//	line(img, cnt + Point2d(w, w), cnt + Point2d(w, -1.0 * w), clr,2,8,0);
//	line(img, cnt + Point2d(w, -1.0 * w), cnt + Point2d(-1.0 * w, -1.0 * w), clr,2,8,0);

}

void getOrgImgRectAvgClr(Mat& img, const struct rect& rct, const Mat &fldToOrgHMat, Point3d &avgClr)
{
	avgClr = Point3d(.0, .0, .0);
	vector<Point2d> srcLosVec, dstLosVec;
	int step = 2;
	for(int x = rct.a.x; x <= rct.c.x; x += step)
	  for(int y = rct.a.y; y <= rct.c.y; y += step)
		  srcLosVec.push_back(Point2d(x, y));
	perspectiveTransform(srcLosVec, dstLosVec, fldToOrgHMat);
	for(unsigned int i = 0; i < dstLosVec.size(); ++i)
	{
		  Point3_<uchar>* p = img.ptr<Point3_<uchar> >(int(dstLosVec[i].y), int(dstLosVec[i].x));
		  avgClr.x += p->x;//B
		  avgClr.y += p->y;//G
		  avgClr.z += p->z;//R
	}
	avgClr *= 1.0 / (double)dstLosVec.size();
}

void getOffensePlayers(vector<Point2d> &playersLocSet, vector<Point2d> &pLocSetFld,
		play* p, vector<struct rect> &players, direction offDir)
{
	int fontFace = 0;
	double fontScale = 1;
	int thickness = 2;

	vector<Point2d> pInsideFldSet, pInsideFldSetFld;
	vector<struct rect> playersInsideFld;
	for(unsigned int i = 0; i< pLocSetFld.size(); ++i)
	{
		if(p->fld->isPointInsideFld(pLocSetFld[i]))
		{
			pInsideFldSet.push_back(playersLocSet[i]);
			pInsideFldSetFld.push_back(pLocSetFld[i]);
			playersInsideFld.push_back(players[i]);

		}
	}
	Mat samples(playersInsideFld.size(), 3, CV_32F);
	for(unsigned int i = 0; i < playersInsideFld.size(); ++i)
	{
//		plotRect(mosFrame, players[i], Scalar(0, 0, 255));
		Point3d avgClr;
		plotRectAvgClr(p->mosFrame, playersInsideFld[i], Scalar(0, 0, 255), avgClr);
		samples.at<float>(i, 0) = avgClr.x;
		samples.at<float>(i, 1) = avgClr.y;
		samples.at<float>(i, 2) = avgClr.z;
	}

	int K = 2;
	Mat labels;
	int attempts = 5;
	Mat centers;
	kmeans(samples, K, labels, TermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 10000, 0.0001), attempts, KMEANS_PP_CENTERS, centers );

	int num1 = 0, num2 = 0;
	for(unsigned int i = 0; i < pInsideFldSetFld.size(); ++i)
	{
		if((offDir == leftDir && pInsideFldSetFld[i].x < p->rectLosCnt.x) ||
				(offDir == rightDir && pInsideFldSetFld[i].x > p->rectLosCnt.x))
		{
			if(labels.at<int>(i, 0) == 0)
				++num1;
			else if(labels.at<int>(i, 0) == 1)
				++num2;
		}
	}

	int offLabel;
	if(num1 > num2)
		offLabel = 0;
	else if(num1 < num2)
		offLabel = 1;
	else
	{
//		offLabel = 0;
		cout << "Can not decide the color of offense players." << endl;
		return;
	}

	vector<Point2d> pOffSet, pOffSetFld;
	vector<struct rect> playersOff;

	for(unsigned int i = 0; i < pInsideFldSetFld.size(); ++i)
	{
		ostringstream convertScore;
		convertScore << labels.at<int>(i, 0);
		string scoreStr = convertScore.str();
		putText(p->mosFrame, scoreStr, playersInsideFld[i].a, fontFace, fontScale, CV_RGB(0, 0, 255), thickness,8);

		if(labels.at<int>(i, 0) == offLabel)
		{
			//get rid of the players which are on the defense side and far away(5 yard) from the LOS
			if((offDir == leftDir && pInsideFldSetFld[i].x < (p->rectLosCnt.x + 5 * 3 * 5)) ||
					(offDir == rightDir && pInsideFldSetFld[i].x > (p->rectLosCnt.x - 5 * 3 * 5)))
			{
				pOffSet.push_back(pInsideFldSet[i]);
				pOffSetFld.push_back(pInsideFldSetFld[i]);
				playersOff.push_back(playersInsideFld[i]);
			}
		}
	}

	playersLocSet = pOffSet;
	pLocSetFld = pOffSetFld;
	players = playersOff;

	for(unsigned int i = 0; i < players.size(); ++i)
		plotRect(p->mosFrame, players[i], Scalar(255, 0, 0));

}

void getOffsPlayersByLos(vector<Point2d> &playersLocSet, vector<Point2d> &pLocSetFld, const vector<Point2d> &pFeetLocSetFld,
		play* p, vector<struct rect> &players, direction offDir)
{
	vector<Point2d> pInsideFldSet, pInsideFldSetFld;
	vector<struct rect> playersInsideFld;
	for(unsigned int i = 0; i< pLocSetFld.size(); ++i)
	{
//		if(p->fld->isPointInsideFld(pLocSetFld[i]))
		if(p->fld->isPointInsideFld(pLocSetFld[i]) ||
				//in case the center of wide receivers on top is outside of field
				// 45/15 = 3 yards
				p->fld->isPointInsideFld(pLocSetFld[i] + Point2d(0, 45)))
//				p->fld->isPointInsideFld(pFeetLocSetFld[i]))
		{
			pInsideFldSet.push_back(playersLocSet[i]);
			pInsideFldSetFld.push_back(pLocSetFld[i]);
			playersInsideFld.push_back(players[i]);

		}
	}

	vector<Point2d> pOffSet, pOffSetFld;
	vector<struct rect> playersOff;

	for(unsigned int i = 0; i < pInsideFldSetFld.size(); ++i)
	{
//		if((offDir == leftDir && pInsideFldSetFld[i].x < p->rectLosCnt.x) ||
//				(offDir == rightDir && pInsideFldSetFld[i].x > p->rectLosCnt.x))
		//relax LOS by 1 yard
//		if((offDir == leftDir && pInsideFldSetFld[i].x < p->rectLosCnt.x + 15) ||
//				(offDir == rightDir && pInsideFldSetFld[i].x > p->rectLosCnt.x - 15))
		if((offDir == leftDir && pInsideFldSetFld[i].x < p->rectLosCnt.x + 5) ||
				(offDir == rightDir && pInsideFldSetFld[i].x > p->rectLosCnt.x - 5))
		{
			pOffSet.push_back(pInsideFldSet[i]);
			pOffSetFld.push_back(pInsideFldSetFld[i]);
			playersOff.push_back(playersInsideFld[i]);
		}
	}

	playersLocSet = pOffSet;
	pLocSetFld = pOffSetFld;
	players = playersOff;

//	for(unsigned int i = 0; i < players.size(); ++i)
//		plotRect(p->mosFrame, players[i], Scalar(255, 0, 0));
}

void getPlayersInsdFld(vector<Point2d> &playersLocSet, vector<Point2d> &pLocSetFld,
		play* p, vector<struct rect> &players)
{
	vector<Point2d> pInsideFldSet, pInsideFldSetFld;
	vector<struct rect> playersInsideFld;
	for(unsigned int i = 0; i< pLocSetFld.size(); ++i)
	{
//		if(p->fld->isPointInsideFld(pLocSetFld[i]))
		if(p->fld->isPointInsideFld(pLocSetFld[i]) ||
				//in case the center of wide receivers on top is outside of field
				// 45/15 = 3 yards
				p->fld->isPointInsideFld(pLocSetFld[i] + Point2d(0, 45)))
//				p->fld->isPointInsideFld(pFeetLocSetFld[i]))
		{
			pInsideFldSet.push_back(playersLocSet[i]);
			pInsideFldSetFld.push_back(pLocSetFld[i]);
			playersInsideFld.push_back(players[i]);

		}
	}
	playersLocSet = pInsideFldSet;
	pLocSetFld = pInsideFldSetFld;
	players = playersInsideFld;
}

void getOffsPlayersByLos(vector<Point2d> &pLocSetFld, play* p, direction offDir)
{
	vector<Point2d> pInsideFldSet, pInsideFldSetFld;
	vector<struct rect> playersInsideFld;
	for(unsigned int i = 0; i< pLocSetFld.size(); ++i)
	{
//		if(p->fld->isPointInsideFld(pLocSetFld[i]))
		if(p->fld->isPointInsideFld(pLocSetFld[i]) ||
				//in case the center of wide receivers on top is outside of field
				// 30/15 = 2 yards
				p->fld->isPointInsideFld(pLocSetFld[i] + Point2d(0, 30)))
		{
			pInsideFldSetFld.push_back(pLocSetFld[i]);

		}
	}

	vector<Point2d> pOffSet, pOffSetFld;
	vector<struct rect> playersOff;

	for(unsigned int i = 0; i < pInsideFldSetFld.size(); ++i)
	{
//		if((offDir == leftDir && pInsideFldSetFld[i].x < p->rectLosCnt.x) ||
//				(offDir == rightDir && pInsideFldSetFld[i].x > p->rectLosCnt.x))
		//relax LOS by 1 yard
		if((offDir == leftDir && pInsideFldSetFld[i].x < p->rectLosCnt.x + 15) ||
				(offDir == rightDir && pInsideFldSetFld[i].x > p->rectLosCnt.x - 15))
		{
			pOffSet.push_back(pInsideFldSet[i]);
			pOffSetFld.push_back(pInsideFldSetFld[i]);
			playersOff.push_back(playersInsideFld[i]);
		}
	}

	pLocSetFld = pOffSetFld;
}

//void getOffensePlayers(vector<Point2d> &playersLocSet, vector<Point2d> &pLocSetFld,
//		play* p, vector<struct rect> &players, direction offDir, vector<double> &scores)
//{
//	scores.clear();
//	int fontFace = 0;
//	double fontScale = 1;
//	int thickness = 2;
//
//	vector<Point2d> pInsideFldSet, pInsideFldSetFld;
//	vector<struct rect> playersInsideFld;
//	for(unsigned int i = 0; i< pLocSetFld.size(); ++i)
//	{
//		if(p->fld->isPointInsideFld(pLocSetFld[i]))
//		{
//			pInsideFldSet.push_back(playersLocSet[i]);
//			pInsideFldSetFld.push_back(pLocSetFld[i]);
//			playersInsideFld.push_back(players[i]);
//
//		}
//	}
//	Mat samples(playersInsideFld.size(), 3, CV_32F);
//	for(unsigned int i = 0; i < playersInsideFld.size(); ++i)
//	{
////		plotRect(mosFrame, players[i], Scalar(0, 0, 255));
//		Point3d avgClr;
//		plotRectAvgClr(p->mosFrame, playersInsideFld[i], Scalar(0, 0, 255), avgClr);
//		samples.at<float>(i, 0) = avgClr.x;
//		samples.at<float>(i, 1) = avgClr.y;
//		samples.at<float>(i, 2) = avgClr.z;
//	}
//
//	int K = 2;
//	Mat labels;
//	int attempts = 5;
//	Mat centers;
//	kmeans(samples, K, labels, TermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 10000, 0.0001), attempts, KMEANS_PP_CENTERS, centers );
//
//	int num1 = 0, num2 = 0;
//	for(unsigned int i = 0; i < pInsideFldSetFld.size(); ++i)
//	{
//		if((offDir == leftDir && pInsideFldSetFld[i].x < p->rectLosCnt.x) ||
//				(offDir == rightDir && pInsideFldSetFld[i].x > p->rectLosCnt.x))
//		{
//			int label = labels.at<int>(i, 0);
//			if(label == 0)
//				++num1;
//			else if(label == 1)
//				++num2;
//			Point3d uniformClr(samples.at<float>(i, 0), samples.at<float>(i, 1), samples.at<float>(i, 2));
//			Point3d cntClr(centers.at<float>(label, 0), centers.at<float>(label, 1), centers.at<float>(label, 2));
//			scores.push_back(norm(uniformClr - cntClr));
//		}
//	}
//
//	int offLabel;
//	if(num1 > num2)
//		offLabel = 0;
//	else if(num1 < num2)
//		offLabel = 1;
//	else
//	{
////		offLabel = 0;
//		cout << "Can not decide the color of offense players." << endl;
//		return;
//	}
//
//	vector<Point2d> pOffSet, pOffSetFld;
//	vector<struct rect> playersOff;
//
//	for(unsigned int i = 0; i < pInsideFldSetFld.size(); ++i)
//	{
//		ostringstream convertScore;
//		convertScore << labels.at<int>(i, 0);
//		string scoreStr = convertScore.str();
//		putText(p->mosFrame, scoreStr, playersInsideFld[i].a, fontFace, fontScale, CV_RGB(0, 0, 255), thickness,8);
//
//		if(labels.at<int>(i, 0) == offLabel)
//		{
//			//get rid of the players which are on the defense side and far away(5 yard) from the LOS
//			if((offDir == leftDir && pInsideFldSetFld[i].x < (p->rectLosCnt.x + 5 * 3 * 5)) ||
//					(offDir == rightDir && pInsideFldSetFld[i].x > (p->rectLosCnt.x - 5 * 3 * 5)))
//			{
//				pOffSet.push_back(pInsideFldSet[i]);
//				pOffSetFld.push_back(pInsideFldSetFld[i]);
//				playersOff.push_back(playersInsideFld[i]);
//			}
//		}
//	}
//
//	playersLocSet = pOffSet;
//	pLocSetFld = pOffSetFld;
//	players = playersOff;
//
//	for(unsigned int i = 0; i < players.size(); ++i)
//		plotRect(p->mosFrame, players[i], Scalar(255, 0, 0));
//
//}


void getRectLosPnts(const struct rect &rectLosBndBox, std::vector<cv::Point2d> &olLocSet)
{
	int xMin = rectLosBndBox.a.x;
	int xMax = rectLosBndBox.c.x;
	int yMin = rectLosBndBox.a.y;
	int yMax = rectLosBndBox.c.y;
//	int step = 5;
	int step = 1;
	for(int x = xMin; x <= xMax; x += step)
		for(int y = yMin; y <= yMax; y += step)
			olLocSet.push_back(Point2d(x, y));
}

int convertPTypeToPId(const string &pType)
{
	int pId = -1;
	if(pType.compare("WRTop") == 0 || pType.compare("WRBot") == 0
			|| pType.compare("WRMid") == 0 || pType.compare("WR") == 0)
		pId = 1;
	if(pType.compare("QB") == 0)
		pId = 2;
	if(pType.compare("RB") == 0)
		pId = 3;
	if(pType.compare("C") == 0)
		pId = 4;
	if(pType.compare("G") == 0)
		pId = 5;
	if(pType.compare("T") == 0)
		pId = 6;
	if(pType.compare("TE") == 0)
		pId = 7;
	if(pType.compare("H-b") == 0)
		pId = 8;
	if(pId == -1)
		cout << "Convert to pID wrong!" << endl;
	return pId;
}

string convertPIdToPType(int pId)
{
//	if(pId <= 0 || pId > 3)
//	{
//		cout << "Convert to pType wrong!" << endl;
//		return NULL;
//	}
	string pType;
	switch(pId){
	case 1:
		pType = "WR";
		break;
	case 2:
		pType = "QB";
		break;
	case 3:
		pType = "RB";
		break;
	case 4:
		pType = "C";
		break;
	case 5:
		pType = "G";
		break;
	case 6:
		pType = "T";
		break;
	case 7:
		pType = "TE";
		break;
	case 8:
		pType = "H-b";
		break;
	default:
		cout << "Convert to pType wrong!" << endl;
		break;
	}

	return pType;
}

void computeLosCntPosCost(play* p, const vector<Point2d> &offsPLocSetFld, const Mat &trainFeaturesMat,
		const Mat &trainLabelsMat, vector<double> &allCosts, vector<string> &playersTypes)
{

		Mat testFeaturesMat = Mat(offsPLocSetFld.size(), 2, CV_32FC1);
		for(unsigned int i = 0; i < offsPLocSetFld.size(); ++i)
		{
			Point2d pToLosVec = offsPLocSetFld[i] - p->rectLosCnt;
			testFeaturesMat.at<float>(i, 0) = pToLosVec.x;
			testFeaturesMat.at<float>(i, 1) = pToLosVec.y;
		}

		int K = 3;
		CvKNearest knn(trainFeaturesMat, trainLabelsMat, Mat(), false, K);
		Mat neighborResponses(offsPLocSetFld.size(), K, CV_32FC1);
		Mat results(offsPLocSetFld.size(), 1, CV_32FC1), dists(offsPLocSetFld.size(), K, CV_32FC1);
		knn.find_nearest(testFeaturesMat, K, results, neighborResponses, dists);

//		vector<string> playersTypes;
		unsigned int QBNum = 0, QBIdx = -1;
		double minQBCost = INF;
		for(unsigned int i = 0; i < offsPLocSetFld.size(); ++i)
		{
			string pType = convertPIdToPType(int(results.at<float>(i, 0)));
//			cout << pType << endl;
			playersTypes.push_back(pType);
			double cost = .0;
			for(int k = 0; k < K; ++k)
				//convert from pixel distance to feet(/5), then to yard(/3)
				//dists is the square of Euclidean distance!
				cost += sqrt(dists.at<float>(i, k)) / 15;
			cost /= K;
			//cout << "cost: " << cost << endl;
			allCosts.push_back(cost);
			if(pType == "QB")
			{
				++QBNum;
				if(cost < minQBCost)
				{
					minQBCost = cost;
					QBIdx = i;
				}
			}
		}
//		for(unsigned int i = 0; i < playersTypes.size(); ++i)
//			cout << playersTypes[i] << endl;
//		sort(allCosts.begin(), allCosts.end());
		if(QBNum >= 2)
		{
			cout << "More than 1 QB!" << endl;
			Mat trainFeaturesMatNoQB = Mat(1, 2, CV_32FC1);
			Mat trainLabelsMatNoQB = Mat(1, 1, CV_32FC1, -1);
			for(int i = 0; i < trainLabelsMat.rows; ++i)
			{
				// not QB in training set
				if(trainLabelsMat.at<float>(i, 0) != 2)
				{
//					if(trainLabelsMatNoQB.rows == 1)
					if(trainLabelsMatNoQB.at<float>(0, 0) == -1)
					{
//						cout << "i " << i << "trainLabelsMatNoQB.rows " << trainLabelsMatNoQB.rows << endl;
						trainLabelsMatNoQB.at<float>(0, 0) = trainLabelsMat.at<float>(i, 0);
//						cout << trainLabelsMat.at<float>(1, 0) << endl;
//						cout << trainLabelsMat.at<float>(i, 0) << endl;
						trainFeaturesMatNoQB.at<float>(0, 0) = trainFeaturesMat.at<float>(i, 0);
						trainFeaturesMatNoQB.at<float>(0, 1) = trainFeaturesMat.at<float>(i, 1);
					}
					else
					{
//						cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"<< endl;
						vconcat(trainLabelsMatNoQB, trainLabelsMat.row(i), trainLabelsMatNoQB);
						vconcat(trainFeaturesMatNoQB, trainFeaturesMat.row(i), trainFeaturesMatNoQB);
					}
				}
			}
			CvKNearest knnNoQB(trainFeaturesMatNoQB, trainLabelsMatNoQB, Mat(), false, K);

			for(unsigned int i = 0; i < offsPLocSetFld.size(); ++i)
			{
				if(i == QBIdx)
					continue;

				Mat testFeaturesMatNoQB = Mat(1, 2, CV_32FC1);
				testFeaturesMatNoQB.at<float>(0, 0) = testFeaturesMat.at<float>(i, 0);
				testFeaturesMatNoQB.at<float>(0, 1) = testFeaturesMat.at<float>(i, 1);
				Mat neighborResponsesNoQB(1, K, CV_32FC1);
				Mat resultsNoQB(1, 1, CV_32FC1), distsNoQB(1, K, CV_32FC1);
//				cout << "trainFeaturesMat" << endl << trainFeaturesMat << endl;
//				cout << "trainLabelsMat" << endl << trainLabelsMat << endl;
//				cout << "trainFeaturesMatNoQB" << endl << trainFeaturesMatNoQB << endl;
//				cout << "trainLabelsMatNoQB" << endl << trainLabelsMatNoQB << endl;
				float resultNoQB = knnNoQB.find_nearest(testFeaturesMatNoQB, K, resultsNoQB, neighborResponsesNoQB, distsNoQB);

				string pType = convertPIdToPType(int(resultNoQB));
				playersTypes[i] = pType;
				double cost = .0;
				for(int k = 0; k < K; ++k)
					//convert from pixel distance to feet(/5), then to yard(/3)
					//dists is the square of Euclidean distance!
					cost += sqrt(distsNoQB.at<float>(0, k)) / 15;
				cost /= K;
				//cout << "cost: " << cost << endl;
				allCosts[i] = cost;
			}
		}
}

double getUniformClrCost(const vector<struct rect> &offsPlayers, play* p)
{
	vector<Point3d> allUfmClrs;
	Point3d ufmClrMean(0, 0, 0);
	for(unsigned int i = 0; i < offsPlayers.size(); ++i)
	{
//		plotRect(mosFrame, players[i], Scalar(0, 0, 255));
		Point3d ufmClr;
		getRectAvgClr(p->mosFrame, offsPlayers[i], ufmClr);
		ufmClrMean += ufmClr;
		allUfmClrs.push_back(ufmClr);
	}

	ufmClrMean *= 1.0 / offsPlayers.size();

	double clrCost = 0;
	for(unsigned int i = 0; i < allUfmClrs.size(); ++i)
		clrCost += norm(allUfmClrs[i] - ufmClrMean);

	clrCost /= offsPlayers.size();
	return clrCost;
}

double getColorDif(const Rect &r, Mat &img)
{
//	Mat m1(img, Rect(r.x, r.y, r.width * 0.5, r.height));
//	Mat m2(img, Rect(r.x + r.width * 0.5, r.y, r.width * 0.5, r.height));
//	Scalar mean1 = mean(m1);
//	Scalar mean2 = mean(m2);
//	double diff = 0;
//	for(int i = 0; i < 3; ++i)
//		diff += abs(mean1[i] - mean2[i]);
//	return diff;

	rect rLeft, rRight;
	rLeft.a = Point2d(r.x, r.y);
	rLeft.b = Point2d(r.x, r.y + r.height * 0.5);
	rLeft.c = Point2d(r.x + r.width * 0.5, r.y + r.height * 0.5);
	rLeft.d = Point2d(r.x + r.width * 0.5, r.y);
	rRight.a = rLeft.a + Point2d(r.width * 0.5);
	rRight.b = rLeft.b + Point2d(r.width * 0.5);
	rRight.c = rLeft.c + Point2d(r.width * 0.5);
	rRight.d = rLeft.d + Point2d(r.width * 0.5);
	Point3d leftClr, rightClr;
	getRectAvgClr(img, rLeft, leftClr);
	getRectAvgClr(img, rRight, rightClr);
	double diff = abs(leftClr.x - rightClr.x + leftClr.y - rightClr.y +
			leftClr.z - rightClr.z);
	return diff;
}

double getOrgImgColorDif(const Rect &r, Mat &img, const Mat &fldToOrgHMat)
{
	rect rLeft, rRight;
	rLeft.a = Point2d(r.x, r.y);
	rLeft.b = Point2d(r.x, r.y + r.height * 0.5);
	rLeft.c = Point2d(r.x + r.width * 0.5, r.y + r.height * 0.5);
	rLeft.d = Point2d(r.x + r.width * 0.5, r.y);
	rRight.a = rLeft.a + Point2d(r.width * 0.5);
	rRight.b = rLeft.b + Point2d(r.width * 0.5);
	rRight.c = rLeft.c + Point2d(r.width * 0.5);
	rRight.d = rLeft.d + Point2d(r.width * 0.5);
	Point3d leftClr, rightClr;
	getOrgImgRectAvgClr(img, rLeft, fldToOrgHMat, leftClr);
	getOrgImgRectAvgClr(img, rRight, fldToOrgHMat, rightClr);
	double diff = abs(leftClr.x - rightClr.x + leftClr.y - rightClr.y +
			leftClr.z - rightClr.z);
	return diff;
}

double getFgAreaRatio(const Rect &r, const Mat &img)
{
	double fgPixelsNum = 0;
	for(int y = r.y; y < r.y + r.height; ++y)
		for(int x = r.x; x < r.x + r.width; ++x)
		{
//			cout << "###" << endl;
//			x = y = 1;
//			if( y < 0 || y >= fld->fieldLength)
//				continue;
//			if( x < 0 || x >= fld->fieldWidth)
//				continue;
			const Point3_<uchar>* p = img.ptr<Point3_<uchar> >(y, x);
			if(int(p->z) == 255)
			{
					++fgPixelsNum;
			}
		}
//	cout << r.width * r.height << endl;
	return (fgPixelsNum / (r.width * r.height));
}

void readKltTracks(string filePath, vector<track> &trks)
{
	ifstream fin(filePath.c_str());

	if(!fin.is_open())
	{
		cout << "Can't open file " << filePath << endl;
		return;
	}

	fin.seekg(0, ios::end);
	if (fin.tellg() == 0) {
		cout << "Empty file " << filePath << endl;
		return;
	}
	fin.seekg(0, ios::beg);
	while(!fin.eof())
	{
		track t;
		t.startFrm = -1;
		fin >> t.startFrm >> t.endFrm >> t.startPos.x >> t.startPos.y;
		fin >> t.endPos.x >> t.endPos.y;
		if(t.startFrm == -1)
			break;
		trks.push_back(t);
	}
	fin.close();
}

void transTracksFromOrgToFld(const vector<track> &trks, const Mat &orgToFldHMat, vector<track> &rectTrks)
{
	rectTrks.resize(trks.size());
	vector<Point2d> srcVec, dstVec;
	for(unsigned int i = 0; i < trks.size(); ++i)
	{
		srcVec.push_back(trks[i].startPos);
		srcVec.push_back(trks[i].endPos);
	}
	perspectiveTransform(srcVec, dstVec, orgToFldHMat);
	for(unsigned int i = 0; i < rectTrks.size(); ++i)
	{
		rectTrks[i].startPos = dstVec[2 * i];
		rectTrks[i].endPos = dstVec[2 * i + 1];
	}
}
void drawKltTracks(play *p, const vector<track> &trks, const Mat &orgToFldHMat)
{
	vector<track> rectTrks;
//	vector<Point2d> srcVec, dstVec;
//	for(unsigned int i = 0; i < trks.size(); ++i)
//	{
//		srcVec.push_back(trks[i].startPos);
//		srcVec.push_back(trks[i].endPos);
//	}
//	perspectiveTransform(srcVec, dstVec, orgToFldHMat);
//	for(unsigned int i = 0; i < rectTrks.size(); ++i)
//	{
//		rectTrks[i].startPos = dstVec[2 * i];
//		rectTrks[i].endPos = dstVec[2 * i + 1];
//	}
	transTracksFromOrgToFld(trks, orgToFldHMat, rectTrks);

//	int fontFace = 0;
//	double fontScale = 1;
	int thickness = 2;
	Scalar clr = CV_RGB(0, 255, 255);
	for(unsigned int i = 0; i < trks.size(); ++i)
	{
		line(p->mosFrame, trks[i].startPos, trks[i].endPos, clr, 1, 8, 0);
		circle(p->mosFrame, trks[i].endPos, 1, clr, thickness);
		line(p->rectMosFrame, rectTrks[i].startPos, rectTrks[i].endPos, clr, 1, 8, 0);
		circle(p->rectMosFrame, rectTrks[i].endPos, 1, clr, thickness);
	}

}

bool isPlayerByKltTracks(const vector<track> &trks, const Point2d &pos, const Mat &orgToFldHMat)
{
	vector<track> rectTrks;
	transTracksFromOrgToFld(trks, orgToFldHMat, rectTrks);

	int len = 30;
	double totalTrkLen = 0;
	for(unsigned int i = 0; i < rectTrks.size(); ++i)
	{
		Point2d mid = 0.5 * (rectTrks[i].startPos + rectTrks[i].endPos);
//		Point2d mid = rectTrks[i].startPos;
//		cout << "mid.x " << mid.x << "mid.y " << mid.y << endl;
//		cout << "pos.x " << pos.x << "pos.y " << pos.y << endl;
		if((mid.x >= pos.x - len) && (mid.x <= pos.x + len) &&
				(mid.y >= pos.y - len) && (mid.y <= pos.y + len))
		{
			double trkLen = norm(rectTrks[i].endPos - rectTrks[i].startPos);
			if(trkLen >= 5)
				totalTrkLen += trkLen;
		}
	}
//	cout << "totalTrkLen " << totalTrkLen << endl;
	int ratio = 0.5;
	if(totalTrkLen > 2 * len * ratio)
		return true;
	return false;
}



