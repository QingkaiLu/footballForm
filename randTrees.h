#ifndef _RAND_TREES_H_
#define _RAND_TREES_H_

//VarImportance == 1 => show variable importance
//VarImportance == 0 => don't show variable importance
#define VarImportance 0

//Proximities == 1 => show sample proximities
//Proximities == 0 => don't show sample proximities
#define Proximities 1

bool readData(const std::vector<std::string> &fileNames, std::vector<std::vector<double> > &features, std::vector<double> &labels);

void randTreeTrainTest(const std::vector<int> &trainGames, const std::vector<int> &testGames,
		const std::vector<std::vector<playId> > &pIdsTestGames);

void randTreeTrainTest(const std::vector<int> &trainGames, const std::vector<int> &testGames);

void randTreeLeavePlayOut(const std::vector<int> &trainTestGames, const std::vector<std::vector<playId> > &pIdsAllGames);

void leaveGamesOutTest(const std::vector<int> &games);

void leavePlayOutTest(const std::vector<int> &games);

void splitOdSamples(const cv::Mat &featureMat, const cv::Mat &labelsMat,
		std::vector<cv::Mat> &dMats, std::vector<cv::Mat> &oMats);

void getOdSamplesProx(const std::vector<cv::Mat> &dMats, const std::vector<cv::Mat> &oMats, const std::string &fileName, CvRTrees* rtree);

#endif
