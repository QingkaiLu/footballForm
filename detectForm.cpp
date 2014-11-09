#include <vector>
#include <iostream>
#include <string>
#include <cv.h>
#include <ml.h>
#include "play.h"
#include "playerType.h"
#include "extractOdVidFeats.h"
#include "detectForm.h"

using namespace std;
using namespace cv;

void detectForm(const vector<int> &games, const vector<int> &gamesFld, const vector<playerTypeId> &pTypeIds)
{
	for(unsigned int g = 0; g < games.size(); ++g)
	{

		int gameId = games[g];
		ostringstream convertGameId;
		convertGameId << gameId ;
		string gameIdStr = convertGameId.str();

		if(gameId < 10)
			gameIdStr = "0" + gameIdStr;

		string odLabelFilePath = "randTreesTrainData/odPlays/odGame" + gameIdStr + "Rect";
		vector<playId> pIds;
		vector<string> dirs, odLabels;

		readOdLabels(odLabelFilePath, pIds, dirs, odLabels);

		cout << "Detect formations of Game" << gameIdStr << "..."<< endl;

		for(unsigned int i = 0; i < pIds.size(); ++i)
		{
			play *p = NULL;
			cout << "gameId: " << pIds[i].gameId << " vidId: " << pIds[i].vidId << endl;
			if (p != NULL)
				delete p;
			p = new play(pIds[i], gamesFld[g]);
			direction offSide;
			if(odLabels[i] == "o")
				offSide = leftDir;
			else if(odLabels[i] == "d")
				offSide = rightDir;
			else
			{
				cout << "Wrong od labels!" << endl;
				return;
			}
//			p->detectPlayerTypesPosRect(pTypeIds, offSide);
			p->detectPlayerTypesPosOrig(pTypeIds, offSide);
		}
	}
}

void detectForm(const vector<int> &games, const vector<int> &gamesFld)
{
	for(unsigned int g = 0; g < games.size(); ++g)
	{

		int gameId = games[g];
		ostringstream convertGameId;
		convertGameId << gameId ;
		string gameIdStr = convertGameId.str();

		if(gameId < 10)
			gameIdStr = "0" + gameIdStr;

		string odLabelFilePath = "randTreesTrainData/odPlays/odGame" + gameIdStr + "Rect";
//		string odLabelFilePath = "randTreesTrainData/odPlays/odGame" + gameIdStr + "RectForm";
		vector<playId> pIds;
		vector<string> dirs, odLabels;

		readOdLabels(odLabelFilePath, pIds, dirs, odLabels);

		cout << "Detect formations of Game" << gameIdStr << "..."<< endl;

		for(unsigned int i = 0; i < pIds.size(); ++i)
		{
//			if(pIds[i].vidId != 13)
//				continue;
			play *p = NULL;
			cout << "gameId: " << pIds[i].gameId << " vidId: " << pIds[i].vidId << endl;
			if (p != NULL)
				delete p;
			p = new play(pIds[i], gamesFld[g]);
			direction offSide;
			if(odLabels[i] == "o")
				offSide = leftDir;
			else if(odLabels[i] == "d")
				offSide = rightDir;
			else
			{
				cout << "Wrong od labels!" << endl;
				return;
			}
//			p->detectForms(offSide);
//			p->detectFormsGt(offSide);
//			p->labelPlayersAngleGt();
//			p->labelPlayersAngle(offSide);
//			p->getLosBndBoxByUfmClr();
			p->getLosBndBoxByClrAndFg();
		}
	}
}


