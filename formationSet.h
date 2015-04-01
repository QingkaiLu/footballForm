#include "commonStructs.h"

class formationSet{
public:
	formationSet(std::string formFilePath);
	void getPlayersPosType();
	void computeFeatureSet(int fld);
	std::vector<std::vector<std::vector<double> > > getFeatureFormSet();
	std::vector<std::vector<cv::Point2d> > getRectPlayersFormSet();
	std::vector<std::vector<std::string> > getPlayersTypeFormSet();
	std::vector<playId> getPIds();
	void getTrainingFeats(int fld, std::vector<std::string> playerTypes,
			std::vector<std::vector<std::vector<double> > > &features, std::vector<std::vector<int> > &labels);
	void getTrainingFeats(int fld, std::string playerType,
			std::vector<std::vector<double> > &features, std::vector<int> &labels);
	std::vector<double> perceptronTrain(std::pair<int, int> leaveOutRng, const std::vector<std::vector<double> > &features,
			const std::vector<int> &labels);
	std::vector<std::vector<double> > votedPerceptronTrain(std::pair<int, int> leaveOutRng, const std::vector<std::vector<double> > &features,
			const std::vector<int> &labels, std::vector<int> &votes);
	double crossValid();
	double crossValidVoted();
	std::vector<std::vector<double> > getPlayerWts(std::vector<std::string> playerTypes, std::pair<int, int> leaveOutRng);
	void findSimForm();
	std::vector<double> computeStarModelFVec(int pIdsIdx1, int pIdsIdx2, std::vector<cv::Point2d> &transPlayPos);
	std::vector<std::vector<double> > trainStarModelCostWt(std::pair<int, int> leaveOutRng);
	int testToPickBestForm(std::pair<int, int> leaveOutRng, const std::vector<std::vector<double> > &weights);
	int testToPickBestForm(int pTestIdx, const std::vector<std::vector<double> > &weights);
	void leaveOneOutBestFormTest();
	void testBestFormTrainingSet();
private:
	std::vector<playId> pIds;
	std::vector<std::vector<std::vector<double> > > featureFormSet;
	std::vector<std::vector<double> > maxFeatFormSet;
	std::vector<std::vector<cv::Point2d> > rectPlayersFormSet;
	std::vector<std::vector<std::string> > playersTypeFormSet;
	std::vector<std::string> odLabels;
};
