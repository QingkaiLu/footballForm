#ifndef EXTRACT_OD_VIDS_FEATURES_H_
#define EXTRACT_OD_VIDS_FEATURES_H_
/*
 * Extract feature vectors for offense and defense on the left/right of LOS
 */

#include "commonStructs.h"

void readOdLabels(std::string odLabelFilePath, std::vector<playId> &pIds, std::vector<char> &dir, std::vector<char> &odLabel);

bool readOdFeatData(const std::vector<std::string> &fileNames, std::vector<std::vector<double> > &features,
		std::vector<double> &labels, int featureNum);

void setUpGrids(std::vector<CvSize> &gridSizes, std::vector<cv::Point2i> &gridsNum);

bool readLosCntIds(const std::vector<std::string> &fileNames, std::vector<int> &losCntIds);

void extracOdVidFeatsRts(int gameId, std::vector<playId> &pIds);

//void extracOdVidFeatSvm();

//void extracOdMissFeatsRts(int gameId, std::vector<playId> &pIds);

void plotOdExp(const std::vector<std::vector<int> > &fVecOPlaysExp,
		const std::vector<std::vector<int> > &fVecDPlaysExp,
		const std::vector<CvSize> &gridSizes, const std::vector<cv::Point2i> &gridsNum);

//compute the feature vectors on the left and right side of los
void computeLeftRightFeats(const std::vector<int> &games, int expMode);

void getPlayIds(const std::vector<int> &games, std::vector<playId> &pIds);

void getPlayIds(int gameId, std::vector<playId> &pIds);

//compute the expectation of o and d features
void computeOdExpFeats(const std::vector<int> &games);

void readOdExpFeats(std::vector<std::vector<int> > &fVecOPlaysExp,
		std::vector<std::vector<int> > &fVecDPlaysExp);

//compute od features of plays replacing the
//missing features with expected features
//odMode == 1: Use the ground truth of label o, d on los sides
//to replace missing features.
//odMode == 2: Left o and right d on los sides
//to replace missing features.
//odMode == 3: Left d and right o on los sides
//to replace missing features.
void computeOdFeatsMissExp(const std::vector<int> &games, int odMode);

//compute od features by concatenating left and right features
//to avoid discarding features when subtracting features.
void computeOdConcaFMissExp(const std::vector<int> &games, int odMode);

// if fMode is 1, subtract left side features with right features
// if fMode is 2, concatenate left and right side features
void computeOdFeatsNoExp(const std::vector<int> &games, int fMode);

//void computeOdConcaFeats(const std::vector<int> &games);

////Left o and right d on los sides
////to replace missing features.
//void computeOdFLeftOExp(const std::vector<int> &games);
//
////Left d and right o on los sides
////to replace missing features.
//void computeOdFLeftDExp(const std::vector<int> &games);

//expMode == 0: no expectation
//expMode == 1: with expectation
// if fMode is 1, subtract left side features with right features
// if fMode is 2, concatenate left and right side features
int extractFeatures(const std::vector<int> &games, int expMode, int fMode);


#endif
