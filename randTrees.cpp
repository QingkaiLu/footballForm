#include <cv.h>
#include <ml.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <fstream>

#include "extractOdVidFeats.h"
#include "randTrees.h"

using namespace cv;
using namespace std;


#define ATTRIBUTES_PER_SAMPLE 122
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


void splitOdSamples(const Mat &featureMat, const Mat &labelsMat,
		vector<Mat> &dMats, vector<Mat> &oMats)
{
	for(int i = 0; i < featureMat.rows; ++i)
	{
		char l = (char)labelsMat.at<float>(i, 0);
		//cout << l << endl;
		if(l == 'd')
		{
			Mat m = featureMat.row(i);
			dMats.push_back(m);
		}
		else if(l == 'o')
		{
			Mat m = featureMat.row(i);
			oMats.push_back(m);
		}
		else
		{
			cout << "split od samples error." << endl;
			return;
		}
	}
}

void getOdSamplesProx(const vector<Mat> &dMats, const vector<Mat> &oMats, const string &fileName, CvRTrees* rtree)
{
	ofstream fout(fileName.c_str());

	fout << "defense sample proximites: " << endl;
	for(unsigned int i = 0; i < dMats.size(); ++i)
	{
		CvMat sample1 = dMats[i];
		for(unsigned int j = 0; j < dMats.size(); ++j)
		{
			CvMat sample2 = dMats[j];
//			double prox = rtree->get_proximity( &sample1, &sample2 );
//			prox = 0.01 * (int(prox * 100));
			int prox = rtree->get_proximity( &sample1, &sample2) * 100;
			if(prox < 10)
				fout << "  " << prox << " ";
			else if(prox < 100)
				fout << " " << prox << " ";
			else
				fout << prox << " ";
		}
		fout << endl;
	}
	fout << "####################################################################################################" << endl;

	fout << "ofense sample proximites: " << endl;
	for(unsigned int i = 0; i < oMats.size(); ++i)
	{
		CvMat sample1 = dMats[i];
		for(unsigned int j = 0; j < oMats.size(); ++j)
		{
			CvMat sample2 = dMats[j];
			int prox = rtree->get_proximity( &sample1, &sample2) * 100;
			if(prox < 10)
				fout << "  " << prox << " ";
			else if(prox < 100)
				fout << " " << prox << " ";
			else
				fout << prox << " ";
		}
		fout << endl;
	}
	fout << "####################################################################################################" << endl;

	fout << "do sample proximites: " << endl;
	for(unsigned int i = 0; i < dMats.size(); ++i)
	{
		CvMat sample1 = dMats[i];
		for(unsigned int j = 0; j < oMats.size(); ++j)
		{
			CvMat sample2 = oMats[j];
			int prox = rtree->get_proximity( &sample1, &sample2) * 100;
			if(prox < 10)
				fout << "  " << prox << " ";
			else if(prox < 100)
				fout << " " << prox << " ";
			else
				fout << prox << " ";
		}
		fout << endl;
	}
	fout << "####################################################################################################" << endl;

	fout.close();
}

void randTreeTrainTest(const vector<int> &trainGames, const vector<int> &testGames, const vector<vector<playId> > &pIdsTestGames)
{
	vector<playId> pTestIds;
	for(unsigned int i = 0; i < pIdsTestGames.size(); ++i)
	{
		for(unsigned int j = 0; j < pIdsTestGames[i].size(); ++j)
			pTestIds.push_back(pIdsTestGames[i][j]);
	}


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

	CvRTParams params = CvRTParams(10,10,0,false,15,0,true,4,1,0.01f,CV_TERMCRIT_ITER);

	CvRTrees* rtree = new CvRTrees;
	rtree->train(trainFeaturesMat, CV_ROW_SAMPLE, trainLabelsMat,
				 Mat(), Mat(), varType, Mat(), params);

#if VarImportance == 1
	Mat varImp = rtree->getVarImportance();
	double totalImp = .0;
	for(int i = 0; i < varImp.cols; ++i)
	//for(int i = 0; i < ATTRIBUTES_PER_SAMPLE; ++i)
	{
		cout << i << " " << varImp.at<float>(0, i) << endl;
		totalImp += varImp.at<float>(0, i);
	}
	cout << "totalImp: " << totalImp << endl;
#endif

#if Proximities == 1
	vector<Mat> dMats, oMats;
	splitOdSamples(testFeaturesMat, testLabelsMat, dMats, oMats);
	getOdSamplesProx(dMats, oMats, "odProximities", rtree);
#endif

	cout << "number of trees: " << rtree->get_tree_count() << endl;

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
//			cout << "Testing "<< i << "th sample, detected as: " << (char)result << endl;
//			cout << "True label: " << (char)testLabelsMat.at<float>(i, 0) << endl;
			cout << "Game " << pTestIds[i].gameId << ", play " << pTestIds[i].vidId << " is wrong. " << endl;
			cout << "Detected as: " << (char)result << endl;
			cout << "True label: " << (char)testLabelsMat.at<float>(i, 0) << endl;
		}
	}

	cout << "Testing accuracy: " << (1.0 - errSamples / (double)testLabels.size() ) << endl;


}

