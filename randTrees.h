#ifndef _RAND_TREES_H_
#define _RAND_TREES_H_

bool readData(const std::vector<std::string> &fileNames, std::vector<std::vector<double> > &features, std::vector<double> &labels);

void randTreeTrainTest(std::vector<int> &trainGames, std::vector<int> &testGames);

#endif
