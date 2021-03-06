#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include "extractOdVidFeats.h"
#include "play.h"
#include "imgRectification.h"
#include "playAuxFunc.h"
#include "fieldModel.h"

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

bool readOdFeatData(const vector<string> &fileNames, vector<vector<double> > &features,
		vector<double> &labels, int featureNum)
{
	for(unsigned int i = 0; i < fileNames.size(); ++i)
	{
		cout << "reading " << fileNames[i] << endl;
		ifstream fin(fileNames[i].c_str());
		if(!fin.is_open())
		{
			cout << "Can't open file " << fileNames[i] << endl;
			return false;
		}

		fin.seekg(0, ios::end);
		if (fin.tellg() == 0) {
			cout << "Empty file " << fileNames[i] << endl;
			return false;
		}
		fin.seekg(0, ios::beg);
		while(!fin.eof())
		{
			char oneLabel, comma;
			fin >> oneLabel >> comma;

			vector<double> featuresOneSample;
			bool fileEnd = false;
			for(int j = 0; j < featureNum; ++j)
			{
				double oneFeature = NEGINF;
				fin >> oneFeature >> comma;
				featuresOneSample.push_back(oneFeature);
				if(oneFeature == NEGINF)
				{
					fileEnd = true;
					break;
				}
			}

			if(fileEnd)
				break;

			labels.push_back(double(oneLabel));
			features.push_back(featuresOneSample);

		}
	fin.close();

	}

	return true;
}

bool readOdFeatData(const vector<string> &fileNames, vector<vector<double> > &features,
		vector<double> &labels, int featureNum, const vector<string> &losCntFileNames, int losCntIdx)
{
	vector<int> losCntIds;
	readLosCntIds(losCntFileNames, losCntIds);

	int p = 0;
	for(unsigned int i = 0; i < fileNames.size(); ++i)
	{
		cout << "reading " << fileNames[i] << endl;
		ifstream fin(fileNames[i].c_str());
		if(!fin.is_open())
		{
			cout << "Can't open file " << fileNames[i] << endl;
			return false;
		}

		fin.seekg(0, ios::end);
		if (fin.tellg() == 0) {
			cout << "Empty file " << fileNames[i] << endl;
			return false;
		}
		fin.seekg(0, ios::beg);
		while(!fin.eof())
		{
			char oneLabel, comma;
			fin >> oneLabel >> comma;

			vector<double> featuresOneSample;
			bool fileEnd = false;
			for(int j = 0; j < featureNum; ++j)
			{
				double oneFeature = NEGINF;
				fin >> oneFeature >> comma;
				featuresOneSample.push_back(oneFeature);
				if(oneFeature == NEGINF)
				{
					fileEnd = true;
					break;
				}
			}

			if(fileEnd)
				break;

			if(losCntIds[p] == losCntIdx)
			{
				labels.push_back(double(oneLabel));
				features.push_back(featuresOneSample);
			}
			++p;
		}


	fin.close();

	}

	return true;
}


bool readLosCntIds(const vector<string> &fileNames, vector<int> &losCntIds)
{
	for(unsigned int i = 0; i < fileNames.size(); ++i)
	{
		cout << "reading " << fileNames[i] << endl;
		ifstream fin(fileNames[i].c_str());
		if(!fin.is_open())
		{
			cout << "Can't open file " << fileNames[i] << endl;
			return false;
		}

		fin.seekg(0, ios::end);
		if (fin.tellg() == 0) {
			cout << "Empty file " << fileNames[i] << endl;
			return false;
		}
		fin.seekg(0, ios::beg);
		while(!fin.eof())
		{
			int losCntIdx = -1;
			fin >> losCntIdx;
			if(losCntIdx == -1)
				break;
			losCntIds.push_back(losCntIdx);
		}


	fin.close();

	}

	return true;
}


void extracOdVidFeatsRts(int gameId, vector<playId> &pIds)
{
	ostringstream convertGameId;
	convertGameId << gameId ;
	string gameIdStr = convertGameId.str();

	if(gameId < 10)
		gameIdStr = "0" + gameIdStr;
	string odLabelFilePath = "randTreesTrainData/odPlays/odGame" + gameIdStr + "Rect";
	vector<string> dirs, odLabels;

	readOdLabels(odLabelFilePath, pIds, dirs, odLabels);

	cout << "Computing feature vectors... " << endl;

	string featuresFile = "randTreesTrainData/features/featuresGame" + gameIdStr + "Rect";
	ofstream fout(featuresFile.c_str());
	vector<CvSize> gridSizes;
	vector<Point2i> gridsNum;
	setUpGrids(gridSizes, gridsNum, 1);

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
		d = leftDir;
		p->extractOdGridsFeatRect(d, fVecOnePlayLeft, gridSizes, gridsNum, 0);
		delete p;
		p = new play(pIds[i]);
		p->setUp();
		d = rightDir;
		p->extractOdGridsFeatRect(d, fVecOnePlayRight, gridSizes, gridsNum, 0);
		for(unsigned int j = 0; j < fVecOnePlayLeft.size(); ++j)
			fVecOnePlay.push_back(fVecOnePlayLeft[j] - fVecOnePlayRight[j]);

		fout << odLabels[i] << ",";
		int maxFVal = -1;
		for(unsigned int j = 0; j < fVecOnePlay.size(); ++j)
			if(abs(fVecOnePlay[j]) > maxFVal)
				maxFVal = abs(fVecOnePlay[j]);

		if(maxFVal != 0)
		{
			for(unsigned int j = 0; j < fVecOnePlay.size(); ++j)
				fout << (fVecOnePlay[j] + 0.0) /  (maxFVal + 0.0) << ",";
		}
		else
		{
			for(unsigned int j = 0; j < fVecOnePlay.size(); ++j)
				fout << (fVecOnePlay[j] + 0.0) << ",";
		}


		fout << endl;
	}

	fout.close();
	return;
}

