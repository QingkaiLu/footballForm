#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include "extractOdVidFeats.h"
#include "play.h"

using namespace std;

void readOdLabels(string odLabelFilePath, vector<playId> &pIds, vector<string> &dirs, vector<string> &odLabels)
{
	cout << "reading " << odLabelFilePath << endl;
	ifstream fin(odLabelFilePath.c_str());
	if(!fin.is_open())
	{
		cout << "Can't open file " << odLabelFilePath << endl;
		return;
	}

	fin.seekg(0, ios::end);
	if (fin.tellg() == 0) {
		cout << "Empty file " << odLabelFilePath << endl;
		return;
	}
	fin.seekg(0, ios::beg);

	while(!fin.eof())
	{
		playId pId;
		string d, odLabl;
		pId.gameId = -1;
		fin >> pId.gameId >> pId.vidId >> d >> odLabl;
		if(pId.gameId == -1)//file ends
			break;
		cout << pId.gameId << " " << pId.vidId  << " " << d << " " << odLabl;
		cout << endl;
		pIds.push_back(pId);
		dirs.push_back(d);
		odLabels.push_back(odLabl);
	}
	cout << endl;


	fin.close();
}

void extracOdVidFeatsRts(int gameId)
{
	///home/qingkai/workspace/picStrucWR/randTreesTrainData/ods
//	string odLabelFilePath = "randTreesTrainData/ods";
//	string odLabelFilePath = "randTreesTrainData/odEachVidDifViewMultiGames";

	ostringstream convertGameId;
	convertGameId << gameId ;
	string gameIdStr = convertGameId.str();

	if(gameId < 10)
		gameIdStr = "0" + gameIdStr;
	string odPIdsPath = "odGame" + gameIdStr;

//	string odLabelFilePath = "randTreesTrainData/odGame" + gameIdStr;
	string odLabelFilePath = "randTreesTrainData/odGame" + gameIdStr + "Rect";
	vector<playId> pIds;
	vector<string> dirs, odLabels;

	readOdLabels(odLabelFilePath, pIds, dirs, odLabels);

	cout << "Computing feature vectors... " << endl;

	//vector< vector<int> > fVecsAllPlay;
	//string featuresFile = "randTreesTrainData/featureVecsGame" + gameIdStr;
	string featuresFile = "randTreesTrainData/featuresGame" + gameIdStr + "Rect";
	ofstream fout(featuresFile.c_str());
	//compute feature vector for each play with all models
	for(unsigned int i = 0; i < pIds.size(); ++i)
//	for(unsigned int i = 10; i < 12; ++i)
	{
		vector<int> fVecOnePlay, fVecOnePlayLeft, fVecOnePlayRight;
		play *p = NULL;
		cout << "gameId: " << pIds[i].gameId << " vidId: " << pIds[i].vidId << endl;
		if (p != NULL)
			delete p;
		p = new play(pIds[i]);
		p->setUp();

		//p->rectification();

		direction d = nonDir;
//		if(dirs[i] == "l")
//			d = leftDir;
//		else if (dirs[i] == "r")
//			d = rightDir;
//		p->extractOdGridsFeature(d, fVecOnePlay);
		d = leftDir;
		//p->extractOdStripsFeature(d, fVecOnePlayLeft);
//		Mat H;
//		p->getOverheadFieldHomo(H);
//		p->extractOdStripsFeatRect(d, fVecOnePlayLeft);
		//p->extractOdGridsFeatRect(d, fVecOnePlayLeft);
		//p->extractOdStripsFeatFldCrd(d, fVecOnePlayLeft);
		p->extractOdGridsFeatFldCrd(d, fVecOnePlayLeft);
		delete p;
		p = new play(pIds[i]);
		p->setUp();
		d = rightDir;
		//p->extractOdStripsFeature(d, fVecOnePlayRight);
		//p->extractOdStripsFeatRect(d, fVecOnePlayRight);
		//p->extractOdGridsFeatRect(d, fVecOnePlayRight);
//		p->extractOdStripsFeatFldCrd(d, fVecOnePlayRight);
		p->extractOdGridsFeatFldCrd(d, fVecOnePlayRight);
		for(unsigned int j = 0; j < fVecOnePlayLeft.size(); ++j)
			fVecOnePlay.push_back(fVecOnePlayLeft[j] - fVecOnePlayRight[j]);

		fout << odLabels[i] << ",";
		//vector<double> fVecOnePlayd;
		int maxFVal = -1;
		for(unsigned int j = 0; j < fVecOnePlay.size(); ++j)
			if(abs(fVecOnePlay[j]) > maxFVal)
				maxFVal = abs(fVecOnePlay[j]);
		//cout << "maxFVal: " << maxFVal << endl;
		if(maxFVal != 0)
		{
			for(unsigned int j = 0; j < (fVecOnePlay.size() - 1); ++j)
				fout << (fVecOnePlay[j] + 0.0) /  (maxFVal + 0.0) << ",";
			fout << (fVecOnePlay[fVecOnePlay.size() - 1] + 0.0)  /  (maxFVal + 0.0);
		}
		else
		{
			for(unsigned int j = 0; j < (fVecOnePlay.size() - 1); ++j)
				fout << (fVecOnePlay[j] + 0.0) << ",";
			fout << (fVecOnePlay[fVecOnePlay.size() - 1] + 0.0);
		}

//		for(unsigned int j = 0; j < fVecOnePlay.size(); ++j)
//			fout << fVecOnePlay[j]  << ",";

		fout << endl;
		//fout << maxFVal << endl;
//			p->computeFormDir(pTypesOfAllClasses[j], modelWt[j], featureNumEachPlay, (j + 1));
//			fVecOnePlay.push_back(p->form->totalScore);//model score
//			fVecOnePlay.push_back(1.0);//bias
		//fVecsAllPlay.push_back(fVecOnePlay);
	}

	fout.close();
	return;
}

