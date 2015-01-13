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

//		cout << "Detect formations of Game" << gameIdStr << "..."<< endl;

		for(unsigned int i = 0; i < pIds.size(); ++i)
		{
//			if(pIds[i].vidId != 2)
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
//			p->getLosBndBoxByClrAndFg();
//			if(!p->getPlayersFromFg().empty())
//				cout << pIds[i].gameId << " " << pIds[i].vidId << endl;
			p->genFilledNewFg();
			p->getPlayersFromFg();
			p->transPlayersFgToFld();
		}
	}
}


void computeRectLosCntGt(const vector<int> &games, const vector<int> &gamesFld)
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

//		cout << "Detect formations of Game" << gameIdStr << "..."<< endl;

		for(unsigned int i = 0; i < pIds.size(); ++i)
		{
//			if(pIds[i].vidId != 13)
//				continue;
			play *p = NULL;
//			cout << "gameId: " << pIds[i].gameId << " vidId: " << pIds[i].vidId << endl;
			if (p != NULL)
				delete p;
			p = new play(pIds[i], gamesFld[g]);
//			direction offSide;
//			if(odLabels[i] == "o")
//				offSide = leftDir;
//			else if(odLabels[i] == "d")
//				offSide = rightDir;
//			else
//			{
//				cout << "Wrong od labels!" << endl;
//				return;
//			}
//			p->detectForms(offSide);
//			p->detectFormsGt(offSide);
//			p->labelPlayersAngleGt();
//			p->labelPlayersAngle(offSide);
//			p->getLosBndBoxByUfmClr();
//			p->getLosBndBoxByClrAndFg();
			p->getLosCntGt();
			p->computeRectLosCntGt();
			cout << pIds[i].vidId << " " << p->rectLosCntGt.x << " " << p->rectLosCntGt.y << endl;
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
//			cout<< pIds[i].vidId << " ";
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


void getFormLearningSamples(const vector<int> &games, const vector<int> &gamesFld,
		vector<vector<Point2d> > &pToLosVecAllPlays, vector<vector<int> > &pTypesIdAllPlays)
{
//	vector<Point2d> pToLosVecAllPlays;
//	vector<int> pTypesIdAllPlays;
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
			pToLosVecAllPlays.push_back(pToLosVec);
			pTypesIdAllPlays.push_back(pTypesId);
		}
	}
}

void saveExemplarForms(const vector<int> &games, const vector<int> &gamesFld)
{
//	vector<Point2d> pToLosVecAllPlays;
//	vector<int> pTypesIdAllPlays;
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
			vector<Point2d> positions;
			vector<string> pTypes;
			p->getPlayersLosPos(positions, pTypes);
			p->savePlayersLosPos(positions, pTypes);
		}
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

void inferMissPlayersAllPlays(vector<int> &games, vector<int> &gamesFld,
		Mat &trainFeaturesMat, Mat &trainLabelsMat, const vector<vector<Point2d> > &pToLosVecAllPlays,
		const vector<vector<int> > &pTypesIdAllPlays)
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
			p->inferMissingPlayers(offSide, trainFeaturesMat, trainLabelsMat, pToLosVecAllPlays, pTypesIdAllPlays);
		}
	}
}

//int main()
////int detectForm()
//{
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
////	Mat trainFeaturesMat, trainLabelsMat;
////	getPTypesLearningSamples(games, gamesFld, trainFeaturesMat, trainLabelsMat);
////	lablePTypesKNN(games, gamesFld, trainFeaturesMat, trainLabelsMat);
//
////	vector<vector<Point2d> > pToLosVecAllPlays;
////	vector<vector<int> > pTypesIdAllPlays;
////	getFormLearningSamples(games, gamesFld, pToLosVecAllPlays, pTypesIdAllPlays);
////	inferMissPlayersAllPlays(games, gamesFld, trainFeaturesMat, trainLabelsMat, pToLosVecAllPlays, pTypesIdAllPlays);
//
////	detectForm(games, gamesFld);
////	computeRectLosCntGt(games, gamesFld);
//
////	saveExemplarForms(games, gamesFld);
//
//	return 1;
//}
