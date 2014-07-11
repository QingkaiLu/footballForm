#include <iostream>
#include <vector>
#include <fstream>
#include "extractOdVidFeats.h"
#include "shotgunClassification.h"
//#include "randTrees.h"
//#include "play.h"
#include "kernelSvm.h"

using namespace std;

void readShotgunLabels(string sgLabelFilePath, vector<string> &sgLabels)
{
	cout << "reading " << sgLabelFilePath << endl;
//	vector<playId> pIds;
//	vector<string> dirs;

	ifstream fin(sgLabelFilePath.c_str());
	if(!fin.is_open())
	{
		cout << "Can't open file " << sgLabelFilePath << endl;
		return;
	}

	fin.seekg(0, ios::end);
	if (fin.tellg() == 0) {
		cout << "Empty file " << sgLabelFilePath << endl;
		return;
	}
	fin.seekg(0, ios::beg);

	while(!fin.eof())
	{
		playId pId;
		string d, odLabl, sgLabl;
		pId.gameId = -1;
		fin >> pId.gameId >> pId.vidId >> d >> odLabl >> sgLabl;
		if(pId.gameId == -1)//file ends
			break;
		cout << pId.gameId << " " << pId.vidId  << " " << d << " "
				<< odLabl << " " << sgLabl;
		cout << endl;
//		pIds.push_back(pId);
//		dirs.push_back(d);
		sgLabels.push_back(sgLabl);
	}
	cout << endl;


	fin.close();
}