void extracOdVidFeatsSvm()
{
	string odLabelFilePath = "randTreesTrainData/odEachVidMultiGames";
	vector<playId> pIds;
	vector<string> dirs, odLabels;

	readOdLabels(odLabelFilePath, pIds, dirs, odLabels);

	cout << "Computing feature vectors... " << endl;
	//int main()
	//{
	////	int games[4] = {2, 8, 9, 10};
	////	for(int i = 0; i < 4; ++i)
	////	{
	////		vector<int> trainGames, testGames;
	////		for(int j = 0; j < 4; ++j)
	////		{
	////			if(j == i)
	////				testGames.push_back(games[j]);
	////			else
	////				trainGames.push_back(games[j]);
	////		}
	////
	////		cout << "Leave game " << games[i] << " out... " << endl;
	////		randTreeTrainTest(trainGames, testGames);
	////	}
	//	vector<int> trainGames, testGames;
	//	//trainGames.push_back(2);
	//	trainGames.push_back(8);
	////	trainGames.push_back(9);
	////	trainGames.push_back(10);
	//	//testGames.push_back(2);
	//	testGames.push_back(8);
	////	testGames.push_back(9);
	////	testGames.push_back(10);
	//	randTreeTrainTest(trainGames, testGames);
	//	return 1;
	//}
	//vector< vector<int> > fVecsAllPlay;
	ofstream fout("randTreesTrainData/featureVecsSvm");
	//compute feature vector for each play with all models
	for(unsigned int i = 0; i < pIds.size(); ++i)
	{
		vector<int> fVecOnePlay, fVecOnePlayLeft, fVecOnePlayRight;
		play *p = NULL;
		cout << "gameId: " << pIds[i].gameId << " vidId: " << pIds[i].vidId << endl;
		if (p != NULL)
			delete p;
		p = new play(pIds[i]);
		p->setUp();
		direction d = nonDir;
//		if(dirs[i] == "l")
//			d = leftDir;
//		else if (dirs[i] == "r")
//			d = rightDir;
//		p->extractOdGridsFeature(d, fVecOnePlay);
		d = leftDir;
		p->extractOdStripsFeature(d, fVecOnePlayLeft);
		delete p;
		p = new play(pIds[i]);
		p->setUp();
		d = rightDir;
		p->extractOdStripsFeature(d, fVecOnePlayRight);
		for(unsigned int j = 0; j < fVecOnePlayLeft.size(); ++j)
			fVecOnePlay.push_back(fVecOnePlayLeft[j] - fVecOnePlayRight[j]);

		if(odLabels[i] == "d")
			fout << 1 << " ";
		else if (odLabels[i] == "o")
			fout << 2 << " ";
		else
		{
			cout << "wrong labels!" << endl;
			return;
		}
		//vector<double> fVecOnePlayd;
		int maxFVal = -1;
		for(unsigned int j = 0; j < fVecOnePlay.size(); ++j)
			if(abs(fVecOnePlay[j]) > maxFVal)
				maxFVal = abs(fVecOnePlay[j]);
		for(unsigned int j = 0; j < (fVecOnePlay.size() - 1); ++j)
			fout << j + 1 << ":" << (fVecOnePlay[j] + 0.0) /  (maxFVal + 0.0) << " ";
		fout << fVecOnePlay.size() << ":" << (fVecOnePlay[fVecOnePlay.size() - 1] + 0.0)  /  (maxFVal + 0.0);

//		for(unsigned int j = 0; j < fVecOnePlay.size(); ++j)
//			fout << fVecOnePlay[j]  << ",";

		fout << endl;
	}

	fout.close();
	return;
}

//int main()
//{
//	extracOdVidFeatsRts(2);
//	extracOdVidFeatsRts(8);
////	extracOdVidFeatsRts(9);
////	extracOdVidFeatsRts(10);
//	return 1;
//}
