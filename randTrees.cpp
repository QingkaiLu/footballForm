#include <cv.h>
#include <ml.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <fstream>

#include "randTrees.h"

using namespace cv;
using namespace std;


#define ATTRIBUTES_PER_SAMPLE 5
#define INF std::numeric_limits<double>/*or float*/::infinity()
#define NEGINF -1.0 * INF

bool readData(const vector<string> &fileNames, vector<vector<double> > &features, vector<double> &labels)
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
			for(int j = 0; j < ATTRIBUTES_PER_SAMPLE; ++j)
			{
				double oneFeature = NEGINF;
				if(j < (ATTRIBUTES_PER_SAMPLE - 1) )
					fin >> oneFeature >> comma;
				else
					fin >> oneFeature;
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
//			for(int j = 0; j < ATTRIBUTES_PER_SAMPLE; ++j)
//			{
//				cout << featuresOneSample[j] << " ";
//			}
//			cout << endl;

		}


	fin.close();

	}

	return true;
}


void randTreeTrainTest(vector<int> &trainGames, vector<int> &testGames)
{
	vector<string> trainGameFilePaths, testGameFilePaths;
	for(unsigned int i = 0; i < trainGames.size(); ++i)
	{
		ostringstream convertGameId;
		convertGameId << trainGames[i] ;
		string gameIdStr = convertGameId.str();

		if(trainGames[i] < 10)
			gameIdStr = "0" + gameIdStr;
//		string path = "randTreesTrainData/featureVecsGame" + gameIdStr;
		string path = "randTreesTrainData/featuresGame" + gameIdStr + "Rect";
		trainGameFilePaths.push_back(path);
	}

	for(unsigned int i = 0; i < testGames.size(); ++i)
	{
		ostringstream convertGameId;
		convertGameId << testGames[i] ;
		string gameIdStr = convertGameId.str();

		if(testGames[i] < 10)
			gameIdStr = "0" + gameIdStr;
//		string path = "randTreesTrainData/featureVecsGame" + gameIdStr;
		string path = "randTreesTrainData/featuresGame" + gameIdStr + "Rect";
		testGameFilePaths.push_back(path);
	}
	vector<vector<double> > trainFeatures, testFeatures;
	vector<double> trainLabels, testLabels;
	readData(trainGameFilePaths, trainFeatures, trainLabels);
	readData(testGameFilePaths, testFeatures, testLabels);

	Mat trainFeaturesMat = Mat(trainFeatures.size(), ATTRIBUTES_PER_SAMPLE, CV_32FC1);
	Mat trainLabelsMat = Mat(trainLabels.size(), 1, CV_32FC1);

	for(unsigned int i = 0; i < trainLabels.size(); ++i)
	{
		trainLabelsMat.at<float>(i, 0) = trainLabels[i];

		for(unsigned int j = 0; j < ATTRIBUTES_PER_SAMPLE; ++j)
			trainFeaturesMat.at<float>(i, j) = trainFeatures[i][j];
	}

	Mat testFeaturesMat = Mat(testFeatures.size(), ATTRIBUTES_PER_SAMPLE, CV_32FC1);
	Mat testLabelsMat = Mat(testLabels.size(), 1, CV_32FC1);

	for(unsigned int i = 0; i < testLabels.size(); ++i)
	{
		testLabelsMat.at<float>(i, 0) = testLabels[i];

		for(unsigned int j = 0; j < ATTRIBUTES_PER_SAMPLE; ++j)
			testFeaturesMat.at<float>(i, j) = testFeatures[i][j];
	}


	Mat varType = Mat(ATTRIBUTES_PER_SAMPLE + 1, 1, CV_8U );
	varType.setTo(Scalar(CV_VAR_NUMERICAL) );
	varType.at<uchar>(ATTRIBUTES_PER_SAMPLE, 0) = CV_VAR_CATEGORICAL;

	CvRTParams params = CvRTParams(10,10,0,false,15,0,true,4,100,0.01f,CV_TERMCRIT_ITER);

	CvRTrees* rtree = new CvRTrees;
	rtree->train(trainFeaturesMat, CV_ROW_SAMPLE, trainLabelsMat,
				 Mat(), Mat(), varType, Mat(), params);

	double errSamples = 0.0;
	for(unsigned int i = 0; i < testLabels.size(); ++i)
	{
		Mat testSample = testFeaturesMat.row(i);
		double result = rtree->predict(testSample, Mat());

//		cout << "Testing "<< i << "th sample, detected as: " << (char)result << endl;
//		cout << "True label: " << (char)testLabelsMat.at<float>(i, 0) << endl;

		// if the prediction and the (true) testing classification are the same
		// (N.B. openCV uses a floating point decision tree implementation!)
		if (fabs(result - testLabelsMat.at<float>(i, 0))>= FLT_EPSILON)
		{
			++errSamples;
			cout << "Testing "<< i << "th sample, detected as: " << (char)result << endl;
			cout << "True label: " << (char)testLabelsMat.at<float>(i, 0) << endl;
		}
	}

	cout << "Testing accuracy: " << (1.0 - errSamples / (double)testLabels.size() ) << endl;


}

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
//	trainGames.push_back(8);
//	//trainGames.push_back(8);
////	trainGames.push_back(9);
////	trainGames.push_back(10);
//	//testGames.push_back(2);
//	testGames.push_back(8);
////	testGames.push_back(9);
////	testGames.push_back(10);
//	randTreeTrainTest(trainGames, testGames);
//	return 1;
//}
