#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <limits>
#include "playAuxFunc.h"
#include "play.h"

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
