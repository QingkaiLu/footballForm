#include <cv.h>
#include <highgui.h>
#include <iostream>
#include <fstream>
#include <string>

#include "commonStructs.h"
#include "imgRectification.h"

using namespace std;
using namespace cv;

void drawFieldModel(Mat &fieldModel) {
//	fieldModel.create(FieldLength, FieldWidth, CV_32FC3);
//	fieldModel = Scalar(0,255,0);
	//draw yard lines
	for (int i = EndZoneWidth - 1; i < (FieldWidth - EndZoneWidth); i += YardLinesDist) {
		line(fieldModel, Point2i(i, 0), Point2i(i, FieldLength - 1), CV_RGB(200, 200, 200), 2, 8, 0);
	}
	//draw hash lines
	for (int i = EndZoneWidth - 1; i < (FieldWidth - EndZoneWidth); i += HashLinesDist) {
		line(fieldModel, Point2i(i, HashToSideLineDist - 1 - HashLinesLen * 0.5),
				Point2i(i, HashToSideLineDist - 1 + HashLinesLen * 0.5), CV_RGB(200, 200, 200), 2, 8, 0);
		line(fieldModel, Point2i(i, (FieldLength - 1 - HashToSideLineDist) - HashLinesLen * 0.5),
				Point2i(i, (FieldLength - 1 - HashToSideLineDist) + HashLinesLen * 0.5), CV_RGB(200, 200, 200), 2, 8, 0);
	}
	return;
}

void getFieldYardLines(vector<vector<Point2d> > &yardLines)
{
	for (int i = EndZoneWidth - 1; i < (FieldWidth - EndZoneWidth); i += YardLinesDist) {
		vector<Point2d> yardLine;
		yardLine.push_back(Point2i(i, 0));
		yardLine.push_back(Point2i(i, FieldLength - 1));
		yardLines.push_back(yardLine);
	}
}

bool readMatches(string matchesFile, vector<Point2f> &srcPoints, vector<Point2f> &dstPoints)
{
	ifstream fin(matchesFile.c_str());
	//x: 250 y: 82 Yard: r 40 Hash: 0

	if(!fin.is_open())
	{
		cout << "Can't open file " << matchesFile << endl;
		return false;
	}

	fin.seekg(0, ios::end);
	if (fin.tellg() == 0) {
		cout << "Empty file " << matchesFile << endl;
		return false;
	}
	fin.seekg(0, ios::beg);
	while(!fin.eof())
	{
		string tmpStr, yardLineSide;
		Point2f srcPnt, dstPnt;
		//hashId: up to down, 0 is the up side line, 1 is the 1st side line, ...,
		//3 is the low side
		//yardLnId: left to right, from 0 to 20
		int hashId, yardLnDist;
		fin >> tmpStr >> srcPnt.x >> tmpStr >> srcPnt.y;
		fin >> tmpStr >> yardLineSide >> yardLnDist >> tmpStr >> hashId;
		if(tmpStr.empty())
			break;

		srcPoints.push_back(srcPnt);

		if(yardLineSide == "r")
			yardLnDist = 100 - yardLnDist;

		int yardLnId = yardLnDist / 5;

		dstPnt.x = EndZoneWidth + YardLinesDist * yardLnId;
		//dstPnt.y = HashToSideLineDist * hashId;
		switch (hashId){
		case 0:
			dstPnt.y = .0;
			break;
		case 1:
			dstPnt.y = HashToSideLineDist;
			break;
		case 2:
			dstPnt.y = FieldLength - HashToSideLineDist;
			break;

		case 3:
			dstPnt.y = FieldLength;
			break;
		default:
			cout << "Wrong hash id" << endl;
			break;
		}

		dstPoints.push_back(dstPnt);

		//cout << srcPnt.x << " " << srcPnt.y << " " << dstPnt.x << " " << dstPnt.y << endl;

	}

	fin.close();

	return true;
}

bool readMatchesNew(string matchesFile, vector<Point2f> &srcPoints, vector<Point2f> &dstPoints)
{
	ifstream fin(matchesFile.c_str());
	//x: 250 y: 82 Yard: r 40 Hash: 0

	if(!fin.is_open())
	{
		cout << "Can't open file " << matchesFile << endl;
		return false;
	}

	fin.seekg(0, ios::end);
	if (fin.tellg() == 0) {
		cout << "Empty file " << matchesFile << endl;
		return false;
	}
	fin.seekg(0, ios::beg);
	while(!fin.eof())
	{
		string tmpStr, yardLineSide;
		Point2f srcPnt, dstPnt;
		dstPnt.x = -1;
		fin >> dstPnt.x >> dstPnt.y >> srcPnt.x >> srcPnt.y;
		if(dstPnt.x == -1)
			break;

		srcPoints.push_back(srcPnt);
		dstPoints.push_back(dstPnt);

		//cout << srcPnt.x << " " << srcPnt.y << " " << dstPnt.x << " " << dstPnt.y << endl;

	}

	fin.close();

	return true;
}

//void homoTransPoint(const Point2d &srcPnt, const Mat &homoMat, Point2d &dstPnt)
//{
//	Mat srcPntVec(3, 1, CV_64FC1), dstPntVec(3, 1, CV_32FC1);
//	srcPntVec.at<double>(0, 0) = srcPnt.x;
//	srcPntVec.at<double>(1, 0) = srcPnt.y;
//	srcPntVec.at<double>(2, 0) = 1.0;
//	dstPntVec = homoMat * srcPntVec;
//	float w = srcPntVec.at<double>(2, 0);
//	if(w < EPS)
//		w= EPS;
//	dstPnt.x = dstPntVec.at<double>(0, 0) / w;
//	dstPnt.y = dstPntVec.at<double>(1, 0) / w;
//}

void rectifyImageToField(string matchesFile, const Mat &srcImg, Mat &dstImg, Mat &homoMat)
{
	vector<Point2f> srcPoints, dstPoints;
//	readMatches(matchesFile, srcPoints, dstPoints);
	readMatchesNew(matchesFile, srcPoints, dstPoints);
	homoMat = findHomography(srcPoints, dstPoints);
	Size distImgSize(FieldWidth, FieldLength);
	dstImg.create(FieldLength, FieldWidth, CV_32FC3);
	warpPerspective(srcImg, dstImg, homoMat, distImgSize);
	drawFieldModel(dstImg);
}

void transFieldToImage(string matchesFile, Mat &dstImg, Mat &homoMat)
{
	vector<Point2f> srcPoints, dstPoints;
//	readMatches(matchesFile, dstPoints, srcPoints);
	readMatchesNew(matchesFile, dstPoints, srcPoints);
	homoMat = findHomography(srcPoints, dstPoints);;
	Mat srcImg;
	srcImg.create(FieldLength, FieldWidth, CV_32FC3);
	drawFieldModel(srcImg);
	Size distImgSize(imgXLen, imgYLen);
	dstImg.create(imgYLen, imgXLen, CV_32FC3);
	warpPerspective(srcImg, dstImg, homoMat, distImgSize);
}

//int main(int argc, char* argv[])
//{
//	Mat fieldModel;
//	drawFieldModel(fieldModel);
//	imshow("field", fieldModel);
//	imwrite("field.jpg", fieldModel);
//	cvWaitKey();
//}
