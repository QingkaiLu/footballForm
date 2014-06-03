#include <cv.h>
#include <ml.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <fstream>

#include "commonStructs.h"
#include "extractOdVidFeats.h"
#include "randTrees.h"

using namespace cv;
using namespace std;


//#define featNumPerPlay 122
//#define INF std::numeric_limits<double>/*or float*/::infinity()
//#define NEGINF -1.0 * INF


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
		string path = "randTreesTrainData/features/featuresGame" + gameIdStr + "Rect";
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
		string path = "randTreesTrainData/features/featuresGame" + gameIdStr + "Rect";
		testGameFilePaths.push_back(path);
	}
	vector<vector<double> > trainFeatures, testFeatures;
	vector<double> trainLabels, testLabels;
	readOdFeatData(trainGameFilePaths, trainFeatures, trainLabels, featNumPerPlay);
	readOdFeatData(testGameFilePaths, testFeatures, testLabels, featNumPerPlay);

	Mat trainFeaturesMat = Mat(trainFeatures.size(), featNumPerPlay, CV_32FC1);
	Mat trainLabelsMat = Mat(trainLabels.size(), 1, CV_32FC1);

	for(unsigned int i = 0; i < trainLabels.size(); ++i)
	{
		trainLabelsMat.at<float>(i, 0) = trainLabels[i];

		for(unsigned int j = 0; j < featNumPerPlay; ++j)
			trainFeaturesMat.at<float>(i, j) = trainFeatures[i][j];
	}

	Mat testFeaturesMat = Mat(testFeatures.size(), featNumPerPlay, CV_32FC1);
	Mat testLabelsMat = Mat(testLabels.size(), 1, CV_32FC1);

	for(unsigned int i = 0; i < testLabels.size(); ++i)
	{
		testLabelsMat.at<float>(i, 0) = testLabels[i];

		for(unsigned int j = 0; j < featNumPerPlay; ++j)
			testFeaturesMat.at<float>(i, j) = testFeatures[i][j];
	}


	Mat varType = Mat(featNumPerPlay + 1, 1, CV_8U );
	varType.setTo(Scalar(CV_VAR_NUMERICAL) );
	varType.at<uchar>(featNumPerPlay, 0) = CV_VAR_CATEGORICAL;

	CvRTParams params = CvRTParams(10,10,0,false,15,0,true,4,1,0.01f,CV_TERMCRIT_ITER);

	CvRTrees* rtree = new CvRTrees;
	rtree->train(trainFeaturesMat, CV_ROW_SAMPLE, trainLabelsMat,
				 Mat(), Mat(), varType, Mat(), params);

#if VarImportance == 1
	Mat varImp = rtree->getVarImportance();
	double totalImp = .0;
	for(int i = 0; i < varImp.cols; ++i)
	//for(int i = 0; i < featNumPerPlay; ++i)
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
		string path = "randTreesTrainData/features/featuresGame" + gameIdStr + "Rect";
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
		string path = "randTreesTrainData/features/featuresGame" + gameIdStr + "Rect";
		testGameFilePaths.push_back(path);
	}
	vector<vector<double> > trainFeatures, testFeatures;
	vector<double> trainLabels, testLabels;
	readOdFeatData(trainGameFilePaths, trainFeatures, trainLabels, featNumPerPlay);
	readOdFeatData(testGameFilePaths, testFeatures, testLabels, featNumPerPlay);

	Mat trainFeaturesMat = Mat(trainFeatures.size(), featNumPerPlay, CV_32FC1);
	Mat trainLabelsMat = Mat(trainLabels.size(), 1, CV_32FC1);

	for(unsigned int i = 0; i < trainLabels.size(); ++i)
	{
		trainLabelsMat.at<float>(i, 0) = trainLabels[i];

		for(unsigned int j = 0; j < featNumPerPlay; ++j)
			trainFeaturesMat.at<float>(i, j) = trainFeatures[i][j];
	}

	Mat testFeaturesMat = Mat(testFeatures.size(), featNumPerPlay, CV_32FC1);
	Mat testLabelsMat = Mat(testLabels.size(), 1, CV_32FC1);

	for(unsigned int i = 0; i < testLabels.size(); ++i)
	{
		testLabelsMat.at<float>(i, 0) = testLabels[i];

		for(unsigned int j = 0; j < featNumPerPlay; ++j)
			testFeaturesMat.at<float>(i, j) = testFeatures[i][j];
	}


	Mat varType = Mat(featNumPerPlay + 1, 1, CV_8U );
	varType.setTo(Scalar(CV_VAR_NUMERICAL) );
	varType.at<uchar>(featNumPerPlay, 0) = CV_VAR_CATEGORICAL;

	CvRTParams params = CvRTParams(10,10,0,false,15,0,true,4,1000,0.01f,CV_TERMCRIT_ITER);

	CvRTrees* rtree = new CvRTrees;
	rtree->train(trainFeaturesMat, CV_ROW_SAMPLE, trainLabelsMat,
				 Mat(), Mat(), varType, Mat(), params);