void getPTypesLearningSamples(const vector<int> &games, const vector<int> &gamesFld,
		Mat &trainFeaturesMat, Mat &trainLabelsMat)
{
	vector<Point2d> pToLosVecAllPlays;
	vector<int> pTypesIdAllPlays;
	for(unsigned int g = 0; g < games.size(); ++g)
	{

		int gameId = games[g];
		ostringstream convertGameId;
		convertGameId << gameId ;
		string gameIdStr = convertGameId.str();

		if(gameId < 10)
			gameIdStr = "0" + gameIdStr;

//		string odLabelFilePath = "randTreesTrainData/odPlays/odGame" + gameIdStr + "Rect";
		string odLabelFilePath = "randTreesTrainData/odPlays/odGame" + gameIdStr + "RectForm";
		vector<playId> pIds;
		vector<string> dirs, odLabels;

		readOdLabels(odLabelFilePath, pIds, dirs, odLabels);

		cout << "Getting learning samples of Game" << gameIdStr << "..."<< endl;

		for(unsigned int i = 0; i < pIds.size(); ++i)
		{
			play *p = NULL;
			cout << "gameId: " << pIds[i].gameId << " vidId: " << pIds[i].vidId << endl;
			if (p != NULL)
				delete p;
			p = new play(pIds[i], gamesFld[g]);
			vector<Point2d> pToLosVec;
			vector<int> pTypesId;
			p->getPlayersToLosVecs(pToLosVec, pTypesId);
			for(unsigned int i = 0; i < pToLosVec.size(); ++i)
			{
				pToLosVecAllPlays.push_back(pToLosVec[i]);
				pTypesIdAllPlays.push_back(pTypesId[i]);
			}
		}
	}

	trainFeaturesMat = Mat(pToLosVecAllPlays.size(), 2, CV_32FC1);
	trainLabelsMat = Mat(pToLosVecAllPlays.size(), 1, CV_32FC1);
	for(unsigned int i = 0; i < pToLosVecAllPlays.size(); ++i)
	{
		trainLabelsMat.at<float>(i, 0) = pTypesIdAllPlays[i];
		trainFeaturesMat.at<float>(i, 0) = pToLosVecAllPlays[i].x;
		trainFeaturesMat.at<float>(i, 1) = pToLosVecAllPlays[i].y;
	}

}

void lablePTypesKNN(const vector<int> &games, const vector<int> &gamesFld,
		const Mat &trainFeaturesMat, const Mat &trainLabelsMat)
{
	for(unsigned int g = 0; g < games.size(); ++g)
	{

		int gameId = games[g];
		ostringstream convertGameId;
		convertGameId << gameId ;
		string gameIdStr = convertGameId.str();

		if(gameId < 10)
			gameIdStr = "0" + gameIdStr;

		string odLabelFilePath = "randTreesTrainData/odPlays/odGame" + gameIdStr + "Rect";
//		string odLabelFilePath = "randTreesTrainData/odPlays/odGame" + gameIdStr + "RectForm";
		vector<playId> pIds;
		vector<string> dirs, odLabels;

		readOdLabels(odLabelFilePath, pIds, dirs, odLabels);

		cout << "Detect formations of Game" << gameIdStr << "..."<< endl;

		for(unsigned int i = 0; i < pIds.size(); ++i)
		{
			play *p = NULL;
			cout << "gameId: " << pIds[i].gameId << " vidId: " << pIds[i].vidId << endl;
			if (p != NULL)
				delete p;
			p = new play(pIds[i], gamesFld[g]);
			direction offSide;
			if(odLabels[i] == "o")
				offSide = leftDir;
			else if(odLabels[i] == "d")
				offSide = rightDir;
			else
			{
				cout << "Wrong od labels!" << endl;
				return;
			}
//			p->lablePTypesKnnFixedLosCnt(offSide, trainFeaturesMat, trainLabelsMat);
			p->lablePTypesKnnVarLosCnts(offSide, trainFeaturesMat, trainLabelsMat);
		}
	}
}

