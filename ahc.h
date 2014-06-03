#ifndef _AHC_H_
#define _AHC_H_

//compute the offense features
void computeOffFeatsNoExp(const std::vector<int> &games, std::vector<std::vector<double> > &offFeatsVec);

void computeOffSpPmdPKernel(const std::vector<int> &games, int& playsNum);

bool readSpKernelAhc(std::string fileName,
		std::vector<std::vector<double> > &kernels, int playsNum);

#endif