#if VarImportance == 1
	Mat varImp = rtree->getVarImportance();
	double totalImp = .0;
	for(int i = 0; i < varImp.cols; ++i)
	//for(int i = 0; i < featNumPerPlay; ++i)
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


	vector<string> trainTestGamePaths, losCntFileNames;
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
//		string losCntIdsFile = "randTreesTrainData/losCnt/losCntIdsGame" + gameIdStr;
//		losCntFileNames.push_back(losCntIdsFile);
	}

	vector<vector<double> > trainTestFeats;
	vector<double> trainTestLabs;
	readOdFeatData(trainTestGamePaths, trainTestFeats, trainTestLabs, featNumPerPlay);
//	readOdFeatData(trainTestGamePaths, trainTestFeats, trainTestLabs, featNumPerPlay, losCntFileNames, 1);


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

		Mat varType = Mat(featNumPerPlay + 1, 1, CV_8U);
		varType.setTo(Scalar(CV_VAR_NUMERICAL) );
		varType.at<uchar>(featNumPerPlay, 0) = CV_VAR_CATEGORICAL;

		CvRTParams params = CvRTParams(10,10,0,false,15,0,true,4,1000,0.01f,CV_TERMCRIT_ITER);

		CvRTrees* rtree = new CvRTrees;
		rtree->train(trainFeaturesMat, CV_ROW_SAMPLE, trainLabelsMat,
					 Mat(), Mat(), varType, Mat(), params);

#if VarImportance == 1
		Mat varImp = rtree->getVarImportance();
		double totalImp = .0;
		for(int i = 0; i < varImp.cols; ++i)
		//for(int i = 0; i < featNumPerPlay; ++i)
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
			cout << "Game " << pId.gameId << ", play " << pId.vidId << " is wrong." << endl;
			cout << "Detected as: " << (char)result << endl;
			cout << "True label: " << (char)testLabelsMat.at<float>(0, 0) << endl;
		}
//		else
//		{
//			cout << "Game " << pId.gameId << ", play " << pId.vidId << " is correct. " << endl;
//		}

	}

//	cout << errPlays << endl;
	cout << "Leave out plays overall accuracy: " << (1.0 - errPlays / (double)trainTestLabs.size() ) << endl;
	cout << "samples number: " << trainTestLabs.size() << endl;
	cout << "errors number: " << errPlays << endl;

}


