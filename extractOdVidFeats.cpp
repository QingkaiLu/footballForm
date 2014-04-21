#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include "extractOdVidFeats.h"
#include "play.h"
#include "imgRectification.h"

using namespace std;
using namespace cv;

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


void extracOdVidFeatsRts(int gameId, vector<playId> &pIds)
{
	ostringstream convertGameId;
	convertGameId << gameId ;
	string gameIdStr = convertGameId.str();

	if(gameId < 10)
		gameIdStr = "0" + gameIdStr;
	string odPIdsPath = "odGame" + gameIdStr;

//	string odLabelFilePath = "randTreesTrainData/odGame" + gameIdStr;
	string odLabelFilePath = "randTreesTrainData/odGame" + gameIdStr + "Rect";
//	vector<playId> pIds;
	vector<string> dirs, odLabels;

	readOdLabels(odLabelFilePath, pIds, dirs, odLabels);

	cout << "Computing feature vectors... " << endl;

	//string featuresFile = "randTreesTrainData/featureVecsGame" + gameIdStr;
	string featuresFile = "randTreesTrainData/featuresGame" + gameIdStr + "Rect";
	ofstream fout(featuresFile.c_str());
	vector<CvSize> gridSizes;
	vector<Point2i> gridsNum;
	gridSizes.push_back(cvSize(YardLinesDist * 6, YardLinesDist * 8));
	gridsNum.push_back(Point2i(1, 2));
	gridSizes.push_back(cvSize(YardLinesDist * 2, YardLinesDist * 2));
	gridsNum.push_back(Point2i(3, 8));
	gridSizes.push_back(cvSize(YardLinesDist, YardLinesDist));
	gridsNum.push_back(Point2i(6, 16));

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
//		p->extractOdGridsFeature(d, fVecOnePlay);
		d = leftDir;
		//p->extractOdStripsFeature(d, fVecOnePlayLeft);
//		p->extractOdStripsFeatRect(d, fVecOnePlayLeft);
//		p->extractOdGridsFeatRect(d, fVecOnePlayLeft);
		p->extractOdGridsFeatRect(d, fVecOnePlayLeft, gridSizes, gridsNum);
		//p->extractOdStripsFeatFldCrd(d, fVecOnePlayLeft);
		//p->extractOdGridsFeatFldCrd(d, fVecOnePlayLeft);
		delete p;
		p = new play(pIds[i]);
		p->setUp();
		d = rightDir;
		//p->extractOdStripsFeature(d, fVecOnePlayRight);
//		p->extractOdStripsFeatRect(d, fVecOnePlayRight);
//		p->extractOdGridsFeatRect(d, fVecOnePlayRight);
		p->extractOdGridsFeatRect(d, fVecOnePlayRight, gridSizes, gridsNum);
		//p->extractOdStripsFeatFldCrd(d, fVecOnePlayRight);
		//p->extractOdGridsFeatFldCrd(d, fVecOnePlayRight);
		for(unsigned int j = 0; j < fVecOnePlayLeft.size(); ++j)
			fVecOnePlay.push_back(fVecOnePlayLeft[j] - fVecOnePlayRight[j]);

		fout << odLabels[i] << ",";
		int maxFVal = -1;
		for(unsigned int j = 0; j < fVecOnePlay.size(); ++j)
			if(abs(fVecOnePlay[j]) > maxFVal)
				maxFVal = abs(fVecOnePlay[j]);
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

		fout << endl;
	}

	fout.close();
	return;
}


//int main()
//{
//	vector<playId> pIds;
//	extracOdVidFeatsRts(2, pIds);
////	extracOdVidFeatsRts(8, pIds);
////	extracOdVidFeatsRts(9, pIds);
////	extracOdVidFeatsRts(10);
//	return 1;
//}