void setUpGrids(vector<CvSize> &gridSizes, vector<Point2i> &gridsNum, int fldModel)
{
	fieldModel fldMod(fldModel);
//	gridSizes.push_back(cvSize(fldMod.yardLinesDist * 8, fldMod.yardLinesDist * 8));
//	gridsNum.push_back(Point2i(1, 2));
//	gridSizes.push_back(cvSize(fldMod.yardLinesDist * 4, fldMod.yardLinesDist * 4));
//	gridsNum.push_back(Point2i(2, 4));
//	gridSizes.push_back(cvSize(fldMod.yardLinesDist * 2, fldMod.yardLinesDist * 2));
//	gridsNum.push_back(Point2i(4, 8));
//	gridSizes.push_back(cvSize(fldMod.yardLinesDist, fldMod.yardLinesDist));
//	gridsNum.push_back(Point2i(8, 16));

//	gridSizes.push_back(cvSize(10, fldMod.yardLinesDist));
//	gridsNum.push_back(Point2i(80, 160));
	gridSizes.push_back(cvSize(fldMod.yardLinesDist, fldMod.yardLinesDist * 2));
	gridsNum.push_back(Point2i(4, 8));
	gridSizes.push_back(cvSize(fldMod.yardLinesDist * 0.5, fldMod.yardLinesDist));
	gridsNum.push_back(Point2i(8, 16));

}

void setUpGrids(vector<CvSize> &gridSizes, int fldModel)
{
	fieldModel fldMod(fldModel);
//	gridSizes.push_back(cvSize(80 * 2, fldMod.yardLinesDist * 2.5));
//	gridSizes.push_back(cvSize(80 * 2, fldMod.yardLinesDist * 2));
//	gridSizes.push_back(cvSize(40 * 2, fldMod.yardLinesDist * 2));
//	gridSizes.push_back(cvSize(40 * 2, fldMod.yardLinesDist * 1.5));
//	gridSizes.push_back(cvSize(20 * 2, fldMod.yardLinesDist * 1.5));
//	gridSizes.push_back(cvSize(20 * 2, fldMod.yardLinesDist));
//	gridSizes.push_back(cvSize(10 * 2, fldMod.yardLinesDist));

	gridSizes.push_back(cvSize(fldMod.yardLinesDist, fldMod.yardLinesDist * 2));
	gridSizes.push_back(cvSize(fldMod.yardLinesDist * 0.5, fldMod.yardLinesDist));

//	gridSizes.push_back(cvSize(fldMod.yardLinesDist * 0.5, fldMod.yardLinesDist * 2));

}

void computeLeftRightFeats(const vector<int> &games, int expMode, const vector<int> &gamesFld)
{

	for(unsigned int g = 0; g < games.size(); ++g)
	{
		vector<CvSize> gridSizes;
		vector<Point2i> gridsNum;
		setUpGrids(gridSizes, gridsNum, gamesFld[g]);

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

		cout << "Computing feature vectors of Game" << gameIdStr << "..."<< endl;

		//string featuresFile = "randTreesTrainData/featureVecsGame" + gameIdStr;
		string leftFeatFile = "randTreesTrainData/features/featsLeftRectGame" + gameIdStr;
		string rightFeatFile = "randTreesTrainData/features/featsRightRectGame" + gameIdStr;
		string losCntIdsFile = "randTreesTrainData/losCnt/losCntIdsGame" + gameIdStr;
		ofstream foutLeft(leftFeatFile.c_str());
		ofstream foutRight(rightFeatFile.c_str());
		ofstream foutLosIds(losCntIdsFile.c_str());

		//compute feature vector for each play with all models
		for(unsigned int i = 0; i < pIds.size(); ++i)
		{
			vector<int> fVecOnePlayLeft, fVecOnePlayRight;
			play *p = NULL;
			cout << "gameId: " << pIds[i].gameId << " vidId: " << pIds[i].vidId << endl;
			if (p != NULL)
				delete p;
			p = new play(pIds[i], gamesFld[g]);
			p->setUp();

			direction d = nonDir;
			d = leftDir;
			p->extractOdGridsFeatRect(d, fVecOnePlayLeft, gridSizes, gridsNum, expMode);

			delete p;
			p = new play(pIds[i], gamesFld[g]);
			p->setUp();
			d = rightDir;
			p->extractOdGridsFeatRect(d, fVecOnePlayRight, gridSizes, gridsNum, expMode);

			int losCntIdx = p->getLosCntIdx();
			foutLosIds << losCntIdx << endl;

			foutLeft << odLabels[i] << ",";
			for(unsigned int j = 0; j < fVecOnePlayLeft.size(); ++j)
				foutLeft << fVecOnePlayLeft[j] << ",";
			foutLeft << endl;

			foutRight << odLabels[i] << ",";
			for(unsigned int j = 0; j < fVecOnePlayRight.size(); ++j)
				foutRight << fVecOnePlayRight[j] << ",";
			foutRight << endl;

		}

		foutLeft.close();
		foutRight.close();
		foutLosIds.close();
	}

}


void computeOffsFeats(const vector<int> &games, const vector<int> &gamesFld)
{
	for(unsigned int g = 0; g < games.size(); ++g)
	{
		vector<CvSize> gridSizes;
		vector<Point2i> gridsNum;
		setUpGrids(gridSizes, gridsNum, gamesFld[g]);

		int gameId = games[g];
		ostringstream convertGameId;
		convertGameId << gameId ;
		string gameIdStr = convertGameId.str();

		if(gameId < 10)
			gameIdStr = "0" + gameIdStr;

		string odLabelFilePath = "offsFeats/odGame" + gameIdStr + "_FgNew";
//		string odLabelFilePath = "offsFeats/odGame" + gameIdStr;
		vector<playId> pIds;
		vector<string> dirs, odLabels;

		readOdLabels(odLabelFilePath, pIds, dirs, odLabels);

		cout << "Computing offense feature vectors of Game" << gameIdStr << "..."<< endl;

		string offFeatFile = "offsFeats/Game" + gameIdStr + "/offFeats";
		ofstream foutOffFeat(offFeatFile.c_str());

		//compute feature vector for each play with all models
		for(unsigned int i = 0; i < pIds.size(); ++i)
		{
			vector<int> fVecOnePlay;
			play *p = NULL;
			cout << "gameId: " << pIds[i].gameId << " vidId: " << pIds[i].vidId << endl;
			if (p != NULL)
				delete p;
			p = new play(pIds[i], gamesFld[g]);
			p->setUp();

			direction d = nonDir;
			if(odLabels[i] == "o")
				d = leftDir;
			else if(odLabels[i] == "d")
				d = rightDir;
//			p->extractOffsGridsFeatRect(d, fVecOnePlay, gridSizes, gridsNum);
			p->extractOffsGridsFeatRectPlayers(d, fVecOnePlay, gridSizes, gridsNum);

			foutOffFeat << odLabels[i] << ",";
			for(unsigned int j = 0; j < fVecOnePlay.size(); ++j)
				foutOffFeat << fVecOnePlay[j] << ",";
			foutOffFeat << endl;

		}

		foutOffFeat.close();
	}
}