void randTreeLeavePlayOutExp(const vector<int> &trainTestGames, const vector<vector<playId> > &pIdsAllGames, int odExpMode)
{
	vector<playId> pIdsAll;
	for(unsigned int i = 0; i < pIdsAllGames.size(); ++i)
	{
		for(unsigned int j = 0; j < pIdsAllGames[i].size(); ++j)
			pIdsAll.push_back(pIdsAllGames[i][j]);
	}


	vector<string> featsGtOdPaths, featsLeftOPaths, featsLeftDPaths;
	for(unsigned int i = 0; i < trainTestGames.size(); ++i)
	{
		ostringstream convertGameId;
		convertGameId << trainTestGames[i];
		string gameIdStr = convertGameId.str();

		if(trainTestGames[i] < 10)
			gameIdStr = "0" + gameIdStr;

		string featsExpGtOd = "randTreesTrainData/features/featsGame" + gameIdStr + "RectExp";
		string featsExpLeftO = "randTreesTrainData/features/featsGame" + gameIdStr + "RectExpLeftO";
		string featsExpLeftD = "randTreesTrainData/features/featsGame" + gameIdStr + "RectExpLeftD";
		featsGtOdPaths.push_back(featsExpGtOd);
		featsLeftOPaths.push_back(featsExpLeftO);
		featsLeftDPaths.push_back(featsExpLeftD);
	}

	vector<vector<double> > featsGtOd, featsLeftO, featsLeftD;
	vector<double> labelsGtOd, labelsLeftO, labelsLeftD;
	readOdFeatData(featsGtOdPaths, featsGtOd, labelsGtOd, featNumPerPlay);
	readOdFeatData(featsLeftOPaths, featsLeftO, labelsLeftO, featNumPerPlay);
	readOdFeatData(featsLeftDPaths, featsLeftD, labelsLeftD, featNumPerPlay);

	int errPlays = 0;
	int oneAgreeNum = 0, bothAgreeNum = 0,
			bothNotAgreeNum = 0;

	int oneAgreeErrNum = 0, bothAgreeErrNum = 0,
			bothNotAgreeErrNum = 0;

	for(unsigned int k = 0; k < labelsGtOd.size(); ++k)
	{
		Mat trainFeaturesMat = Mat(featsGtOd.size() - 1, featNumPerPlay, CV_32FC1);
		Mat trainLabelsMat = Mat(labelsGtOd.size() - 1, 1, CV_32FC1);

		Mat testFeatsMatLeftO = Mat(1, featNumPerPlay, CV_32FC1);
		Mat testLabelsMatLeftO = Mat(1, 1, CV_32FC1);

		Mat testFeatsMatLeftD= Mat(1, featNumPerPlay, CV_32FC1);
		Mat testLabelsMatLeftD = Mat(1, 1, CV_32FC1);

		Mat testFeatsMatGtOd= Mat(1, featNumPerPlay, CV_32FC1);
		Mat testLabelsMatGtOd = Mat(1, 1, CV_32FC1);
//
		playId pId;
		for(unsigned int i = 0; i < labelsGtOd.size(); ++i)
		{
			if(i == k)
			{
				pId = pIdsAll[i];

				//cout << "Leave Game " << pId.gameId << ", play " << pId.vidId << " out..." << endl;
				testLabelsMatLeftO.at<float>(0, 0) = labelsLeftO[i];
				for(unsigned int j = 0; j < featNumPerPlay; ++j)
					testFeatsMatLeftO.at<float>(0, j) = featsLeftO[i][j];

				testLabelsMatLeftD.at<float>(0, 0) = labelsLeftD[i];
				for(unsigned int j = 0; j < featNumPerPlay; ++j)
					testFeatsMatLeftD.at<float>(0, j) = featsLeftD[i][j];

				testLabelsMatGtOd.at<float>(0, 0) = labelsGtOd[i];
				for(unsigned int j = 0; j < featNumPerPlay; ++j)
					testFeatsMatGtOd.at<float>(0, j) = featsGtOd[i][j];
			}
			else
			{
				int idx = -1;
				if(i > k)
					idx = i - 1;
				else
					idx = i;
				trainLabelsMat.at<float>(idx, 0) = labelsGtOd[i];

				for(unsigned int j = 0; j < featNumPerPlay; ++j)
					trainFeaturesMat.at<float>(idx, j) = featsGtOd[i][j];
			}
		}

		Mat varType = Mat(featNumPerPlay + 1, 1, CV_8U );
		varType.setTo(Scalar(CV_VAR_NUMERICAL) );
		varType.at<uchar>(featNumPerPlay, 0) = CV_VAR_CATEGORICAL;

		CvRTParams params = CvRTParams(10,10,0,false,15,0,true,4,1000,0.01f,CV_TERMCRIT_ITER);

		CvRTrees* rtree = new CvRTrees;
		rtree->train(trainFeaturesMat, CV_ROW_SAMPLE, trainLabelsMat,
					 Mat(), Mat(), varType, Mat(), params);

#if VarImportance == 1
		Mat varImp = rtree->getVarImportance();
		double totalImp = .0;
		for(int i = 0; i < varImp.cols; ++i)
		//for(int i = 0; i < featNumPerPlay; ++i)
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
	if(odExpMode == 2)
	{
		double resultLeftO = rtree->predict(testFeatsMatLeftO, Mat());
//		cout << "resultLeftO " << resultLeftO << endl;
		bool leftOErr = false;
		if (fabs(resultLeftO - (double)'o')>= FLT_EPSILON)
			leftOErr = true;

		double resultLeftD = rtree->predict(testFeatsMatLeftD, Mat());
		bool leftDErr = false;
		if (fabs(resultLeftD - (double)'d')>= FLT_EPSILON)
			leftDErr = true;

		double result = -1.0;

		if(leftOErr == leftDErr)
		{
			double leftOProb = rtree->predict_prob(testFeatsMatLeftO);
			if(leftOProb < 0.5)
				leftOProb = 1 - leftOProb;
			double leftDProb = rtree->predict_prob(testFeatsMatLeftD);
			if(leftDProb < 0.5)
				leftDProb = 1 - leftDProb;
			if(leftOProb > leftDProb)
				result = resultLeftO;
			else
				result = resultLeftD;

			if(leftOErr)
				++bothNotAgreeNum;
			else
				++bothAgreeNum;

			if (fabs(result - testLabelsMatLeftO.at<float>(0, 0))>= FLT_EPSILON)
			{
				if(leftOErr)
					++bothNotAgreeErrNum;
				else
					++bothAgreeErrNum;
			}

//			cout << "leftOErr: " << leftOErr << endl;
			cout << "leftOProb: " << leftOProb << endl;
			cout << "leftDProb: " << leftDProb << endl;
//			if(leftOErr)
//				cout << "**********" << endl;
//			result = testLabelsMatLeftO.at<float>(0, 0);
		}
		else
		{
			//cout << "different" << endl;
			if(leftOErr)
				result = resultLeftO;
			if(leftDErr)
				result = resultLeftD;

			++oneAgreeNum;

			if (fabs(result - testLabelsMatLeftO.at<float>(0, 0))>= FLT_EPSILON)
				++oneAgreeErrNum;

			//result = testLabelsMatLeftO.at<float>(0, 0);
		}

		if (fabs(result - testLabelsMatLeftO.at<float>(0, 0))>= FLT_EPSILON)
		{
			++errPlays;
			cout << "Game " << pId.gameId << ", play " << pId.vidId << " is wrong. " << endl;
			cout << "Detected as: " << (char)result << endl;
			cout << "True label: " << (char)testLabelsMatLeftO.at<float>(0, 0) << endl;
		}

	}
	else if(odExpMode == 1)
	{
		double result = rtree->predict(testFeatsMatGtOd);
//		double resultProb = rtree->predict_prob(testFeatsMatGtOd);
//		cout << (char)testLabelsMatLeftO.at<float>(0, 0) << " " << resultProb << endl;
		if (fabs(result - testLabelsMatLeftO.at<float>(0, 0))>= FLT_EPSILON)
		{
			++errPlays;
			cout << "Game " << pId.gameId << ", play " << pId.vidId << " is wrong. " << endl;
			cout << "Detected as: " << (char)result << endl;
			cout << "True label: " << (char)testLabelsMatLeftO.at<float>(0, 0) << endl;
		}
	}

	}

	cout << "Leave out plays overall accuracy: " << (1.0 - errPlays / (double)labelsGtOd.size() ) << endl;
	if(odExpMode == 2)
	{
	cout << "oneAgreeNum: " <<  oneAgreeNum << endl;
	cout << "oneAgree Acc: " << 1.0 - (double)oneAgreeErrNum / (double)oneAgreeNum << endl;
	cout << "bothAgreeNum: " <<  bothAgreeNum << endl;
	cout << "bothAgree Acc: " << 1.0 -  (double)bothAgreeErrNum / (double)bothAgreeNum << endl;
	cout << "bothNotAgreeNum: " <<  bothNotAgreeNum << endl;
	cout << "bothNotAgree Acc: " << 1.0 -  (double)bothNotAgreeErrNum / (double)bothNotAgreeNum << endl;
	}

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

void leavePlayOutTest(const vector<int> &games, int expMode, int odExpMode)
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

	if(expMode == 0)
		randTreeLeavePlayOut(trainTestGames, pIdsTestGames);
	else if(expMode == 1)
		randTreeLeavePlayOutExp(trainTestGames, pIdsTestGames, odExpMode);

}

//int main()
int randTrees()
{
	vector<int> games;
	games.push_back(2);
	games.push_back(8);
	games.push_back(9);
	games.push_back(10);
//	games.push_back(9);

	int expMode = 0;
	int odExpMode = 2;
	int featureMode = 2;

//	extractFeatures(games, expMode, featureMode);

//	leaveGamesOutTest(games);
//	expMode = 0;
	leavePlayOutTest(games, expMode, odExpMode);

	return 1;
}
