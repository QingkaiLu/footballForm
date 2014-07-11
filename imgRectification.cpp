#include <cv.h>
#include <highgui.h>
#include <iostream>
#include <fstream>
#include <string>

#include "commonStructs.h"
#include "imgRectification.h"
#include "fieldModel.h"

using namespace std;
using namespace cv;

//void setFldModel(int fldModel)
//{
//	if(fldModel == 1)
//	{
//		//high school football field model with size: 300 * 159(approximation of 160) feet
//		//http://www.sportsknowhow.com/football/field-dimensions/high-school-football-field-dimensions.html
//		fld->fieldLength = 795;
//		fld->fieldWidth = 1800;
//		fld->endZoneWidth = 150;
//		fld->yardLinesDist = 75;
//		fld->hashLinesDist = 15;
//		fld->hashToSideLineDist = 265;
//		fld->hashLinesLen = 10;
//	}
//	else if(fldModel == 2)
//	{
//		//college football field model with size: 300 * 160 feet
//		//http://www.sportsknowhow.com/football/field-dimensions/ncaa-football-field-dimensions.html
//		fld->fieldLength = 800;
//		fld->fieldWidth = 1800;
//		fld->endZoneWidth = 150;
//		fld->yardLinesDist = 75;
//		fld->hashLinesDist = 15;
//		fld->hashToSideLineDist = 300;
//		fld->hashLinesLen = 10;
//	}
//	else
//	{
//		cout << "Wrong field model!" << endl;
//	}
//}

imgRectfication::imgRectfication(int fldModel)
{
	fld = new fieldModel(fldModel);
}
imgRectfication::~imgRectfication()
{
	if(fld != NULL)
		delete fld;
}
void imgRectfication::drawFieldModel(Mat &fieldModel) {
//	fieldModel.create(fld->fieldLength, fld->fieldWidth, CV_32FC3);
//	fieldModel = Scalar(0,255,0);
	//draw yard lines
	for (int i = fld->endZoneWidth - 1; i < (fld->fieldWidth - fld->endZoneWidth); i += fld->yardLinesDist) {
		line(fieldModel, Point2i(i, 0), Point2i(i, fld->fieldLength - 1), CV_RGB(200, 200, 200), 2, 8, 0);
	}
	//draw hash lines
	for (int i = fld->endZoneWidth - 1; i < (fld->fieldWidth - fld->endZoneWidth); i += fld->hashLinesDist) {
		line(fieldModel, Point2i(i, fld->hashToSideLineDist - 1 - fld->hashLinesLen * 0.5),
				Point2i(i, fld->hashToSideLineDist - 1 + fld->hashLinesLen * 0.5), CV_RGB(200, 200, 200), 2, 8, 0);
		line(fieldModel, Point2i(i, (fld->fieldLength - 1 - fld->hashToSideLineDist) - fld->hashLinesLen * 0.5),
				Point2i(i, (fld->fieldLength - 1 - fld->hashToSideLineDist) + fld->hashLinesLen * 0.5), CV_RGB(200, 200, 200), 2, 8, 0);
	}
	return;
}

void imgRectfication::getFieldYardLines(vector<vector<Point2d> > &yardLines)
{
	for (int i = fld->endZoneWidth - 1; i < (fld->fieldWidth - fld->endZoneWidth); i += fld->yardLinesDist) {
		vector<Point2d> yardLine;
		yardLine.push_back(Point2i(i, 0));
		yardLine.push_back(Point2i(i, fld->fieldLength - 1));
		yardLines.push_back(yardLine);
	}
}