void getPlayIds(const std::vector<int> &games, vector<playId> &pIds)
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
		vector<string> dirs, odLabels;

		vector<playId> pIdsOneGame;
		readOdLabels(odLabelFilePath, pIdsOneGame, dirs, odLabels);

		for(unsigned int i = 0; i < pIdsOneGame.size(); ++i)
			pIds.push_back(pIdsOneGame[i]);
	}
}

void getPlayIds(int gameId, vector<playId> &pIds)
{
	ostringstream convertGameId;
	convertGameId << gameId ;
	string gameIdStr = convertGameId.str();

	if(gameId < 10)
		gameIdStr = "0" + gameIdStr;

	string odLabelFilePath = "randTreesTrainData/odPlays/odGame" + gameIdStr + "Rect";
	vector<string> dirs, odLabels;

	vector<playId> pIdsOneGame;
	readOdLabels(odLabelFilePath, pIdsOneGame, dirs, odLabels);

	for(unsigned int i = 0; i < pIdsOneGame.size(); ++i)
		pIds.push_back(pIdsOneGame[i]);
}

void computeOdExpFeats(const vector<int> &games)
{
	vector<string> leftFeatFiles, rightFeatFiles, losCntFileNames;
	for(unsigned int g = 0; g < games.size(); ++g)
	{
		int gameId = games[g];
		ostringstream convertGameId;
		convertGameId << gameId ;
		string gameIdStr = convertGameId.str();

		if(gameId < 10)
			gameIdStr = "0" + gameIdStr;

		cout << "Computing expectation of feature vectors of Game" << gameIdStr << "..."<< endl;

		//string featuresFile = "randTreesTrainData/featureVecsGame" + gameIdStr;
		string leftFeatFile = "randTreesTrainData/features/featsLeftRectGame" + gameIdStr;
		leftFeatFiles.push_back(leftFeatFile);
		string rightFeatFile = "randTreesTrainData/features/featsRightRectGame" + gameIdStr;
		rightFeatFiles.push_back(rightFeatFile);
		string losCntIdsFile = "randTreesTrainData/losCnt/losCntIdsGame" + gameIdStr;
		losCntFileNames.push_back(losCntIdsFile);
	}

	vector<vector<double> > fVecPlaysLeft, fVecPlaysRight;
	vector<double> leftLabels, rightLabels;

	readOdFeatData(leftFeatFiles, fVecPlaysLeft, leftLabels, fNumPerPlayOneSide);
	readOdFeatData(rightFeatFiles, fVecPlaysRight, rightLabels, fNumPerPlayOneSide);

	vector<int> losCntIds;
	readLosCntIds(losCntFileNames, losCntIds);

	vector<int> zerosVec(fNumPerPlayOneSide, 0);
	vector<vector<int> > fVecOPlaysExp(losCntBins, zerosVec), fVecDPlaysExp(losCntBins, zerosVec);
	vector<vector<int> > expFeatNumGridsO(losCntBins, zerosVec), expFeatNumGridsD(losCntBins, zerosVec);

	//cout << "leftLabels.size() " << leftLabels.size() << endl;
	for(unsigned int i = 0; i < leftLabels.size(); ++i)
	{
		vector<double> fVecOnePlayLeft = fVecPlaysLeft[i],
				fVecOnePlayRight = fVecPlaysRight[i];

		int losCntIdx = losCntIds[i];
		if((char)leftLabels[i] == 'o')
		{
			for(unsigned int j = 0; j < fVecOnePlayLeft.size(); ++j)
			{
				//cout << fVecOnePlayLeft[j] << endl;
				if(fVecOnePlayLeft[j] >= .0)
				{
					fVecOPlaysExp[losCntIdx][j] += fVecOnePlayLeft[j];
					++expFeatNumGridsO[losCntIdx][j];
				}
			}

			for(unsigned int j = 0; j < fVecOnePlayRight.size(); ++j)
			{
				if(fVecOnePlayRight[j] >= .0)
				{
					fVecDPlaysExp[losCntIdx][j] += fVecOnePlayRight[j];
					++expFeatNumGridsD[losCntIdx][j];
				}
			}

		}
		else if((char)leftLabels[i] == 'd')
		{
			for(unsigned int j = 0; j < fVecOnePlayLeft.size(); ++j)
			{
				//cout << fVecOnePlayLeft[j] << endl;
				if(fVecOnePlayLeft[j] >= .0)
				{
					fVecDPlaysExp[losCntIdx][j] += fVecOnePlayLeft[j];
					++expFeatNumGridsD[losCntIdx][j];
				}
			}

			for(unsigned int j = 0; j < fVecOnePlayRight.size(); ++j)
			{
				if(fVecOnePlayRight[j] >= .0)
				{
					fVecOPlaysExp[losCntIdx][j] += fVecOnePlayRight[j];
					++expFeatNumGridsO[losCntIdx][j];
				}
			}
		}
		else
		{
			cout << "od labels are wrong!" << endl;
			return;
		}
	}

	for(unsigned int i = 0; i < fVecOPlaysExp.size(); ++i)
	{
		for(unsigned int j = 0; j < fVecOPlaysExp[i].size(); ++j)
		{
			if(expFeatNumGridsO[i][j] != 0)
				fVecOPlaysExp[i][j] /= expFeatNumGridsO[i][j];
			if(expFeatNumGridsD[i][j] != 0)
				fVecDPlaysExp[i][j] /= expFeatNumGridsD[i][j];
			//cout << fVecOPlaysExp[i][j] << " " << fVecDPlaysExp[i][j] << endl;
		}
	}

	string oExpFile = "randTreesTrainData/features/oExp";
	string dExpFile = "randTreesTrainData/features/dExp";
	ofstream fOutOExp(oExpFile.c_str()), fOutDExp(dExpFile.c_str());
	for(unsigned int i = 0; i < fVecOPlaysExp.size(); ++i)
	{
		for(unsigned int j = 0; j < fVecOPlaysExp[i].size(); ++j)
		{
			fOutOExp << fVecOPlaysExp[i][j] << " ";
			fOutDExp << fVecDPlaysExp[i][j] << " ";
		}
		fOutOExp << endl;
		fOutDExp << endl;
	}

	fOutOExp.close();
	fOutDExp.close();

}


