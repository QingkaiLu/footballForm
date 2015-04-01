#include <vector>
#include <iostream>
#include <string>
#include <cv.h>
#include <ml.h>
#include "play.h"
#include "playerType.h"
#include "extractOdVidFeats.h"
#include "detectForm.h"
#include "formationSet.h"

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
//		string odLabelFilePath = "offsFeats/odGame" + gameIdStr + "_FgNew";
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
//			cout << "gameId: " << pIds[i].gameId << " vidId: " << pIds[i].vidId << endl;
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
//			if(p->genFilledNewFg())
//				cout << pIds[i].gameId << " " << pIds[i].vidId << " l " << odLabels[i] << endl;
			p->drawKlt();
//			p->getPlayersFromFg();
//			p->transPlayersFgToFld();
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
//		string odLabelFilePath = "randTreesTrainData/odPlays/odGame" + gameIdStr + "RectForm";
		//formsLearningSet/odGame02RectForm
		string odLabelFilePath = "formsLearningSet/odGame" + gameIdStr + "RectForm";
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
		string odLabelFilePath = "formations/odGame02RectFg";
//		string odLabelFilePath = "randTreesTrainData/odPlays/odGame" + gameIdStr + "Rect";
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
//			p->drawKlt();
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

inline double getGridSpCost(vector<CvSize> &gridSizes, Point2d p1, Point2d p2)
{
	int levelsNum = gridSizes.size();
	double cost = .0;
	for(int l = 0; l < levelsNum; ++l)
//	for(int l = 0; l < 1; ++l)
	{
		Point2d tmp1 = Point2d((int)p1.x / gridSizes[l].width, (int)p1.y / gridSizes[l].height);
		Point2d tmp2 = Point2d((int)p2.x / gridSizes[l].width, (int)p2.y / gridSizes[l].height);
		tmp1.x *= gridSizes[l].width;
		tmp1.y *= gridSizes[l].height;
		tmp2.x *= gridSizes[l].width;
		tmp2.y *= gridSizes[l].height;
//		Point2d tmp1 = Point2d((int)p1.x / 10, (int)p1.y / 75);
//		Point2d tmp2 = Point2d((int)p2.x / 10, (int)p2.y / 75);
//		tmp1.y *= 7.5;
//		tmp2.y *= 7.5;
//		cout << "p1.x " << p1.x << "p1.y " << p1.y << endl;
//		cout << "gridSizes[l].width " << gridSizes[l].width << "gridSizes[l].height " << gridSizes[l].height << endl;
//		cout << "tmp1.x " << tmp1.x << "tmp1.y " << tmp1.y << endl;
//		cout << "tmp2.x " << tmp2.x << "tmp2.y " << tmp2.y << endl;
		double oneLevelCost = norm(tmp1 - tmp2);
//		if(l == 0)
//			cost += pow(0.5, levelsNum - 1) * cost;
//		else
//			cost += pow(0.5, levelsNum - 1 - l + 1) * cost;
//		cost += pow(0.5, levelsNum - l ) * oneLevelCost;
		cost += oneLevelCost;
	}
	cost /= levelsNum;
	return cost;
}

inline bool isLinemen(string playerType)
{
	return (playerType == "T" || playerType == "TE" || playerType == "G"
			|| playerType == "C");
}

