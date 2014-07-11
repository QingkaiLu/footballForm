#ifndef _SHOTGUN_CLASSIFICATION_H_
#define _SHOTGUN_CLASSIFICATION_H_

//compute the offense features for shotgun classification
void computeOffFeatsShotgun(const std::vector<int> &games);//, std::vector<std::vector<double> > &offFeatsVec);

void readShotgunLabels(std::string sgLabelFilePath, std::vector<std::string> &sgLabels);

//int extractSgFeats(const std::vector<int> &games, int expMode);

void computeOffSgFeatsSvm(const std::vector<int> &games);

void computeOffSgSpPmdPKernel(const std::vector<int> &games);

#endif