bool imgRectfication::readMatches(string matchesFile, vector<Point2f> &srcPoints, vector<Point2f> &dstPoints)
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

		dstPnt.x = fld->endZoneWidth + fld->yardLinesDist * yardLnId;
		//dstPnt.y = fld->hashToSideLineDist * hashId;
		switch (hashId){
		case 0:
			dstPnt.y = .0;
			break;
		case 1:
			dstPnt.y = fld->hashToSideLineDist;
			break;
		case 2:
			dstPnt.y = fld->fieldLength - fld->hashToSideLineDist;
			break;
		case 3:
			dstPnt.y = fld->fieldLength;
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

bool imgRectfication::readMatchesNew(string matchesFile, vector<Point2f> &srcPoints, vector<Point2f> &dstPoints)
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

void imgRectfication::rectifyImageToField(string matchesFile, const Mat &srcImg, Mat &dstImg, Mat &homoMat)
{
	vector<Point2f> srcPoints, dstPoints;
//	readMatches(matchesFile, srcPoints, dstPoints);
	readMatchesNew(matchesFile, srcPoints, dstPoints);
	homoMat = findHomography(srcPoints, dstPoints);
	Size distImgSize(fld->fieldWidth, fld->fieldLength);
	dstImg.create(fld->fieldLength, fld->fieldWidth, CV_32FC3);
	warpPerspective(srcImg, dstImg, homoMat, distImgSize);
	drawFieldModel(dstImg);
}

void imgRectfication::rectifyImageToField(string matchesFile, const Mat &srcImg, Mat &dstImg, Mat &homoMat, const Mat &tMosFrmToPMosFrmHomo)
{
	vector<Point2f> srcPoints, dstPoints;
//	readMatches(matchesFile, srcPoints, dstPoints);
	readMatchesNew(matchesFile, srcPoints, dstPoints);

	vector<Point2f> transSrcPnts;
	perspectiveTransform(srcPoints, transSrcPnts, tMosFrmToPMosFrmHomo);
	homoMat = findHomography(transSrcPnts, dstPoints);
	Size distImgSize(fld->fieldWidth, fld->fieldLength);
	dstImg.create(fld->fieldLength, fld->fieldWidth, CV_32FC3);
	warpPerspective(srcImg, dstImg, homoMat, distImgSize);
	drawFieldModel(dstImg);
}

void imgRectfication::transFieldToImage(string matchesFile, Mat &dstImg, Mat &homoMat)
{
	vector<Point2f> srcPoints, dstPoints;
//	readMatches(matchesFile, dstPoints, srcPoints);
	readMatchesNew(matchesFile, dstPoints, srcPoints);
	homoMat = findHomography(srcPoints, dstPoints);;
	Mat srcImg;
	srcImg.create(fld->fieldLength, fld->fieldWidth, CV_32FC3);
	drawFieldModel(srcImg);
	Size distImgSize(imgXLen, imgYLen);
	dstImg.create(imgYLen, imgXLen, CV_32FC3);
	warpPerspective(srcImg, dstImg, homoMat, distImgSize);
}

void imgRectfication::transFieldToImage(string matchesFile, const Mat &srcImg, Mat &dstImg, Mat &homoMat)
{
	vector<Point2f> srcPoints, dstPoints;
//	readMatches(matchesFile, dstPoints, srcPoints);
	readMatchesNew(matchesFile, dstPoints, srcPoints);
	homoMat = findHomography(srcPoints, dstPoints);;
	Size distImgSize(imgXLen, imgYLen);
	dstImg.create(imgYLen, imgXLen, CV_32FC3);
	warpPerspective(srcImg, dstImg, homoMat, distImgSize);
}

void imgRectfication::saveMat(string fileName, Mat& matData)
{
    if (matData.empty())
    {
        cout << "Empty matrix!" << endl;
        return;
    }

    ofstream outFile(fileName.c_str(), ios_base::out);
    if (!outFile.is_open())
    {
        cout << "Cann't open file!" << endl;
        return;
    }

    for (int r = 0; r < matData.rows; r++)
    {
        for (int c = 0; c < matData.cols; c++)
        {
            outFile << matData.at<float>(r,c) << " " ;
        }
        outFile << endl;
    }

    outFile.close();

    return;
}

void imgRectfication::readMat(string fileName, Mat& matData, int rows, int cols)
{
	ifstream fin(fileName.c_str());

	if(!fin.is_open())
	{
		cout << "Can't open file " << fileName << endl;
		return;
	}

	fin.seekg(0, ios::end);
	if (fin.tellg() == 0) {
		cout << "Empty file " << fileName << endl;
		return;
	}
	fin.seekg(0, ios::beg);

	if (!matData.empty())
		matData.release();
	matData.create(rows, cols, CV_32F);

    for (int r = 0; r < matData.rows; r++)
    {
        for (int c = 0; c < matData.cols; c++)
        {
        	float data = .0;
        	fin >> data;
        	matData.at<float>(r,c) = data;
        }
    }

    fin.close();

}

//int main(int argc, char* argv[])
//{
//	Mat fieldModel;
//	drawFieldModel(fieldModel);
//	imshow("field", fieldModel);
//	imwrite("field.jpg", fieldModel);
//	cvWaitKey();
//}
