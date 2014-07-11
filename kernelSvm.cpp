#include <cv.h>
#include <ml.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <math.h>
#include <fstream>

#include "commonStructs.h"
#include "extractOdVidFeats.h"
#include "kernelSvm.h"

using namespace cv;
using namespace std;

double computeTwoFVecKernel(const vector<double> &fVec1, const vector<double> &fVec2)
{
	vector<CvSize> gridSizes;
	vector<Point2i> gridsNum;
	setUpGrids(gridSizes, gridsNum, 1);

	int startIdx = 0;
	int levelsNum = gridsNum.size();
	vector<double> kernelsDifLevels(levelsNum, 0.0);
	for(int l = 0; l < levelsNum; ++l)
	{
		int gNum = gridsNum[l].x * gridsNum[l].y;
		for(int j = startIdx; j < gNum; ++j)
			kernelsDifLevels[l] += min(fVec1[j], fVec2[j]);
		startIdx += gNum;
	}

	double kernel = .0;
	for(unsigned int l = 0; l < levelsNum; ++l)
	{
		if(l == 0)
			kernel += pow(0.5, levelsNum - 1) * kernelsDifLevels[l];
		else
			kernel += pow(0.5, levelsNum - 1 - l + 1) * kernelsDifLevels[l];
	}

	return kernel;

//	vector<vector<double> > fVec1DifLevels, fVec2DifLevels;
//
//	int startIdx = 0;
//	int levelsNum = gridsNum.size();
//	for(unsigned int l = 0; l < levelsNum; ++l)
//	{
//		vector<double> fVec1OneLevel, fVec2OneLevel;
//		int gNum = gridsNum[l].x * gridsNum[l].y;
//		for(int j = startIdx; j < gNum; ++j)
//		{
//			fVec1OneLevel.push_back(fVec1[j]);
//			fVec2OneLevel.push_back(fVec2[j]);
//		}
//		startIdx += gNum;
//		fVec1DifLevels.push_back(fVec1OneLevel);
//		fVec2DifLevels.push_back(fVec2OneLevel);
//	}


}

void computeOdSpPmdPKernel(const vector<int> &games)
{
	cout << "Computing spatial pyramid feature kernel..."<< endl;

//	vector<playId> pIds;
//	getPlayIds(games, pIds);

	vector<vector<double> > fVecPlaysLeftAllGames, fVecPlaysRightAllGames;
	vector<double> leftLabelsAllGames;//, rightLabelsAllGames;
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

		for(unsigned int i = 0; i < fVecPlaysLeft.size(); ++i)
			fVecPlaysLeftAllGames.push_back(fVecPlaysLeft[i]);
		for(unsigned int i = 0; i < leftLabels.size(); ++i)
			leftLabelsAllGames.push_back(leftLabels[i]);

		for(unsigned int i = 0; i < fVecPlaysRight.size(); ++i)
			fVecPlaysRightAllGames.push_back(fVecPlaysRight[i]);
//		for(unsigned int i = 0; i < rightLabels.size(); ++i)
//			rightLabelsAllGames.push_back(rightLabels[i]);
	}

	//normlization
	for(unsigned int i = 0; i < fVecPlaysLeftAllGames.size(); ++i)
	{
		vector<double> fVecOnePlayLeft = fVecPlaysLeftAllGames[i];
		vector<double> fVecOnePlayRight = fVecPlaysRightAllGames[i];
		double maxFVal = .0;
		for(unsigned int j = 0; j < fVecOnePlayLeft.size(); ++j)
				maxFVal += fVecOnePlayLeft[j] + fVecOnePlayRight[j];
		if(maxFVal != 0)
		{
			for(unsigned int j = 0; j < fVecOnePlayLeft.size(); ++j)
			{
				fVecPlaysLeftAllGames[i][j] /= maxFVal;
				fVecPlaysRightAllGames[i][j] /= maxFVal;
			}
		}


	}

	vector<vector<double> > kernels;
	for(unsigned int i = 0; i < fVecPlaysLeftAllGames.size(); ++i)
	{
		vector<double> kerVec;
		for(unsigned int j = 0; j < fVecPlaysLeftAllGames.size(); ++j)
		{
			if(j >= i)
			{
				double kLeft = computeTwoFVecKernel(fVecPlaysLeftAllGames[i], fVecPlaysLeftAllGames[j]);
				double kRight = computeTwoFVecKernel(fVecPlaysRightAllGames[i], fVecPlaysRightAllGames[j]);
				double k = kLeft + kRight;
				kerVec.push_back(k);
			}
			else
				kerVec.push_back(kernels[j][i]);
		}
		kernels.push_back(kerVec);
	}

	string kernelPath = "randTreesTrainData/features/svmKernels";
	ofstream fout(kernelPath.c_str());
	for(unsigned int i = 0; i < fVecPlaysLeftAllGames.size(); ++i)
	{
		fout << leftLabelsAllGames[i] << " ";
		fout << "0:" << i + 1 << " ";
		for(unsigned int j = 0; j < fVecPlaysLeftAllGames.size(); ++j)
			fout << j + 1 << ":" << kernels[i][j] << " ";
		fout << endl;
	}

	fout.close();
}