void computeOffFeatsShotgun(const vector<int> &games)//, vector<vector<double> > &offFeatsVec)
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

		cout << "Computing offense feature vectors of Game" << gameIdStr
				<< " without expectation compensation..."<< endl;

		//string featuresFile = "randTreesTrainData/featureVecsGame" + gameIdStr;
		string leftFeatFile = "randTreesTrainData/features/featsLeftRectGame" + gameIdStr;
		leftFeatFiles.push_back(leftFeatFile);
		string rightFeatFile = "randTreesTrainData/features/featsRightRectGame" + gameIdStr;
		rightFeatFiles.push_back(rightFeatFile);

		vector<vector<double> > fVecPlaysLeft, fVecPlaysRight;
		vector<double> leftLabels, rightLabels;

		readOdFeatData(leftFeatFiles, fVecPlaysLeft, leftLabels, fNumPerPlayOneSide);
		readOdFeatData(rightFeatFiles, fVecPlaysRight, rightLabels, fNumPerPlayOneSide);

		vector<string> sgLabels;
		//file:///scratch/workspace/picStrucWR/shotgunLabels/shotgunGame08
		string sgLabelFilePath = "shotgunLabels/shotgunGame" + gameIdStr;
		readShotgunLabels(sgLabelFilePath, sgLabels);

		string featuresFile = "randTreesTrainData/features/offSgFeatsGame" + gameIdStr + "Rect";
		ofstream fout(featuresFile.c_str());

		for(unsigned int i = 0; i < fVecPlaysLeft.size(); ++i)
		{
			vector<double> fVecOnePlayLeft = fVecPlaysLeft[i];
			vector<double> fVecOnePlayRight = fVecPlaysRight[i];
			vector<double> fVecOnePlay;
			if((char)leftLabels[i] == 'o')
			{
				for(unsigned int j = 0; j < fVecOnePlayLeft.size(); ++j)
						fVecOnePlay.push_back(fVecOnePlayLeft[j]);

//				for(unsigned int j = 0; j < fVecOnePlayRight.size(); ++j)
//						fVecOnePlay.push_back(fVecOnePlayRight[j]);
			}
			else if((char)leftLabels[i] == 'd')
			{
				for(unsigned int j = 0; j < fVecOnePlayRight.size(); ++j)
						fVecOnePlay.push_back(fVecOnePlayRight[j]);

//				for(unsigned int j = 0; j < fVecOnePlayLeft.size(); ++j)
//						fVecOnePlay.push_back(fVecOnePlayLeft[j]);
			}
			else
			{
				cout << "Wrong od label." << endl;
				return;
			}
//			offFeatsVec.push_back(fVecOnePlay);

			//fout << (char)leftLabels[i] << ",";sgLabels
			fout << sgLabels[i] << ",";

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

//int extractSgFeats(const vector<int> &games, int expMode)
//{
//	computeLeftRightFeats(games, expMode);
//	computeOffFeatsShotgun(games);
//
//	return 1;
//}

void computeOffSgFeatsSvm(const vector<int> &games)
{
	cout << "Computing offense feature vectors for SVM of shotgun classification..." << endl;
	string featuresFile = "randTreesTrainData/features/svmSgFeatures";
	ofstream fout(featuresFile.c_str());
	for(unsigned int g = 0; g < games.size(); ++g)
	{
		vector<string> leftFeatFiles, rightFeatFiles, losCntFileNames;
		int gameId = games[g];
		ostringstream convertGameId;
		convertGameId << gameId ;
		string gameIdStr = convertGameId.str();

		if(gameId < 10)
			gameIdStr = "0" + gameIdStr;

		string leftFeatFile = "randTreesTrainData/features/featsLeftRectGame" + gameIdStr;
		leftFeatFiles.push_back(leftFeatFile);
		string rightFeatFile = "randTreesTrainData/features/featsRightRectGame" + gameIdStr;
		rightFeatFiles.push_back(rightFeatFile);

		vector<vector<double> > fVecPlaysLeft, fVecPlaysRight;
		vector<double> leftLabels, rightLabels;

		readOdFeatData(leftFeatFiles, fVecPlaysLeft, leftLabels, fNumPerPlayOneSide);
		readOdFeatData(rightFeatFiles, fVecPlaysRight, rightLabels, fNumPerPlayOneSide);

		vector<string> sgLabels;
		string sgLabelFilePath = "shotgunLabels/shotgunGame" + gameIdStr;
		readShotgunLabels(sgLabelFilePath, sgLabels);

		for(unsigned int i = 0; i < fVecPlaysLeft.size(); ++i)
		{
			vector<double> fVecOnePlayLeft = fVecPlaysLeft[i];
			vector<double> fVecOnePlayRight = fVecPlaysRight[i];
			vector<double> fVecOnePlay;

//			for(unsigned int j = 0; j < fVecOnePlayLeft.size(); ++j)
//					fVecOnePlay.push_back(fVecOnePlayLeft[j]);
//			for(unsigned int j = 0; j < fVecOnePlayRight.size(); ++j)
//					fVecOnePlay.push_back(fVecOnePlayRight[j]);

			if((char)leftLabels[i] == 'o')
			{
				for(unsigned int j = 0; j < fVecOnePlayLeft.size(); ++j)
						fVecOnePlay.push_back(fVecOnePlayLeft[j]);

//				for(unsigned int j = 0; j < fVecOnePlayRight.size(); ++j)
//						fVecOnePlay.push_back(fVecOnePlayRight[j]);
			}
			else if((char)leftLabels[i] == 'd')
			{
				for(unsigned int j = 0; j < fVecOnePlayRight.size(); ++j)
						fVecOnePlay.push_back(fVecOnePlayRight[j]);

//				for(unsigned int j = 0; j < fVecOnePlayLeft.size(); ++j)
//						fVecOnePlay.push_back(fVecOnePlayLeft[j]);
			}
			else
			{
				cout << "Wrong od label." << endl;
				return;
			}


//			fout << leftLabels[i] << " ";
//			fout << (double)sgLabels[i] << " ";
			double sgLab = 0;
			if(sgLabels[i] == "s")
				sgLab = 1;
			else if(sgLabels[i] == "n")
				sgLab = -1;
			else
			{
				cout << "Wrong shotgun labels!" << endl;
				return;
			}

			fout << sgLab << " ";

			double maxFVal = .0;
			for(unsigned int j = 0; j < fVecOnePlay.size(); ++j)
					maxFVal += fVecOnePlay[j];
			//double total = 0;
			if(maxFVal != 0)
			{
				for(unsigned int j = 0; j < fVecOnePlay.size(); ++j)
				{
					fout << j + 1 << ":";
					fout << fVecOnePlay[j] /  maxFVal << " ";
					//total += (fVecOnePlay[j] + 0.0) /  (maxFVal + 0.0);
				}
			}
			else
			{
				for(unsigned int j = 0; j < fVecOnePlay.size(); ++j)
				{
					fout << j + 1 << ":";
					fout << fVecOnePlay[j] << " ";
				}
			}
			fout << endl;
			//cout << "total: "  << total << endl;
		}
	}

	fout.close();

}


void computeOffSgSpPmdPKernel(const vector<int> &games)
{
	cout << "Computing spatial pyramid feature kernel..."<< endl;

//	vector<playId> pIds;
//	getPlayIds(games, pIds);

//	vector<vector<double> > fVecPlaysLeftAllGames, fVecPlaysRightAllGames;
	vector<vector<double> > fVecPlaysOffAllGames;
//	vector<double> leftLabelsAllGames;//, rightLabelsAllGames;
	vector<double> sgLabelsAllGames;
	for(unsigned int g = 0; g < games.size(); ++g)
	{
		vector<string> leftFeatFiles, rightFeatFiles, losCntFileNames;
		int gameId = games[g];
		ostringstream convertGameId;
		convertGameId << gameId ;
		string gameIdStr = convertGameId.str();

		if(gameId < 10)
			gameIdStr = "0" + gameIdStr;

		string leftFeatFile = "randTreesTrainData/features/featsLeftRectGame" + gameIdStr;
		leftFeatFiles.push_back(leftFeatFile);
		string rightFeatFile = "randTreesTrainData/features/featsRightRectGame" + gameIdStr;
		rightFeatFiles.push_back(rightFeatFile);

		vector<vector<double> > fVecPlaysLeft, fVecPlaysRight;
		vector<double> leftLabels, rightLabels;

		readOdFeatData(leftFeatFiles, fVecPlaysLeft, leftLabels, fNumPerPlayOneSide);
		readOdFeatData(rightFeatFiles, fVecPlaysRight, rightLabels, fNumPerPlayOneSide);

//		for(unsigned int i = 0; i < leftLabels.size(); ++i)
//			leftLabelsAllGames.push_back(leftLabels[i]);

//		for(unsigned int i = 0; i < fVecPlaysLeft.size(); ++i)
//			fVecPlaysLeftAllGames.push_back(fVecPlaysLeft[i]);
//		for(unsigned int i = 0; i < fVecPlaysRight.size(); ++i)
//			fVecPlaysRightAllGames.push_back(fVecPlaysRight[i]);


		for(unsigned int i = 0; i < fVecPlaysLeft.size(); ++i)
		{

			if((char)leftLabels[i] == 'o')
			{
				fVecPlaysOffAllGames.push_back(fVecPlaysLeft[i]);
			}
			else if((char)leftLabels[i] == 'd')
			{
				fVecPlaysOffAllGames.push_back(fVecPlaysRight[i]);
			}
			else
			{
				cout << "Wrong od label." << endl;
				return;
			}
		}

		vector<string> sgLabels;
		string sgLabelFilePath = "shotgunLabels/shotgunGame" + gameIdStr;
		readShotgunLabels(sgLabelFilePath, sgLabels);

		for(unsigned int i = 0; i < sgLabels.size(); ++i)
		{
			double sgLab = 0;
			if(sgLabels[i] == "s")
				sgLab = 1;
			else if(sgLabels[i] == "n")
				sgLab = -1;
			else
			{
				cout << "Wrong shotgun labels!" << endl;
				return;
			}

			sgLabelsAllGames.push_back(sgLab);
		}

	}

	//normlization
	for(unsigned int i = 0; i < fVecPlaysOffAllGames.size(); ++i)
	{
		vector<double> fVecOnePlayOff = fVecPlaysOffAllGames[i];
		double maxFVal = .0;
		for(unsigned int j = 0; j < fVecOnePlayOff.size(); ++j)
				maxFVal += fVecOnePlayOff[j];
		if(maxFVal != 0)
		{
			for(unsigned int j = 0; j < fVecOnePlayOff.size(); ++j)
			{
				fVecPlaysOffAllGames[i][j] /= maxFVal;
			}
		}


	}

	vector<vector<double> > kernels;
	for(unsigned int i = 0; i < fVecPlaysOffAllGames.size(); ++i)
	{
		vector<double> kerVec;
		for(unsigned int j = 0; j < fVecPlaysOffAllGames.size(); ++j)
		{
			if(j >= i)
			{
				double k = computeTwoFVecKernel(fVecPlaysOffAllGames[i], fVecPlaysOffAllGames[j]);
				kerVec.push_back(k);
			}
			else
				kerVec.push_back(kernels[j][i]);
		}
		kernels.push_back(kerVec);
	}

	string kernelPath = "randTreesTrainData/features/svmSgKernels";
	ofstream fout(kernelPath.c_str());
	for(unsigned int i = 0; i < fVecPlaysOffAllGames.size(); ++i)
	{
		fout << sgLabelsAllGames[i] << " ";
		fout << "0:" << i + 1 << " ";
		for(unsigned int j = 0; j < fVecPlaysOffAllGames.size(); ++j)
			fout << j + 1 << ":" << kernels[i][j] << " ";
		fout << endl;
	}

	fout.close();
}

//int main()
int sgClassification()
{
	vector<int> games;
//	games.push_back(2);
	games.push_back(8);
//	games.push_back(9);
//	games.push_back(10);
//	games.push_back(9);

//	int expMode = 0;

	//extractSgFeats(games, expMode);

	computeOffSgFeatsSvm(games);
	computeOffSgSpPmdPKernel(games);

	return 1;
}



