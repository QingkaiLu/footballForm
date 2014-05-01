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
//			string tmp;
//			fin >> tmp;
//			if(tmp.empty())
//			{
//				break;
//			}
			char oneLabel, comma;
			fin >> oneLabel >> comma;

			vector<double> featuresOneSample;
			bool fileEnd = false;
//			for(int j = 0; j < fNumPerPlayOneSide; ++j)
			for(int j = 0; j < featureNum; ++j)
			{
				double oneFeature = NEGINF;
//				if(j < (fNumPerPlayOneSide - 1) )
//					fin >> oneFeature >> comma;
//				else
//					fin >> oneFeature;
//				featuresOneSample.push_back(oneFeature);
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
//			cout << oneLabel << " ";
//			for(int j = 0; j < fNumPerPlayOneSide; ++j)
//			{
//				cout << featuresOneSample[j] << " ";
//			}
//			cout << endl;

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
//	string odPIdsPath = "odGame" + gameIdStr;

//	string odLabelFilePath = "randTreesTrainData/odGame" + gameIdStr;
	string odLabelFilePath = "randTreesTrainData/odPlays/odGame" + gameIdStr + "Rect";
//	vector<playId> pIds;
	vector<string> dirs, odLabels;

	readOdLabels(odLabelFilePath, pIds, dirs, odLabels);

	cout << "Computing feature vectors... " << endl;

	//string featuresFile = "randTreesTrainData/featureVecsGame" + gameIdStr;
	string featuresFile = "randTreesTrainData/features/featuresGame" + gameIdStr + "Rect";
	ofstream fout(featuresFile.c_str());
	vector<CvSize> gridSizes;
	vector<Point2i> gridsNum;
	setUpGrids(gridSizes, gridsNum);

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
		p->extractOdGridsFeatRect(d, fVecOnePlayLeft, gridSizes, gridsNum, 0);
		//p->extractOdStripsFeatFldCrd(d, fVecOnePlayLeft);
		//p->extractOdGridsFeatFldCrd(d, fVecOnePlayLeft);
		delete p;
		p = new play(pIds[i]);
		p->setUp();
		d = rightDir;
		//p->extractOdStripsFeature(d, fVecOnePlayRight);
//		p->extractOdStripsFeatRect(d, fVecOnePlayRight);
//		p->extractOdGridsFeatRect(d, fVecOnePlayRight);
		p->extractOdGridsFeatRect(d, fVecOnePlayRight, gridSizes, gridsNum, 0);
		//p->extractOdStripsFeatFldCrd(d, fVecOnePlayRight);
		//p->extractOdGridsFeatFldCrd(d, fVecOnePlayRight);
		for(unsigned int j = 0; j < fVecOnePlayLeft.size(); ++j)
			fVecOnePlay.push_back(fVecOnePlayLeft[j] - fVecOnePlayRight[j]);

		fout << odLabels[i] << ",";
		int maxFVal = -1;
		for(unsigned int j = 0; j < fVecOnePlay.size(); ++j)
			if(abs(fVecOnePlay[j]) > maxFVal)
				maxFVal = abs(fVecOnePlay[j]);
//		if(maxFVal != 0)
//		{
//			for(unsigned int j = 0; j < (fVecOnePlay.size() - 1); ++j)
//				fout << (fVecOnePlay[j] + 0.0) /  (maxFVal + 0.0) << ",";
//			fout << (fVecOnePlay[fVecOnePlay.size() - 1] + 0.0)  /  (maxFVal + 0.0);
//		}
//		else
//		{
//			for(unsigned int j = 0; j < (fVecOnePlay.size() - 1); ++j)
//				fout << (fVecOnePlay[j] + 0.0) << ",";
//			fout << (fVecOnePlay[fVecOnePlay.size() - 1] + 0.0);
//		}

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

void setUpGrids(vector<CvSize> &gridSizes, vector<Point2i> &gridsNum)
{
	gridSizes.push_back(cvSize(YardLinesDist * 6, YardLinesDist * 8));
	gridsNum.push_back(Point2i(1, 2));
	gridSizes.push_back(cvSize(YardLinesDist * 2, YardLinesDist * 2));
	gridsNum.push_back(Point2i(3, 8));
	gridSizes.push_back(cvSize(YardLinesDist, YardLinesDist));
	gridsNum.push_back(Point2i(6, 16));

}

void plotOdExp(const vector<vector<int> > &fVecOPlaysExp,
		const vector<vector<int> > &fVecDPlaysExp,
		const vector<CvSize> &gridSizes, const vector<Point2i> &gridsNum)
{
//	Mat fld(FieldLength, FieldWidth, CV_32FC3);
//	drawFieldModel(fld);
//
//	vector<rect> scanRects;
//	Point2d rectScrimCnt = Point2d(FieldWidth / 2.0, FieldLength / 2.0);
	Point2d rectScrimCnt = Point2d(FieldWidth / 2.0, FieldLength);

	int d = 0;
	direction dir = leftDir;
	if(dir == leftDir)
	{
		d = -1;
	}
	else if (dir == rightDir)
	{
		d = 1;
	}
	else
		return;

	int gridsIdx = 0;
	for(unsigned int k = 0; k < gridSizes.size(); ++k)
		{
			int gridsIdOneLevel = 0;
			vector<rect> scanRectsOneLevel;
			vector<int> zerosVec(gridsNum[k].x * gridsNum[k].y, 0);
			vector<vector<int> > fVecOPlaysExpOneLevel(losCntBins, zerosVec), fVecDPlaysExpOneLevel(losCntBins, zerosVec);

			for(int i = 0; i < gridsNum[k].x; ++i)
			{
				double maxYardLnXCoord, minYardLnXCoord;

				if(dir == rightDir)
				{
					maxYardLnXCoord = rectScrimCnt.x + d * (i + 1) * gridSizes[k].width;
					minYardLnXCoord = rectScrimCnt.x + d * i * gridSizes[k].width;
				}
				else
				{
					maxYardLnXCoord = rectScrimCnt.x + d * i * gridSizes[k].width;
					minYardLnXCoord = rectScrimCnt.x + d * (i + 1) * gridSizes[k].width;
				}

				for(int j = -0.5 * gridsNum[k].y; j < gridsNum[k].y * 0.5; ++j)
				{
					struct rect scanR;

		//			double lowY = rectScrimCnt.y + j * (FieldLength / 10.0);
		//			double upY = rectScrimCnt.y + (j + 1) * (FieldLength / 10.0);
					double lowY = rectScrimCnt.y + j * gridSizes[k].height;
					double upY = rectScrimCnt.y + (j + 1) * gridSizes[k].height;
		//			cout << "lowY: " << lowY << endl;
		//			cout << "upY: " << upY << endl;
					scanR.a = Point2d(minYardLnXCoord, lowY);
					scanR.b = Point2d(minYardLnXCoord, upY);
					scanR.c = Point2d(maxYardLnXCoord, upY);
					scanR.d = Point2d(maxYardLnXCoord, lowY);

					scanRectsOneLevel.push_back(scanR);
					for(unsigned int i = 0; i < fVecOPlaysExp.size(); ++i)
					{
						fVecOPlaysExpOneLevel[i][gridsIdOneLevel] = fVecOPlaysExp[i][gridsIdx];
						fVecDPlaysExpOneLevel[i][gridsIdOneLevel] = fVecDPlaysExp[i][gridsIdx];
					}

					++gridsIdOneLevel;
					++gridsIdx;

				}
			}

			ostringstream convertLev;
			convertLev << k;
			string levStr = convertLev.str();

			for(unsigned int i = 0; i < fVecOPlaysExpOneLevel.size(); ++i)
			{
				Mat fld(2.0 * FieldLength, FieldWidth, CV_32FC3);
				fld = Scalar(0,0,0);
				//drawFieldModel(fld);
				ostringstream convertIdx;
				convertIdx << i;
				string idxStr = convertIdx.str();
				plotScanLines(fld, scanRectsOneLevel, fVecOPlaysExpOneLevel[i]);
				string plotPath = "ExpPlots/expOLos" + idxStr + "lev" + levStr + ".jpg";
				imwrite(plotPath, fld);
			}

			for(unsigned int i = 0; i < fVecDPlaysExpOneLevel.size(); ++i)
			{
				Mat fld(2.0 * FieldLength, FieldWidth, CV_32FC3);
				fld = Scalar(0,0,0);
				//drawFieldModel(fld);
				ostringstream convertIdx;
				convertIdx << i;
				string idxStr = convertIdx.str();
				plotScanLines(fld, scanRectsOneLevel, fVecDPlaysExpOneLevel[i]);
				string plotPath = "ExpPlots/expDLos" + idxStr + "lev" + levStr + ".jpg";
				imwrite(plotPath, fld);
			}
		}



}

void computeLeftRightFeats(const vector<int> &games, int expMode)
{
	vector<CvSize> gridSizes;
	vector<Point2i> gridsNum;
	setUpGrids(gridSizes, gridsNum);

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

		cout << "Computing feature vectors of Game" << gameIdStr << "..."<< endl;

		//string featuresFile = "randTreesTrainData/featureVecsGame" + gameIdStr;
		string leftFeatFile = "randTreesTrainData/features/featsLeftRectGame" + gameIdStr;
		string rightFeatFile = "randTreesTrainData/features/featsRightRectGame" + gameIdStr;
		string losCntIdsFile = "randTreesTrainData/losCnt/losCntIdsGame" + gameIdStr;
		ofstream foutLeft(leftFeatFile.c_str());
		ofstream foutRight(rightFeatFile.c_str());
		ofstream foutLosIds(losCntIdsFile.c_str());
//		vector<CvSize> gridSizes;
//		vector<Point2i> gridsNum;
//		gridSizes.push_back(cvSize(YardLinesDist * 6, YardLinesDist * 8));
//		gridsNum.push_back(Point2i(1, 2));
//		gridSizes.push_back(cvSize(YardLinesDist * 2, YardLinesDist * 2));
//		gridsNum.push_back(Point2i(3, 8));
//		gridSizes.push_back(cvSize(YardLinesDist, YardLinesDist));
//		gridsNum.push_back(Point2i(6, 16));

		//compute feature vector for each play with all models
		for(unsigned int i = 0; i < pIds.size(); ++i)
		{
			vector<int> fVecOnePlayLeft, fVecOnePlayRight;
			play *p = NULL;
			cout << "gameId: " << pIds[i].gameId << " vidId: " << pIds[i].vidId << endl;
			if (p != NULL)
				delete p;
			p = new play(pIds[i]);
			p->setUp();

			direction d = nonDir;
			d = leftDir;
			p->extractOdGridsFeatRect(d, fVecOnePlayLeft, gridSizes, gridsNum, expMode);

			delete p;
			p = new play(pIds[i]);
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

	vector<CvSize> gridSizes;
	vector<Point2i> gridsNum;
	setUpGrids(gridSizes, gridsNum);
	plotOdExp(fVecOPlaysExp, fVecDPlaysExp, gridSizes, gridsNum);


}

void readOdExpFeats(vector<vector<int> > &fVecOPlaysExp, vector<vector<int> > &fVecDPlaysExp)
{
	string oExpFile = "randTreesTrainData/features/oExp";
	string dExpFile = "randTreesTrainData/features/dExp";
	vector<int> zerosVec(fNumPerPlayOneSide, 0);
	fVecOPlaysExp = vector<vector<int> >(losCntBins, zerosVec);
	fVecDPlaysExp = vector<vector<int> >(losCntBins, zerosVec);

	//ifstream finOExp(oExpFile.c_str()), finDExp(dExpFile.c_str());
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
			//cout << fVecOPlaysExp[i][j] << " ";
			finDExp >> fVecDPlaysExp[i][j];
			//cout << fVecDPlaysExp[i][j] << " ";
		}
		//cout << endl;
	}

	finOExp.close();
	finDExp.close();

}

