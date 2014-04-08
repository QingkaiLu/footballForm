#ifndef RUN_MULTI_MODELS_H_
#define RUN_MULTI_MODELS_H_

#include "commonStructs.h"

void setUpMultiModes(std::vector<std::string> &modelWtFilePaths, std::vector<std::string> &playLabelFilePaths,
		std::vector< std::vector<playerType> > &pTypesOfAllClasses);
void runMultiModels();
void readModelWeight(const std::vector<std::string> &modelWtFilePaths, std::vector< std::vector<double> > &modelWt);
void readPlayLabel(const std::vector<std::string> &playLabelFilePaths, std::vector<playId> &pIds, std::vector<int> &pLabels);

#endif