void randTreeTrainTest(const vector<int> &trainGames, const vector<int> &testGames)
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

	CvRTParams params = CvRTParams(10,10,0,false,15,0,true,4,1000,0.01f,CV_TERMCRIT_ITER);

	CvRTrees* rtree = new CvRTrees;
	rtree->train(trainFeaturesMat, CV_ROW_SAMPLE, trainLabelsMat,
				 Mat(), Mat(), varType, Mat(), params);
#if VarImportance == 1
	Mat varImp = rtree->getVarImportance();
	double totalImp = .0;
	for(int i = 0; i < varImp.cols; ++i)
	//for(int i = 0; i < ATTRIBUTES_PER_SAMPLE; ++i)
	{
		cout << i << " " << varImp.at<float>(0, i) << endl;
		totalImp += varImp.at<float>(0, i);
	}
	cout << "totalImp: " << totalImp << endl;
#endif

#if Proximities == 1
	vector<Mat> dMats, oMats;
	splitOdSamples(testFeaturesMat, testLabelsMat, dMats, oMats);
	getOdSamplesProx(dMats, oMats, "odProximities", rtree);
#endif

	cout << "number of trees: " << rtree->get_tree_count() << endl;


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


void randTreeLeavePlayOut(const vector<int> &trainTestGames, const vector<vector<playId> > &pIdsAllGames)
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
		convertGameId << trainTestGames[i] ;
		string gameIdStr = convertGameId.str();

		if(trainTestGames[i] < 10)
			gameIdStr = "0" + gameIdStr;
//		string path = "randTreesTrainData/featureVecsGame" + gameIdStr;
		string path = "randTreesTrainData/featuresGame" + gameIdStr + "Rect";
		trainTestGamePaths.push_back(path);
	}

	vector<vector<double> > trainTestFeats;
	vector<double> trainTestLabs;
	readData(trainTestGamePaths, trainTestFeats, trainTestLabs);


	int errPlays = 0;
	for(unsigned int k = 0; k < trainTestLabs.size(); ++k)
	{
		Mat trainFeaturesMat = Mat(trainTestFeats.size() - 1, ATTRIBUTES_PER_SAMPLE, CV_32FC1);
		Mat trainLabelsMat = Mat(trainTestLabs.size() - 1, 1, CV_32FC1);

		Mat testFeaturesMat = Mat(1, ATTRIBUTES_PER_SAMPLE, CV_32FC1);
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

				for(unsigned int j = 0; j < ATTRIBUTES_PER_SAMPLE; ++j)
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

				for(unsigned int j = 0; j < ATTRIBUTES_PER_SAMPLE; ++j)
					trainFeaturesMat.at<float>(idx, j) = trainTestFeats[i][j];
			}
		}

		Mat varType = Mat(ATTRIBUTES_PER_SAMPLE + 1, 1, CV_8U );
		varType.setTo(Scalar(CV_VAR_NUMERICAL) );
		varType.at<uchar>(ATTRIBUTES_PER_SAMPLE, 0) = CV_VAR_CATEGORICAL;

		CvRTParams params = CvRTParams(10,10,0,false,15,0,true,4,100,0.01f,CV_TERMCRIT_ITER);

		CvRTrees* rtree = new CvRTrees;
		rtree->train(trainFeaturesMat, CV_ROW_SAMPLE, trainLabelsMat,
					 Mat(), Mat(), varType, Mat(), params);