void computeOverallExpFeats(const vector<int> &games)
{
	vector<string> leftFeatFiles, rightFeatFiles, losCntFileNames;
	for(unsigned int g = 0; g < games.size(); ++g)
	{
		int gameId = games[g];
		ostringstream convertGameId;
		convertGameId << gameId ;
		string gameIdStr = convertGameId.str();

		if(gameId < 10)
			gameIdStr = "0" + gameIdStr;

		cout << "Computing expectation of feature vectors of Game" << gameIdStr << "..."<< endl;

		//string featuresFile = "randTreesTrainData/featureVecsGame" + gameIdStr;
		string leftFeatFile = "randTreesTrainData/features/featsLeftRectGame" + gameIdStr;
		leftFeatFiles.push_back(leftFeatFile);
		string rightFeatFile = "randTreesTrainData/features/featsRightRectGame" + gameIdStr;
		rightFeatFiles.push_back(rightFeatFile);
		string losCntIdsFile = "randTreesTrainData/losCnt/losCntIdsGame" + gameIdStr;
		losCntFileNames.push_back(losCntIdsFile);
	}

	vector<vector<double> > fVecPlaysLeft, fVecPlaysRight;
	vector<double> leftLabels, rightLabels;

	readOdFeatData(leftFeatFiles, fVecPlaysLeft, leftLabels, fNumPerPlayOneSide);
	readOdFeatData(rightFeatFiles, fVecPlaysRight, rightLabels, fNumPerPlayOneSide);

	vector<int> losCntIds;
	readLosCntIds(losCntFileNames, losCntIds);

	vector<int> zerosVec(fNumPerPlayOneSide, 0);
	vector<vector<int> > fVecLeftPlaysExp(losCntBins, zerosVec), fVecRightPlaysExp(losCntBins, zerosVec);
	vector<vector<int> > expFeatNumGridsL(losCntBins, zerosVec), expFeatNumGridsR(losCntBins, zerosVec);

	//cout << "leftLabels.size() " << leftLabels.size() << endl;
	for(unsigned int i = 0; i < leftLabels.size(); ++i)
	{
		vector<double> fVecOnePlayLeft = fVecPlaysLeft[i],
				fVecOnePlayRight = fVecPlaysRight[i];

		int losCntIdx = losCntIds[i];

		for(unsigned int j = 0; j < fVecOnePlayLeft.size(); ++j)
		{
			//cout << fVecOnePlayLeft[j] << endl;
			if(fVecOnePlayLeft[j] >= .0)
			{
				fVecLeftPlaysExp[losCntIdx][j] += fVecOnePlayLeft[j];
				++expFeatNumGridsL[losCntIdx][j];
			}
		}

		for(unsigned int j = 0; j < fVecOnePlayRight.size(); ++j)
		{
			if(fVecOnePlayRight[j] >= .0)
			{
				fVecRightPlaysExp[losCntIdx][j] += fVecOnePlayRight[j];
				++expFeatNumGridsR[losCntIdx][j];
			}
		}
	}

	for(unsigned int i = 0; i < fVecLeftPlaysExp.size(); ++i)
	{
		for(unsigned int j = 0; j < fVecLeftPlaysExp[i].size(); ++j)
		{
			if(expFeatNumGridsL[i][j] != 0)
				fVecLeftPlaysExp[i][j] /= expFeatNumGridsL[i][j];
			if(expFeatNumGridsR[i][j] != 0)
				fVecRightPlaysExp[i][j] /= expFeatNumGridsR[i][j];
			//cout << fVecOPlaysExp[i][j] << " " << fVecDPlaysExp[i][j] << endl;
		}
	}

	string leftExpFile = "randTreesTrainData/features/leftExp";
	string rightExpFile = "randTreesTrainData/features/rightExp";
	ofstream fOutLeftExp(leftExpFile.c_str()), fOutRightExp(rightExpFile.c_str());
	for(unsigned int i = 0; i < fVecLeftPlaysExp.size(); ++i)
	{
		for(unsigned int j = 0; j < fVecRightPlaysExp[i].size(); ++j)
		{
			fOutLeftExp << fVecLeftPlaysExp[i][j] << " ";
			fOutRightExp << fVecRightPlaysExp[i][j] << " ";
		}
		fOutLeftExp << endl;
		fOutRightExp << endl;
	}

	fOutLeftExp.close();
	fOutRightExp.close();
}



void readOdExpFeats(vector<vector<int> > &fVecOPlaysExp, vector<vector<int> > &fVecDPlaysExp,
		string oExpFile, string dExpFile)
{
	vector<int> zerosVec(fNumPerPlayOneSide, 0);
	fVecOPlaysExp = vector<vector<int> >(losCntBins, zerosVec);
	fVecDPlaysExp = vector<vector<int> >(losCntBins, zerosVec);

	cout << "reading " << oExpFile << endl;
	ifstream finOExp(oExpFile.c_str());
	if(!finOExp.is_open())
	{
		cout << "Can't open file " << oExpFile << endl;
		return;
	}

	finOExp.seekg(0, ios::end);
	if (finOExp.tellg() == 0) {
		cout << "Empty file " << oExpFile << endl;
		return;
	}
	finOExp.seekg(0, ios::beg);

	cout << "reading " << dExpFile << endl;
	ifstream finDExp(dExpFile.c_str());
	if(!finDExp.is_open())
	{
		cout << "Can't open file " << dExpFile << endl;
		return;
	}

	finDExp.seekg(0, ios::end);
	if (finDExp.tellg() == 0) {
		cout << "Empty file " << dExpFile << endl;
		return;
	}
	finDExp.seekg(0, ios::beg);

	for(unsigned int i = 0; i < fVecOPlaysExp.size(); ++i)
	{
		for(unsigned int j = 0; j < fVecOPlaysExp[i].size(); ++j)
		{
			finOExp >> fVecOPlaysExp[i][j];
			finDExp >> fVecDPlaysExp[i][j];
		}
	}

	finOExp.close();
	finDExp.close();

}