double detectFormMltEviOnePlay(play* p, formationSet* formSet,
		vector<Point2d> &playersFormPos, vector<string> &playersFormTypes, playId &bestPId, string odLabel)
{
	double minCostBestForm = INF;
	vector<vector<vector<double> > > featureFormSet = formSet->getFeatureFormSet();
	vector<vector<string> > playersTypeFormSet = formSet->getPlayersTypeFormSet();
	vector<playId> pIds =  formSet->getPIds();

	vector<double> maxFeatureVec;
	vector<Point2d> allPos;
	vector<vector<double> > allPosFeatureVecs = p->getAllPosFeatureVecs(maxFeatureVec, allPos, odLabel);
	vector<CvSize> gridSizes;
	setUpGrids(gridSizes, p->fldModType);
	for(unsigned int i = 0; i < pIds.size(); ++i)
//	for(unsigned int i = 0; i < 2; ++i)
	{
		//leave-one-out
		if(pIds[i].gameId == p->pId.gameId && pIds[i].vidId == p->pId.vidId)
			continue;
		cout << i << endl;
		double minCostForm = 0;
		vector<Point2d> bestPlayersPosForm;
		for(unsigned int j = 0; j < featureFormSet[i].size(); ++j)
		{
			Point3d appFormVec(featureFormSet[i][j][0], featureFormSet[i][j][1], featureFormSet[i][j][2]);
			Point2d spFormVec(featureFormSet[i][j][3], featureFormSet[i][j][4]);
			double minCostPart = INF;
			Point2d bestPosPart;
			for(unsigned int k = 0; k < allPosFeatureVecs.size(); ++k)
			{
				Point3d appPlayVec(allPosFeatureVecs[k][0], allPosFeatureVecs[k][1], allPosFeatureVecs[k][2]);
				Point2d spPlayVec(allPosFeatureVecs[k][3], allPosFeatureVecs[k][4]);
				double tmpAppCost = norm(appFormVec - appPlayVec);
				spFormVec.x = abs(spFormVec.x);
				spPlayVec.x = abs(spPlayVec.x);

//				double tmpSpCost = norm(spFormVec - spPlayVec);
				double tmpSpCost = getGridSpCost(gridSizes, spFormVec, spPlayVec);
//				cout << "tmpAppCost " << tmpAppCost << endl;
//				cout << "tmpSpCost " << tmpSpCost << endl;
				double tmpCost = 100 * tmpAppCost + tmpSpCost;
//				double tmpCost = 100 * tmpAppCost + tmpSpCost;
				//cout << "tmpCost " << tmpCost << endl;
				if(tmpCost < minCostPart)
				{
					minCostPart = tmpCost;
					bestPosPart = allPos[k];
				}
//				bestPosPart = allPos[0];
			}
			minCostForm += minCostPart;
			bestPlayersPosForm.push_back(bestPosPart);
		}
		//normalization of # of players in formation annotations
//		minCostForm += (11 - featureFormSet[i].size()) * 1000;
		minCostForm += (11 - featureFormSet[i].size()) * 1000;
		//cout << "minCostForm " << minCostForm << endl;
		if(minCostForm < minCostBestForm)
		{
			minCostBestForm = minCostForm;
			playersFormPos = bestPlayersPosForm;
			playersFormTypes = playersTypeFormSet[i];
			bestPId = pIds[i];
		}
	}

	return minCostBestForm;
}

void detectFormMltEvidence(const vector<int> &games, const vector<int> &gamesFld)
{
	string formLearningSetFile = "formsLearningSet/odGame02RectForm";
	formationSet *formSet = new formationSet(formLearningSetFile);
	formSet->computeFeatureSet(1);
	for(unsigned int g = 0; g < games.size(); ++g)
	{

		int gameId = games[g];
		ostringstream convertGameId;
		convertGameId << gameId ;
		string gameIdStr = convertGameId.str();
		if(gameId < 10)
			gameIdStr = "0" + gameIdStr;
		//file:///scratch/workspace/picStrucWR/formations/odGame02RectFg
		string odLabelFilePath = "formations/odGame" + gameIdStr + "RectFg";
		vector<playId> pIds;
		vector<string> dirs, odLabels;

		readOdLabels(odLabelFilePath, pIds, dirs, odLabels);

//		cout << "Detect formations of Game" << gameIdStr << "..."<< endl;

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
			vector<Point2d> playersFormPos;
			vector<string> playersFormTypes;
			playId bestPId;
			detectFormMltEviOnePlay(p, formSet, playersFormPos, playersFormTypes, bestPId, odLabels[i]);
			p->showForm(playersFormPos, playersFormTypes, bestPId);
		}
	}
}

int main()
//int detectForm()
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
//
//	vector<vector<Point2d> > pToLosVecAllPlays;
//	vector<vector<int> > pTypesIdAllPlays;
//	getFormLearningSamples(games, gamesFld, pToLosVecAllPlays, pTypesIdAllPlays);
//	inferMissPlayersAllPlays(games, gamesFld, trainFeaturesMat, trainLabelsMat, pToLosVecAllPlays, pTypesIdAllPlays);

//	detectForm(games, gamesFld);
//	computeRectLosCntGt(games, gamesFld);

//	saveExemplarForms(games, gamesFld);

//	detectFormMltEvidence(games, gamesFld);
	string formLearningSetFile = "formsLearningSet/odGame02RectFormTest";
	formationSet *formSet = new formationSet(formLearningSetFile);
	//formSet->crossValidVoted();
	//formSet->findSimForm();
	formSet->leaveOneOutBestFormTest();
	//formSet->testBestFormTrainingSet();
	return 1;
}