#if VarImportance == 1
		Mat varImp = rtree->getVarImportance();
		double totalImp = .0;
		for(int i = 0; i < varImp.cols; ++i)
		//for(int i = 0; i < ATTRIBUTES_PER_SAMPLE; ++i)
		{
			cout << i << " " << varImp.at<float>(0, i) << endl;
			totalImp += varImp.at<float>(0, i);
		}
		cout << "totalImp: " << totalImp << endl;
#endif

#if Proximities == 1
	vector<Mat> dMats, oMats;
	splitOdSamples(testFeaturesMat, testLabelsMat, dMats, oMats);
	getOdSamplesProx(dMats, oMats, "odProximities", rtree);
#endif

//		Mat testSample = testFeaturesMat.row(0);
//		double result = rtree->predict(testSample, Mat());
		double result = rtree->predict(testFeaturesMat, Mat());
		if (fabs(result - testLabelsMat.at<float>(0, 0))>= FLT_EPSILON)
		{
			++errPlays;
			cout << "Game " << pId.gameId << ", play " << pId.vidId << " is wrong. " << endl;
			cout << "Detected as: " << (char)result << endl;
			cout << "True label: " << (char)testLabelsMat.at<float>(0, 0) << endl;
		}

	}

	cout << "Leave out plays overall accuracy: " << (1.0 - errPlays / (double)trainTestLabs.size() ) << endl;


}

void leaveGamesOutTest(const vector<int> &games)
{
	//int games[3] = {2, 8, 9};//, 10};

	vector<vector<playId> > pIdsAllGames;
	for(unsigned int i = 0; i < games.size(); ++i)
	{
		vector<playId> pIds;
		extracOdVidFeatsRts(games[i], pIds);
		pIdsAllGames.push_back(pIds);
	}

	for(unsigned int i = 0; i < games.size(); ++i)
	{
		vector<int> trainGames, testGames;
		vector<vector<playId> > pIdsTestGames;

		for(unsigned int j = 0; j < games.size(); ++j)
		{
			if(j == i)
			{
				testGames.push_back(games[j]);
				pIdsTestGames.push_back(pIdsAllGames[j]);
			}
			else
				trainGames.push_back(games[j]);
		}

		cout << "Leave game " << games[i] << " out... " << endl;
		randTreeTrainTest(trainGames, testGames, pIdsTestGames);
		//randTreeTrainTest(trainGames, testGames);
	}
}

void leavePlayOutTest(const std::vector<int> &games)
{
//	int games[3] = {2, 8, 9};//, 10};

	vector<vector<playId> > pIdsTestGames;
	vector<int> trainTestGames;

	for(unsigned int i = 0; i < games.size(); ++i)
	{
		vector<playId> pIds;
		extracOdVidFeatsRts(games[i], pIds);
		pIdsTestGames.push_back(pIds);
		trainTestGames.push_back(games[i]);
	}

	randTreeLeavePlayOut(trainTestGames, pIdsTestGames);

}

int main()
{
	vector<int> games;
	games.push_back(2);
	games.push_back(8);
	games.push_back(9);

//	leaveGamesOutTest(games);
//	leavePlayOutTest(games);

//	extracOdVidFeatsRts(2);
//	extracOdVidFeatsRts(8);
//	extracOdVidFeatsRts(9);

//	vector<playId> pIds;
//	extracOdVidFeatsRts(2, pIds);
//	pIds.clear();
//	extracOdVidFeatsRts(8, pIds);
//	pIds.clear();
//	extracOdVidFeatsRts(9, pIds);

	vector<int> trainGames, testGames;
	trainGames.push_back(2);
	trainGames.push_back(8);
	trainGames.push_back(9);
//	trainGames.push_back(10);
	testGames.push_back(2);
	testGames.push_back(8);
	testGames.push_back(9);
//	testGames.push_back(10);
	randTreeTrainTest(trainGames, testGames);


	return 1;
}