void computeOdSubFeatsMissExp(const vector<int> &games, int odMode)
{
	vector<vector<int> > fVecOPlaysExp, fVecDPlaysExp;

	string oExpFile = "randTreesTrainData/features/oExp";
	string dExpFile = "randTreesTrainData/features/dExp";
	readOdExpFeats(fVecOPlaysExp, fVecDPlaysExp, oExpFile, dExpFile);

	for(unsigned int g = 0; g < games.size(); ++g)
	{
		vector<string> leftFeatFiles, rightFeatFiles, losCntFileNames;
		int gameId = games[g];
		ostringstream convertGameId;
		convertGameId << gameId ;
		string gameIdStr = convertGameId.str();

		if(gameId < 10)
			gameIdStr = "0" + gameIdStr;

		cout << "Computing feature vectors of Game" << gameIdStr << " with expectation compensation..."<< endl;

		//string featuresFile = "randTreesTrainData/featureVecsGame" + gameIdStr;
		string leftFeatFile = "randTreesTrainData/features/featsLeftRectGame" + gameIdStr;
		leftFeatFiles.push_back(leftFeatFile);
		string rightFeatFile = "randTreesTrainData/features/featsRightRectGame" + gameIdStr;
		rightFeatFiles.push_back(rightFeatFile);
		string losCntIdsFile = "randTreesTrainData/losCnt/losCntIdsGame" + gameIdStr;
		losCntFileNames.push_back(losCntIdsFile);

		vector<vector<double> > fVecPlaysLeft, fVecPlaysRight;
		vector<double> leftLabels, rightLabels;

		readOdFeatData(leftFeatFiles, fVecPlaysLeft, leftLabels, fNumPerPlayOneSide);
		readOdFeatData(rightFeatFiles, fVecPlaysRight, rightLabels, fNumPerPlayOneSide);

		vector<int> losCntIds;
		readLosCntIds(losCntFileNames, losCntIds);

		string odLabelFilePath = "randTreesTrainData/odPlays/odGame" + gameIdStr + "Rect";
		vector<string> dirs, odLabels;
		vector<playId> pIds;
		readOdLabels(odLabelFilePath, pIds, dirs, odLabels);

		//string featuresFile = "randTreesTrainData/featureVecsGame" + gameIdStr;
		string featuresFile;
		if(odMode == 1)
			featuresFile = "randTreesTrainData/features/featsGame" + gameIdStr + "RectExp";
		else if(odMode == 2)
			featuresFile = "randTreesTrainData/features/featsGame" + gameIdStr + "RectExpLeftO";
		else if(odMode == 3)
			featuresFile = "randTreesTrainData/features/featsGame" + gameIdStr + "RectExpLeftD";
		else
		{
			cout << "odMode error." << endl;
			return;
		}
		ofstream fout(featuresFile.c_str());

		for(unsigned int i = 0; i < pIds.size(); ++i)
			{
				vector<double> fVecOnePlayLeft = fVecPlaysLeft[i];
				vector<double> fVecOnePlayRight = fVecPlaysRight[i];

				int losCntIdx = losCntIds[i];

				if(odMode == 1)
				{
					if(odLabels[i] == "o")
					{
						for(unsigned int j = 0; j < fVecOnePlayLeft.size(); ++j)
						{
							if(fVecOnePlayLeft[j] == -1)
								fVecOnePlayLeft[j] = fVecOPlaysExp[losCntIdx][j];
						}

						for(unsigned int j = 0; j < fVecOnePlayRight.size(); ++j)
						{
							if(fVecOnePlayRight[j] == -1)
								fVecOnePlayRight[j] = fVecDPlaysExp[losCntIdx][j];
						}

					}
					else if(odLabels[i] == "d")
					{
						for(unsigned int j = 0; j < fVecOnePlayLeft.size(); ++j)
						{
							if(fVecOnePlayLeft[j] == -1)
								fVecOnePlayLeft[j] = fVecDPlaysExp[losCntIdx][j];
						}

						for(unsigned int j = 0; j < fVecOnePlayRight.size(); ++j)
						{
							if(fVecOnePlayRight[j] == -1)
								fVecOnePlayRight[j] = fVecOPlaysExp[losCntIdx][j];
						}
					}
				}
				else if(odMode == 2)
				{
					for(unsigned int j = 0; j < fVecOnePlayLeft.size(); ++j)
					{
						if(fVecOnePlayLeft[j] == -1)
							fVecOnePlayLeft[j] = fVecOPlaysExp[losCntIdx][j];
					}

					for(unsigned int j = 0; j < fVecOnePlayRight.size(); ++j)
					{
						if(fVecOnePlayRight[j] == -1)
							fVecOnePlayRight[j] = fVecDPlaysExp[losCntIdx][j];
					}

				}
				else if(odMode == 3)
				{

					for(unsigned int j = 0; j < fVecOnePlayLeft.size(); ++j)
					{
						if(fVecOnePlayLeft[j] == -1)
							fVecOnePlayLeft[j] = fVecDPlaysExp[losCntIdx][j];
					}

					for(unsigned int j = 0; j < fVecOnePlayRight.size(); ++j)
					{
						if(fVecOnePlayRight[j] == -1)
							fVecOnePlayRight[j] = fVecOPlaysExp[losCntIdx][j];
					}
				}

				vector<double> fVecOnePlay;
				vector<bool> inFldOnePlay;
				for(unsigned int j = 0; j < fVecOnePlayLeft.size(); ++j)
				{
					if(fVecOnePlayLeft[j] != -2.0 && fVecOnePlayRight[j] != -2.0)
					{
						fVecOnePlay.push_back(fVecOnePlayLeft[j] - fVecOnePlayRight[j]);
						inFldOnePlay.push_back(true);
					}
					else
					{
						fVecOnePlay.push_back(-2.0);
						inFldOnePlay.push_back(false);
					}
				}

				fout << odLabels[i] << ",";
				double maxFVal = -1.0;
				for(unsigned int j = 0; j < fVecOnePlay.size(); ++j)
					if(abs(fVecOnePlay[j]) > maxFVal && inFldOnePlay[j])
						maxFVal = abs(fVecOnePlay[j]);

				if(maxFVal != .0)
				{
					for(unsigned int j = 0; j < fVecOnePlay.size(); ++j)
					{
						if(inFldOnePlay[j])
							fout << fVecOnePlay[j] / maxFVal << ",";
						else
							fout << fVecOnePlay[j] << ",";
					}
				}
				else
				{
					for(unsigned int j = 0; j < fVecOnePlay.size(); ++j)
					{
							fout << fVecOnePlay[j] << ",";
					}
				}

				fout << endl;
			}
		fout.close();
	}

}