//int main()
////int detectFormMain()
//{
////    if(system("touch hi.txt")) cout<<"not done";
////    if(system("cd ../hungarian")) cout<<"not done";
////    if(system("./../hungarian/hungarian -v 0 -i scoreMat2")) cout<<"not done";
////	if(system("./../hungarian/hungarian -v 0 -i Hungarian/score.mat")) cout << " not done";
//	vector<int> games;
//	vector<int> gamesFld;
//	games.push_back(2);
//	gamesFld.push_back(1);
////	games.push_back(8);
////	gamesFld.push_back(1);
////	games.push_back(10);
////	gamesFld.push_back(1);
////	games.push_back(9);
////	gamesFld.push_back(2);
//
////	vector<playerTypeId> pTypeIds;
////	pTypeIds.push_back(upWR);
////	pTypeIds.push_back(lowWR);
////	pTypeIds.push_back(runBack);
////	detectForm(games, gamesFld, pTypeIds);
//
//	detectForm(games, gamesFld);
//
//	return 1;
//}

int main()
{
	vector<int> games;
	vector<int> gamesFld;
	games.push_back(2);
	gamesFld.push_back(1);
//	games.push_back(8);
//	gamesFld.push_back(1);
//	games.push_back(10);
//	gamesFld.push_back(1);
//	games.push_back(9);
//	gamesFld.push_back(2);
//	Mat trainFeaturesMat, trainLabelsMat;
//	getPTypesLearningSamples(games, gamesFld, trainFeaturesMat, trainLabelsMat);
//	lablePTypesKNN(games, gamesFld, trainFeaturesMat, trainLabelsMat);

	detectForm(games, gamesFld);
//	int *a, *b;
//	a = new int;
//	*a = 10;
//	cout << *a << endl;
//	b = new int;
//	*b = 10;
//	cout << *b << endl;

//	Mat trainFeaturesMat = Mat(2, 3, CV_32FC1);
//	trainFeaturesMat.at<float>(0,0) = 0;
//	trainFeaturesMat.at<float>(0,1) = 0;
//	trainFeaturesMat.at<float>(0,2) = 0;
//	trainFeaturesMat.at<float>(1,0) = 20;
//	trainFeaturesMat.at<float>(1,1) = 1;
//	trainFeaturesMat.at<float>(1,2) = 1;
//	Mat trainLabelsMat = Mat(2, 1, CV_32FC1);
//	cout << trainFeaturesMat << endl;
//	trainLabelsMat.at<float>(0,0) = 10;
//	trainLabelsMat.at<float>(1,0) = 20;
//	Mat testFeaturesMat = Mat(1, 3, CV_32FC1);
//	testFeaturesMat.at<float>(0,0) = 1;
//	testFeaturesMat.at<float>(0,1) = 1;
//	testFeaturesMat.at<float>(0,2) = 1;
//	cout << testFeaturesMat << endl;
//	int K = 1;
//	CvKNearest knn(trainFeaturesMat, trainLabelsMat, Mat(), false, K);
//	Mat neighborResponses(1, K, CV_32FC1);
//	Mat results(1, 1, CV_32FC1), dists(1, K, CV_32FC1);
//	int result = knn.find_nearest(testFeaturesMat, K, results, neighborResponses, dists);
//	cout << result << endl;
//	cout << results.at<float>(0,0) << endl;
//	cout << dists.at<float>(0,0) << endl;

//	Mat M(2,2, CV_8UC3, Scalar(1,0,255));
//	    cout << "M = " << endl << " " << M << endl << endl;
//	Scalar mn = mean(M);
//	cout << mn[3] << endl;
//	Mat M(3,2, CV_8UC1, 1);
//		cout << "M = " << endl << " " << M << endl << endl;
//	Mat A(1,2, CV_8UC1, 2);
//	vconcat(A, M.row(0), A);
//	cout << "A = " << endl << " " << A << endl << endl;
//	cout << "M = " << endl << " " << M << endl << endl;
////	A.row(0) = M.row(1);
////	cout << "A = " << endl << " " << A << endl << endl;
//	cout << M.cols << endl;

//	Point3d ufmClrMean(0, 0, 0);
//	cout << ufmClrMean << endl;

	return 1;
}