void computeOdFeatsMissExp(const vector<int> &games, int odMode)
{
	vector<vector<int> > fVecOPlaysExp, fVecDPlaysExp;
	readOdExpFeats(fVecOPlaysExp, fVecDPlaysExp);

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
//				for(unsigned int j = 0; j < fVecOnePlay.size() ; ++j)
//					fout << fVecOnePlay[j] << ",";
//				fout << endl;
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
	readOdExpFeats(fVecOPlaysExp, fVecDPlaysExp);

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
//				for(unsigned int j = 0; j < fVecOnePlay.size() ; ++j)
//					fout << fVecOnePlay[j] << ",";
//				fout << endl;
//				double maxFVal = -1.0;
//				for(unsigned int j = 0; j < fVecOnePlay.size(); ++j)
//					if(abs(fVecOnePlay[j]) > maxFVal && fVecOnePlay[j] > .0)
//						maxFVal = abs(fVecOnePlay[j]);
//
//				if(maxFVal != .0)
//				{
//					for(unsigned int j = 0; j < fVecOnePlay.size(); ++j)
//					{
//						if(fVecOnePlay[j] > .0)
//							fout << fVecOnePlay[j] / maxFVal << ",";
//						else
//							fout << fVecOnePlay[j] << ",";
//					}
//				}
//				else
//				{
//					for(unsigned int j = 0; j < fVecOnePlay.size(); ++j)
//					{
//						fout << fVecOnePlay[j] << ",";
//					}
//				}

				for(unsigned int j = 0; j < fVecOnePlay.size(); ++j)
					fout << fVecOnePlay[j] << ",";

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
	}

}


//int main()
int extractFeatures(const vector<int> &games, int expMode, int fMode)
{
//	vector<int> games;
//	games.push_back(2);
//	games.push_back(8);
//	games.push_back(9);
//	games.push_back(10);

	computeLeftRightFeats(games, expMode);

	if(expMode == 1)
	{
		//with expectation
//		computeLeftRightFeats(games, 1);
		computeOdExpFeats(games);

		if(fMode == 1)
		{
			//subtraction
			computeOdFeatsMissExp(games, 1);
			computeOdFeatsMissExp(games, 2);
			computeOdFeatsMissExp(games, 3);
		}
		else if(fMode == 2)
		{
			//concatenation
			computeOdConcaFMissExp(games, 1);
			computeOdConcaFMissExp(games, 2);
			computeOdConcaFMissExp(games, 3);
		}
	}
	else if(expMode == 0)
	{
		//no expectation
	//	computeLeftRightFeats(games, 0);
		//subtraction
		computeOdFeatsNoExp(games, fMode);
		//concatenation
		//computeOdFeatsNoExp(games, fMode);
	}

	return 1;
}