void computeOdConcaFMissExp(const std::vector<int> &games, int odMode)
{
	vector<vector<int> > fVecOPlaysExp, fVecDPlaysExp;
	string oExpFile = "randTreesTrainData/features/oExp";
	string dExpFile = "randTreesTrainData/features/dExp";
	readOdExpFeats(fVecOPlaysExp, fVecDPlaysExp, oExpFile, dExpFile);

	for(unsigned int g = 0; g < games.size(); ++g)
	{
		vector<string> leftFeatFiles, rightFeatFiles, losCntFileNames;
		int gameId = games[g];
		ostringstream convertGameId;
		convertGameId << gameId ;
		string gameIdStr = convertGameId.str();

		if(gameId < 10)
			gameIdStr = "0" + gameIdStr;

		cout << "Computing concatenating feature vectors of Game"
				<< gameIdStr << " with expectation compensation..."<< endl;

		string leftFeatFile = "randTreesTrainData/features/featsLeftRectGame" + gameIdStr;
		leftFeatFiles.push_back(leftFeatFile);
		string rightFeatFile = "randTreesTrainData/features/featsRightRectGame" + gameIdStr;
		rightFeatFiles.push_back(rightFeatFile);
		string losCntIdsFile = "randTreesTrainData/losCnt/losCntIdsGame" + gameIdStr;
		losCntFileNames.push_back(losCntIdsFile);

		vector<vector<double> > fVecPlaysLeft, fVecPlaysRight;
		vector<double> leftLabels, rightLabels;

		readOdFeatData(leftFeatFiles, fVecPlaysLeft, leftLabels, fNumPerPlayOneSide);
		readOdFeatData(rightFeatFiles, fVecPlaysRight, rightLabels, fNumPerPlayOneSide);

		vector<int> losCntIds;
		readLosCntIds(losCntFileNames, losCntIds);

		string odLabelFilePath = "randTreesTrainData/odPlays/odGame" + gameIdStr + "Rect";
		vector<string> dirs, odLabels;
		vector<playId> pIds;
		readOdLabels(odLabelFilePath, pIds, dirs, odLabels);

		//string featuresFile = "randTreesTrainData/featureVecsGame" + gameIdStr;
		string featuresFile;
		if(odMode == 1)
			featuresFile = "randTreesTrainData/features/featsGame" + gameIdStr + "RectExp";
		else if(odMode == 2)
			featuresFile = "randTreesTrainData/features/featsGame" + gameIdStr + "RectExpLeftO";
		else if(odMode == 3)
			featuresFile = "randTreesTrainData/features/featsGame" + gameIdStr + "RectExpLeftD";
		else
		{
			cout << "odMode error." << endl;
			return;
		}
		ofstream fout(featuresFile.c_str());

		for(unsigned int i = 0; i < pIds.size(); ++i)
			{
				vector<double> fVecOnePlayLeft = fVecPlaysLeft[i];
				vector<double> fVecOnePlayRight = fVecPlaysRight[i];


				int losCntIdx = losCntIds[i];

				if(odMode == 1)
				{
					if(odLabels[i] == "o")
					{
						for(unsigned int j = 0; j < fVecOnePlayLeft.size(); ++j)
						{
							if(fVecOnePlayLeft[j] == -1)
								fVecOnePlayLeft[j] = fVecOPlaysExp[losCntIdx][j];
						}

						for(unsigned int j = 0; j < fVecOnePlayRight.size(); ++j)
						{
							if(fVecOnePlayRight[j] == -1)
								fVecOnePlayRight[j] = fVecDPlaysExp[losCntIdx][j];
						}

					}
					else if(odLabels[i] == "d")
					{
						for(unsigned int j = 0; j < fVecOnePlayLeft.size(); ++j)
						{
							if(fVecOnePlayLeft[j] == -1)
								fVecOnePlayLeft[j] = fVecDPlaysExp[losCntIdx][j];
						}

						for(unsigned int j = 0; j < fVecOnePlayRight.size(); ++j)
						{
							if(fVecOnePlayRight[j] == -1)
								fVecOnePlayRight[j] = fVecOPlaysExp[losCntIdx][j];
						}
					}
					else
					{
						cout << "wrong odlabels." << endl;
					}
				}
				else if(odMode == 2)
				{
					for(unsigned int j = 0; j < fVecOnePlayLeft.size(); ++j)
					{
						if(fVecOnePlayLeft[j] == -1)
							fVecOnePlayLeft[j] = fVecOPlaysExp[losCntIdx][j];
					}

					for(unsigned int j = 0; j < fVecOnePlayRight.size(); ++j)
					{
						if(fVecOnePlayRight[j] == -1)
							fVecOnePlayRight[j] = fVecDPlaysExp[losCntIdx][j];
					}

				}
				else if(odMode == 3)
				{

					for(unsigned int j = 0; j < fVecOnePlayLeft.size(); ++j)
					{
						if(fVecOnePlayLeft[j] == -1)
							fVecOnePlayLeft[j] = fVecDPlaysExp[losCntIdx][j];
					}

					for(unsigned int j = 0; j < fVecOnePlayRight.size(); ++j)
					{
						if(fVecOnePlayRight[j] == -1)
							fVecOnePlayRight[j] = fVecOPlaysExp[losCntIdx][j];
					}
				}

				vector<double> fVecOnePlay;
				for(unsigned int j = 0; j < fVecOnePlayLeft.size(); ++j)
					fVecOnePlay.push_back(fVecOnePlayLeft[j]);

				for(unsigned int j = 0; j < fVecOnePlayRight.size(); ++j)
					fVecOnePlay.push_back(fVecOnePlayRight[j]);

				fout << odLabels[i] << ",";

				for(unsigned int j = 0; j < fVecOnePlay.size(); ++j)
					fout << fVecOnePlay[j] << ",";

				fout << endl;
			}
		fout.close();
	}

}



