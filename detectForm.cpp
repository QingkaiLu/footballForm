#include <vector>
#include <iostream>
#include <string>
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
			p->detectForms(offSide);
		}
	}
}

int main()
//int detectFormMain()
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

//	vector<playerTypeId> pTypeIds;
//	pTypeIds.push_back(upWR);
//	pTypeIds.push_back(lowWR);
//	pTypeIds.push_back(runBack);
//	detectForm(games, gamesFld, pTypeIds);

	detectForm(games, gamesFld);

	return 1;
}
