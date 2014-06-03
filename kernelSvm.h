#ifndef _KERNEL_SVM_H_
#define _KERNEL_SVM_H_

void computeOdSpPmdPKernel(const std::vector<int> &games);
//		const std::vector<CvSize> &gridSizes,
//		const std::vector<cv::Point2i> &gridsNum);


double computeTwoFVecKernel(const std::vector<double> &fVec1, const std::vector<double> &fVec2);

void computeOdFeatsSvm(const std::vector<int> &games);

void computeOdConcaFOverallExpSvm(const std::vector<int> &games);

void computeOdFeatsIndRspSvm(const std::vector<int> &games, int fMode);

#endif