void computeOdConcaFOverallExp(const vector<int> &games)
{
	vector<vector<int> > fVecLPlaysExp, fVecRPlaysExp;
	string leftExpFile = "randTreesTrainData/features/leftExp";
	string rightExpFile = "randTreesTrainData/features/rightExp";
	readOdExpFeats(fVecLPlaysExp, fVecRPlaysExp, leftExpFile, rightExpFile);

	for(unsigned int g = 0; g < games.size(); ++g)
	{
		vector<string> leftFeatFiles, rightFeatFiles, losCntFileNames;
		int gameId = games[g];
		ostringstream convertGameId;
		convertGameId << gameId ;
		string gameIdStr = convertGameId.str();

		if(gameId < 10)
			gameIdStr = "0" + gameIdStr;

		cout << "Computing concatenating feature vectors of Game"
				<< gameIdStr << " with expectation compensation..."<< endl;

		//string featuresFile = "randTreesTrainData/featureVecsGame" + gameIdStr;
		string leftFeatFile = "randTreesTrainData/features/featsLeftRectGame" + gameIdStr;
		leftFeatFiles.push_back(leftFeatFile);
		string rightFeatFile = "randTreesTrainData/features/featsRightRectGame" + gameIdStr;
		rightFeatFiles.push_back(rightFeatFile);
		string losCntIdsFile = "randTreesTrainData/losCnt/losCntIdsGame" + gameIdStr;
		losCntFileNames.push_back(losCntIdsFile);

		vector<vector<double> > fVecPlaysLeft, fVecPlaysRight;
		vector<double> leftLabels, rightLabels;

		readOdFeatData(leftFeatFiles, fVecPlaysLeft, leftLabels, fNumPerPlayOneSide);
		readOdFeatData(rightFeatFiles, fVecPlaysRight, rightLabels, fNumPerPlayOneSide);

		vector<int> losCntIds;
		readLosCntIds(losCntFileNames, losCntIds);

		string odLabelFilePath = "randTreesTrainData/odPlays/odGame" + gameIdStr + "Rect";
		vector<string> dirs, odLabels;
		vector<playId> pIds;
		readOdLabels(odLabelFilePath, pIds, dirs, odLabels);

		//string featuresFile = "randTreesTrainData/featureVecsGame" + gameIdStr;
		string featuresFile = "randTreesTrainData/features/featsGame" + gameIdStr + "RectOverallExp";
		ofstream fout(featuresFile.c_str());

		for(unsigned int i = 0; i < pIds.size(); ++i)
			{
				vector<double> fVecOnePlayLeft = fVecPlaysLeft[i];
				vector<double> fVecOnePlayRight = fVecPlaysRight[i];


				int losCntIdx = losCntIds[i];

				for(unsigned int j = 0; j < fVecOnePlayLeft.size(); ++j)
				{
					if(fVecOnePlayLeft[j] == -1)
						fVecOnePlayLeft[j] = fVecLPlaysExp[losCntIdx][j];
				}

				for(unsigned int j = 0; j < fVecOnePlayRight.size(); ++j)
				{
					if(fVecOnePlayRight[j] == -1)
						fVecOnePlayRight[j] = fVecRPlaysExp[losCntIdx][j];
				}

				vector<double> fVecOnePlay;
				for(unsigned int j = 0; j < fVecOnePlayLeft.size(); ++j)
					fVecOnePlay.push_back(fVecOnePlayLeft[j]);

				for(unsigned int j = 0; j < fVecOnePlayRight.size(); ++j)
					fVecOnePlay.push_back(fVecOnePlayRight[j]);

				fout << odLabels[i] << ",";

				double maxFVal = .0;
				for(unsigned int j = 0; j < fVecOnePlay.size(); ++j)
					if(fVecOnePlay[j] > .0)
						maxFVal += fVecOnePlay[j];

				if(maxFVal != 0)
				{
					for(unsigned int j = 0; j < fVecOnePlay.size(); ++j)
						fout << fVecOnePlay[j] / maxFVal << ",";
				}
				else
				{
					for(unsigned int j = 0; j < fVecOnePlay.size(); ++j)
						fout << fVecOnePlay[j] << ",";
				}

				fout << endl;
			}
		fout.close();
	}

}




