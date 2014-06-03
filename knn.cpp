#include <cv.h>
#include <ml.h>
#include <stdio.h>
#include <string>
#include <vector>
//#include <fstream>

#include "commonStructs.h"
#include "extractOdVidFeats.h"
#include "knn.h"

using namespace cv;
using namespace std;


void knnLeavePlayOut(vector<int> &trainTestGames, vector<std::vector<playId> > &pIdsAllGames)
{
	vector<playId> pIdsAll;
	for(unsigned int i = 0; i < pIdsAllGames.size(); ++i)
	{
		for(unsigned int j = 0; j < pIdsAllGames[i].size(); ++j)
			pIdsAll.push_back(pIdsAllGames[i][j]);
	}


	vector<string> trainTestGamePaths;
	for(unsigned int i = 0; i < trainTestGames.size(); ++i)
	{
		ostringstream convertGameId;
		convertGameId << trainTestGames[i];
		string gameIdStr = convertGameId.str();

		if(trainTestGames[i] < 10)
			gameIdStr = "0" + gameIdStr;
//		string path = "randTreesTrainData/featureVecsGame" + gameIdStr;
		string path = "randTreesTrainData/features/featuresGame" + gameIdStr + "Rect";
//		string path = "randTreesTrainData/features/featsGame" + gameIdStr + "RectOverallExp";
//		string path = "randTreesTrainData/features/featuresGame" + gameIdStr + "RectIndRsp";
		trainTestGamePaths.push_back(path);
	}

	vector<vector<double> > trainTestFeats;
	vector<double> trainTestLabs;
	readOdFeatData(trainTestGamePaths, trainTestFeats, trainTestLabs, featNumPerPlay);

	int errPlays = 0;
	for(unsigned int k = 0; k < trainTestLabs.size(); ++k)
	{
		Mat trainFeaturesMat = Mat(trainTestFeats.size() - 1, featNumPerPlay, CV_32FC1);
		Mat trainLabelsMat = Mat(trainTestLabs.size() - 1, 1, CV_32FC1);

		Mat testFeaturesMat = Mat(1, featNumPerPlay, CV_32FC1);
		Mat testLabelsMat = Mat(1, 1, CV_32FC1);
//
		playId pId;
		for(unsigned int i = 0; i < trainTestLabs.size(); ++i)
		{
			if(i == k)
			{
				pId = pIdsAll[i];

				//cout << "Leave Game " << pId.gameId << ", play " << pId.vidId << " out..." << endl;
				testLabelsMat.at<float>(0, 0) = trainTestLabs[i];

				for(unsigned int j = 0; j < featNumPerPlay; ++j)
					testFeaturesMat.at<float>(0, j) = trainTestFeats[i][j];
			}
			else
			{
				int idx = -1;
				if(i > k)
					idx = i - 1;
				else
					idx = i;
				trainLabelsMat.at<float>(idx, 0) = trainTestLabs[i];

				for(unsigned int j = 0; j < featNumPerPlay; ++j)
					trainFeaturesMat.at<float>(idx, j) = trainTestFeats[i][j];
			}
		}
		int K = 1;
	    CvKNearest knn(trainFeaturesMat, trainLabelsMat, Mat(), false, K);
	   //CvMat* nearests = cvCreateMat( 1, K, CV_32FC1);
	    Mat neighborResponses(1, K, CV_32FC1);
	    Mat results(1, 1, CV_32FC1), dists(1, K, CV_32FC1);
//	    float** neighbors = new float*[10];
//	    CvMat** nbs;
//	    CvArr* neighbors;
//	    const float a[1] = {0};
//	    const float** b = &a;
	    float result = knn.find_nearest(testFeaturesMat, K, results, neighborResponses, dists);

	    int nbAgreeNum = 0;
        for( int k = 0; k < K; k++ )
        {
            if(neighborResponses.at<float>(0, k) == result)
            	nbAgreeNum++;
//            cout << "neighborResponses.at<float>(1, k) " << neighborResponses.at<float>(0, k) << endl;
//            cout << "result " << result << endl;
        }

		if (fabs(result - testLabelsMat.at<float>(0, 0))>= FLT_EPSILON)
		{
			++errPlays;
			cout << "Game " << pId.gameId << ", play " << pId.vidId << " is wrong. " << endl;
			cout << "Detected as: " << (char)result << endl;
			cout << "True label: " << (char)testLabelsMat.at<float>(0, 0) << endl;
			cout << "nbAgreeNum: " << nbAgreeNum << endl;
		}
//		else
//		{
//			cout << "Game " << pId.gameId << ", play " << pId.vidId << " is correct. " << endl;
//			cout << "nbAgreeNum: " << nbAgreeNum << endl;
//		}

	}

	cout << "Leave out plays overall accuracy: " << (1.0 - errPlays / (double)trainTestLabs.size() ) << endl;
}

void leavePlayOutTestKnn(const vector<int> &games)
{
//	int games[3] = {2, 8, 9};//, 10};

	vector<vector<playId> > pIdsTestGames;
	vector<int> trainTestGames;

	for(unsigned int i = 0; i < games.size(); ++i)
	{
		vector<playId> pIds;
		//extracOdVidFeatsRts(games[i], pIds);
		getPlayIds(games[i], pIds);
		pIdsTestGames.push_back(pIds);
		trainTestGames.push_back(games[i]);
	}
	knnLeavePlayOut(trainTestGames, pIdsTestGames);

}

int main()
//int knn()
{
	vector<int> games;
	games.push_back(2);
	games.push_back(8);
	games.push_back(9);
	games.push_back(10);

//	int expMode = 0;
//	int odExpMode = 1;
//	int featureMode = 2;

//	extractFeatures(games, expMode, featureMode);

//	leaveGamesOutTest(games);
	leavePlayOutTestKnn(games);

	return 1;
}

