#ifndef _DETECT_FORM_H_
#define _DETECT_FORM_H_

#include "commonStructs.h"

class play;
class formationSet;

void detectForm(const std::vector<int> &games, const std::vector<int> &gamesFld, const std::vector<playerTypeId> &pTypeIds);

void detectForm(const std::vector<int> &games, const std::vector<int> &gamesFld);

void getPTypesLearningSamples(const std::vector<int> &games, const std::vector<int> &gamesFld,
		cv::Mat &trainFeaturesMat, cv::Mat &trainLabelsMat);

void getFormLearningSamples(const vector<int> &games, const vector<int> &gamesFld,
		vector<vector<Point2d> > &pToLosVecAllPlays, vector<vector<int> > &pTypesIdAllPlays);

void saveExemplarForms(const vector<int> &games, const vector<int> &gamesFld);

void lablePTypesKNN(const std::vector<int> &games, const std::vector<int> &gamesFld,
		const cv::Mat &trainFeaturesMat, const cv::Mat &trainLabelsMat);

void inferMissPlayersAllPlays(const std::vector<int> &games, const std::vector<int> &gamesFld,
		const cv::Mat &trainFeaturesMat, const cv::Mat &trainLabelsMat,
		const vector<vector<Point2d> > &pToLosVecAllPlays, const vector<vector<int> > &pTypesIdAllPlays);

inline double getGridSpCost(vector<CvSize> &gridSizes, Point2d p1, Point2d p2);

double detectFormMltEviOnePlay(play* p, formationSet* formSet,
		std::vector<cv::Point2d> &playersFormPos, std::vector<std::string> &playersFormTypes, playId &bestPId, string odLabel);

void detectFormMltEvidence(const std::vector<int> &games, const std::vector<int> &gamesFld);


#endif