void computeOdFeatsNoExp(const vector<int> &games, int fMode)
{

	for(unsigned int g = 0; g < games.size(); ++g)
	{
		vector<string> leftFeatFiles, rightFeatFiles, losCntFileNames;
		int gameId = games[g];
		ostringstream convertGameId;
		convertGameId << gameId ;
		string gameIdStr = convertGameId.str();

		if(gameId < 10)
			gameIdStr = "0" + gameIdStr;

		cout << "Computing feature vectors of Game" << gameIdStr << " without expectation compensation..."<< endl;

		//string featuresFile = "randTreesTrainData/featureVecsGame" + gameIdStr;
		string leftFeatFile = "randTreesTrainData/features/featsLeftRectGame" + gameIdStr;
		leftFeatFiles.push_back(leftFeatFile);
		string rightFeatFile = "randTreesTrainData/features/featsRightRectGame" + gameIdStr;
		rightFeatFiles.push_back(rightFeatFile);

		vector<vector<double> > fVecPlaysLeft, fVecPlaysRight;
		vector<double> leftLabels, rightLabels;

		readOdFeatData(leftFeatFiles, fVecPlaysLeft, leftLabels, fNumPerPlayOneSide);
		readOdFeatData(rightFeatFiles, fVecPlaysRight, rightLabels, fNumPerPlayOneSide);

		string featuresFile = "randTreesTrainData/features/featuresGame" + gameIdStr + "Rect";
		ofstream fout(featuresFile.c_str());

		for(unsigned int i = 0; i < fVecPlaysLeft.size(); ++i)
			{
				vector<double> fVecOnePlayLeft = fVecPlaysLeft[i];
				vector<double> fVecOnePlayRight = fVecPlaysRight[i];
				vector<double> fVecOnePlay;
				if(fMode == 1)
				{
					for(unsigned int j = 0; j < fVecOnePlayLeft.size(); ++j)
							fVecOnePlay.push_back(fVecOnePlayLeft[j] - fVecOnePlayRight[j]);
				}
				else if(fMode ==2)
				{
					for(unsigned int j = 0; j < fVecOnePlayLeft.size(); ++j)
							fVecOnePlay.push_back(fVecOnePlayLeft[j]);
					for(unsigned int j = 0; j < fVecOnePlayRight.size(); ++j)
							fVecOnePlay.push_back(fVecOnePlayRight[j]);
				}
				else
				{
					cout << "Wrong fMode." << endl;
					return;
				}


				fout << (char)leftLabels[i] << ",";

				double maxFVal = .0;
				for(unsigned int j = 0; j < fVecOnePlay.size(); ++j)
						maxFVal += fVecOnePlay[j];

				if(maxFVal != 0)
				{
					for(unsigned int j = 0; j < fVecOnePlay.size(); ++j)
						fout << fVecOnePlay[j] / maxFVal << ",";
				}
				else
				{
					for(unsigned int j = 0; j < fVecOnePlay.size(); ++j)
						fout << fVecOnePlay[j] << ",";
				}
				fout << endl;
			}
		fout.close();
	}

}


void computeOdFeatsIndRspNoExp(const vector<int> &games, int fMode)
{

	for(unsigned int g = 0; g < games.size(); ++g)
	{
		vector<string> leftFeatFiles, rightFeatFiles, losCntFileNames;
		int gameId = games[g];
		ostringstream convertGameId;
		convertGameId << gameId ;
		string gameIdStr = convertGameId.str();

		if(gameId < 10)
			gameIdStr = "0" + gameIdStr;

		cout << "Computing feature vectors of Game" << gameIdStr << " without expectation compensation..."<< endl;

		//string featuresFile = "randTreesTrainData/featureVecsGame" + gameIdStr;
		string leftFeatFile = "randTreesTrainData/features/featsLeftRectGame" + gameIdStr;
		leftFeatFiles.push_back(leftFeatFile);
		string rightFeatFile = "randTreesTrainData/features/featsRightRectGame" + gameIdStr;
		rightFeatFiles.push_back(rightFeatFile);

		vector<vector<double> > fVecPlaysLeft, fVecPlaysRight;
		vector<double> leftLabels, rightLabels;

		readOdFeatData(leftFeatFiles, fVecPlaysLeft, leftLabels, fNumPerPlayOneSide);
		readOdFeatData(rightFeatFiles, fVecPlaysRight, rightLabels, fNumPerPlayOneSide);

		string featuresFile = "randTreesTrainData/features/featuresGame" + gameIdStr + "RectIndRsp";
		ofstream fout(featuresFile.c_str());

		for(unsigned int i = 0; i < fVecPlaysLeft.size(); ++i)
			{
				vector<double> fVecOnePlayLeft = fVecPlaysLeft[i];
				vector<double> fVecOnePlayRight = fVecPlaysRight[i];
				vector<double> fVecOnePlay;
				if(fMode == 1)
				{
					for(unsigned int j = 0; j < fVecOnePlayLeft.size(); ++j)
							fVecOnePlay.push_back(fVecOnePlayLeft[j] - fVecOnePlayRight[j]);
				}
				else if(fMode ==2)
				{
					for(unsigned int j = 0; j < fVecOnePlayLeft.size(); ++j)
							fVecOnePlay.push_back(fVecOnePlayLeft[j]);
					for(unsigned int j = 0; j < fVecOnePlayRight.size(); ++j)
							fVecOnePlay.push_back(fVecOnePlayRight[j]);
				}
				else
				{
					cout << "Wrong fMode." << endl;
					return;
				}


				fout << (char)leftLabels[i] << ",";

				double maxFVal = .0;
				for(unsigned int j = 0; j < fVecOnePlay.size(); j += 2)
						maxFVal += fVecOnePlay[j];

				if(maxFVal != 0)
				{
					for(unsigned int j = 0; j < fVecOnePlay.size(); ++j)
						if(j % 2 == 0)
							fout << fVecOnePlay[j] / maxFVal << ",";
						else
							fout << fVecOnePlay[j] << ",";
				}
				else
				{
					for(unsigned int j = 0; j < fVecOnePlay.size(); ++j)
						fout << fVecOnePlay[j] << ",";
				}
				fout << endl;
			}

		fout.close();
	}

}


//int main()
int extractFeatures(const vector<int> &games, const vector<int> &gamesFld, int expMode, int fMode)
{
	computeLeftRightFeats(games, expMode, gamesFld);

	if(expMode == 1)
	{
		computeOverallExpFeats(games);

		if(fMode == 1)
		{
			//subtraction
			computeOdSubFeatsMissExp(games, 1);
			computeOdSubFeatsMissExp(games, 2);
			computeOdSubFeatsMissExp(games, 3);
		}
		else if(fMode == 2)
		{
			//concatenation
			computeOdConcaFOverallExp(games);
		}
	}
	else if(expMode == 0)
	{
		//no expectation
		computeOdFeatsNoExp(games, fMode);
	}

	return 1;
}


int extractFeatures(const vector<int> &games, const vector<int> &gamesFld)
{
	computeOffsFeats(games, gamesFld);

	return 1;
}