void computeOdFeatsSvm(const vector<int> &games)
{
	cout << "Computing feature vectors for SVM..." << endl;
	string featuresFile = "randTreesTrainData/features/svmFeatures";
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

		for(unsigned int i = 0; i < fVecPlaysLeft.size(); ++i)
		{
			vector<double> fVecOnePlayLeft = fVecPlaysLeft[i];
			vector<double> fVecOnePlayRight = fVecPlaysRight[i];
			vector<double> fVecOnePlay;

			for(unsigned int j = 0; j < fVecOnePlayLeft.size(); ++j)
					fVecOnePlay.push_back(fVecOnePlayLeft[j]);
			for(unsigned int j = 0; j < fVecOnePlayRight.size(); ++j)
					fVecOnePlay.push_back(fVecOnePlayRight[j]);

			fout << leftLabels[i] << " ";
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

void computeOdConcaFOverallExpSvm(const vector<int> &games)
{
	vector<vector<int> > fVecLPlaysExp, fVecRPlaysExp;
	string leftExpFile = "randTreesTrainData/features/leftExp";
	string rightExpFile = "randTreesTrainData/features/rightExp";
	readOdExpFeats(fVecLPlaysExp, fVecRPlaysExp, leftExpFile, rightExpFile);

	string featuresFile = "randTreesTrainData/features/svmFeaturesOverallExp";
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

//		string odLabelFilePath = "randTreesTrainData/odPlays/odGame" + gameIdStr + "Rect";
//		vector<string> dirs, odLabels;
//		vector<playId> pIds;
//		readOdLabels(odLabelFilePath, pIds, dirs, odLabels);

		//string featuresFile = "randTreesTrainData/featureVecsGame" + gameIdStr;

		for(unsigned int i = 0; i < leftLabels.size(); ++i)
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

				fout << leftLabels[i] << " ";

				double maxFVal = .0;
				for(unsigned int j = 0; j < fVecOnePlay.size(); ++j)
					if(fVecOnePlay[j] > .0)
						maxFVal += fVecOnePlay[j];

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

//				for(unsigned int j = 0; j < fVecOnePlay.size(); ++j)
//					fout << fVecOnePlay[j] << ",";

//				fout << endl;
			}
	}

	fout.close();

}

void computeOdFeatsIndRspSvm(const vector<int> &games, int fMode)
{
	string featuresFile = "randTreesTrainData/features/svmFeaturesIndRsp";
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


				fout << leftLabels[i] << " ";

				double maxFVal = .0;
				for(unsigned int j = 0; j < fVecOnePlay.size(); j += 2)
						maxFVal += fVecOnePlay[j];

				if(maxFVal != 0)
				{
					for(unsigned int j = 0; j < fVecOnePlay.size(); ++j)
					{
						fout << j + 1 << ":";
						if(j % 2 == 0)
							fout << fVecOnePlay[j] / maxFVal << " ";
						else
							fout << fVecOnePlay[j] << " ";
					}
				}
				else
				{
					for(unsigned int j = 0; j < fVecOnePlay.size(); ++j)
					{
						fout << j + 1 << ":";
						fout << fVecOnePlay[j] << ",";
					}
				}
				fout << endl;
			}
	}
	fout.close();

}


//int main()
int kernelSvm()
{
	vector<int> games;
	games.push_back(2);
	games.push_back(8);
	games.push_back(9);
	games.push_back(10);

//	int expMode = 0;
//	computeLeftRightFeats(games, expMode);

	computeOdSpPmdPKernel(games);
	computeOdFeatsSvm(games);
//	computeOdConcaFOverallExpSvm(games);
//	computeOdFeatsIndRspSvm(games, 2);

	return 1;
}





















