#ifndef _DETECT_FORM_H_
#define _DETECT_FORM_H_

#include "commonStructs.h"

void detectForm(const std::vector<int> &games, const std::vector<int> &gamesFld, const std::vector<playerTypeId> &pTypeIds);

void detectForm(const std::vector<int> &games, const std::vector<int> &gamesFld);

void getPTypesLearningSamples(const std::vector<int> &games, const std::vector<int> &gamesFld,
		cv::Mat &trainFeaturesMat, cv::Mat &trainLabelsMat);

void lablePTypesKNN(const std::vector<int> &games, const std::vector<int> &gamesFld,
		const cv::Mat &trainFeaturesMat, const cv::Mat &trainLabelsMat);

#endif
